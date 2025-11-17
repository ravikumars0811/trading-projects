"""
Real-World Example: Complete HFT System with AI Integration

This example shows how to integrate AI into a production HFT system for:
1. Real-time news sentiment analysis
2. Trading signal generation
3. Risk assessment
4. Trade explanation and reporting

Performance targets:
- News processing: <100ms latency
- Signal generation: <200ms latency
- API cost: <$0.01 per decision
"""

import asyncio
import os
from datetime import datetime
from typing import List, Dict, Any, Optional
from dataclasses import dataclass, field
import json
from collections import deque
import time

import openai
import anthropic
from dotenv import load_dotenv

load_dotenv()


@dataclass
class MarketEvent:
    """Market event data."""
    timestamp: datetime
    event_type: str  # NEWS, PRICE, VOLUME, ORDER_BOOK
    ticker: str
    data: Dict[str, Any]
    priority: int = 1  # 1=critical, 2=high, 3=normal


@dataclass
class AIDecision:
    """AI-generated trading decision."""
    ticker: str
    action: str  # BUY, SELL, HOLD
    size: int
    confidence: float
    reasoning: str
    latency_ms: float
    timestamp: datetime


class OptimizedAIEngine:
    """
    Production-optimized AI engine for HFT.

    Optimizations:
    - Caching frequent queries
    - Batch processing non-urgent items
    - Model selection based on latency requirements
    - Async operations
    - Cost tracking
    """

    def __init__(self):
        """Initialize AI engine."""
        # Use faster models for time-critical operations
        self.openai_client = openai.OpenAI(api_key=os.getenv("OPENAI_API_KEY"))
        self.claude_client = anthropic.Anthropic(api_key=os.getenv("ANTHROPIC_API_KEY"))

        # Model selection by priority
        self.models = {
            1: "gpt-3.5-turbo",  # Critical: fastest
            2: "gpt-4-turbo-preview",  # High: balanced
            3: "claude-3-5-sonnet-20241022"  # Normal: best quality
        }

        # Caching
        self.cache = {}
        self.cache_ttl = 60  # seconds

        # Performance tracking
        self.stats = {
            "total_requests": 0,
            "cache_hits": 0,
            "total_latency": 0,
            "total_cost": 0
        }

        # Batch queue for non-urgent items
        self.batch_queue = deque(maxlen=100)

    async def analyze_news(
        self,
        news_text: str,
        ticker: str,
        priority: int = 2
    ) -> Dict[str, Any]:
        """
        Analyze news with latency optimization.

        Args:
            news_text: News content
            ticker: Stock ticker
            priority: 1=critical, 2=high, 3=normal

        Returns:
            Sentiment analysis
        """
        start_time = time.time()

        # Check cache
        cache_key = f"{ticker}:{hash(news_text)}"
        if cache_key in self.cache:
            cached_data, cache_time = self.cache[cache_key]
            if time.time() - cache_time < self.cache_ttl:
                self.stats["cache_hits"] += 1
                cached_data["from_cache"] = True
                cached_data["latency_ms"] = (time.time() - start_time) * 1000
                return cached_data

        # Select model based on priority
        if priority == 1:
            # Critical: use fastest model
            result = await self._analyze_with_gpt35(news_text, ticker)
        elif priority == 2:
            # High priority: use GPT-4 Turbo
            result = await self._analyze_with_gpt4(news_text, ticker)
        else:
            # Normal: use Claude for best quality
            result = await self._analyze_with_claude(news_text, ticker)

        # Calculate latency
        latency_ms = (time.time() - start_time) * 1000
        result["latency_ms"] = latency_ms

        # Update stats
        self.stats["total_requests"] += 1
        self.stats["total_latency"] += latency_ms

        # Cache result
        self.cache[cache_key] = (result, time.time())

        return result

    async def _analyze_with_gpt35(
        self,
        news_text: str,
        ticker: str
    ) -> Dict[str, Any]:
        """Fast sentiment analysis with GPT-3.5."""
        prompt = f"""Analyze sentiment for {ticker}: {news_text}

Respond with JSON only:
{{"sentiment": "BULLISH/BEARISH/NEUTRAL", "score": -1 to 1, "confidence": 0-1}}"""

        try:
            response = self.openai_client.chat.completions.create(
                model="gpt-3.5-turbo",
                messages=[{"role": "user", "content": prompt}],
                temperature=0.3,
                max_tokens=100,
                response_format={"type": "json_object"}
            )

            result = json.loads(response.choices[0].message.content)
            result["model"] = "gpt-3.5-turbo"
            result["cost"] = 0.0015 * (len(prompt) / 1000)  # Rough estimate

            return result
        except Exception as e:
            return {
                "sentiment": "NEUTRAL",
                "score": 0,
                "confidence": 0,
                "error": str(e)
            }

    async def _analyze_with_gpt4(
        self,
        news_text: str,
        ticker: str
    ) -> Dict[str, Any]:
        """Balanced analysis with GPT-4."""
        prompt = f"""Analyze this news for {ticker} trading:

{news_text}

Provide JSON:
{{
    "sentiment": "BULLISH/BEARISH/NEUTRAL",
    "score": -1.0 to 1.0,
    "confidence": 0.0 to 1.0,
    "key_factors": ["factor1", "factor2"],
    "urgency": "LOW/MEDIUM/HIGH",
    "price_impact_expected": "percentage estimate"
}}"""

        try:
            response = self.openai_client.chat.completions.create(
                model="gpt-4-turbo-preview",
                messages=[{"role": "user", "content": prompt}],
                temperature=0.3,
                max_tokens=200,
                response_format={"type": "json_object"}
            )

            result = json.loads(response.choices[0].message.content)
            result["model"] = "gpt-4-turbo"
            result["cost"] = 0.01 * (len(prompt) / 1000)

            return result
        except Exception as e:
            return {
                "sentiment": "NEUTRAL",
                "score": 0,
                "confidence": 0,
                "error": str(e)
            }

    async def _analyze_with_claude(
        self,
        news_text: str,
        ticker: str
    ) -> Dict[str, Any]:
        """High-quality analysis with Claude."""
        prompt = f"""Analyze this market news for {ticker}:

{news_text}

Provide detailed JSON with sentiment, score, confidence, key factors, and expected market impact."""

        try:
            response = self.claude_client.messages.create(
                model="claude-3-5-sonnet-20241022",
                max_tokens=300,
                temperature=0.3,
                messages=[{"role": "user", "content": prompt}]
            )

            result_text = response.content[0].text
            if "```json" in result_text:
                result_text = result_text.split("```json")[1].split("```")[0].strip()

            result = json.loads(result_text)
            result["model"] = "claude-3-sonnet"
            result["cost"] = 0.003 * (len(prompt) / 1000)

            return result
        except Exception as e:
            return {
                "sentiment": "NEUTRAL",
                "score": 0,
                "confidence": 0,
                "error": str(e)
            }

    async def generate_trading_decision(
        self,
        ticker: str,
        market_data: Dict[str, Any],
        sentiment_analysis: Dict[str, Any],
        current_position: int = 0
    ) -> AIDecision:
        """
        Generate trading decision from analysis.

        Args:
            ticker: Stock ticker
            market_data: Current market data
            sentiment_analysis: AI sentiment analysis
            current_position: Current position size

        Returns:
            Trading decision
        """
        start_time = time.time()

        # Simple decision logic based on sentiment
        score = sentiment_analysis.get("score", 0)
        confidence = sentiment_analysis.get("confidence", 0)

        # Decision thresholds
        if score > 0.5 and confidence > 0.7:
            action = "BUY"
            size = min(1000, int(confidence * 1500))
        elif score < -0.5 and confidence > 0.7:
            action = "SELL"
            size = min(abs(current_position), int(confidence * 1500))
        else:
            action = "HOLD"
            size = 0

        reasoning = (
            f"Sentiment: {sentiment_analysis.get('sentiment', 'NEUTRAL')} "
            f"(score: {score:.2f}, confidence: {confidence:.2f})"
        )

        latency_ms = (time.time() - start_time) * 1000

        return AIDecision(
            ticker=ticker,
            action=action,
            size=size,
            confidence=confidence,
            reasoning=reasoning,
            latency_ms=latency_ms,
            timestamp=datetime.now()
        )

    def get_performance_stats(self) -> Dict[str, Any]:
        """Get AI engine performance statistics."""
        total_requests = self.stats["total_requests"]
        if total_requests == 0:
            return {"message": "No requests processed yet"}

        return {
            "total_requests": total_requests,
            "cache_hit_rate": f"{(self.stats['cache_hits'] / total_requests * 100):.1f}%",
            "avg_latency_ms": self.stats["total_latency"] / total_requests,
            "total_cost": f"${self.stats['total_cost']:.4f}",
            "cost_per_request": f"${self.stats['total_cost'] / total_requests:.6f}"
        }


class HFTAITradingSystem:
    """
    Complete HFT system with AI integration.
    """

    def __init__(self):
        """Initialize trading system."""
        self.ai_engine = OptimizedAIEngine()
        self.positions = {}  # {ticker: size}
        self.decisions_history = []

    async def process_market_event(self, event: MarketEvent) -> Optional[AIDecision]:
        """
        Process market event and generate trading decision.

        Args:
            event: Market event

        Returns:
            Trading decision if generated
        """
        if event.event_type == "NEWS":
            # Analyze news sentiment
            sentiment = await self.ai_engine.analyze_news(
                news_text=event.data.get("text", ""),
                ticker=event.ticker,
                priority=event.priority
            )

            # Generate trading decision
            decision = await self.ai_engine.generate_trading_decision(
                ticker=event.ticker,
                market_data=event.data,
                sentiment_analysis=sentiment,
                current_position=self.positions.get(event.ticker, 0)
            )

            # Store decision
            self.decisions_history.append(decision)

            # Update positions (simulated)
            if decision.action == "BUY":
                self.positions[event.ticker] = (
                    self.positions.get(event.ticker, 0) + decision.size
                )
            elif decision.action == "SELL":
                self.positions[event.ticker] = (
                    self.positions.get(event.ticker, 0) - decision.size
                )

            return decision

        return None

    async def run_simulation(self, events: List[MarketEvent]):
        """
        Run simulation with market events.

        Args:
            events: List of market events
        """
        print(f"Processing {len(events)} market events...\n")

        for event in events:
            decision = await self.process_market_event(event)

            if decision:
                print(f"[{decision.timestamp.strftime('%H:%M:%S')}] {decision.ticker}")
                print(f"  Action: {decision.action} {decision.size} shares")
                print(f"  Confidence: {decision.confidence:.2f}")
                print(f"  Reasoning: {decision.reasoning}")
                print(f"  Latency: {decision.latency_ms:.1f}ms")
                print()

        # Print performance stats
        print("\n" + "="*80)
        print("AI Engine Performance Statistics")
        print("="*80)
        stats = self.ai_engine.get_performance_stats()
        for key, value in stats.items():
            print(f"{key}: {value}")


async def main():
    """Run complete HFT AI integration example."""

    print("="*80)
    print("HFT System with AI Integration - Real-World Example")
    print("="*80)
    print()

    # Check API keys
    if not (os.getenv("OPENAI_API_KEY") or os.getenv("ANTHROPIC_API_KEY")):
        print("Warning: No API keys configured.")
        print("Set OPENAI_API_KEY or ANTHROPIC_API_KEY to run this example.")
        return

    # Create sample market events
    events = [
        MarketEvent(
            timestamp=datetime.now(),
            event_type="NEWS",
            ticker="TSLA",
            data={
                "text": "Tesla announces record Q3 deliveries of 435,000 vehicles, beating analyst estimates",
                "price": 242.50
            },
            priority=1  # Critical
        ),
        MarketEvent(
            timestamp=datetime.now(),
            event_type="NEWS",
            ticker="AAPL",
            data={
                "text": "Apple suppliers report weaker iPhone 15 demand in China",
                "price": 178.20
            },
            priority=2  # High
        ),
        MarketEvent(
            timestamp=datetime.now(),
            event_type="NEWS",
            ticker="NVDA",
            data={
                "text": "NVIDIA CEO discusses new AI chip architecture at conference",
                "price": 475.30
            },
            priority=3  # Normal
        ),
        MarketEvent(
            timestamp=datetime.now(),
            event_type="NEWS",
            ticker="SPY",
            data={
                "text": "Federal Reserve signals potential pause in rate hikes",
                "price": 445.80
            },
            priority=1  # Critical
        ),
    ]

    # Initialize and run trading system
    system = HFTAITradingSystem()
    await system.run_simulation(events)

    # Show final positions
    print("\n" + "="*80)
    print("Final Positions")
    print("="*80)
    for ticker, size in system.positions.items():
        print(f"{ticker}: {size:+,} shares")


if __name__ == "__main__":
    asyncio.run(main())
