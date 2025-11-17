"""
Module 3.1: Real-time Market Sentiment Analysis for HFT

Use LLMs to analyze news, tweets, and market commentary in real-time.
Critical for: Alpha generation, risk management, trade timing.
"""

import openai
import anthropic
import os
from typing import List, Dict, Any, Tuple
from datetime import datetime
import json
from collections import deque
import asyncio
import aiohttp
from dotenv import load_dotenv

load_dotenv()


class RealTimeSentimentAnalyzer:
    """
    Production-grade sentiment analysis for HFT systems.

    Features:
    - Sub-second latency for critical signals
    - Batch processing for efficiency
    - Caching for repeated queries
    - Multiple sentiment sources
    """

    def __init__(self, provider: str = "openai"):
        """
        Initialize sentiment analyzer.

        Args:
            provider: 'openai' or 'claude'
        """
        self.provider = provider

        if provider == "openai":
            self.client = openai.OpenAI(api_key=os.getenv("OPENAI_API_KEY"))
            self.model = "gpt-4-turbo-preview"
        else:
            self.client = anthropic.Anthropic(api_key=os.getenv("ANTHROPIC_API_KEY"))
            self.model = "claude-3-5-sonnet-20241022"

        # Cache for repeated analyses
        self.cache = {}

        # Sliding window for sentiment aggregation
        self.sentiment_window = deque(maxlen=100)

    def analyze_single_item(
        self,
        text: str,
        ticker: str = None,
        use_cache: bool = True
    ) -> Dict[str, Any]:
        """
        Analyze sentiment of single news item/tweet.
        Optimized for low latency.

        Args:
            text: Text to analyze
            ticker: Optional stock ticker for context
            use_cache: Use cached results if available

        Returns:
            Sentiment analysis with score and reasoning
        """
        # Check cache
        cache_key = f"{ticker}:{text[:100]}"
        if use_cache and cache_key in self.cache:
            return self.cache[cache_key]

        # Construct efficient prompt
        prompt = f"""Analyze the market sentiment of this text{f' for {ticker}' if ticker else ''}.

Text: {text}

Respond ONLY with JSON:
{{
    "sentiment": "BULLISH" or "BEARISH" or "NEUTRAL",
    "score": -1.0 to 1.0,
    "confidence": 0.0 to 1.0,
    "key_factors": ["brief", "factors"],
    "urgency": "LOW" or "MEDIUM" or "HIGH"
}}"""

        if self.provider == "openai":
            response = self.client.chat.completions.create(
                model="gpt-3.5-turbo",  # Faster for simple tasks
                messages=[
                    {
                        "role": "system",
                        "content": "You are a financial sentiment analyzer. Respond only with valid JSON."
                    },
                    {"role": "user", "content": prompt}
                ],
                temperature=0.3,
                max_tokens=200,
                response_format={"type": "json_object"}
            )
            result_text = response.choices[0].message.content
        else:
            response = self.client.messages.create(
                model=self.model,
                max_tokens=200,
                temperature=0.3,
                messages=[{"role": "user", "content": prompt}]
            )
            result_text = response.content[0].text

        # Parse result
        try:
            if "```json" in result_text:
                result_text = result_text.split("```json")[1].split("```")[0]
            result = json.loads(result_text)
        except:
            result = {
                "sentiment": "NEUTRAL",
                "score": 0.0,
                "confidence": 0.0,
                "key_factors": ["parsing_error"],
                "urgency": "LOW"
            }

        # Add metadata
        result["timestamp"] = datetime.now().isoformat()
        result["source_text"] = text[:200]
        result["ticker"] = ticker

        # Cache result
        if use_cache:
            self.cache[cache_key] = result

        # Add to sliding window
        self.sentiment_window.append(result)

        return result

    async def analyze_batch_async(
        self,
        items: List[Tuple[str, str]]
    ) -> List[Dict[str, Any]]:
        """
        Analyze multiple items concurrently.
        Critical for processing high-volume news feeds.

        Args:
            items: List of (text, ticker) tuples

        Returns:
            List of sentiment analyses
        """
        tasks = []
        for text, ticker in items:
            task = asyncio.create_task(
                self._async_analyze_single(text, ticker)
            )
            tasks.append(task)

        return await asyncio.gather(*tasks)

    async def _async_analyze_single(self, text: str, ticker: str) -> Dict[str, Any]:
        """Helper for async analysis."""
        # Run synchronous analysis in executor
        loop = asyncio.get_event_loop()
        return await loop.run_in_executor(
            None,
            self.analyze_single_item,
            text,
            ticker
        )

    def get_aggregate_sentiment(
        self,
        ticker: str = None,
        window_size: int = 20
    ) -> Dict[str, Any]:
        """
        Get aggregate sentiment from recent analyses.
        Use case: Overall market sentiment, ticker-specific sentiment.

        Args:
            ticker: Filter by ticker
            window_size: Number of recent items to consider

        Returns:
            Aggregate sentiment metrics
        """
        # Filter by ticker if specified
        if ticker:
            relevant_items = [
                item for item in list(self.sentiment_window)[-window_size:]
                if item.get("ticker") == ticker
            ]
        else:
            relevant_items = list(self.sentiment_window)[-window_size:]

        if not relevant_items:
            return {
                "aggregate_score": 0.0,
                "aggregate_sentiment": "NEUTRAL",
                "sample_size": 0
            }

        # Calculate aggregate metrics
        scores = [item["score"] for item in relevant_items]
        confidences = [item["confidence"] for item in relevant_items]

        # Weighted average by confidence
        weighted_score = sum(
            s * c for s, c in zip(scores, confidences)
        ) / sum(confidences) if sum(confidences) > 0 else 0

        # Sentiment distribution
        sentiment_counts = {
            "BULLISH": sum(1 for item in relevant_items if item["sentiment"] == "BULLISH"),
            "BEARISH": sum(1 for item in relevant_items if item["sentiment"] == "BEARISH"),
            "NEUTRAL": sum(1 for item in relevant_items if item["sentiment"] == "NEUTRAL")
        }

        # Determine aggregate sentiment
        if weighted_score > 0.3:
            aggregate_sentiment = "BULLISH"
        elif weighted_score < -0.3:
            aggregate_sentiment = "BEARISH"
        else:
            aggregate_sentiment = "NEUTRAL"

        return {
            "aggregate_score": weighted_score,
            "aggregate_sentiment": aggregate_sentiment,
            "sentiment_distribution": sentiment_counts,
            "sample_size": len(relevant_items),
            "avg_confidence": sum(confidences) / len(confidences),
            "score_range": (min(scores), max(scores)),
            "ticker": ticker
        }

    def detect_sentiment_shift(
        self,
        ticker: str,
        threshold: float = 0.5
    ) -> Dict[str, Any]:
        """
        Detect sudden sentiment shifts.
        Critical for: Breaking news detection, risk management.

        Args:
            ticker: Stock ticker
            threshold: Minimum score change to flag

        Returns:
            Shift detection results
        """
        relevant_items = [
            item for item in self.sentiment_window
            if item.get("ticker") == ticker
        ]

        if len(relevant_items) < 2:
            return {"shift_detected": False}

        # Compare recent vs historical
        recent = relevant_items[-5:]
        historical = relevant_items[-20:-5] if len(relevant_items) > 5 else []

        if not historical:
            return {"shift_detected": False}

        recent_avg = sum(item["score"] for item in recent) / len(recent)
        historical_avg = sum(item["score"] for item in historical) / len(historical)

        shift_magnitude = recent_avg - historical_avg

        return {
            "shift_detected": abs(shift_magnitude) > threshold,
            "shift_magnitude": shift_magnitude,
            "shift_direction": "POSITIVE" if shift_magnitude > 0 else "NEGATIVE",
            "recent_sentiment": recent_avg,
            "historical_sentiment": historical_avg,
            "confidence": sum(item["confidence"] for item in recent) / len(recent),
            "ticker": ticker,
            "timestamp": datetime.now().isoformat()
        }


class HFTSentimentSignalGenerator:
    """
    Convert sentiment analysis to trading signals.
    """

    def __init__(self):
        self.analyzer = RealTimeSentimentAnalyzer()

    def generate_signal(
        self,
        ticker: str,
        news_items: List[str],
        current_position: float = 0.0
    ) -> Dict[str, Any]:
        """
        Generate trading signal from news sentiment.

        Args:
            ticker: Stock ticker
            news_items: Recent news items
            current_position: Current position in shares

        Returns:
            Trading signal
        """
        # Analyze all news items
        sentiments = []
        for news in news_items:
            sentiment = self.analyzer.analyze_single_item(news, ticker)
            sentiments.append(sentiment)

        # Get aggregate sentiment
        aggregate = self.analyzer.get_aggregate_sentiment(ticker)

        # Detect sentiment shift
        shift = self.analyzer.detect_sentiment_shift(ticker)

        # Generate signal logic
        score = aggregate["aggregate_score"]
        confidence = aggregate["avg_confidence"]

        # Signal strength based on score and confidence
        signal_strength = abs(score) * confidence

        # Determine action
        if shift["shift_detected"] and shift["shift_magnitude"] > 0.5:
            # Strong positive shift
            action = "BUY"
            size = min(1000, int(signal_strength * 2000))
            urgency = "HIGH"
        elif shift["shift_detected"] and shift["shift_magnitude"] < -0.5:
            # Strong negative shift
            action = "SELL"
            size = min(abs(current_position), int(signal_strength * 2000))
            urgency = "HIGH"
        elif score > 0.5 and confidence > 0.7:
            # Strong bullish sentiment
            action = "BUY"
            size = int(signal_strength * 1000)
            urgency = "MEDIUM"
        elif score < -0.5 and confidence > 0.7:
            # Strong bearish sentiment
            action = "SELL"
            size = min(abs(current_position), int(signal_strength * 1000))
            urgency = "MEDIUM"
        else:
            # No clear signal
            action = "HOLD"
            size = 0
            urgency = "LOW"

        return {
            "ticker": ticker,
            "action": action,
            "size": size,
            "urgency": urgency,
            "signal_strength": signal_strength,
            "aggregate_sentiment": aggregate,
            "sentiment_shift": shift,
            "individual_sentiments": sentiments,
            "timestamp": datetime.now().isoformat(),
            "reasoning": self._generate_reasoning(score, confidence, shift)
        }

    def _generate_reasoning(
        self,
        score: float,
        confidence: float,
        shift: Dict[str, Any]
    ) -> str:
        """Generate human-readable reasoning."""
        reasoning_parts = []

        if shift["shift_detected"]:
            reasoning_parts.append(
                f"Sentiment shift detected: {shift['shift_direction']} "
                f"({shift['shift_magnitude']:.2f})"
            )

        sentiment_desc = "bullish" if score > 0 else "bearish" if score < 0 else "neutral"
        reasoning_parts.append(
            f"Overall sentiment: {sentiment_desc} (score: {score:.2f}, "
            f"confidence: {confidence:.2f})"
        )

        return ". ".join(reasoning_parts)


def main():
    """Run sentiment analysis demonstrations."""

    print("HFT Sentiment Analysis Examples")
    print("="*80)

    analyzer = RealTimeSentimentAnalyzer()

    # Example 1: Single news item analysis
    print("\nExample 1: Real-time News Sentiment")
    print("="*80)

    news_items = [
        ("Tesla announces record quarterly deliveries, beating expectations by 15%", "TSLA"),
        ("Federal Reserve signals aggressive rate hikes amid inflation concerns", "SPY"),
        ("Apple's China sales decline 8% year-over-year on weak demand", "AAPL"),
        ("NVIDIA announces new AI chip, stock surges in after-hours trading", "NVDA"),
    ]

    for text, ticker in news_items:
        if os.getenv("OPENAI_API_KEY") or os.getenv("ANTHROPIC_API_KEY"):
            try:
                result = analyzer.analyze_single_item(text, ticker)
                print(f"\n{ticker}: {text[:60]}...")
                print(f"  Sentiment: {result['sentiment']} (score: {result['score']:.2f})")
                print(f"  Confidence: {result['confidence']:.2f}")
                print(f"  Urgency: {result['urgency']}")
                print(f"  Key factors: {', '.join(result['key_factors'])}")
            except Exception as e:
                print(f"  Error: {e}")

    # Example 2: Aggregate sentiment
    print("\n\nExample 2: Aggregate Sentiment Analysis")
    print("="*80)

    if os.getenv("OPENAI_API_KEY") or os.getenv("ANTHROPIC_API_KEY"):
        try:
            aggregate = analyzer.get_aggregate_sentiment("TSLA")
            print(f"\nAggregate sentiment for TSLA:")
            print(f"  Score: {aggregate['aggregate_score']:.2f}")
            print(f"  Sentiment: {aggregate['aggregate_sentiment']}")
            print(f"  Distribution: {aggregate['sentiment_distribution']}")
            print(f"  Sample size: {aggregate['sample_size']}")
        except Exception as e:
            print(f"Error: {e}")

    # Example 3: Trading signal generation
    print("\n\nExample 3: Trading Signal Generation")
    print("="*80)

    signal_gen = HFTSentimentSignalGenerator()

    tsla_news = [
        "Tesla Q3 deliveries smash estimates, production ramps up",
        "Elon Musk tweets about new manufacturing breakthrough",
        "Tesla stock upgraded by major investment bank",
    ]

    if os.getenv("OPENAI_API_KEY") or os.getenv("ANTHROPIC_API_KEY"):
        try:
            signal = signal_gen.generate_signal("TSLA", tsla_news, current_position=0)
            print(f"\nTrading Signal for TSLA:")
            print(f"  Action: {signal['action']}")
            print(f"  Size: {signal['size']} shares")
            print(f"  Urgency: {signal['urgency']}")
            print(f"  Signal Strength: {signal['signal_strength']:.2f}")
            print(f"  Reasoning: {signal['reasoning']}")
        except Exception as e:
            print(f"Error: {e}")

    # Example 4: Async batch processing
    print("\n\nExample 4: High-Volume Batch Processing")
    print("="*80)

    async def batch_example():
        batch_items = [
            ("Fed minutes show divided opinions on rate path", "SPY"),
            ("Tech stocks rally on strong earnings", "QQQ"),
            ("Oil prices spike on OPEC production cuts", "USO"),
            ("Dollar weakens against major currencies", "UUP"),
            ("Gold hits 6-month high on inflation hedge demand", "GLD"),
        ] * 4  # 20 items total

        print(f"\nProcessing {len(batch_items)} news items concurrently...")
        start_time = datetime.now()

        try:
            results = await analyzer.analyze_batch_async(batch_items)
            end_time = datetime.now()

            print(f"Processed in {(end_time - start_time).total_seconds():.2f} seconds")
            print(f"Average: {(end_time - start_time).total_seconds() / len(batch_items):.3f}s per item")

            # Show sample results
            print("\nSample results:")
            for result in results[:3]:
                print(f"  {result['ticker']}: {result['sentiment']} ({result['score']:.2f})")
        except Exception as e:
            print(f"Error: {e}")

    if os.getenv("OPENAI_API_KEY") or os.getenv("ANTHROPIC_API_KEY"):
        try:
            asyncio.run(batch_example())
        except Exception as e:
            print(f"Error: {e}")


if __name__ == "__main__":
    main()
