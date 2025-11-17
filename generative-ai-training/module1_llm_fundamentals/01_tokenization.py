"""
Module 1.1: Tokenization - The Foundation of LLMs

Tokenization is the process of converting text into numerical tokens that models can understand.
This is critical for all LLM operations.
"""

import tiktoken
from transformers import AutoTokenizer
import numpy as np


class TokenizationDemo:
    """Demonstrates various tokenization techniques used in modern LLMs."""

    def __init__(self):
        # OpenAI's tokenizer (used by GPT models)
        self.openai_tokenizer = tiktoken.encoding_for_model("gpt-4")

        # HuggingFace tokenizer (BERT-based)
        self.hf_tokenizer = AutoTokenizer.from_pretrained("bert-base-uncased")

    def basic_tokenization(self, text: str) -> dict:
        """
        Demonstrates basic tokenization with different tokenizers.

        Args:
            text: Input text to tokenize

        Returns:
            Dictionary containing tokenization results
        """
        # OpenAI tokenization (BPE - Byte Pair Encoding)
        openai_tokens = self.openai_tokenizer.encode(text)
        openai_decoded = self.openai_tokenizer.decode(openai_tokens)

        # HuggingFace tokenization (WordPiece)
        hf_tokens = self.hf_tokenizer.encode(text)
        hf_decoded = self.hf_tokenizer.decode(hf_tokens)

        return {
            "original_text": text,
            "openai": {
                "tokens": openai_tokens,
                "count": len(openai_tokens),
                "decoded": openai_decoded,
                "token_strings": [
                    self.openai_tokenizer.decode([t]) for t in openai_tokens
                ]
            },
            "huggingface": {
                "tokens": hf_tokens,
                "count": len(hf_tokens),
                "decoded": hf_decoded,
                "token_strings": self.hf_tokenizer.convert_ids_to_tokens(hf_tokens)
            }
        }

    def financial_text_tokenization(self):
        """
        Examples with financial text - important for HFT and investment banking.
        """
        financial_texts = [
            "Apple Inc. stock surged 5.2% after reporting Q4 earnings of $1.46 per share.",
            "The Federal Reserve announced a 0.25% rate hike to combat inflation.",
            "Merger arbitrage opportunity: MSFT acquiring ATVI for $95/share, trading at $93.",
        ]

        results = []
        for text in financial_texts:
            result = self.basic_tokenization(text)
            results.append(result)

            print(f"\n{'='*80}")
            print(f"Text: {text}")
            print(f"OpenAI tokens: {result['openai']['count']}")
            print(f"HF tokens: {result['huggingface']['count']}")
            print(f"Token breakdown (OpenAI): {result['openai']['token_strings'][:10]}...")

        return results

    def estimate_api_cost(self, text: str, model: str = "gpt-4") -> dict:
        """
        Estimate API costs based on token count.
        Critical for production systems!

        Args:
            text: Input text
            model: Model name

        Returns:
            Cost estimation
        """
        tokens = self.openai_tokenizer.encode(text)
        token_count = len(tokens)

        # Pricing as of 2024 (check current pricing!)
        pricing = {
            "gpt-4": {"input": 0.03 / 1000, "output": 0.06 / 1000},
            "gpt-4-turbo": {"input": 0.01 / 1000, "output": 0.03 / 1000},
            "gpt-3.5-turbo": {"input": 0.0015 / 1000, "output": 0.002 / 1000},
        }

        model_pricing = pricing.get(model, pricing["gpt-4"])

        # Estimate assuming similar output length
        estimated_output_tokens = token_count

        input_cost = token_count * model_pricing["input"]
        output_cost = estimated_output_tokens * model_pricing["output"]
        total_cost = input_cost + output_cost

        return {
            "input_tokens": token_count,
            "estimated_output_tokens": estimated_output_tokens,
            "input_cost": input_cost,
            "output_cost": output_cost,
            "total_cost": total_cost,
            "model": model
        }

    def batch_cost_analysis(self, texts: list, model: str = "gpt-4"):
        """
        Analyze costs for batch processing - important for HFT news analysis.
        """
        total_cost = 0
        total_tokens = 0

        print(f"\n{'='*80}")
        print(f"Batch Cost Analysis for {model}")
        print(f"{'='*80}")

        for i, text in enumerate(texts, 1):
            cost_info = self.estimate_api_cost(text, model)
            total_cost += cost_info["total_cost"]
            total_tokens += cost_info["input_tokens"]

            print(f"\nText {i}: {text[:50]}...")
            print(f"  Tokens: {cost_info['input_tokens']}")
            print(f"  Cost: ${cost_info['total_cost']:.6f}")

        print(f"\n{'='*80}")
        print(f"Total Tokens: {total_tokens}")
        print(f"Total Cost: ${total_cost:.6f}")
        print(f"Cost per 1M tokens: ${(total_cost/total_tokens)*1000000:.2f}")
        print(f"{'='*80}")

        return {
            "total_tokens": total_tokens,
            "total_cost": total_cost,
            "texts_processed": len(texts)
        }


def main():
    """Run tokenization demonstrations."""

    demo = TokenizationDemo()

    # Example 1: Basic tokenization
    print("Example 1: Basic Tokenization")
    print("="*80)

    sample_text = "The S&P 500 index rose 2.3% on strong employment data."
    result = demo.basic_tokenization(sample_text)

    print(f"Original: {result['original_text']}")
    print(f"\nOpenAI Tokenization:")
    print(f"  Tokens: {result['openai']['tokens']}")
    print(f"  Count: {result['openai']['count']}")
    print(f"  Breakdown: {result['openai']['token_strings']}")

    # Example 2: Financial text tokenization
    print("\n\nExample 2: Financial Text Tokenization")
    demo.financial_text_tokenization()

    # Example 3: Cost estimation
    print("\n\nExample 3: API Cost Estimation")
    print("="*80)

    long_financial_report = """
    Apple Inc. reported record quarterly revenue of $123.9 billion for Q4 2023,
    representing a 8% year-over-year increase. The company's services segment
    continued to show strong growth with revenue of $22.3 billion, up 16% YoY.
    iPhone sales reached $69.7 billion, driven by strong demand for the iPhone 15 Pro.
    The board authorized an additional $110 billion for share buybacks.
    Earnings per share came in at $2.18, beating analyst estimates of $2.10.
    """

    cost_info = demo.estimate_api_cost(long_financial_report, "gpt-4")
    print(f"Model: {cost_info['model']}")
    print(f"Input tokens: {cost_info['input_tokens']}")
    print(f"Estimated total cost: ${cost_info['total_cost']:.6f}")

    # Example 4: Batch processing cost analysis
    print("\n\nExample 4: Batch Processing Cost Analysis")

    news_headlines = [
        "Fed announces surprise rate cut of 50 basis points",
        "Tesla Q3 deliveries exceed expectations at 435,000 units",
        "Oil prices surge 7% on OPEC+ production cut announcement",
        "Dollar strengthens against euro amid inflation concerns",
        "Nvidia stock splits 10-for-1 as shares hit all-time high",
    ] * 20  # Simulate 100 headlines

    demo.batch_cost_analysis(news_headlines, "gpt-4-turbo")


if __name__ == "__main__":
    main()
