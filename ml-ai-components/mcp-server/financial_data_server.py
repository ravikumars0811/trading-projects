"""
MCP Server: Financial Data Provider
====================================
Real-world example: Providing financial market data and analytics through MCP protocol

Industry Use Case: Financial institutions use MCP servers to provide real-time
market data, historical prices, and analytical insights to trading systems and AI models.
"""

import asyncio
import json
from datetime import datetime, timedelta
from typing import Any, Dict, List
import random


class FinancialDataServer:
    """
    MCP-compliant financial data server
    Simulates a production-grade market data provider
    """

    def __init__(self):
        self.name = "financial-data-mcp"
        self.version = "1.0.0"
        self.symbols = ["AAPL", "GOOGL", "MSFT", "TSLA", "AMZN", "META", "NVDA"]

    def get_server_info(self) -> Dict[str, Any]:
        """MCP protocol: Server information endpoint"""
        return {
            "name": self.name,
            "version": self.version,
            "protocol_version": "2024-11-05",
            "capabilities": {
                "tools": True,
                "resources": True,
                "prompts": False
            }
        }

    def list_tools(self) -> List[Dict[str, Any]]:
        """MCP protocol: List available tools"""
        return [
            {
                "name": "get_stock_price",
                "description": "Get current stock price for a symbol",
                "input_schema": {
                    "type": "object",
                    "properties": {
                        "symbol": {
                            "type": "string",
                            "description": "Stock ticker symbol (e.g., AAPL)"
                        }
                    },
                    "required": ["symbol"]
                }
            },
            {
                "name": "get_historical_data",
                "description": "Get historical price data for a symbol",
                "input_schema": {
                    "type": "object",
                    "properties": {
                        "symbol": {"type": "string"},
                        "days": {
                            "type": "integer",
                            "description": "Number of days of historical data"
                        }
                    },
                    "required": ["symbol", "days"]
                }
            },
            {
                "name": "calculate_volatility",
                "description": "Calculate stock volatility (standard deviation of returns)",
                "input_schema": {
                    "type": "object",
                    "properties": {
                        "symbol": {"type": "string"},
                        "days": {"type": "integer"}
                    },
                    "required": ["symbol", "days"]
                }
            },
            {
                "name": "get_market_sentiment",
                "description": "Get aggregated market sentiment analysis",
                "input_schema": {
                    "type": "object",
                    "properties": {
                        "symbol": {"type": "string"}
                    },
                    "required": ["symbol"]
                }
            }
        ]

    def list_resources(self) -> List[Dict[str, Any]]:
        """MCP protocol: List available resources"""
        return [
            {
                "uri": f"stock://{symbol}",
                "name": f"{symbol} Stock Data",
                "description": f"Real-time data for {symbol}",
                "mime_type": "application/json"
            }
            for symbol in self.symbols
        ]

    async def call_tool(self, name: str, arguments: Dict[str, Any]) -> Dict[str, Any]:
        """MCP protocol: Execute tool with given arguments"""
        if name == "get_stock_price":
            return await self._get_stock_price(arguments["symbol"])
        elif name == "get_historical_data":
            return await self._get_historical_data(
                arguments["symbol"],
                arguments["days"]
            )
        elif name == "calculate_volatility":
            return await self._calculate_volatility(
                arguments["symbol"],
                arguments["days"]
            )
        elif name == "get_market_sentiment":
            return await self._get_market_sentiment(arguments["symbol"])
        else:
            raise ValueError(f"Unknown tool: {name}")

    async def _get_stock_price(self, symbol: str) -> Dict[str, Any]:
        """Simulate real-time stock price retrieval"""
        # Simulate API latency
        await asyncio.sleep(0.1)

        base_prices = {
            "AAPL": 180.0, "GOOGL": 140.0, "MSFT": 380.0,
            "TSLA": 250.0, "AMZN": 150.0, "META": 350.0, "NVDA": 500.0
        }

        base_price = base_prices.get(symbol, 100.0)
        current_price = base_price * (1 + random.uniform(-0.02, 0.02))

        return {
            "symbol": symbol,
            "price": round(current_price, 2),
            "currency": "USD",
            "timestamp": datetime.now().isoformat(),
            "volume": random.randint(1000000, 50000000),
            "change_percent": round(random.uniform(-3, 3), 2)
        }

    async def _get_historical_data(self, symbol: str, days: int) -> Dict[str, Any]:
        """Generate historical price data"""
        await asyncio.sleep(0.2)

        base_prices = {
            "AAPL": 180.0, "GOOGL": 140.0, "MSFT": 380.0,
            "TSLA": 250.0, "AMZN": 150.0, "META": 350.0, "NVDA": 500.0
        }

        base_price = base_prices.get(symbol, 100.0)
        data = []

        for i in range(days):
            date = datetime.now() - timedelta(days=days-i)
            price = base_price * (1 + random.uniform(-0.05, 0.05))

            data.append({
                "date": date.strftime("%Y-%m-%d"),
                "open": round(price * 0.99, 2),
                "high": round(price * 1.02, 2),
                "low": round(price * 0.97, 2),
                "close": round(price, 2),
                "volume": random.randint(1000000, 50000000)
            })

        return {
            "symbol": symbol,
            "data": data,
            "period": f"{days} days"
        }

    async def _calculate_volatility(self, symbol: str, days: int) -> Dict[str, Any]:
        """Calculate historical volatility"""
        historical = await self._get_historical_data(symbol, days)

        # Calculate returns
        prices = [d["close"] for d in historical["data"]]
        returns = [
            (prices[i] - prices[i-1]) / prices[i-1]
            for i in range(1, len(prices))
        ]

        # Calculate standard deviation (volatility)
        mean_return = sum(returns) / len(returns)
        variance = sum((r - mean_return) ** 2 for r in returns) / len(returns)
        volatility = variance ** 0.5

        # Annualized volatility
        annualized_volatility = volatility * (252 ** 0.5)  # 252 trading days

        return {
            "symbol": symbol,
            "period_days": days,
            "daily_volatility": round(volatility * 100, 2),
            "annualized_volatility": round(annualized_volatility * 100, 2),
            "risk_level": self._classify_risk(annualized_volatility)
        }

    def _classify_risk(self, volatility: float) -> str:
        """Classify risk level based on volatility"""
        if volatility < 0.15:
            return "LOW"
        elif volatility < 0.30:
            return "MEDIUM"
        else:
            return "HIGH"

    async def _get_market_sentiment(self, symbol: str) -> Dict[str, Any]:
        """Simulate sentiment analysis from news and social media"""
        await asyncio.sleep(0.15)

        sentiments = ["BULLISH", "NEUTRAL", "BEARISH"]
        sentiment = random.choice(sentiments)

        confidence = random.uniform(0.6, 0.95)

        return {
            "symbol": symbol,
            "sentiment": sentiment,
            "confidence": round(confidence, 2),
            "sources_analyzed": random.randint(50, 500),
            "timestamp": datetime.now().isoformat(),
            "indicators": {
                "news_sentiment": round(random.uniform(-1, 1), 2),
                "social_media_buzz": round(random.uniform(0, 100), 2),
                "analyst_rating": round(random.uniform(1, 5), 1)
            }
        }


async def main():
    """Demo: Run MCP server and test various tools"""
    print("=" * 70)
    print("MCP SERVER: Financial Data Provider")
    print("=" * 70)

    server = FinancialDataServer()

    # 1. Server Info
    print("\n1. SERVER INFORMATION:")
    print(json.dumps(server.get_server_info(), indent=2))

    # 2. List Available Tools
    print("\n2. AVAILABLE TOOLS:")
    for tool in server.list_tools():
        print(f"  - {tool['name']}: {tool['description']}")

    # 3. List Resources
    print("\n3. AVAILABLE RESOURCES:")
    for resource in server.list_resources()[:3]:
        print(f"  - {resource['uri']}: {resource['name']}")

    # 4. Test Tools
    symbol = "AAPL"
    print(f"\n4. TESTING TOOLS FOR {symbol}:")

    print(f"\n  a) Current Stock Price:")
    price_data = await server.call_tool("get_stock_price", {"symbol": symbol})
    print(f"     ${price_data['price']} ({price_data['change_percent']:+.2f}%)")

    print(f"\n  b) Historical Data (7 days):")
    historical = await server.call_tool("get_historical_data", {
        "symbol": symbol, "days": 7
    })
    print(f"     Retrieved {len(historical['data'])} days of data")
    for day in historical['data'][-3:]:
        print(f"     {day['date']}: Close ${day['close']}")

    print(f"\n  c) Volatility Analysis (30 days):")
    volatility = await server.call_tool("calculate_volatility", {
        "symbol": symbol, "days": 30
    })
    print(f"     Daily Volatility: {volatility['daily_volatility']}%")
    print(f"     Annualized Volatility: {volatility['annualized_volatility']}%")
    print(f"     Risk Level: {volatility['risk_level']}")

    print(f"\n  d) Market Sentiment:")
    sentiment = await server.call_tool("get_market_sentiment", {"symbol": symbol})
    print(f"     Sentiment: {sentiment['sentiment']}")
    print(f"     Confidence: {sentiment['confidence']}")
    print(f"     News Sentiment Score: {sentiment['indicators']['news_sentiment']}")

    print("\n" + "=" * 70)
    print("MCP SERVER DEMO COMPLETE")
    print("=" * 70)


if __name__ == "__main__":
    asyncio.run(main())
