"""
Financial LLM Quick Start Example
Demonstrates basic usage of the Financial LLM system
"""

import sys
import os

# Add parent directory to path
sys.path.append(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))

from src.model.transformer import create_financial_llm, FinancialLLMConfig
from src.model.tokenizer import FinancialTokenizer
from src.inference.engine import FinancialLLMInferenceEngine, InferenceConfig
import torch


def example_1_basic_usage():
    """Example 1: Basic model creation and inference"""
    print("=" * 60)
    print("Example 1: Basic Model Creation")
    print("=" * 60)

    # Create a small model for demo
    model = create_financial_llm(vocab_size=10000, size='small')
    print(f"Model created with {model.get_num_params():,} parameters")

    # Create tokenizer
    tokenizer = FinancialTokenizer(vocab_size=10000)

    # Test tokenization
    test_text = "AAPL stock rallied 5.2% to $175.50 on strong volume"
    tokens = tokenizer.tokenize(test_text)
    print(f"\nOriginal text: {test_text}")
    print(f"Tokens: {tokens[:10]}...")

    # Encode
    token_ids = tokenizer.encode(test_text)
    print(f"Token IDs: {token_ids[:10]}...")

    # Test forward pass
    input_tensor = torch.tensor([token_ids[:128]], dtype=torch.long)
    with torch.no_grad():
        outputs = model(input_tensor)
        logits = outputs[0]

    print(f"\nModel output shape: {logits.shape}")
    print(f"Output: [batch_size={logits.shape[0]}, seq_len={logits.shape[1]}, vocab_size={logits.shape[2]}]")


def example_2_financial_tokenizer():
    """Example 2: Financial tokenizer features"""
    print("\n" + "=" * 60)
    print("Example 2: Financial Tokenizer Features")
    print("=" * 60)

    tokenizer = FinancialTokenizer(vocab_size=50000)

    # Test financial-specific tokenization
    financial_texts = [
        "The S&P 500 broke through resistance at 4500 points",
        "Market volatility spiked as the VIX reached 25",
        "High-frequency traders captured the bid-ask spread",
        "Portfolio managers adjusted delta hedges in options markets",
        "MSFT upgraded to overweight with $400 price target"
    ]

    print("\nFinancial Text Tokenization:")
    for text in financial_texts:
        tokens = tokenizer.tokenize(text)
        print(f"\nText: {text}")
        print(f"Tokens: {tokens[:15]}...")
        print(f"Count: {len(tokens)} tokens")


def example_3_market_sentiment():
    """Example 3: Market sentiment classification (simulated)"""
    print("\n" + "=" * 60)
    print("Example 3: Market Sentiment Classification (Demo)")
    print("=" * 60)

    # Create small model for demo
    model = create_financial_llm(vocab_size=10000, size='small')
    tokenizer = FinancialTokenizer(vocab_size=10000)

    # Sample market texts
    market_texts = [
        "Stock market rallies on strong earnings reports, S&P 500 up 2%",
        "Markets tumble as Fed signals aggressive rate hikes",
        "Tech sector shows strength, NASDAQ leads gains",
        "Banking crisis fears weigh on financial stocks",
        "Inflation concerns persist despite cooling data"
    ]

    print("\nMarket Sentiment Analysis (Demo):")
    for text in market_texts:
        # Tokenize
        token_ids = tokenizer.encode(text, max_length=128, truncation=True)
        input_tensor = torch.tensor([token_ids], dtype=torch.long)

        # Forward pass
        with torch.no_grad():
            outputs = model(input_tensor)
            logits = outputs[0]

            # Get last token prediction
            last_token_logits = logits[0, -1, :]
            probs = torch.softmax(last_token_logits, dim=-1)

            # Get top predictions
            top_k = 5
            top_probs, top_indices = torch.topk(probs, top_k)

        print(f"\nText: {text}")
        print(f"Top {top_k} predictions (token IDs): {top_indices.tolist()}")
        print(f"Probabilities: {[f'{p:.4f}' for p in top_probs.tolist()]}")


def example_4_generation():
    """Example 4: Text generation"""
    print("\n" + "=" * 60)
    print("Example 4: Text Generation (Demo)")
    print("=" * 60)

    model = create_financial_llm(vocab_size=10000, size='small')
    tokenizer = FinancialTokenizer(vocab_size=10000)

    # Generation prompts
    prompts = [
        "Market analysis for technology sector:",
        "Investment thesis for AAPL:",
        "Risk assessment for portfolio:"
    ]

    print("\nText Generation:")
    for prompt in prompts:
        # Tokenize prompt
        token_ids = tokenizer.encode(prompt, max_length=64, truncation=True)
        input_tensor = torch.tensor([token_ids], dtype=torch.long)

        # Generate (simplified for demo)
        with torch.no_grad():
            generated = model.generate(
                input_tensor,
                max_new_tokens=20,
                temperature=0.8,
                top_k=50
            )

        # Decode
        generated_text = tokenizer.decode(generated[0].tolist())

        print(f"\nPrompt: {prompt}")
        print(f"Generated: {generated_text[:200]}...")


def example_5_performance_benchmark():
    """Example 5: Performance benchmarking"""
    print("\n" + "=" * 60)
    print("Example 5: Performance Benchmark")
    print("=" * 60)

    import time

    model = create_financial_llm(vocab_size=10000, size='small')
    tokenizer = FinancialTokenizer(vocab_size=10000)

    # Prepare test data
    test_texts = [
        "Stock market analysis " * 10,
        "Federal Reserve policy " * 10,
        "Market volatility concerns " * 10
    ] * 10  # 30 texts

    # Benchmark
    print("\nBenchmarking classification speed:")
    latencies = []

    for text in test_texts:
        token_ids = tokenizer.encode(text, max_length=128, truncation=True)
        input_tensor = torch.tensor([token_ids], dtype=torch.long)

        start_time = time.perf_counter()

        with torch.no_grad():
            outputs = model(input_tensor)

        end_time = time.perf_counter()
        latency_ms = (end_time - start_time) * 1000
        latencies.append(latency_ms)

    import numpy as np
    latencies_array = np.array(latencies)

    print(f"\nResults from {len(test_texts)} inferences:")
    print(f"  Mean latency: {np.mean(latencies_array):.2f} ms")
    print(f"  Median (p50): {np.percentile(latencies_array, 50):.2f} ms")
    print(f"  p95: {np.percentile(latencies_array, 95):.2f} ms")
    print(f"  p99: {np.percentile(latencies_array, 99):.2f} ms")
    print(f"  Min: {np.min(latencies_array):.2f} ms")
    print(f"  Max: {np.max(latencies_array):.2f} ms")
    print(f"\n  Throughput: {1000 / np.mean(latencies_array):.1f} inferences/second")


def example_6_api_usage():
    """Example 6: API usage demonstration"""
    print("\n" + "=" * 60)
    print("Example 6: API Usage (Code Examples)")
    print("=" * 60)

    print("\nREST API Usage:")
    print("""
# Classification
import requests

response = requests.post(
    'http://localhost:8000/v1/classify',
    json={'text': 'Stock market rallies on strong earnings'}
)
print(response.json())

# Generation
response = requests.post(
    'http://localhost:8000/v1/generate',
    json={
        'prompt': 'Market analysis:',
        'max_new_tokens': 100,
        'temperature': 0.7
    }
)
print(response.json())
    """)

    print("\ngRPC API Usage:")
    print("""
from src.api.grpc_client import FinancialLLMgRPCClient

client = FinancialLLMgRPCClient('localhost', 50051)

# Classify
result = client.classify('Market text...')
print(result)

# Generate
result = client.generate('Prompt...', max_new_tokens=100)
print(result)
    """)


def main():
    """Run all examples"""
    print("\n" + "=" * 60)
    print("Financial LLM - Quick Start Examples")
    print("=" * 60)

    examples = [
        ("Basic Usage", example_1_basic_usage),
        ("Financial Tokenizer", example_2_financial_tokenizer),
        ("Market Sentiment", example_3_market_sentiment),
        ("Text Generation", example_4_generation),
        ("Performance Benchmark", example_5_performance_benchmark),
        ("API Usage", example_6_api_usage)
    ]

    print("\nAvailable examples:")
    for i, (name, _) in enumerate(examples, 1):
        print(f"  {i}. {name}")

    print("\nRunning all examples...\n")

    for name, func in examples:
        try:
            func()
        except Exception as e:
            print(f"\nError in {name}: {e}")
            import traceback
            traceback.print_exc()

    print("\n" + "=" * 60)
    print("Examples Complete!")
    print("=" * 60)
    print("\nNext steps:")
    print("1. Train your own model with financial data")
    print("2. Start the API server")
    print("3. Integrate with your trading systems")
    print("4. See docs/ for detailed documentation")


if __name__ == "__main__":
    main()
