"""
Training Pipeline for Financial LLM
Optimized for financial data and low-latency requirements
"""

import torch
import torch.nn as nn
from torch.utils.data import Dataset, DataLoader
from torch.optim import AdamW
from torch.optim.lr_scheduler import CosineAnnealingLR, OneCycleLR
import torch.distributed as dist
from torch.nn.parallel import DistributedDataParallel as DDP

import os
import time
import json
from typing import Dict, Optional, List, Tuple
from dataclasses import dataclass, asdict
import numpy as np
from pathlib import Path

import sys
sys.path.append(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))

from model.transformer import FinancialLLM, FinancialLLMConfig
from model.tokenizer import FinancialTokenizer


@dataclass
class TrainingConfig:
    """Training configuration"""
    # Model
    model_size: str = 'base'
    vocab_size: int = 50000

    # Training
    batch_size: int = 32
    gradient_accumulation_steps: int = 4
    max_epochs: int = 10
    max_steps: Optional[int] = None
    learning_rate: float = 3e-4
    weight_decay: float = 0.01
    warmup_steps: int = 1000
    max_grad_norm: float = 1.0

    # Data
    max_seq_length: int = 2048
    train_data_path: str = "data/train"
    val_data_path: str = "data/val"

    # Optimization
    use_mixed_precision: bool = True
    use_gradient_checkpointing: bool = False
    compile_model: bool = True

    # Distributed
    use_distributed: bool = False
    local_rank: int = -1

    # Logging and checkpointing
    log_interval: int = 100
    eval_interval: int = 1000
    save_interval: int = 5000
    output_dir: str = "checkpoints"

    # Resume
    resume_from_checkpoint: Optional[str] = None


class FinancialTextDataset(Dataset):
    """Dataset for financial text"""

    def __init__(self, data_path: str, tokenizer: FinancialTokenizer,
                 max_length: int = 2048, cache_size: int = 10000):
        self.data_path = data_path
        self.tokenizer = tokenizer
        self.max_length = max_length
        self.cache_size = cache_size

        # Load or create file list
        self.files = self._get_files()
        self.cache = {}

        print(f"Loaded dataset with {len(self.files)} files")

    def _get_files(self) -> List[str]:
        """Get list of data files"""
        if os.path.isfile(self.data_path):
            return [self.data_path]
        elif os.path.isdir(self.data_path):
            files = []
            for ext in ['.txt', '.json', '.jsonl']:
                files.extend(Path(self.data_path).rglob(f'*{ext}'))
            return [str(f) for f in files]
        else:
            raise ValueError(f"Invalid data path: {self.data_path}")

    def __len__(self) -> int:
        return len(self.files) * 100  # Approximate, adjust based on your data

    def __getitem__(self, idx: int) -> Dict[str, torch.Tensor]:
        """Get a training sample"""
        # Determine which file
        file_idx = idx % len(self.files)
        file_path = self.files[file_idx]

        # Load from cache or file
        if file_idx in self.cache:
            text = self.cache[file_idx]
        else:
            with open(file_path, 'r', encoding='utf-8') as f:
                if file_path.endswith('.json') or file_path.endswith('.jsonl'):
                    data = json.load(f)
                    text = data.get('text', '')
                else:
                    text = f.read()

            # Update cache (LRU-like)
            if len(self.cache) >= self.cache_size:
                self.cache.pop(next(iter(self.cache)))
            self.cache[file_idx] = text

        # Tokenize
        tokens = self.tokenizer.encode(text, max_length=self.max_length + 1, truncation=True)

        # Create input and target (shifted by 1)
        if len(tokens) < 2:
            # Fallback for very short texts
            tokens = [self.tokenizer.SPECIAL_TOKENS['[CLS]'],
                     self.tokenizer.SPECIAL_TOKENS['[SEP]']]

        input_ids = tokens[:-1]
        labels = tokens[1:]

        # Pad to max_length
        pad_length = self.max_length - len(input_ids)
        if pad_length > 0:
            input_ids = input_ids + [self.tokenizer.SPECIAL_TOKENS['[PAD]']] * pad_length
            labels = labels + [-100] * pad_length  # -100 is ignored in loss

        return {
            'input_ids': torch.tensor(input_ids, dtype=torch.long),
            'labels': torch.tensor(labels, dtype=torch.long)
        }


class FinancialLLMTrainer:
    """Trainer for Financial LLM"""

    def __init__(self, config: TrainingConfig, model: FinancialLLM,
                 tokenizer: FinancialTokenizer):
        self.config = config
        self.model = model
        self.tokenizer = tokenizer

        # Device setup
        if config.use_distributed:
            self.device = torch.device(f'cuda:{config.local_rank}')
            dist.init_process_group(backend='nccl')
            self.model = DDP(model.to(self.device), device_ids=[config.local_rank])
        else:
            self.device = torch.device('cuda' if torch.cuda.is_available() else 'cpu')
            self.model = model.to(self.device)

        # Optimizer
        self.optimizer = AdamW(
            self.model.parameters(),
            lr=config.learning_rate,
            weight_decay=config.weight_decay,
            betas=(0.9, 0.95)
        )

        # Mixed precision
        self.scaler = torch.cuda.amp.GradScaler() if config.use_mixed_precision else None

        # Datasets
        self.train_dataset = FinancialTextDataset(
            config.train_data_path, tokenizer, config.max_seq_length
        )
        self.train_loader = DataLoader(
            self.train_dataset,
            batch_size=config.batch_size,
            shuffle=True,
            num_workers=4,
            pin_memory=True
        )

        if os.path.exists(config.val_data_path):
            self.val_dataset = FinancialTextDataset(
                config.val_data_path, tokenizer, config.max_seq_length
            )
            self.val_loader = DataLoader(
                self.val_dataset,
                batch_size=config.batch_size,
                shuffle=False,
                num_workers=4,
                pin_memory=True
            )
        else:
            self.val_loader = None

        # Learning rate scheduler
        if config.max_steps:
            total_steps = config.max_steps
        else:
            total_steps = len(self.train_loader) * config.max_epochs // config.gradient_accumulation_steps

        self.scheduler = OneCycleLR(
            self.optimizer,
            max_lr=config.learning_rate,
            total_steps=total_steps,
            pct_start=config.warmup_steps / total_steps,
            anneal_strategy='cos'
        )

        # Training state
        self.global_step = 0
        self.epoch = 0
        self.best_val_loss = float('inf')

        # Create output directory
        os.makedirs(config.output_dir, exist_ok=True)

        # Model compilation (PyTorch 2.0+)
        if config.compile_model and hasattr(torch, 'compile'):
            print("Compiling model with torch.compile()...")
            self.model = torch.compile(self.model)

        print(f"Trainer initialized on device: {self.device}")
        print(f"Total parameters: {sum(p.numel() for p in self.model.parameters()):,}")

    def train_step(self, batch: Dict[str, torch.Tensor]) -> float:
        """Single training step"""
        self.model.train()

        input_ids = batch['input_ids'].to(self.device)
        labels = batch['labels'].to(self.device)

        # Forward pass with mixed precision
        if self.scaler:
            with torch.cuda.amp.autocast():
                outputs = self.model(input_ids)
                logits = outputs[0]

                # Compute loss
                loss = nn.functional.cross_entropy(
                    logits.view(-1, logits.size(-1)),
                    labels.view(-1),
                    ignore_index=-100
                )
                loss = loss / self.config.gradient_accumulation_steps

            # Backward pass
            self.scaler.scale(loss).backward()
        else:
            outputs = self.model(input_ids)
            logits = outputs[0]

            loss = nn.functional.cross_entropy(
                logits.view(-1, logits.size(-1)),
                labels.view(-1),
                ignore_index=-100
            )
            loss = loss / self.config.gradient_accumulation_steps

            loss.backward()

        return loss.item() * self.config.gradient_accumulation_steps

    def update_weights(self):
        """Update model weights"""
        # Gradient clipping
        if self.scaler:
            self.scaler.unscale_(self.optimizer)

        torch.nn.utils.clip_grad_norm_(self.model.parameters(), self.config.max_grad_norm)

        # Optimizer step
        if self.scaler:
            self.scaler.step(self.optimizer)
            self.scaler.update()
        else:
            self.optimizer.step()

        self.scheduler.step()
        self.optimizer.zero_grad()

    @torch.no_grad()
    def evaluate(self) -> Dict[str, float]:
        """Evaluate on validation set"""
        if self.val_loader is None:
            return {}

        self.model.eval()
        total_loss = 0
        total_tokens = 0

        for batch in self.val_loader:
            input_ids = batch['input_ids'].to(self.device)
            labels = batch['labels'].to(self.device)

            outputs = self.model(input_ids)
            logits = outputs[0]

            loss = nn.functional.cross_entropy(
                logits.view(-1, logits.size(-1)),
                labels.view(-1),
                ignore_index=-100,
                reduction='sum'
            )

            total_loss += loss.item()
            total_tokens += (labels != -100).sum().item()

        avg_loss = total_loss / total_tokens
        perplexity = np.exp(avg_loss)

        return {
            'val_loss': avg_loss,
            'val_perplexity': perplexity
        }

    def save_checkpoint(self, path: str, is_best: bool = False):
        """Save model checkpoint"""
        # Get model state (unwrap DDP if needed)
        if isinstance(self.model, DDP):
            model_state = self.model.module.state_dict()
        else:
            model_state = self.model.state_dict()

        checkpoint = {
            'model_state_dict': model_state,
            'optimizer_state_dict': self.optimizer.state_dict(),
            'scheduler_state_dict': self.scheduler.state_dict(),
            'global_step': self.global_step,
            'epoch': self.epoch,
            'best_val_loss': self.best_val_loss,
            'config': asdict(self.config)
        }

        torch.save(checkpoint, path)
        print(f"Checkpoint saved to {path}")

        if is_best:
            best_path = os.path.join(self.config.output_dir, 'best_model.pt')
            torch.save(checkpoint, best_path)
            print(f"Best model saved to {best_path}")

    def load_checkpoint(self, path: str):
        """Load model checkpoint"""
        checkpoint = torch.load(path, map_location=self.device)

        # Load model state
        if isinstance(self.model, DDP):
            self.model.module.load_state_dict(checkpoint['model_state_dict'])
        else:
            self.model.load_state_dict(checkpoint['model_state_dict'])

        self.optimizer.load_state_dict(checkpoint['optimizer_state_dict'])
        self.scheduler.load_state_dict(checkpoint['scheduler_state_dict'])
        self.global_step = checkpoint['global_step']
        self.epoch = checkpoint['epoch']
        self.best_val_loss = checkpoint['best_val_loss']

        print(f"Checkpoint loaded from {path}")

    def train(self):
        """Main training loop"""
        print("Starting training...")

        # Resume from checkpoint if specified
        if self.config.resume_from_checkpoint:
            self.load_checkpoint(self.config.resume_from_checkpoint)

        start_time = time.time()
        accumulated_loss = 0

        for epoch in range(self.epoch, self.config.max_epochs):
            self.epoch = epoch

            for batch_idx, batch in enumerate(self.train_loader):
                # Training step
                loss = self.train_step(batch)
                accumulated_loss += loss

                # Update weights every gradient_accumulation_steps
                if (batch_idx + 1) % self.config.gradient_accumulation_steps == 0:
                    self.update_weights()
                    self.global_step += 1

                    # Logging
                    if self.global_step % self.config.log_interval == 0:
                        avg_loss = accumulated_loss / self.config.log_interval
                        lr = self.scheduler.get_last_lr()[0]
                        elapsed = time.time() - start_time
                        steps_per_sec = self.config.log_interval / elapsed

                        print(f"Epoch {epoch} | Step {self.global_step} | "
                              f"Loss: {avg_loss:.4f} | LR: {lr:.2e} | "
                              f"Steps/sec: {steps_per_sec:.2f}")

                        accumulated_loss = 0
                        start_time = time.time()

                    # Evaluation
                    if self.global_step % self.config.eval_interval == 0:
                        metrics = self.evaluate()
                        if metrics:
                            print(f"Validation - Loss: {metrics['val_loss']:.4f} | "
                                  f"Perplexity: {metrics['val_perplexity']:.2f}")

                            # Save best model
                            if metrics['val_loss'] < self.best_val_loss:
                                self.best_val_loss = metrics['val_loss']
                                self.save_checkpoint(
                                    os.path.join(self.config.output_dir, f'checkpoint_step_{self.global_step}.pt'),
                                    is_best=True
                                )

                    # Checkpointing
                    if self.global_step % self.config.save_interval == 0:
                        self.save_checkpoint(
                            os.path.join(self.config.output_dir, f'checkpoint_step_{self.global_step}.pt')
                        )

                    # Max steps check
                    if self.config.max_steps and self.global_step >= self.config.max_steps:
                        print(f"Reached max steps: {self.config.max_steps}")
                        return

        print("Training complete!")


if __name__ == "__main__":
    # Example training script
    from model.transformer import create_financial_llm

    # Create tokenizer
    tokenizer = FinancialTokenizer(vocab_size=50000)

    # Create model
    model = create_financial_llm(vocab_size=50000, size='base')

    # Training config
    config = TrainingConfig(
        model_size='base',
        batch_size=8,
        gradient_accumulation_steps=4,
        max_epochs=3,
        learning_rate=3e-4,
        train_data_path='data/train',
        val_data_path='data/val',
        output_dir='checkpoints/financial-llm-base'
    )

    # Create trainer
    trainer = FinancialLLMTrainer(config, model, tokenizer)

    # Train
    trainer.train()
