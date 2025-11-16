"""
Module 2.1: OpenAI API Basics

Learn how to use OpenAI's API for financial applications.
Covers: chat completions, function calling, structured outputs.
"""

import openai
import os
import json
from typing import List, Dict, Any
from dotenv import load_dotenv
import time

load_dotenv()


class OpenAIFinancialAssistant:
    """
    Production-ready OpenAI integration for financial applications.
    """

    def __init__(self, api_key: str = None, model: str = "gpt-4-turbo-preview"):
        """
        Initialize OpenAI client.

        Args:
            api_key: OpenAI API key (or from env)
            model: Model to use
        """
        self.client = openai.OpenAI(api_key=api_key or os.getenv("OPENAI_API_KEY"))
        self.model = model

    def simple_completion(self, prompt: str, temperature: float = 0.7) -> str:
        """
        Simple text completion.

        Args:
            prompt: Input prompt
            temperature: Randomness (0-2)

        Returns:
            Generated text
        """
        response = self.client.chat.completions.create(
            model=self.model,
            messages=[{"role": "user", "content": prompt}],
            temperature=temperature
        )

        return response.choices[0].message.content

    def analyze_earnings_report(self, report_text: str) -> dict:
        """
        Analyze earnings report using GPT-4.
        Use case: Automated financial document analysis.

        Args:
            report_text: Earnings report text

        Returns:
            Structured analysis
        """
        prompt = f"""
        Analyze the following earnings report and provide:
        1. Key financial metrics (revenue, EPS, margins)
        2. Sentiment (positive/negative/neutral)
        3. Main highlights
        4. Risk factors mentioned
        5. Forward guidance

        Format your response as JSON.

        Earnings Report:
        {report_text}
        """

        response = self.client.chat.completions.create(
            model=self.model,
            messages=[
                {
                    "role": "system",
                    "content": "You are a financial analyst. Analyze earnings reports and return structured JSON data."
                },
                {"role": "user", "content": prompt}
            ],
            temperature=0.3,  # Lower temperature for factual analysis
            response_format={"type": "json_object"}
        )

        return json.loads(response.choices[0].message.content)

    def function_calling_example(self, query: str) -> dict:
        """
        Use OpenAI function calling for structured data extraction.
        Critical for: trading signals, risk assessment, data extraction.

        Args:
            query: Natural language query

        Returns:
            Extracted structured data
        """
        # Define available functions
        functions = [
            {
                "name": "extract_trading_signal",
                "description": "Extract trading signals from market commentary",
                "parameters": {
                    "type": "object",
                    "properties": {
                        "ticker": {
                            "type": "string",
                            "description": "Stock ticker symbol"
                        },
                        "action": {
                            "type": "string",
                            "enum": ["BUY", "SELL", "HOLD"],
                            "description": "Recommended action"
                        },
                        "confidence": {
                            "type": "number",
                            "description": "Confidence level 0-100"
                        },
                        "reasoning": {
                            "type": "string",
                            "description": "Explanation for the signal"
                        },
                        "target_price": {
                            "type": "number",
                            "description": "Price target"
                        }
                    },
                    "required": ["ticker", "action", "confidence", "reasoning"]
                }
            }
        ]

        response = self.client.chat.completions.create(
            model=self.model,
            messages=[{"role": "user", "content": query}],
            functions=functions,
            function_call={"name": "extract_trading_signal"}
        )

        # Extract function call arguments
        message = response.choices[0].message
        if message.function_call:
            return json.loads(message.function_call.arguments)
        else:
            return {"error": "No function call generated"}

    def streaming_response(self, prompt: str):
        """
        Stream responses for real-time user experience.
        Use case: Live market commentary, interactive chat.

        Args:
            prompt: Input prompt

        Yields:
            Response chunks
        """
        stream = self.client.chat.completions.create(
            model=self.model,
            messages=[{"role": "user", "content": prompt}],
            stream=True
        )

        for chunk in stream:
            if chunk.choices[0].delta.content is not None:
                yield chunk.choices[0].delta.content

    def batch_analysis(
        self,
        items: List[str],
        analysis_prompt: str,
        max_concurrent: int = 5
    ) -> List[str]:
        """
        Analyze multiple items efficiently.
        Use case: Batch news analysis, multi-stock screening.

        Args:
            items: List of items to analyze
            analysis_prompt: Analysis instruction
            max_concurrent: Max concurrent requests

        Returns:
            List of analyses
        """
        results = []

        for i in range(0, len(items), max_concurrent):
            batch = items[i:i + max_concurrent]

            batch_results = []
            for item in batch:
                full_prompt = f"{analysis_prompt}\n\nItem to analyze:\n{item}"
                try:
                    result = self.simple_completion(full_prompt, temperature=0.3)
                    batch_results.append(result)
                except Exception as e:
                    batch_results.append(f"Error: {str(e)}")

                # Rate limiting
                time.sleep(0.1)

            results.extend(batch_results)

        return results

    def with_retry(self, func, max_retries: int = 3, *args, **kwargs):
        """
        Wrapper for API calls with retry logic.
        Critical for production systems!

        Args:
            func: Function to call
            max_retries: Maximum retry attempts
            *args, **kwargs: Function arguments

        Returns:
            Function result
        """
        for attempt in range(max_retries):
            try:
                return func(*args, **kwargs)
            except openai.RateLimitError as e:
                if attempt < max_retries - 1:
                    wait_time = (2 ** attempt)  # Exponential backoff
                    print(f"Rate limit hit. Waiting {wait_time}s...")
                    time.sleep(wait_time)
                else:
                    raise
            except openai.APIError as e:
                if attempt < max_retries - 1:
                    wait_time = 2 ** attempt
                    print(f"API error. Retrying in {wait_time}s...")
                    time.sleep(wait_time)
                else:
                    raise


class MarketCommentaryGenerator:
    """
    Generate professional market commentary using GPT-4.
    """

    def __init__(self):
        self.assistant = OpenAIFinancialAssistant()

    def generate_daily_commentary(self, market_data: dict) -> str:
        """
        Generate daily market commentary.

        Args:
            market_data: Dictionary with market data

        Returns:
            Professional commentary
        """
        prompt = f"""
        Generate a professional daily market commentary based on this data:

        Major Indices:
        - S&P 500: {market_data.get('sp500_change', 'N/A')}%
        - NASDAQ: {market_data.get('nasdaq_change', 'N/A')}%
        - DOW: {market_data.get('dow_change', 'N/A')}%

        Top Gainers: {', '.join(market_data.get('gainers', []))}
        Top Losers: {', '.join(market_data.get('losers', []))}

        Economic Data: {market_data.get('economic_news', 'None')}

        Write a 2-3 paragraph professional commentary suitable for institutional clients.
        """

        return self.assistant.simple_completion(prompt, temperature=0.7)

    def generate_trade_explanation(
        self,
        ticker: str,
        action: str,
        reasoning: str
    ) -> str:
        """
        Generate client-friendly trade explanation.

        Args:
            ticker: Stock ticker
            action: BUY/SELL/HOLD
            reasoning: Internal reasoning

        Returns:
            Client-friendly explanation
        """
        prompt = f"""
        Create a clear, professional explanation for this trade recommendation:

        Ticker: {ticker}
        Action: {action}
        Internal Analysis: {reasoning}

        Write a 1-paragraph explanation suitable for sophisticated investors.
        Focus on key factors without jargon.
        """

        return self.assistant.simple_completion(prompt, temperature=0.6)


def main():
    """Run OpenAI API demonstrations."""

    assistant = OpenAIFinancialAssistant()

    # Example 1: Simple completion
    print("Example 1: Simple Market Analysis")
    print("="*80)

    query = "What are the key risks in the current market environment?"
    response = assistant.simple_completion(query)
    print(f"Query: {query}")
    print(f"\nResponse:\n{response}")

    # Example 2: Earnings report analysis
    print("\n\nExample 2: Earnings Report Analysis")
    print("="*80)

    earnings_report = """
    Apple Inc. Q4 2023 Results:
    Revenue: $89.5 billion (up 8% YoY)
    EPS: $1.46 (beat est. $1.39)
    iPhone revenue: $43.8 billion (up 3%)
    Services revenue: $22.3 billion (up 16%)
    Gross margin: 45.2% (up from 43.0%)

    Guidance: Expecting mid-single digit revenue growth in Q1 2024.
    CEO noted strong momentum in Services and emerging markets.
    Supply chain constraints easing significantly.
    """

    if os.getenv("OPENAI_API_KEY"):
        try:
            analysis = assistant.analyze_earnings_report(earnings_report)
            print("\nStructured Analysis:")
            print(json.dumps(analysis, indent=2))
        except Exception as e:
            print(f"Error: {e}")
            print("Note: Requires valid OpenAI API key")

    # Example 3: Function calling for trading signals
    print("\n\nExample 3: Function Calling - Trading Signal Extraction")
    print("="*80)

    market_commentary = """
    Tesla showing strong momentum after Q3 delivery beat. Delivered 435,000 vehicles
    vs expectations of 420,000. Production efficiency improving, margins expanding.
    Price target raised to $300. Strong buy recommendation with high conviction.
    """

    if os.getenv("OPENAI_API_KEY"):
        try:
            signal = assistant.function_calling_example(market_commentary)
            print("\nExtracted Trading Signal:")
            print(json.dumps(signal, indent=2))
        except Exception as e:
            print(f"Error: {e}")

    # Example 4: Streaming response
    print("\n\nExample 4: Streaming Response (Real-time)")
    print("="*80)

    stream_query = "Explain the impact of rising interest rates on tech stocks in simple terms."

    if os.getenv("OPENAI_API_KEY"):
        try:
            print(f"Query: {stream_query}\n\nResponse: ", end="", flush=True)
            for chunk in assistant.streaming_response(stream_query):
                print(chunk, end="", flush=True)
            print("\n")
        except Exception as e:
            print(f"Error: {e}")

    # Example 5: Market commentary generation
    print("\n\nExample 5: Automated Market Commentary")
    print("="*80)

    market_data = {
        'sp500_change': 1.2,
        'nasdaq_change': 1.8,
        'dow_change': 0.9,
        'gainers': ['NVDA +5.2%', 'AAPL +3.1%', 'MSFT +2.4%'],
        'losers': ['XOM -2.1%', 'CVX -1.8%', 'BA -1.5%'],
        'economic_news': 'Strong jobs report, unemployment at 3.8%'
    }

    if os.getenv("OPENAI_API_KEY"):
        try:
            generator = MarketCommentaryGenerator()
            commentary = generator.generate_daily_commentary(market_data)
            print("\nGenerated Market Commentary:")
            print(commentary)
        except Exception as e:
            print(f"Error: {e}")

    # Example 6: Batch analysis
    print("\n\nExample 6: Batch News Analysis")
    print("="*80)

    news_items = [
        "Fed signals potential pause in rate hikes",
        "Tech earnings exceed expectations across the board",
        "Oil prices surge on supply concerns",
    ]

    analysis_prompt = "Summarize this news in one sentence and rate sentiment (positive/negative/neutral):"

    if os.getenv("OPENAI_API_KEY"):
        try:
            results = assistant.batch_analysis(news_items, analysis_prompt)
            for news, result in zip(news_items, results):
                print(f"\nNews: {news}")
                print(f"Analysis: {result}")
        except Exception as e:
            print(f"Error: {e}")


if __name__ == "__main__":
    main()
