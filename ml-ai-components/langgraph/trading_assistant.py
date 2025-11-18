"""
LangGraph: Multi-Agent Trading Assistant
=========================================
Real-world example: Coordinated AI agents for trading research and analysis

Industry Use Case: Investment firms use multi-agent systems for:
- Comprehensive market analysis with specialized agents
- Automated research workflows
- Risk assessment and portfolio management
- Trading signal generation with multiple perspectives
"""

from typing import Dict, List, Any, TypedDict
from datetime import datetime
import random
import json


class AgentState(TypedDict):
    """Shared state between agents"""
    query: str
    symbol: str
    market_data: Dict[str, Any]
    technical_analysis: Dict[str, Any]
    fundamental_analysis: Dict[str, Any]
    sentiment_analysis: Dict[str, Any]
    risk_assessment: Dict[str, Any]
    final_recommendation: Dict[str, Any]
    agent_history: List[str]


class BaseAgent:
    """Base class for specialized agents"""

    def __init__(self, name: str):
        self.name = name

    def log_action(self, state: AgentState, action: str):
        """Log agent action to history"""
        state['agent_history'].append(f"[{self.name}] {action}")


class MarketDataAgent(BaseAgent):
    """Agent responsible for fetching market data"""

    def __init__(self):
        super().__init__("Market Data Agent")

    def execute(self, state: AgentState) -> AgentState:
        """Fetch and process market data"""
        symbol = state['symbol']
        self.log_action(state, f"Fetching market data for {symbol}")

        # Simulate market data retrieval
        base_prices = {
            "AAPL": 180.0, "GOOGL": 140.0, "MSFT": 380.0,
            "TSLA": 250.0, "AMZN": 150.0, "NVDA": 500.0
        }

        base_price = base_prices.get(symbol, 100.0)
        current_price = base_price * (1 + random.uniform(-0.02, 0.02))

        market_data = {
            'symbol': symbol,
            'current_price': round(current_price, 2),
            'volume': random.randint(10000000, 100000000),
            'day_change': round(random.uniform(-3, 3), 2),
            'day_high': round(current_price * 1.03, 2),
            'day_low': round(current_price * 0.97, 2),
            'market_cap': round(current_price * 16000000000, 2),  # Mock market cap
            'pe_ratio': round(random.uniform(15, 40), 2),
            'timestamp': datetime.now().isoformat()
        }

        state['market_data'] = market_data
        self.log_action(state, f"Retrieved market data: ${market_data['current_price']}")

        return state


class TechnicalAnalysisAgent(BaseAgent):
    """Agent for technical analysis"""

    def __init__(self):
        super().__init__("Technical Analysis Agent")

    def execute(self, state: AgentState) -> AgentState:
        """Perform technical analysis"""
        self.log_action(state, "Performing technical analysis")

        if not state.get('market_data'):
            self.log_action(state, "ERROR: No market data available")
            return state

        current_price = state['market_data']['current_price']

        # Simulate technical indicators
        technical_analysis = {
            'rsi': round(random.uniform(30, 70), 2),
            'macd': round(random.uniform(-5, 5), 2),
            'moving_averages': {
                'ma_50': round(current_price * random.uniform(0.95, 1.05), 2),
                'ma_200': round(current_price * random.uniform(0.90, 1.10), 2)
            },
            'bollinger_bands': {
                'upper': round(current_price * 1.1, 2),
                'middle': round(current_price, 2),
                'lower': round(current_price * 0.9, 2)
            },
            'support_level': round(current_price * 0.95, 2),
            'resistance_level': round(current_price * 1.05, 2),
            'trend': random.choice(['BULLISH', 'NEUTRAL', 'BEARISH'])
        }

        # Determine signal
        rsi = technical_analysis['rsi']
        if rsi < 30:
            signal = "OVERSOLD - Consider buying"
        elif rsi > 70:
            signal = "OVERBOUGHT - Consider selling"
        else:
            signal = "NEUTRAL - Hold position"

        technical_analysis['signal'] = signal

        state['technical_analysis'] = technical_analysis
        self.log_action(state, f"Technical signal: {signal}")

        return state


class FundamentalAnalysisAgent(BaseAgent):
    """Agent for fundamental analysis"""

    def __init__(self):
        super().__init__("Fundamental Analysis Agent")

    def execute(self, state: AgentState) -> AgentState:
        """Perform fundamental analysis"""
        self.log_action(state, "Performing fundamental analysis")

        symbol = state['symbol']

        # Simulate fundamental metrics
        fundamental_analysis = {
            'revenue_growth': round(random.uniform(-5, 30), 2),
            'earnings_growth': round(random.uniform(-10, 40), 2),
            'profit_margin': round(random.uniform(5, 30), 2),
            'roe': round(random.uniform(10, 25), 2),  # Return on Equity
            'debt_to_equity': round(random.uniform(0.2, 2.0), 2),
            'current_ratio': round(random.uniform(1.0, 3.0), 2),
            'free_cash_flow': random.randint(1000, 10000),  # millions
            'analyst_rating': round(random.uniform(3.5, 4.8), 1),  # out of 5
            'price_target': round(state['market_data']['current_price'] * random.uniform(0.9, 1.2), 2)
        }

        # Valuation assessment
        pe_ratio = state['market_data'].get('pe_ratio', 25)
        if pe_ratio < 15:
            valuation = "UNDERVALUED"
        elif pe_ratio > 30:
            valuation = "OVERVALUED"
        else:
            valuation = "FAIRLY VALUED"

        fundamental_analysis['valuation'] = valuation

        state['fundamental_analysis'] = fundamental_analysis
        self.log_action(state, f"Valuation: {valuation}")

        return state


class SentimentAnalysisAgent(BaseAgent):
    """Agent for sentiment analysis"""

    def __init__(self):
        super().__init__("Sentiment Analysis Agent")

    def execute(self, state: AgentState) -> AgentState:
        """Analyze market sentiment"""
        self.log_action(state, "Analyzing market sentiment")

        symbol = state['symbol']

        # Simulate sentiment from various sources
        sentiment_analysis = {
            'news_sentiment': random.choice(['POSITIVE', 'NEUTRAL', 'NEGATIVE']),
            'news_confidence': round(random.uniform(0.6, 0.95), 2),
            'social_media_sentiment': random.choice(['BULLISH', 'NEUTRAL', 'BEARISH']),
            'social_media_volume': random.randint(1000, 50000),
            'analyst_sentiment': random.choice(['STRONG_BUY', 'BUY', 'HOLD', 'SELL']),
            'insider_trading': random.choice(['BUYING', 'SELLING', 'NEUTRAL']),
            'institutional_flow': random.choice(['INFLOW', 'OUTFLOW', 'NEUTRAL']),
            'sentiment_score': round(random.uniform(-1, 1), 2)  # -1 to 1
        }

        # Overall sentiment
        score = sentiment_analysis['sentiment_score']
        if score > 0.3:
            overall = "BULLISH"
        elif score < -0.3:
            overall = "BEARISH"
        else:
            overall = "NEUTRAL"

        sentiment_analysis['overall_sentiment'] = overall

        state['sentiment_analysis'] = sentiment_analysis
        self.log_action(state, f"Overall sentiment: {overall}")

        return state


class RiskAssessmentAgent(BaseAgent):
    """Agent for risk assessment"""

    def __init__(self):
        super().__init__("Risk Assessment Agent")

    def execute(self, state: AgentState) -> AgentState:
        """Assess investment risks"""
        self.log_action(state, "Assessing investment risks")

        # Analyze risks from previous analyses
        risk_factors = []

        # Technical risks
        if state.get('technical_analysis'):
            rsi = state['technical_analysis'].get('rsi', 50)
            if rsi > 70:
                risk_factors.append("Overbought technical conditions")
            trend = state['technical_analysis'].get('trend')
            if trend == 'BEARISH':
                risk_factors.append("Negative technical trend")

        # Fundamental risks
        if state.get('fundamental_analysis'):
            debt_ratio = state['fundamental_analysis'].get('debt_to_equity', 1.0)
            if debt_ratio > 1.5:
                risk_factors.append("High debt-to-equity ratio")
            revenue_growth = state['fundamental_analysis'].get('revenue_growth', 0)
            if revenue_growth < 0:
                risk_factors.append("Declining revenue")

        # Sentiment risks
        if state.get('sentiment_analysis'):
            sentiment = state['sentiment_analysis'].get('overall_sentiment')
            if sentiment == 'BEARISH':
                risk_factors.append("Negative market sentiment")

        # Calculate overall risk score (0-100)
        risk_score = min(len(risk_factors) * 20, 100)

        if risk_score < 30:
            risk_level = "LOW"
        elif risk_score < 60:
            risk_level = "MEDIUM"
        else:
            risk_level = "HIGH"

        risk_assessment = {
            'risk_score': risk_score,
            'risk_level': risk_level,
            'risk_factors': risk_factors,
            'volatility_30d': round(random.uniform(15, 45), 2),
            'beta': round(random.uniform(0.8, 1.5), 2),
            'max_drawdown': round(random.uniform(10, 30), 2),
            'sharpe_ratio': round(random.uniform(0.5, 2.5), 2)
        }

        state['risk_assessment'] = risk_assessment
        self.log_action(state, f"Risk level: {risk_level} (Score: {risk_score}/100)")

        return state


class RecommendationAgent(BaseAgent):
    """Agent that synthesizes all analyses into final recommendation"""

    def __init__(self):
        super().__init__("Recommendation Agent")

    def execute(self, state: AgentState) -> AgentState:
        """Generate final investment recommendation"""
        self.log_action(state, "Synthesizing final recommendation")

        symbol = state['symbol']

        # Collect signals from all agents
        signals = []

        # Technical signal
        if state.get('technical_analysis'):
            tech_signal = state['technical_analysis'].get('signal', '')
            if 'buying' in tech_signal.lower():
                signals.append(('TECHNICAL', 'BUY', 1.0))
            elif 'selling' in tech_signal.lower():
                signals.append(('TECHNICAL', 'SELL', 1.0))
            else:
                signals.append(('TECHNICAL', 'HOLD', 0.5))

        # Fundamental signal
        if state.get('fundamental_analysis'):
            valuation = state['fundamental_analysis'].get('valuation', 'FAIRLY VALUED')
            if valuation == 'UNDERVALUED':
                signals.append(('FUNDAMENTAL', 'BUY', 1.5))
            elif valuation == 'OVERVALUED':
                signals.append(('FUNDAMENTAL', 'SELL', 1.5))
            else:
                signals.append(('FUNDAMENTAL', 'HOLD', 1.0))

        # Sentiment signal
        if state.get('sentiment_analysis'):
            sentiment = state['sentiment_analysis'].get('overall_sentiment', 'NEUTRAL')
            if sentiment == 'BULLISH':
                signals.append(('SENTIMENT', 'BUY', 1.0))
            elif sentiment == 'BEARISH':
                signals.append(('SENTIMENT', 'SELL', 1.0))
            else:
                signals.append(('SENTIMENT', 'HOLD', 0.5))

        # Calculate weighted recommendation
        buy_score = sum(weight for _, action, weight in signals if action == 'BUY')
        sell_score = sum(weight for _, action, weight in signals if action == 'SELL')
        hold_score = sum(weight for _, action, weight in signals if action == 'HOLD')

        if buy_score > sell_score and buy_score > hold_score:
            recommendation = "BUY"
        elif sell_score > buy_score and sell_score > hold_score:
            recommendation = "SELL"
        else:
            recommendation = "HOLD"

        # Calculate confidence
        total_score = buy_score + sell_score + hold_score
        confidence = round(max(buy_score, sell_score, hold_score) / total_score, 2) if total_score > 0 else 0.5

        # Risk-adjusted recommendation
        risk_level = state.get('risk_assessment', {}).get('risk_level', 'MEDIUM')
        if risk_level == 'HIGH' and recommendation == 'BUY':
            recommendation = "CAUTIOUS BUY"

        # Generate reasoning
        reasoning = self._generate_reasoning(state, recommendation, signals)

        final_recommendation = {
            'symbol': symbol,
            'recommendation': recommendation,
            'confidence': confidence,
            'price': state['market_data']['current_price'],
            'target_price': state['fundamental_analysis'].get('price_target'),
            'risk_level': risk_level,
            'reasoning': reasoning,
            'signals_breakdown': [
                {'source': source, 'action': action, 'weight': weight}
                for source, action, weight in signals
            ],
            'timestamp': datetime.now().isoformat()
        }

        state['final_recommendation'] = final_recommendation
        self.log_action(state, f"Final recommendation: {recommendation} (confidence: {confidence})")

        return state

    def _generate_reasoning(self, state: AgentState, recommendation: str, signals: List) -> str:
        """Generate human-readable reasoning"""
        reasons = []

        # Technical reasons
        if state.get('technical_analysis'):
            trend = state['technical_analysis'].get('trend', 'NEUTRAL')
            rsi = state['technical_analysis'].get('rsi', 50)
            reasons.append(f"Technical trend is {trend} with RSI at {rsi}")

        # Fundamental reasons
        if state.get('fundamental_analysis'):
            valuation = state['fundamental_analysis'].get('valuation', 'FAIRLY VALUED')
            revenue_growth = state['fundamental_analysis'].get('revenue_growth', 0)
            reasons.append(f"Stock appears {valuation.lower()} with {revenue_growth}% revenue growth")

        # Sentiment reasons
        if state.get('sentiment_analysis'):
            sentiment = state['sentiment_analysis'].get('overall_sentiment', 'NEUTRAL')
            reasons.append(f"Market sentiment is {sentiment.lower()}")

        # Risk reasons
        if state.get('risk_assessment'):
            risk = state['risk_assessment'].get('risk_level', 'MEDIUM')
            reasons.append(f"Overall risk level assessed as {risk.lower()}")

        return ". ".join(reasons) + "."


class TradingAssistantGraph:
    """
    LangGraph-style workflow orchestrator
    Coordinates multiple agents in a defined workflow
    """

    def __init__(self):
        self.agents = {
            'market_data': MarketDataAgent(),
            'technical': TechnicalAnalysisAgent(),
            'fundamental': FundamentalAnalysisAgent(),
            'sentiment': SentimentAnalysisAgent(),
            'risk': RiskAssessmentAgent(),
            'recommendation': RecommendationAgent()
        }

    def create_initial_state(self, query: str, symbol: str) -> AgentState:
        """Initialize the state"""
        return {
            'query': query,
            'symbol': symbol,
            'market_data': {},
            'technical_analysis': {},
            'fundamental_analysis': {},
            'sentiment_analysis': {},
            'risk_assessment': {},
            'final_recommendation': {},
            'agent_history': []
        }

    def run_workflow(self, query: str, symbol: str) -> AgentState:
        """
        Execute the multi-agent workflow
        Graph structure: market_data → [technical, fundamental, sentiment] → risk → recommendation
        """
        print("=" * 70)
        print(f"TRADING ASSISTANT: Analyzing {symbol}")
        print("=" * 70)

        # Initialize state
        state = self.create_initial_state(query, symbol)

        # Execute agents in sequence
        print("\nExecuting agent workflow...")
        print("-" * 70)

        # 1. Market Data (required first)
        state = self.agents['market_data'].execute(state)

        # 2. Parallel analysis agents
        state = self.agents['technical'].execute(state)
        state = self.agents['fundamental'].execute(state)
        state = self.agents['sentiment'].execute(state)

        # 3. Risk assessment (requires all previous analyses)
        state = self.agents['risk'].execute(state)

        # 4. Final recommendation (requires everything)
        state = self.agents['recommendation'].execute(state)

        return state

    def display_results(self, state: AgentState):
        """Display comprehensive analysis results"""
        print("\n" + "=" * 70)
        print("ANALYSIS RESULTS")
        print("=" * 70)

        # Market Data
        print("\n1. MARKET DATA:")
        md = state['market_data']
        print(f"   Price: ${md['current_price']} ({md['day_change']:+.2f}%)")
        print(f"   Volume: {md['volume']:,}")
        print(f"   Market Cap: ${md['market_cap']:,.0f}")
        print(f"   P/E Ratio: {md['pe_ratio']}")

        # Technical Analysis
        print("\n2. TECHNICAL ANALYSIS:")
        ta = state['technical_analysis']
        print(f"   Trend: {ta['trend']}")
        print(f"   RSI: {ta['rsi']}")
        print(f"   MACD: {ta['macd']}")
        print(f"   Signal: {ta['signal']}")

        # Fundamental Analysis
        print("\n3. FUNDAMENTAL ANALYSIS:")
        fa = state['fundamental_analysis']
        print(f"   Valuation: {fa['valuation']}")
        print(f"   Revenue Growth: {fa['revenue_growth']}%")
        print(f"   Profit Margin: {fa['profit_margin']}%")
        print(f"   Price Target: ${fa['price_target']}")

        # Sentiment Analysis
        print("\n4. SENTIMENT ANALYSIS:")
        sa = state['sentiment_analysis']
        print(f"   Overall: {sa['overall_sentiment']}")
        print(f"   News: {sa['news_sentiment']}")
        print(f"   Social Media: {sa['social_media_sentiment']}")
        print(f"   Analyst: {sa['analyst_sentiment']}")

        # Risk Assessment
        print("\n5. RISK ASSESSMENT:")
        ra = state['risk_assessment']
        print(f"   Risk Level: {ra['risk_level']} ({ra['risk_score']}/100)")
        print(f"   Beta: {ra['beta']}")
        print(f"   Sharpe Ratio: {ra['sharpe_ratio']}")
        if ra['risk_factors']:
            print(f"   Risk Factors:")
            for factor in ra['risk_factors']:
                print(f"     - {factor}")

        # Final Recommendation
        print("\n6. FINAL RECOMMENDATION:")
        print("=" * 70)
        rec = state['final_recommendation']
        print(f"   ACTION: {rec['recommendation']}")
        print(f"   CONFIDENCE: {rec['confidence']:.0%}")
        print(f"   CURRENT PRICE: ${rec['price']}")
        print(f"   TARGET PRICE: ${rec['target_price']}")
        print(f"   RISK LEVEL: {rec['risk_level']}")
        print(f"\n   REASONING: {rec['reasoning']}")
        print("=" * 70)


def main():
    """Demo: Multi-agent trading assistant"""
    print("=" * 70)
    print("LANGGRAPH: MULTI-AGENT TRADING ASSISTANT")
    print("=" * 70)

    # Initialize system
    trading_assistant = TradingAssistantGraph()

    # Test symbols
    symbols = ['AAPL', 'TSLA', 'NVDA']

    for symbol in symbols:
        query = f"Should I invest in {symbol}?"

        # Run analysis
        state = trading_assistant.run_workflow(query, symbol)

        # Display results
        trading_assistant.display_results(state)

        # Show agent execution history
        print("\nAgent Execution History:")
        for entry in state['agent_history']:
            print(f"  {entry}")

        print("\n" + "=" * 70 + "\n")

    print("\nPRODUCTION RECOMMENDATIONS:")
    print("- Integrate real market data APIs")
    print("- Add LLM-powered agents for natural language understanding")
    print("- Implement parallel agent execution for speed")
    print("- Add feedback loops between agents")
    print("- Create custom agent graphs for different strategies")
    print("- Add human-in-the-loop approval for high-risk decisions")
    print("- Implement agent memory for learning from past decisions")


if __name__ == "__main__":
    main()
