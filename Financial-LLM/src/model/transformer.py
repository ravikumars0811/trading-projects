"""
Financial LLM - Core Transformer Architecture
Optimized for HFT and Investment Banking Use Cases
"""

import torch
import torch.nn as nn
import torch.nn.functional as F
import math
from typing import Optional, Tuple
from dataclasses import dataclass


@dataclass
class FinancialLLMConfig:
    """Configuration for Financial LLM"""
    vocab_size: int = 50000  # Extended for financial terminology
    max_seq_length: int = 2048
    d_model: int = 768  # Model dimension
    n_heads: int = 12  # Attention heads
    n_layers: int = 12  # Transformer layers
    d_ff: int = 3072  # Feed-forward dimension
    dropout: float = 0.1

    # Financial-specific parameters
    enable_numerical_encoding: bool = True  # Special encoding for numbers
    enable_time_encoding: bool = True  # Temporal awareness
    enable_market_regime_detection: bool = True  # Market condition awareness

    # Performance optimization
    use_flash_attention: bool = True
    use_mixed_precision: bool = True
    compile_model: bool = True


class PositionalEncoding(nn.Module):
    """Positional encoding with optional temporal awareness for time-series data"""

    def __init__(self, d_model: int, max_len: int = 5000, dropout: float = 0.1):
        super().__init__()
        self.dropout = nn.Dropout(p=dropout)

        position = torch.arange(max_len).unsqueeze(1)
        div_term = torch.exp(torch.arange(0, d_model, 2) * (-math.log(10000.0) / d_model))
        pe = torch.zeros(max_len, 1, d_model)
        pe[:, 0, 0::2] = torch.sin(position * div_term)
        pe[:, 0, 1::2] = torch.cos(position * div_term)
        self.register_buffer('pe', pe)

    def forward(self, x: torch.Tensor) -> torch.Tensor:
        """
        Args:
            x: Tensor of shape [seq_len, batch_size, d_model]
        """
        x = x + self.pe[:x.size(0)]
        return self.dropout(x)


class MultiHeadAttention(nn.Module):
    """Multi-head attention with optional flash attention for speed"""

    def __init__(self, d_model: int, n_heads: int, dropout: float = 0.1,
                 use_flash_attention: bool = True):
        super().__init__()
        assert d_model % n_heads == 0

        self.d_model = d_model
        self.n_heads = n_heads
        self.d_k = d_model // n_heads
        self.use_flash_attention = use_flash_attention

        self.q_linear = nn.Linear(d_model, d_model)
        self.k_linear = nn.Linear(d_model, d_model)
        self.v_linear = nn.Linear(d_model, d_model)
        self.out_linear = nn.Linear(d_model, d_model)

        self.dropout = nn.Dropout(dropout)
        self.scale = math.sqrt(self.d_k)

    def forward(self, query: torch.Tensor, key: torch.Tensor, value: torch.Tensor,
                mask: Optional[torch.Tensor] = None) -> torch.Tensor:
        batch_size = query.size(0)

        # Linear projections
        Q = self.q_linear(query).view(batch_size, -1, self.n_heads, self.d_k).transpose(1, 2)
        K = self.k_linear(key).view(batch_size, -1, self.n_heads, self.d_k).transpose(1, 2)
        V = self.v_linear(value).view(batch_size, -1, self.n_heads, self.d_k).transpose(1, 2)

        if self.use_flash_attention and hasattr(F, 'scaled_dot_product_attention'):
            # Use PyTorch 2.0+ optimized attention
            attn_output = F.scaled_dot_product_attention(
                Q, K, V, attn_mask=mask, dropout_p=self.dropout.p if self.training else 0.0
            )
        else:
            # Standard attention
            scores = torch.matmul(Q, K.transpose(-2, -1)) / self.scale

            if mask is not None:
                scores = scores.masked_fill(mask == 0, -1e9)

            attn_weights = F.softmax(scores, dim=-1)
            attn_weights = self.dropout(attn_weights)
            attn_output = torch.matmul(attn_weights, V)

        # Concatenate heads
        attn_output = attn_output.transpose(1, 2).contiguous().view(
            batch_size, -1, self.d_model
        )

        return self.out_linear(attn_output)


class FeedForward(nn.Module):
    """Feed-forward network with GELU activation"""

    def __init__(self, d_model: int, d_ff: int, dropout: float = 0.1):
        super().__init__()
        self.linear1 = nn.Linear(d_model, d_ff)
        self.linear2 = nn.Linear(d_ff, d_model)
        self.dropout = nn.Dropout(dropout)

    def forward(self, x: torch.Tensor) -> torch.Tensor:
        return self.linear2(self.dropout(F.gelu(self.linear1(x))))


class TransformerBlock(nn.Module):
    """Single transformer block with pre-layer normalization"""

    def __init__(self, config: FinancialLLMConfig):
        super().__init__()
        self.attention = MultiHeadAttention(
            config.d_model, config.n_heads, config.dropout, config.use_flash_attention
        )
        self.feed_forward = FeedForward(config.d_model, config.d_ff, config.dropout)
        self.ln1 = nn.LayerNorm(config.d_model)
        self.ln2 = nn.LayerNorm(config.d_model)
        self.dropout = nn.Dropout(config.dropout)

    def forward(self, x: torch.Tensor, mask: Optional[torch.Tensor] = None) -> torch.Tensor:
        # Pre-LN architecture
        attn_output = self.attention(self.ln1(x), self.ln1(x), self.ln1(x), mask)
        x = x + self.dropout(attn_output)

        ff_output = self.feed_forward(self.ln2(x))
        x = x + self.dropout(ff_output)

        return x


class NumericalEncoder(nn.Module):
    """Special encoder for financial numerical values"""

    def __init__(self, d_model: int):
        super().__init__()
        self.value_encoder = nn.Linear(1, d_model // 2)
        self.magnitude_encoder = nn.Linear(1, d_model // 2)

    def forward(self, x: torch.Tensor) -> torch.Tensor:
        """
        Encode numerical values with separate value and magnitude encoding
        Args:
            x: Numerical values of shape [batch_size, seq_len, 1]
        Returns:
            Encoded tensor of shape [batch_size, seq_len, d_model]
        """
        # Normalize value
        value_encoded = self.value_encoder(x)

        # Encode magnitude (log scale)
        magnitude = torch.log1p(torch.abs(x))
        magnitude_encoded = self.magnitude_encoder(magnitude)

        return torch.cat([value_encoded, magnitude_encoded], dim=-1)


class FinancialLLM(nn.Module):
    """
    Financial Large Language Model

    Designed specifically for HFT and investment banking applications with:
    - Custom numerical encoding for financial data
    - Temporal awareness for time-series analysis
    - Market regime detection
    - Low-latency inference optimization
    """

    def __init__(self, config: FinancialLLMConfig):
        super().__init__()
        self.config = config

        # Embeddings
        self.token_embedding = nn.Embedding(config.vocab_size, config.d_model)
        self.position_encoding = PositionalEncoding(config.d_model, config.max_seq_length, config.dropout)

        # Numerical encoder for financial values
        if config.enable_numerical_encoding:
            self.numerical_encoder = NumericalEncoder(config.d_model)

        # Transformer blocks
        self.transformer_blocks = nn.ModuleList([
            TransformerBlock(config) for _ in range(config.n_layers)
        ])

        # Output layers
        self.ln_final = nn.LayerNorm(config.d_model)
        self.lm_head = nn.Linear(config.d_model, config.vocab_size, bias=False)

        # Market regime classifier (optional)
        if config.enable_market_regime_detection:
            self.regime_head = nn.Linear(config.d_model, 5)  # 5 market regimes

        # Initialize weights
        self.apply(self._init_weights)

        # Tie weights between embedding and output
        self.token_embedding.weight = self.lm_head.weight

    def _init_weights(self, module):
        """Initialize weights with scaled initialization"""
        if isinstance(module, nn.Linear):
            torch.nn.init.normal_(module.weight, mean=0.0, std=0.02)
            if module.bias is not None:
                torch.nn.init.zeros_(module.bias)
        elif isinstance(module, nn.Embedding):
            torch.nn.init.normal_(module.weight, mean=0.0, std=0.02)
        elif isinstance(module, nn.LayerNorm):
            torch.nn.init.zeros_(module.bias)
            torch.nn.init.ones_(module.weight)

    def forward(self, input_ids: torch.Tensor,
                numerical_values: Optional[torch.Tensor] = None,
                attention_mask: Optional[torch.Tensor] = None) -> Tuple[torch.Tensor, ...]:
        """
        Forward pass

        Args:
            input_ids: Token IDs [batch_size, seq_len]
            numerical_values: Optional numerical data [batch_size, seq_len, 1]
            attention_mask: Optional attention mask [batch_size, seq_len]

        Returns:
            logits: Language modeling logits [batch_size, seq_len, vocab_size]
            regime_logits: Market regime logits [batch_size, 5] (if enabled)
        """
        # Token embeddings
        x = self.token_embedding(input_ids)  # [batch_size, seq_len, d_model]

        # Add numerical encoding if provided
        if numerical_values is not None and self.config.enable_numerical_encoding:
            num_encoded = self.numerical_encoder(numerical_values)
            x = x + num_encoded

        # Add positional encoding
        x = x.transpose(0, 1)  # [seq_len, batch_size, d_model]
        x = self.position_encoding(x)
        x = x.transpose(0, 1)  # [batch_size, seq_len, d_model]

        # Create causal mask
        seq_len = input_ids.size(1)
        causal_mask = torch.triu(torch.ones(seq_len, seq_len), diagonal=1).bool()
        causal_mask = causal_mask.to(input_ids.device)

        if attention_mask is not None:
            # Combine with provided mask
            attention_mask = attention_mask.unsqueeze(1).unsqueeze(2)
            causal_mask = causal_mask.unsqueeze(0).unsqueeze(0)
            mask = attention_mask & ~causal_mask
        else:
            mask = ~causal_mask

        # Transformer blocks
        for block in self.transformer_blocks:
            x = block(x, mask)

        # Final layer norm
        x = self.ln_final(x)

        # Language modeling head
        logits = self.lm_head(x)

        outputs = (logits,)

        # Market regime detection
        if self.config.enable_market_regime_detection:
            # Use mean pooling of sequence for regime classification
            regime_repr = x.mean(dim=1)
            regime_logits = self.regime_head(regime_repr)
            outputs = outputs + (regime_logits,)

        return outputs

    @torch.no_grad()
    def generate(self, input_ids: torch.Tensor, max_new_tokens: int = 100,
                 temperature: float = 0.8, top_k: int = 50, top_p: float = 0.95) -> torch.Tensor:
        """
        Generate text using the model

        Args:
            input_ids: Input token IDs [batch_size, seq_len]
            max_new_tokens: Maximum number of tokens to generate
            temperature: Sampling temperature
            top_k: Top-k sampling parameter
            top_p: Nucleus sampling parameter

        Returns:
            Generated token IDs
        """
        self.eval()

        for _ in range(max_new_tokens):
            # Crop to max sequence length
            idx_cond = input_ids if input_ids.size(1) <= self.config.max_seq_length else \
                       input_ids[:, -self.config.max_seq_length:]

            # Forward pass
            logits = self.forward(idx_cond)[0]

            # Get logits for last position
            logits = logits[:, -1, :] / temperature

            # Top-k filtering
            if top_k > 0:
                v, _ = torch.topk(logits, min(top_k, logits.size(-1)))
                logits[logits < v[:, [-1]]] = -float('Inf')

            # Top-p (nucleus) filtering
            if top_p < 1.0:
                sorted_logits, sorted_indices = torch.sort(logits, descending=True)
                cumulative_probs = torch.cumsum(F.softmax(sorted_logits, dim=-1), dim=-1)

                sorted_indices_to_remove = cumulative_probs > top_p
                sorted_indices_to_remove[..., 1:] = sorted_indices_to_remove[..., :-1].clone()
                sorted_indices_to_remove[..., 0] = 0

                indices_to_remove = sorted_indices_to_remove.scatter(1, sorted_indices, sorted_indices_to_remove)
                logits[indices_to_remove] = -float('Inf')

            # Sample
            probs = F.softmax(logits, dim=-1)
            idx_next = torch.multinomial(probs, num_samples=1)

            # Append to sequence
            input_ids = torch.cat([input_ids, idx_next], dim=1)

        return input_ids

    def get_num_params(self) -> int:
        """Return the number of parameters in the model"""
        return sum(p.numel() for p in self.parameters())


def create_financial_llm(vocab_size: int = 50000,
                        size: str = 'base') -> FinancialLLM:
    """
    Factory function to create Financial LLM models of different sizes

    Args:
        vocab_size: Vocabulary size
        size: Model size - 'small', 'base', 'large', 'xlarge'

    Returns:
        FinancialLLM model
    """
    configs = {
        'small': FinancialLLMConfig(
            vocab_size=vocab_size,
            d_model=512,
            n_heads=8,
            n_layers=6,
            d_ff=2048
        ),
        'base': FinancialLLMConfig(
            vocab_size=vocab_size,
            d_model=768,
            n_heads=12,
            n_layers=12,
            d_ff=3072
        ),
        'large': FinancialLLMConfig(
            vocab_size=vocab_size,
            d_model=1024,
            n_heads=16,
            n_layers=24,
            d_ff=4096
        ),
        'xlarge': FinancialLLMConfig(
            vocab_size=vocab_size,
            d_model=1536,
            n_heads=24,
            n_layers=32,
            d_ff=6144
        )
    }

    config = configs.get(size, configs['base'])
    model = FinancialLLM(config)

    print(f"Created {size} Financial LLM with {model.get_num_params():,} parameters")

    return model


if __name__ == "__main__":
    # Example usage
    model = create_financial_llm(size='base')

    # Test forward pass
    batch_size, seq_len = 2, 128
    input_ids = torch.randint(0, 50000, (batch_size, seq_len))

    logits = model(input_ids)[0]
    print(f"Output shape: {logits.shape}")

    # Test generation
    generated = model.generate(input_ids[:, :10], max_new_tokens=20)
    print(f"Generated shape: {generated.shape}")
