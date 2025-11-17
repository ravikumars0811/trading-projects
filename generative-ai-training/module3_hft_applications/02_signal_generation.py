"""
Module 3.2: AI-Powered Trading Signal Generation

Use LLMs to generate and validate trading signals from multiple data sources.
Combines: technical analysis, fundamental data, news sentiment, market context.
"""

import openai
import os
from typing import List, Dict, Any, Optional
from datetime import datetime, timedelta
import json
from dataclasses import dataclass, asdict
from dotenv import load_dotenv

load_dotenv()


@dataclass
class MarketData:
    """Market data structure."""
    ticker: str
    price: float
    volume: int
    change_pct: float
    rsi: Optional[float] = None
    macd: Optional[float] = None
    avg_volume: Optional[int] = None
    volatility: Optional[float] = None


@dataclass
class TradingSignal:
    """Trading signal structure."""
    ticker: str
    action: str  # BUY, SELL, HOLD
    confidence: float  # 0-100
    size: int  # Position size
    entry_price: float
    stop_loss: float
    take_profit: float
    time_horizon: str  # SCALP, INTRADAY, SWING
    reasoning: str
    risk_reward: float
    timestamp: str


class AISignalGenerator:
    """
    Generate trading signals using AI analysis.
    """

    def __init__(self, model: str = "gpt-4-turbo-preview"):
        """Initialize signal generator."""
        self.client = openai.OpenAI(api_key=os.getenv("OPENAI_API_KEY"))
        self.model = model

    def generate_signal(
        self,
        market_data: MarketData,
        news_summary: str,
        technical_analysis: Dict[str, Any],
        risk_params: Dict[str, float]
    ) -> TradingSignal:
        """
        Generate comprehensive trading signal.

        Args:
            market_data: Current market data
            news_summary: Recent news summary
            technical_analysis: Technical indicators
            risk_params: Risk management parameters

        Returns:
            Trading signal with full details
        """
        # Construct comprehensive prompt
        prompt = self._build_signal_prompt(
            market_data,
            news_summary,
            technical_analysis,
            risk_params
        )

        # Get AI analysis
        response = self.client.chat.completions.create(
            model=self.model,
            messages=[
                {
                    "role": "system",
                    "content": """You are an expert quantitative trader with 20+ years of experience.
                    Generate trading signals based on comprehensive market analysis.
                    Always consider risk management and proper position sizing.
                    Respond with valid JSON only."""
                },
                {"role": "user", "content": prompt}
            ],
            temperature=0.3,
            response_format={"type": "json_object"}
        )

        # Parse response
        signal_data = json.loads(response.choices[0].message.content)

        # Create signal object
        return TradingSignal(
            ticker=market_data.ticker,
            action=signal_data.get("action", "HOLD"),
            confidence=signal_data.get("confidence", 50),
            size=signal_data.get("size", 0),
            entry_price=signal_data.get("entry_price", market_data.price),
            stop_loss=signal_data.get("stop_loss", market_data.price * 0.98),
            take_profit=signal_data.get("take_profit", market_data.price * 1.02),
            time_horizon=signal_data.get("time_horizon", "INTRADAY"),
            reasoning=signal_data.get("reasoning", "No reasoning provided"),
            risk_reward=signal_data.get("risk_reward", 1.0),
            timestamp=datetime.now().isoformat()
        )

    def _build_signal_prompt(
        self,
        market_data: MarketData,
        news_summary: str,
        technical_analysis: Dict[str, Any],
        risk_params: Dict[str, float]
    ) -> str:
        """Build comprehensive signal generation prompt."""
        return f"""
Generate a trading signal for {market_data.ticker} based on this comprehensive analysis:

CURRENT MARKET DATA:
- Price: ${market_data.price}
- Change: {market_data.change_pct:+.2f}%
- Volume: {market_data.volume:,} (Avg: {market_data.avg_volume:,})
- RSI: {market_data.rsi}
- MACD: {market_data.macd}
- Volatility: {market_data.volatility}%

NEWS SUMMARY:
{news_summary}

TECHNICAL ANALYSIS:
{json.dumps(technical_analysis, indent=2)}

RISK PARAMETERS:
- Max Position Size: ${risk_params.get('max_position_value', 100000)}
- Max Risk per Trade: {risk_params.get('max_risk_pct', 2)}%
- Required Risk/Reward: {risk_params.get('min_risk_reward', 2)}:1

Generate a trading signal with the following JSON structure:
{{
    "action": "BUY" or "SELL" or "HOLD",
    "confidence": 0-100,
    "size": position_size_in_shares,
    "entry_price": recommended_entry_price,
    "stop_loss": stop_loss_price,
    "take_profit": take_profit_price,
    "time_horizon": "SCALP" or "INTRADAY" or "SWING",
    "reasoning": "detailed_explanation",
    "risk_reward": risk_to_reward_ratio,
    "key_factors": ["factor1", "factor2", "factor3"]
}}

Consider:
1. Technical momentum and trend
2. News sentiment and catalysts
3. Risk management and position sizing
4. Market conditions and volatility
5. Volume confirmation
"""

    def validate_signal(self, signal: TradingSignal) -> Dict[str, Any]:
        """
        Validate signal against risk management rules.

        Args:
            signal: Trading signal to validate

        Returns:
            Validation results
        """
        issues = []
        warnings = []

        # Check risk/reward ratio
        if signal.risk_reward < 1.5:
            issues.append(f"Risk/reward ratio too low: {signal.risk_reward:.2f}")

        # Check stop loss placement
        stop_distance = abs(signal.entry_price - signal.stop_loss) / signal.entry_price
        if stop_distance > 0.05:  # 5% max stop
            warnings.append(f"Stop loss too wide: {stop_distance*100:.1f}%")

        # Check confidence threshold
        if signal.confidence < 60:
            warnings.append(f"Low confidence: {signal.confidence}%")

        # Check position size reasonableness
        position_value = signal.size * signal.entry_price
        if position_value > 100000:
            warnings.append(f"Large position: ${position_value:,.0f}")

        return {
            "valid": len(issues) == 0,
            "issues": issues,
            "warnings": warnings,
            "signal": asdict(signal)
        }


class MultiSourceSignalAggregator:
    """
    Aggregate signals from multiple AI models and strategies.
    Reduces model bias and improves reliability.
    """

    def __init__(self):
        """Initialize aggregator."""
        self.generators = [
            AISignalGenerator(model="gpt-4-turbo-preview"),
            AISignalGenerator(model="gpt-3.5-turbo"),
        ]

    def generate_ensemble_signal(
        self,
        market_data: MarketData,
        news_summary: str,
        technical_analysis: Dict[str, Any],
        risk_params: Dict[str, float]
    ) -> Dict[str, Any]:
        """
        Generate signal using ensemble of models.

        Args:
            market_data: Market data
            news_summary: News summary
            technical_analysis: Technical indicators
            risk_params: Risk parameters

        Returns:
            Ensemble signal with confidence
        """
        signals = []

        # Generate signals from multiple sources
        for generator in self.generators:
            try:
                signal = generator.generate_signal(
                    market_data,
                    news_summary,
                    technical_analysis,
                    risk_params
                )
                signals.append(signal)
            except Exception as e:
                print(f"Generator error: {e}")

        if not signals:
            return {
                "action": "HOLD",
                "confidence": 0,
                "reasoning": "No valid signals generated"
            }

        # Aggregate signals
        actions = [s.action for s in signals]
        confidences = [s.confidence for s in signals]

        # Majority vote for action
        action_votes = {
            "BUY": actions.count("BUY"),
            "SELL": actions.count("SELL"),
            "HOLD": actions.count("HOLD")
        }

        ensemble_action = max(action_votes, key=action_votes.get)

        # Average confidence of agreeing signals
        agreeing_confidences = [
            s.confidence for s in signals if s.action == ensemble_action
        ]
        ensemble_confidence = sum(agreeing_confidences) / len(agreeing_confidences)

        # Consensus strength
        consensus_strength = action_votes[ensemble_action] / len(signals)

        return {
            "action": ensemble_action,
            "confidence": ensemble_confidence,
            "consensus_strength": consensus_strength,
            "individual_signals": [asdict(s) for s in signals],
            "action_distribution": action_votes,
            "timestamp": datetime.now().isoformat()
        }


class SignalBacktester:
    """
    Backtest and validate AI-generated signals.
    """

    def __init__(self):
        """Initialize backtester."""
        self.signal_history = []

    def evaluate_signal(
        self,
        signal: TradingSignal,
        actual_prices: Dict[str, float]
    ) -> Dict[str, Any]:
        """
        Evaluate signal performance against actual prices.

        Args:
            signal: Generated signal
            actual_prices: Actual prices at different times

        Returns:
            Performance metrics
        """
        entry_price = signal.entry_price
        stop_loss = signal.stop_loss
        take_profit = signal.take_profit

        # Check if stop loss or take profit was hit
        hit_stop_loss = any(
            price <= stop_loss if signal.action == "BUY" else price >= stop_loss
            for price in actual_prices.values()
        )

        hit_take_profit = any(
            price >= take_profit if signal.action == "BUY" else price <= take_profit
            for price in actual_prices.values()
        )

        # Calculate P&L
        if hit_stop_loss:
            exit_price = stop_loss
            outcome = "LOSS"
        elif hit_take_profit:
            exit_price = take_profit
            outcome = "WIN"
        else:
            # Use final price if still open
            exit_price = list(actual_prices.values())[-1]
            outcome = "OPEN"

        if signal.action == "BUY":
            pnl = (exit_price - entry_price) * signal.size
            pnl_pct = (exit_price - entry_price) / entry_price * 100
        elif signal.action == "SELL":
            pnl = (entry_price - exit_price) * signal.size
            pnl_pct = (entry_price - exit_price) / entry_price * 100
        else:
            pnl = 0
            pnl_pct = 0
            outcome = "NO_TRADE"

        return {
            "signal_id": signal.timestamp,
            "ticker": signal.ticker,
            "action": signal.action,
            "outcome": outcome,
            "entry_price": entry_price,
            "exit_price": exit_price,
            "pnl": pnl,
            "pnl_pct": pnl_pct,
            "hit_stop_loss": hit_stop_loss,
            "hit_take_profit": hit_take_profit,
            "confidence": signal.confidence
        }


def main():
    """Run signal generation demonstrations."""

    print("AI-Powered Trading Signal Generation")
    print("="*80)

    if not os.getenv("OPENAI_API_KEY"):
        print("Warning: OPENAI_API_KEY not set. Examples will be limited.")
        return

    # Example 1: Single signal generation
    print("\nExample 1: Comprehensive Signal Generation")
    print("="*80)

    # Sample market data
    market_data = MarketData(
        ticker="TSLA",
        price=242.50,
        volume=125000000,
        change_pct=3.2,
        rsi=68.5,
        macd=2.1,
        avg_volume=95000000,
        volatility=45.2
    )

    news_summary = """
    Tesla reports Q3 deliveries of 435,000 vehicles, beating estimates.
    Production efficiency improving, new factory in Mexico announced.
    Price cuts in China stabilizing demand. Cybertruck launch next month.
    """

    technical_analysis = {
        "trend": "UPTREND",
        "support_levels": [235.00, 228.50, 220.00],
        "resistance_levels": [250.00, 258.00, 265.00],
        "moving_averages": {
            "sma_20": 238.50,
            "sma_50": 232.00,
            "sma_200": 215.00
        },
        "volume_profile": "ABOVE_AVERAGE",
        "momentum": "STRONG_BULLISH"
    }

    risk_params = {
        "max_position_value": 50000,
        "max_risk_pct": 2,
        "min_risk_reward": 2.0
    }

    try:
        generator = AISignalGenerator()
        signal = generator.generate_signal(
            market_data,
            news_summary,
            technical_analysis,
            risk_params
        )

        print(f"\nGenerated Signal for {signal.ticker}:")
        print(f"  Action: {signal.action}")
        print(f"  Confidence: {signal.confidence}%")
        print(f"  Size: {signal.size} shares")
        print(f"  Entry: ${signal.entry_price:.2f}")
        print(f"  Stop Loss: ${signal.stop_loss:.2f}")
        print(f"  Take Profit: ${signal.take_profit:.2f}")
        print(f"  Risk/Reward: {signal.risk_reward:.2f}")
        print(f"  Time Horizon: {signal.time_horizon}")
        print(f"\nReasoning: {signal.reasoning}")

        # Validate signal
        validation = generator.validate_signal(signal)
        print(f"\nValidation:")
        print(f"  Valid: {validation['valid']}")
        if validation['issues']:
            print(f"  Issues: {validation['issues']}")
        if validation['warnings']:
            print(f"  Warnings: {validation['warnings']}")

    except Exception as e:
        print(f"Error: {e}")

    # Example 2: Ensemble signal generation
    print("\n\nExample 2: Ensemble Signal Generation")
    print("="*80)

    try:
        aggregator = MultiSourceSignalAggregator()
        ensemble_signal = aggregator.generate_ensemble_signal(
            market_data,
            news_summary,
            technical_analysis,
            risk_params
        )

        print(f"\nEnsemble Signal:")
        print(f"  Action: {ensemble_signal['action']}")
        print(f"  Confidence: {ensemble_signal['confidence']:.1f}%")
        print(f"  Consensus Strength: {ensemble_signal['consensus_strength']*100:.0f}%")
        print(f"  Action Distribution: {ensemble_signal['action_distribution']}")
        print(f"\nIndividual Signals: {len(ensemble_signal['individual_signals'])} models")

    except Exception as e:
        print(f"Error: {e}")

    # Example 3: Signal backtesting
    print("\n\nExample 3: Signal Performance Evaluation")
    print("="*80)

    # Simulate actual price movement
    actual_prices = {
        "t+1min": 243.20,
        "t+5min": 245.80,
        "t+15min": 248.50,
        "t+30min": 252.10,  # Hit take profit
        "t+1hr": 251.50,
    }

    try:
        backtester = SignalBacktester()
        if 'signal' in locals():
            performance = backtester.evaluate_signal(signal, actual_prices)

            print(f"\nSignal Performance:")
            print(f"  Outcome: {performance['outcome']}")
            print(f"  Entry: ${performance['entry_price']:.2f}")
            print(f"  Exit: ${performance['exit_price']:.2f}")
            print(f"  P&L: ${performance['pnl']:,.2f} ({performance['pnl_pct']:+.2f}%)")
            print(f"  Hit Take Profit: {performance['hit_take_profit']}")
            print(f"  Hit Stop Loss: {performance['hit_stop_loss']}")

    except Exception as e:
        print(f"Error: {e}")


if __name__ == "__main__":
    main()
