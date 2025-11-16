"""
Low-Latency Inference Engine for Financial LLM
Optimized for HFT and real-time trading applications
"""

import torch
import torch.nn as nn
from torch import Tensor
import numpy as np
from typing import List, Dict, Optional, Tuple, Union
from dataclasses import dataclass
import time
from collections import deque
import threading
from queue import Queue, PriorityQueue

import sys
import os
sys.path.append(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))

from model.transformer import FinancialLLM, FinancialLLMConfig
from model.tokenizer import FinancialTokenizer


@dataclass
class InferenceConfig:
    """Inference engine configuration"""
    # Model
    model_path: str
    tokenizer_path: str

    # Performance
    batch_size: int = 1  # For HFT, usually 1 for lowest latency
    use_kv_cache: bool = True  # Key-value caching for generation
    use_torch_compile: bool = True  # PyTorch 2.0+ compilation
    use_mixed_precision: bool = True  # FP16 inference
    device: str = 'cuda'  # 'cuda' or 'cpu'

    # Generation
    max_new_tokens: int = 100
    temperature: float = 0.8
    top_k: int = 50
    top_p: float = 0.95

    # Latency optimization
    warmup_iterations: int = 10  # Warmup runs
    enable_thread_pool: bool = True  # Thread pool for parallel processing
    num_workers: int = 4

    # Monitoring
    track_latency: bool = True
    latency_percentiles: List[float] = None

    def __post_init__(self):
        if self.latency_percentiles is None:
            self.latency_percentiles = [50, 95, 99, 99.9]


class KVCache:
    """Key-Value cache for efficient generation"""

    def __init__(self, max_batch_size: int, max_seq_len: int,
                 n_layers: int, n_heads: int, d_k: int, device: str):
        self.max_batch_size = max_batch_size
        self.max_seq_len = max_seq_len
        self.n_layers = n_layers
        self.n_heads = n_heads
        self.d_k = d_k
        self.device = device

        # Initialize cache
        self.reset()

    def reset(self):
        """Reset cache"""
        self.cache = {
            'k': torch.zeros(
                self.n_layers, self.max_batch_size, self.n_heads,
                self.max_seq_len, self.d_k, device=self.device
            ),
            'v': torch.zeros(
                self.n_layers, self.max_batch_size, self.n_heads,
                self.max_seq_len, self.d_k, device=self.device
            )
        }
        self.seq_len = 0

    def update(self, layer_idx: int, k: Tensor, v: Tensor) -> Tuple[Tensor, Tensor]:
        """Update cache and return full key/value tensors"""
        batch_size, n_heads, seq_len, d_k = k.shape

        # Store new values
        self.cache['k'][layer_idx, :batch_size, :, self.seq_len:self.seq_len + seq_len] = k
        self.cache['v'][layer_idx, :batch_size, :, self.seq_len:self.seq_len + seq_len] = v

        # Return full cached sequences
        return (
            self.cache['k'][layer_idx, :batch_size, :, :self.seq_len + seq_len],
            self.cache['v'][layer_idx, :batch_size, :, :self.seq_len + seq_len]
        )


class LatencyTracker:
    """Track and analyze inference latency"""

    def __init__(self, window_size: int = 1000):
        self.window_size = window_size
        self.latencies = deque(maxlen=window_size)
        self.lock = threading.Lock()

    def record(self, latency_ms: float):
        """Record a latency measurement"""
        with self.lock:
            self.latencies.append(latency_ms)

    def get_stats(self) -> Dict[str, float]:
        """Get latency statistics"""
        with self.lock:
            if not self.latencies:
                return {}

            latencies_array = np.array(self.latencies)

            stats = {
                'count': len(self.latencies),
                'mean': np.mean(latencies_array),
                'std': np.std(latencies_array),
                'min': np.min(latencies_array),
                'max': np.max(latencies_array),
                'p50': np.percentile(latencies_array, 50),
                'p95': np.percentile(latencies_array, 95),
                'p99': np.percentile(latencies_array, 99),
                'p99.9': np.percentile(latencies_array, 99.9),
            }

            return stats

    def reset(self):
        """Reset latency tracker"""
        with self.lock:
            self.latencies.clear()


class FinancialLLMInferenceEngine:
    """
    High-performance inference engine for Financial LLM

    Optimized for:
    - Ultra-low latency (target < 10ms for classification, < 100ms for generation)
    - High throughput with batching
    - Real-time market data processing
    - Thread-safe concurrent requests
    """

    def __init__(self, config: InferenceConfig):
        self.config = config

        # Setup device
        self.device = torch.device(config.device)

        # Load tokenizer
        print(f"Loading tokenizer from {config.tokenizer_path}")
        self.tokenizer = FinancialTokenizer.load(config.tokenizer_path)

        # Load model
        print(f"Loading model from {config.model_path}")
        checkpoint = torch.load(config.model_path, map_location=self.device)

        # Create model
        model_config = FinancialLLMConfig(**checkpoint.get('config', {}))
        self.model = FinancialLLM(model_config)
        self.model.load_state_dict(checkpoint['model_state_dict'])
        self.model = self.model.to(self.device)
        self.model.eval()

        # Optimization
        if config.use_torch_compile and hasattr(torch, 'compile'):
            print("Compiling model with torch.compile()...")
            self.model = torch.compile(self.model, mode='reduce-overhead')

        # KV cache
        if config.use_kv_cache:
            self.kv_cache = KVCache(
                max_batch_size=config.batch_size,
                max_seq_len=model_config.max_seq_length,
                n_layers=model_config.n_layers,
                n_heads=model_config.n_heads,
                d_k=model_config.d_model // model_config.n_heads,
                device=self.device
            )
        else:
            self.kv_cache = None

        # Latency tracking
        if config.track_latency:
            self.latency_tracker = LatencyTracker()
        else:
            self.latency_tracker = None

        # Thread pool for parallel processing
        if config.enable_thread_pool:
            self.request_queue = Queue()
            self.workers = []
            for _ in range(config.num_workers):
                worker = threading.Thread(target=self._worker_loop, daemon=True)
                worker.start()
                self.workers.append(worker)

        # Warmup
        self._warmup()

        print(f"Inference engine ready on {self.device}")

    def _warmup(self):
        """Warmup model for consistent latency"""
        print(f"Warming up model with {self.config.warmup_iterations} iterations...")

        dummy_input = torch.randint(
            0, self.tokenizer.get_vocab_size(),
            (1, 128),
            device=self.device
        )

        with torch.no_grad():
            for i in range(self.config.warmup_iterations):
                _ = self.model(dummy_input)

                if (i + 1) % 5 == 0:
                    print(f"Warmup: {i + 1}/{self.config.warmup_iterations}")

        # Clear CUDA cache
        if torch.cuda.is_available():
            torch.cuda.synchronize()
            torch.cuda.empty_cache()

        print("Warmup complete")

    def _worker_loop(self):
        """Worker thread loop for processing requests"""
        while True:
            request = self.request_queue.get()
            if request is None:
                break

            text, callback = request
            result = self._generate_internal(text)
            callback(result)

    @torch.no_grad()
    def classify(self, text: str) -> Dict[str, float]:
        """
        Classify market sentiment or regime (fast path)

        Args:
            text: Input text

        Returns:
            Classification probabilities
        """
        start_time = time.perf_counter()

        # Tokenize
        input_ids = self.tokenizer.encode(text, max_length=512, truncation=True)
        input_tensor = torch.tensor([input_ids], device=self.device)

        # Forward pass
        with torch.cuda.amp.autocast(enabled=self.config.use_mixed_precision):
            outputs = self.model(input_tensor)

            # Get regime logits if available
            if len(outputs) > 1:
                regime_logits = outputs[1]
                probs = torch.softmax(regime_logits, dim=-1)[0]

                result = {
                    'bull_market': probs[0].item(),
                    'bear_market': probs[1].item(),
                    'high_volatility': probs[2].item(),
                    'low_volatility': probs[3].item(),
                    'neutral': probs[4].item()
                }
            else:
                # Fallback to simple classification based on output tokens
                logits = outputs[0]
                last_logits = logits[0, -1, :]

                # Get probabilities for key tokens
                buy_id = self.tokenizer.token_to_id.get('[BUY]', 0)
                sell_id = self.tokenizer.token_to_id.get('[SELL]', 0)
                hold_id = self.tokenizer.token_to_id.get('[HOLD]', 0)

                probs_raw = torch.softmax(last_logits, dim=-1)
                result = {
                    'buy': probs_raw[buy_id].item(),
                    'sell': probs_raw[sell_id].item(),
                    'hold': probs_raw[hold_id].item()
                }

        # Track latency
        latency_ms = (time.perf_counter() - start_time) * 1000
        if self.latency_tracker:
            self.latency_tracker.record(latency_ms)

        result['latency_ms'] = latency_ms

        return result

    @torch.no_grad()
    def generate(self, prompt: str, max_new_tokens: Optional[int] = None,
                 temperature: Optional[float] = None,
                 top_k: Optional[int] = None,
                 top_p: Optional[float] = None) -> Dict[str, Union[str, float]]:
        """
        Generate text (e.g., market commentary, trade recommendations)

        Args:
            prompt: Input prompt
            max_new_tokens: Maximum tokens to generate
            temperature: Sampling temperature
            top_k: Top-k sampling
            top_p: Nucleus sampling

        Returns:
            Generated text and metadata
        """
        start_time = time.perf_counter()

        # Use config defaults if not specified
        max_new_tokens = max_new_tokens or self.config.max_new_tokens
        temperature = temperature or self.config.temperature
        top_k = top_k or self.config.top_k
        top_p = top_p or self.config.top_p

        # Tokenize
        input_ids = self.tokenizer.encode(prompt, max_length=1024, truncation=True)
        input_tensor = torch.tensor([input_ids], device=self.device)

        # Reset KV cache
        if self.kv_cache:
            self.kv_cache.reset()

        # Generate
        with torch.cuda.amp.autocast(enabled=self.config.use_mixed_precision):
            generated_ids = self.model.generate(
                input_tensor,
                max_new_tokens=max_new_tokens,
                temperature=temperature,
                top_k=top_k,
                top_p=top_p
            )

        # Decode
        generated_text = self.tokenizer.decode(generated_ids[0].tolist())

        # Track latency
        latency_ms = (time.perf_counter() - start_time) * 1000
        if self.latency_tracker:
            self.latency_tracker.record(latency_ms)

        return {
            'text': generated_text,
            'num_tokens': len(generated_ids[0]),
            'latency_ms': latency_ms,
            'tokens_per_second': len(generated_ids[0]) / (latency_ms / 1000)
        }

    def _generate_internal(self, text: str) -> str:
        """Internal generation for worker threads"""
        result = self.generate(text)
        return result['text']

    def generate_async(self, text: str, callback):
        """Asynchronous generation using thread pool"""
        if not self.config.enable_thread_pool:
            raise RuntimeError("Thread pool not enabled")

        self.request_queue.put((text, callback))

    def batch_classify(self, texts: List[str]) -> List[Dict[str, float]]:
        """
        Batch classification for higher throughput

        Args:
            texts: List of input texts

        Returns:
            List of classification results
        """
        results = []

        # Process in batches
        for i in range(0, len(texts), self.config.batch_size):
            batch_texts = texts[i:i + self.config.batch_size]

            # Tokenize batch
            batch_ids = []
            max_len = 0
            for text in batch_texts:
                ids = self.tokenizer.encode(text, max_length=512, truncation=True)
                batch_ids.append(ids)
                max_len = max(max_len, len(ids))

            # Pad to same length
            for ids in batch_ids:
                ids.extend([self.tokenizer.SPECIAL_TOKENS['[PAD]']] * (max_len - len(ids)))

            # Create tensor
            input_tensor = torch.tensor(batch_ids, device=self.device)

            # Forward pass
            with torch.no_grad():
                with torch.cuda.amp.autocast(enabled=self.config.use_mixed_precision):
                    outputs = self.model(input_tensor)

                    if len(outputs) > 1:
                        regime_logits = outputs[1]
                        probs = torch.softmax(regime_logits, dim=-1)

                        for j in range(len(batch_texts)):
                            results.append({
                                'bull_market': probs[j, 0].item(),
                                'bear_market': probs[j, 1].item(),
                                'high_volatility': probs[j, 2].item(),
                                'low_volatility': probs[j, 3].item(),
                                'neutral': probs[j, 4].item()
                            })
                    else:
                        # Fallback
                        for _ in batch_texts:
                            results.append({'status': 'processed'})

        return results

    def get_latency_stats(self) -> Dict[str, float]:
        """Get latency statistics"""
        if not self.latency_tracker:
            return {}

        return self.latency_tracker.get_stats()

    def reset_latency_stats(self):
        """Reset latency statistics"""
        if self.latency_tracker:
            self.latency_tracker.reset()

    def shutdown(self):
        """Shutdown inference engine"""
        if self.config.enable_thread_pool:
            # Signal workers to stop
            for _ in self.workers:
                self.request_queue.put(None)

            # Wait for workers
            for worker in self.workers:
                worker.join()

        print("Inference engine shutdown complete")


# Specialized inference functions for HFT use cases

def analyze_market_sentiment(engine: FinancialLLMInferenceEngine,
                            news_text: str) -> Dict[str, float]:
    """Analyze market sentiment from news"""
    return engine.classify(news_text)


def generate_trade_signal(engine: FinancialLLMInferenceEngine,
                         market_data: str) -> str:
    """Generate trade signal from market data"""
    prompt = f"Given the following market data:\n{market_data}\n\nTrade recommendation:"
    result = engine.generate(prompt, max_new_tokens=50, temperature=0.5)
    return result['text']


def detect_market_regime(engine: FinancialLLMInferenceEngine,
                        price_data: str) -> Dict[str, float]:
    """Detect current market regime"""
    prompt = f"Market conditions: {price_data}"
    return engine.classify(prompt)


if __name__ == "__main__":
    # Example usage
    config = InferenceConfig(
        model_path='checkpoints/best_model.pt',
        tokenizer_path='checkpoints/tokenizer.pkl',
        device='cuda' if torch.cuda.is_available() else 'cpu',
        batch_size=1,
        use_torch_compile=True,
        track_latency=True
    )

    engine = FinancialLLMInferenceEngine(config)

    # Test classification
    test_text = "Stock market rallied on strong earnings reports. S&P 500 up 2%."
    result = engine.classify(test_text)
    print(f"Classification result: {result}")

    # Test generation
    prompt = "Market analysis for AAPL:"
    generated = engine.generate(prompt, max_new_tokens=50)
    print(f"Generated text: {generated['text']}")

    # Get latency stats
    stats = engine.get_latency_stats()
    print(f"Latency stats: {stats}")
