"""
Example 3: AI-Powered Portfolio Optimization

This example demonstrates:
- Portfolio optimization using AI
- Multiple optimization strategies
- Risk analysis
- Performance comparison
- Rebalancing strategies

Perfect for investment banking applications
"""

import numpy as np
import sys
import os
from datetime import datetime, timedelta

sys.path.append(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))

from src.investment_banking.portfolio_optimizer import (
    AIPortfolioOptimizer,
    OptimizationConstraints
)


def generate_sample_portfolio_data(symbols, num_days=1000):
    """
    Generate synthetic portfolio data

    In production, use real data from:
    - Yahoo Finance (yfinance)
    - Alpha Vantage
    - Bloomberg Terminal
    - Internal data sources
    """
    np.random.seed(42)

    # Expected returns (annualized)
    expected_returns = {
        'AAPL': 0.15,
        'GOOGL': 0.18,
        'MSFT': 0.14,
        'AMZN': 0.20,
        'TSLA': 0.25,
        'SPY': 0.10,  # S&P 500 ETF
        'QQQ': 0.12,  # Nasdaq ETF
        'TLT': 0.03,  # Bond ETF
        'GLD': 0.05,  # Gold ETF
        'VNQ': 0.08   # Real Estate ETF
    }

    # Volatilities (annualized)
    volatilities = {
        'AAPL': 0.25,
        'GOOGL': 0.28,
        'MSFT': 0.23,
        'AMZN': 0.32,
        'TSLA': 0.45,
        'SPY': 0.18,
        'QQQ': 0.22,
        'TLT': 0.10,
        'GLD': 0.15,
        'VNQ': 0.20
    }

    # Generate price series
    historical_data = {}

    for symbol in symbols:
        mu = expected_returns.get(symbol, 0.10) / 252  # Daily return
        sigma = volatilities.get(symbol, 0.20) / np.sqrt(252)  # Daily volatility

        # Generate returns
        returns = np.random.normal(mu, sigma, num_days)

        # Convert to prices
        prices = 100 * np.exp(np.cumsum(returns))

        historical_data[symbol] = prices

    return historical_data


def visualize_allocation(allocation, title="Portfolio Allocation"):
    """Visualize portfolio allocation"""
    print(f"\n  {title}")
    print(f"  {'-' * 50}")

    # Sort by weight
    sorted_weights = sorted(
        allocation.weights.items(),
        key=lambda x: x[1],
        reverse=True
    )

    for symbol, weight in sorted_weights:
        if weight > 0.001:  # Only show significant positions
            bar_length = int(weight * 50)
            bar = '█' * bar_length
            print(f"  {symbol:<8} {bar} {weight * 100:>6.2f}%")


def main():
    """Main execution"""
    print("=" * 70)
    print("Example 3: AI-Powered Portfolio Optimization")
    print("=" * 70)

    # Step 1: Define Investment Universe
    print("\n[Step 1] Defining Investment Universe...")

    universe = ['AAPL', 'GOOGL', 'MSFT', 'AMZN', 'TSLA',
                'SPY', 'QQQ', 'TLT', 'GLD', 'VNQ']

    print(f"\n  Investment Universe ({len(universe)} assets):")
    asset_types = {
        'AAPL': 'Tech Stock',
        'GOOGL': 'Tech Stock',
        'MSFT': 'Tech Stock',
        'AMZN': 'Tech Stock',
        'TSLA': 'Tech Stock',
        'SPY': 'Equity ETF',
        'QQQ': 'Equity ETF',
        'TLT': 'Bond ETF',
        'GLD': 'Commodity ETF',
        'VNQ': 'Real Estate ETF'
    }

    for symbol in universe:
        print(f"    {symbol:<8} - {asset_types[symbol]}")

    # Step 2: Load Historical Data
    print("\n[Step 2] Loading Historical Data...")

    historical_data = generate_sample_portfolio_data(universe, num_days=1000)

    print(f"\n  Data Summary:")
    for symbol, prices in historical_data.items():
        start_price = prices[0]
        end_price = prices[-1]
        total_return = (end_price - start_price) / start_price
        print(f"    {symbol:<8}: ${start_price:>7.2f} → ${end_price:>7.2f} "
              f"({total_return * 100:>+6.2f}%)")

    # Step 3: Equal Weight Portfolio (Baseline)
    print("\n[Step 3] Baseline: Equal Weight Portfolio...")

    equal_weights = {symbol: 1.0 / len(universe) for symbol in universe}
    print(f"\n  Each asset: {100.0 / len(universe):.2f}%")

    # Calculate baseline metrics
    returns_matrix = np.column_stack([
        np.diff(historical_data[symbol]) / historical_data[symbol][:-1]
        for symbol in universe
    ])
    equal_weight_array = np.array([1.0 / len(universe)] * len(universe))
    portfolio_returns = returns_matrix @ equal_weight_array
    baseline_sharpe = (portfolio_returns.mean() * 252) / (portfolio_returns.std() * np.sqrt(252))

    print(f"  Sharpe Ratio: {baseline_sharpe:.2f}")

    # Step 4: Maximum Sharpe Optimization
    print("\n[Step 4] AI-Optimized: Maximum Sharpe Portfolio...")

    optimizer_sharpe = AIPortfolioOptimizer(
        universe=universe,
        method='maximum_sharpe',
        objective='sharpe_ratio'
    )

    constraints = OptimizationConstraints(
        max_position_size=0.30,  # Max 30% per asset
        min_position_size=0.0
    )

    allocation_sharpe = optimizer_sharpe.optimize(
        historical_data=historical_data,
        capital=1_000_000,
        constraints=constraints
    )

    visualize_allocation(allocation_sharpe, "Maximum Sharpe Portfolio")

    print(f"\n  Portfolio Metrics:")
    print(f"    Expected Return: {allocation_sharpe.expected_return * 100:.2f}%")
    print(f"    Expected Risk: {allocation_sharpe.expected_risk * 100:.2f}%")
    print(f"    Sharpe Ratio: {allocation_sharpe.sharpe_ratio:.2f}")
    print(f"    Max Drawdown: {allocation_sharpe.max_drawdown * 100:.2f}%")
    print(f"    Diversification: {allocation_sharpe.diversification_ratio:.2f}x")

    # Step 5: Minimum Variance Optimization
    print("\n[Step 5] AI-Optimized: Minimum Variance Portfolio...")

    optimizer_minvar = AIPortfolioOptimizer(
        universe=universe,
        method='minimum_variance'
    )

    allocation_minvar = optimizer_minvar.optimize(
        historical_data=historical_data,
        capital=1_000_000,
        constraints=constraints
    )

    visualize_allocation(allocation_minvar, "Minimum Variance Portfolio")

    print(f"\n  Portfolio Metrics:")
    print(f"    Expected Return: {allocation_minvar.expected_return * 100:.2f}%")
    print(f"    Expected Risk: {allocation_minvar.expected_risk * 100:.2f}%")
    print(f"    Sharpe Ratio: {allocation_minvar.sharpe_ratio:.2f}")
    print(f"    Max Drawdown: {allocation_minvar.max_drawdown * 100:.2f}%")
    print(f"    Diversification: {allocation_minvar.diversification_ratio:.2f}x")

    # Step 6: Risk Parity Optimization
    print("\n[Step 6] AI-Optimized: Risk Parity Portfolio...")

    optimizer_riskparity = AIPortfolioOptimizer(
        universe=universe,
        method='risk_parity'
    )

    allocation_riskparity = optimizer_riskparity.optimize(
        historical_data=historical_data,
        capital=1_000_000,
        constraints=constraints
    )

    visualize_allocation(allocation_riskparity, "Risk Parity Portfolio")

    print(f"\n  Portfolio Metrics:")
    print(f"    Expected Return: {allocation_riskparity.expected_return * 100:.2f}%")
    print(f"    Expected Risk: {allocation_riskparity.expected_risk * 100:.2f}%")
    print(f"    Sharpe Ratio: {allocation_riskparity.sharpe_ratio:.2f}")
    print(f"    Max Drawdown: {allocation_riskparity.max_drawdown * 100:.2f}%")
    print(f"    Diversification: {allocation_riskparity.diversification_ratio:.2f}x")

    # Step 7: Compare All Strategies
    print(f"\n{'=' * 70}")
    print(f"STRATEGY COMPARISON")
    print(f"{'=' * 70}")

    strategies = [
        ("Equal Weight", baseline_sharpe, 0.0, 0.0),
        ("Maximum Sharpe", allocation_sharpe.sharpe_ratio,
         allocation_sharpe.expected_return, allocation_sharpe.expected_risk),
        ("Minimum Variance", allocation_minvar.sharpe_ratio,
         allocation_minvar.expected_return, allocation_minvar.expected_risk),
        ("Risk Parity", allocation_riskparity.sharpe_ratio,
         allocation_riskparity.expected_return, allocation_riskparity.expected_risk)
    ]

    print(f"\n  {'Strategy':<20} {'Sharpe':<10} {'Return':<12} {'Risk':<12}")
    print(f"  {'-' * 60}")

    for name, sharpe, ret, risk in strategies:
        if name == "Equal Weight":
            print(f"  {name:<20} {sharpe:<10.2f} {'N/A':<12} {'N/A':<12}")
        else:
            print(f"  {name:<20} {sharpe:<10.2f} {ret * 100:<12.2f} {risk * 100:<12.2f}")

    # Find best strategy
    best_strategy = max(strategies, key=lambda x: x[1])
    print(f"\n  🏆 Best Strategy: {best_strategy[0]} (Sharpe: {best_strategy[1]:.2f})")

    # Step 8: Dollar Allocation
    print(f"\n{'=' * 70}")
    print(f"DOLLAR ALLOCATION (Recommended Portfolio)")
    print(f"{'=' * 70}")

    capital = 1_000_000
    recommended = allocation_sharpe  # Using maximum Sharpe

    print(f"\n  Total Capital: ${capital:,.0f}")
    print(f"\n  {'Asset':<10} {'Weight':<10} {'Dollar Amount':<15} {'Shares':<10}")
    print(f"  {'-' * 50}")

    # Assume current prices
    current_prices = {symbol: historical_data[symbol][-1] for symbol in universe}

    for symbol in sorted(recommended.weights.keys(),
                        key=lambda x: recommended.weights[x],
                        reverse=True):
        weight = recommended.weights[symbol]
        if weight > 0.001:
            dollar_amount = capital * weight
            shares = int(dollar_amount / current_prices[symbol])
            print(f"  {symbol:<10} {weight * 100:>6.2f}%   ${dollar_amount:>12,.0f}   {shares:>8,}")

    # Step 9: Rebalancing Strategy
    print(f"\n{'=' * 70}")
    print(f"REBALANCING STRATEGY")
    print(f"{'=' * 70}")

    print(f"\n  Recommended Rebalancing Frequency: Monthly")
    print(f"\n  Rebalancing Triggers:")
    print(f"    • Any position drifts >5% from target")
    print(f"    • Monthly calendar date")
    print(f"    • Major market regime change")
    print(f"    • Risk limit breach")

    print(f"\n  Transaction Costs (Estimate):")
    total_positions = sum(1 for w in recommended.weights.values() if w > 0.001)
    transaction_cost_bps = 5  # 5 basis points
    estimated_cost = capital * (transaction_cost_bps / 10000)
    print(f"    Number of Positions: {total_positions}")
    print(f"    Cost per Rebalance: ${estimated_cost:,.0f}")
    print(f"    Annual Cost (12x): ${estimated_cost * 12:,.0f}")

    # Step 10: Risk Report
    print(f"\n{'=' * 70}")
    print(f"RISK REPORT")
    print(f"{'=' * 70}")

    print(f"\n  Portfolio Risk Metrics:")
    print(f"    Expected Annual Return: {recommended.expected_return * 100:.2f}%")
    print(f"    Expected Annual Volatility: {recommended.expected_risk * 100:.2f}%")
    print(f"    Sharpe Ratio: {recommended.sharpe_ratio:.2f}")
    print(f"    Max Historical Drawdown: {recommended.max_drawdown * 100:.2f}%")

    print(f"\n  Diversification Analysis:")
    print(f"    Number of Assets: {len(universe)}")
    print(f"    Active Positions: {total_positions}")
    print(f"    Diversification Ratio: {recommended.diversification_ratio:.2f}x")
    print(f"    Largest Position: {max(recommended.weights.values()) * 100:.2f}%")

    print(f"\n  Stress Scenarios:")
    stress_scenarios = [
        ("Market Crash (-20%)", -0.15),
        ("Tech Selloff (-30% tech)", -0.08),
        ("Bond Spike (+2% yields)", -0.03),
        ("Flight to Safety", +0.02)
    ]

    for scenario, impact in stress_scenarios:
        impact_dollars = capital * impact
        print(f"    {scenario:<25}: {impact * 100:>+6.2f}% (${impact_dollars:>+12,.0f})")

    print(f"\n{'=' * 70}")
    print(f"Example completed successfully!")
    print(f"{'=' * 70}")

    print(f"\nKey Insights:")
    print(f"  • Maximum Sharpe strategy achieves best risk-adjusted returns")
    print(f"  • Minimum Variance reduces risk but sacrifices returns")
    print(f"  • Risk Parity balances risk contribution across assets")
    print(f"  • Diversification significantly reduces portfolio risk")
    print(f"  • Regular rebalancing maintains target allocation")

    print(f"\nNext steps:")
    print(f"  1. Use real market data for backtesting")
    print(f"  2. Implement transaction cost modeling")
    print(f"  3. Add factor analysis (momentum, value, quality)")
    print(f"  4. Include alternative assets (commodities, crypto)")
    print(f"  5. Set up automated rebalancing alerts")


if __name__ == "__main__":
    main()
