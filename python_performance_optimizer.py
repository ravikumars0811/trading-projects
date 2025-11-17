"""
HFT Performance Optimization Toolkit for Python
================================================

Demonstrates high-performance techniques for pricing engines and trading systems:
1. NumPy vectorization
2. Numba JIT compilation
3. Memory-efficient data structures
4. Multiprocessing for parallelization
5. Cython alternatives

Author: HFT System Optimizer
"""

import numpy as np
import pandas as pd
from numba import jit, njit, prange, vectorize
import time
from typing import Tuple, List
import multiprocessing as mp
from functools import wraps


# ============================================================================
# PERFORMANCE MEASUREMENT DECORATOR
# ============================================================================

def timeit(func):
    """Decorator to measure function execution time"""
    @wraps(func)
    def wrapper(*args, **kwargs):
        start = time.perf_counter()
        result = func(*args, **kwargs)
        end = time.perf_counter()
        print(f"{func.__name__}: {(end - start) * 1000:.2f} ms")
        return result
    return wrapper


# ============================================================================
# 1. VWAP CALCULATION
# ============================================================================

@timeit
def vwap_slow(prices: List[float], volumes: List[int]) -> float:
    """Slow Python loop version"""
    total_value = 0.0
    total_volume = 0
    for p, v in zip(prices, volumes):
        total_value += p * v
        total_volume += v
    return total_value / total_volume


@timeit
def vwap_numpy(prices: np.ndarray, volumes: np.ndarray) -> float:
    """Fast NumPy vectorized version (10-100x faster)"""
    return np.sum(prices * volumes) / np.sum(volumes)


@njit
def vwap_numba(prices: np.ndarray, volumes: np.ndarray) -> float:
    """Numba JIT-compiled version (comparable to C++)"""
    total_value = 0.0
    total_volume = 0.0
    for i in range(len(prices)):
        total_value += prices[i] * volumes[i]
        total_volume += volumes[i]
    return total_value / total_volume


# ============================================================================
# 2. MOVING AVERAGE CALCULATIONS
# ============================================================================

@timeit
def sma_slow(prices: List[float], window: int) -> List[float]:
    """Slow Python version"""
    result = []
    for i in range(len(prices) - window + 1):
        result.append(sum(prices[i:i+window]) / window)
    return result


@timeit
def sma_pandas(prices: pd.Series, window: int) -> pd.Series:
    """Pandas rolling window (optimized)"""
    return prices.rolling(window=window).mean()


@timeit
def sma_numpy(prices: np.ndarray, window: int) -> np.ndarray:
    """NumPy convolution (very fast)"""
    weights = np.ones(window) / window
    return np.convolve(prices, weights, mode='valid')


@njit
def sma_numba(prices: np.ndarray, window: int) -> np.ndarray:
    """Numba version (fastest for large arrays)"""
    n = len(prices)
    result = np.empty(n - window + 1)

    # Calculate first window
    window_sum = 0.0
    for i in range(window):
        window_sum += prices[i]
    result[0] = window_sum / window

    # Sliding window
    for i in range(1, n - window + 1):
        window_sum = window_sum - prices[i - 1] + prices[i + window - 1]
        result[i] = window_sum / window

    return result


# ============================================================================
# 3. OPTION PRICING - BLACK-SCHOLES
# ============================================================================

def norm_cdf_python(x: float) -> float:
    """Standard normal CDF (slow)"""
    from scipy.stats import norm
    return norm.cdf(x)


@vectorize(['float64(float64)'], target='parallel')
def norm_cdf_numba(x):
    """Vectorized normal CDF for Numba"""
    # Abramowitz and Stegun approximation
    a1 = 0.254829592
    a2 = -0.284496736
    a3 = 1.421413741
    a4 = -1.453152027
    a5 = 1.061405429
    p = 0.3275911

    sign = 1.0 if x >= 0 else -1.0
    x = abs(x) / np.sqrt(2.0)

    t = 1.0 / (1.0 + p * x)
    y = 1.0 - (((((a5 * t + a4) * t) + a3) * t + a2) * t + a1) * t * np.exp(-x * x)

    return 0.5 * (1.0 + sign * y)


@njit(parallel=True)
def black_scholes_numba(S: np.ndarray, K: np.ndarray, T: np.ndarray,
                        r: np.ndarray, sigma: np.ndarray) -> np.ndarray:
    """
    Vectorized Black-Scholes option pricing (Numba JIT)

    Parameters:
    -----------
    S : Stock prices
    K : Strike prices
    T : Time to maturity (years)
    r : Risk-free rates
    sigma : Volatilities

    Returns:
    --------
    Call option prices
    """
    n = len(S)
    call_prices = np.empty(n)

    for i in prange(n):  # Parallel loop
        d1 = (np.log(S[i] / K[i]) + (r[i] + 0.5 * sigma[i]**2) * T[i]) / \
             (sigma[i] * np.sqrt(T[i]))
        d2 = d1 - sigma[i] * np.sqrt(T[i])

        # Using approximation for norm_cdf
        Nd1 = 0.5 * (1.0 + np.tanh(np.sqrt(2.0 / np.pi) * (d1 + 0.044715 * d1**3)))
        Nd2 = 0.5 * (1.0 + np.tanh(np.sqrt(2.0 / np.pi) * (d2 + 0.044715 * d2**3)))

        call_prices[i] = S[i] * Nd1 - K[i] * np.exp(-r[i] * T[i]) * Nd2

    return call_prices


@timeit
def price_options_batch(num_options: int) -> np.ndarray:
    """Price multiple options in parallel"""
    S = np.random.uniform(90, 110, num_options)
    K = np.random.uniform(95, 105, num_options)
    T = np.random.uniform(0.1, 2.0, num_options)
    r = np.full(num_options, 0.05)
    sigma = np.random.uniform(0.15, 0.35, num_options)

    return black_scholes_numba(S, K, T, r, sigma)


# ============================================================================
# 4. MONTE CARLO SIMULATION
# ============================================================================

@njit(parallel=True)
def monte_carlo_option_pricing(S0: float, K: float, T: float,
                                 r: float, sigma: float,
                                 num_sims: int, num_steps: int = 252) -> float:
    """
    Monte Carlo option pricing with Numba parallelization

    Parameters:
    -----------
    S0 : Initial stock price
    K : Strike price
    T : Time to maturity (years)
    r : Risk-free rate
    sigma : Volatility
    num_sims : Number of simulation paths
    num_steps : Number of time steps

    Returns:
    --------
    Option price
    """
    dt = T / num_steps
    discount_factor = np.exp(-r * T)
    payoffs = np.zeros(num_sims)

    for i in prange(num_sims):  # Parallel execution
        S = S0
        for _ in range(num_steps):
            z = np.random.standard_normal()
            S = S * np.exp((r - 0.5 * sigma**2) * dt + sigma * np.sqrt(dt) * z)

        payoffs[i] = max(S - K, 0.0)  # Call option payoff

    return discount_factor * np.mean(payoffs)


# ============================================================================
# 5. STATISTICAL ARBITRAGE CALCULATIONS
# ============================================================================

@njit
def calculate_z_score_numba(prices: np.ndarray, window: int) -> np.ndarray:
    """
    Calculate rolling z-score for pairs trading

    z-score = (price - rolling_mean) / rolling_std
    """
    n = len(prices)
    z_scores = np.empty(n - window + 1)

    for i in range(window, n + 1):
        window_data = prices[i - window:i]
        mean = np.mean(window_data)
        std = np.std(window_data)

        if std > 0:
            z_scores[i - window] = (prices[i - 1] - mean) / std
        else:
            z_scores[i - window] = 0.0

    return z_scores


@njit
def cointegration_spread(prices_a: np.ndarray, prices_b: np.ndarray,
                          hedge_ratio: float) -> np.ndarray:
    """
    Calculate cointegration spread for pairs trading

    spread = prices_a - hedge_ratio * prices_b
    """
    return prices_a - hedge_ratio * prices_b


# ============================================================================
# 6. ORDER BOOK ANALYTICS
# ============================================================================

@njit
def calculate_imbalance(bid_volumes: np.ndarray,
                        ask_volumes: np.ndarray) -> np.ndarray:
    """
    Calculate order book imbalance

    imbalance = (bid_volume - ask_volume) / (bid_volume + ask_volume)
    """
    total_volume = bid_volumes + ask_volumes
    # Avoid division by zero
    mask = total_volume > 0
    imbalance = np.zeros_like(bid_volumes, dtype=np.float64)
    imbalance[mask] = (bid_volumes[mask] - ask_volumes[mask]) / total_volume[mask]
    return imbalance


@njit
def calculate_microprice(bid_prices: np.ndarray, bid_volumes: np.ndarray,
                          ask_prices: np.ndarray, ask_volumes: np.ndarray) -> np.ndarray:
    """
    Calculate volume-weighted microprice

    microprice = (bid_price * ask_volume + ask_price * bid_volume) / (bid_volume + ask_volume)
    """
    n = len(bid_prices)
    microprice = np.empty(n)

    for i in range(n):
        total_volume = bid_volumes[i] + ask_volumes[i]
        if total_volume > 0:
            microprice[i] = (bid_prices[i] * ask_volumes[i] + ask_prices[i] * bid_volumes[i]) / total_volume
        else:
            microprice[i] = (bid_prices[i] + ask_prices[i]) / 2.0

    return microprice


# ============================================================================
# 7. MEMORY OPTIMIZATION
# ============================================================================

def optimize_dataframe_memory(df: pd.DataFrame) -> pd.DataFrame:
    """
    Optimize DataFrame memory usage by downcasting types

    Can reduce memory by 50-80%
    """
    start_mem = df.memory_usage(deep=True).sum() / 1024**2
    print(f"Initial memory: {start_mem:.2f} MB")

    for col in df.columns:
        col_type = df[col].dtype

        if col_type == 'float64':
            df[col] = df[col].astype('float32')
        elif col_type == 'int64':
            df[col] = pd.to_numeric(df[col], downcast='integer')
        elif col_type == 'object':
            # Convert strings to category if unique ratio < 50%
            if df[col].nunique() / len(df) < 0.5:
                df[col] = df[col].astype('category')

    end_mem = df.memory_usage(deep=True).sum() / 1024**2
    print(f"Optimized memory: {end_mem:.2f} MB")
    print(f"Reduction: {100 * (start_mem - end_mem) / start_mem:.1f}%")

    return df


# ============================================================================
# 8. PARALLEL PROCESSING
# ============================================================================

def calculate_strategy_signals(params: Tuple[int, float]) -> dict:
    """
    CPU-intensive strategy calculation
    Used for parameter optimization
    """
    lookback, threshold = params

    # Simulate expensive calculation
    prices = np.random.randn(10000)
    z_scores = calculate_z_score_numba(prices, lookback)

    signals = np.where(z_scores > threshold, 1,
                      np.where(z_scores < -threshold, -1, 0))

    returns = np.sum(signals[:-1] * np.diff(prices[-len(signals):]))

    return {
        'lookback': lookback,
        'threshold': threshold,
        'returns': returns
    }


@timeit
def optimize_parameters_parallel():
    """
    Optimize strategy parameters using all CPU cores
    """
    # Generate parameter combinations
    lookbacks = range(10, 100, 10)
    thresholds = np.arange(1.0, 3.0, 0.5)
    params = [(lb, th) for lb in lookbacks for th in thresholds]

    # Parallel execution
    with mp.Pool(mp.cpu_count()) as pool:
        results = pool.map(calculate_strategy_signals, params)

    # Find best parameters
    best = max(results, key=lambda x: x['returns'])
    print(f"Best parameters: lookback={best['lookback']}, threshold={best['threshold']:.2f}")
    print(f"Returns: {best['returns']:.4f}")


# ============================================================================
# 9. BENCHMARKING SUITE
# ============================================================================

def run_benchmarks():
    """Run comprehensive performance benchmarks"""
    print("=" * 70)
    print("HFT PYTHON PERFORMANCE BENCHMARKS")
    print("=" * 70)

    # Test data
    n = 1_000_000
    prices_list = [100.0 + i * 0.01 for i in range(n)]
    volumes_list = [1000 + i for i in range(n)]

    prices_np = np.array(prices_list, dtype=np.float64)
    volumes_np = np.array(volumes_list, dtype=np.int32)

    print("\n1. VWAP Calculation (1M trades)")
    print("-" * 70)
    result_slow = vwap_slow(prices_list[:10000], volumes_list[:10000])  # Only 10k for slow version
    result_numpy = vwap_numpy(prices_np, volumes_np)
    result_numba = vwap_numba(prices_np, volumes_np)

    print("\n2. Simple Moving Average (1M prices, window=100)")
    print("-" * 70)
    sma_slow(prices_list[:10000], 100)  # Only 10k for slow version
    sma_numpy(prices_np, 100)
    sma_numba(prices_np, 100)

    print("\n3. Black-Scholes Option Pricing (100k options)")
    print("-" * 70)
    price_options_batch(100_000)

    print("\n4. Monte Carlo Simulation (100k paths)")
    print("-" * 70)
    start = time.perf_counter()
    mc_price = monte_carlo_option_pricing(100.0, 105.0, 1.0, 0.05, 0.2, 100_000)
    end = time.perf_counter()
    print(f"monte_carlo_option_pricing: {(end - start) * 1000:.2f} ms")
    print(f"Option price: {mc_price:.4f}")

    print("\n5. Z-Score Calculation (1M prices)")
    print("-" * 70)
    start = time.perf_counter()
    z_scores = calculate_z_score_numba(prices_np, 100)
    end = time.perf_counter()
    print(f"calculate_z_score_numba: {(end - start) * 1000:.2f} ms")

    print("\n6. Order Book Analytics (1M levels)")
    print("-" * 70)
    bid_volumes = np.random.randint(100, 10000, n)
    ask_volumes = np.random.randint(100, 10000, n)
    start = time.perf_counter()
    imbalance = calculate_imbalance(bid_volumes, ask_volumes)
    end = time.perf_counter()
    print(f"calculate_imbalance: {(end - start) * 1000:.2f} ms")

    print("\n" + "=" * 70)
    print("BENCHMARKS COMPLETE")
    print("=" * 70)


# ============================================================================
# 10. USAGE EXAMPLES
# ============================================================================

def example_usage():
    """Demonstrate practical usage"""

    print("\n### Example: Real-time VWAP calculation ###")
    prices = np.array([100.5, 100.6, 100.4, 100.7, 100.5])
    volumes = np.array([1000, 1500, 2000, 1200, 1800])
    vwap = vwap_numba(prices, volumes)
    print(f"VWAP: {vwap:.2f}")

    print("\n### Example: Option pricing ###")
    S = np.array([100.0, 105.0, 110.0])
    K = np.array([100.0, 100.0, 100.0])
    T = np.array([1.0, 1.0, 1.0])
    r = np.array([0.05, 0.05, 0.05])
    sigma = np.array([0.2, 0.2, 0.2])

    call_prices = black_scholes_numba(S, K, T, r, sigma)
    print("Call prices:", call_prices)

    print("\n### Example: Pairs trading z-score ###")
    spread_prices = np.random.randn(1000).cumsum() + 100
    z_scores = calculate_z_score_numba(spread_prices, 20)
    print(f"Latest z-score: {z_scores[-1]:.2f}")

    print("\n### Example: Memory optimization ###")
    df = pd.DataFrame({
        'timestamp': np.arange(100000),
        'price': np.random.randn(100000) * 10 + 100,
        'volume': np.random.randint(1000, 10000, 100000),
        'symbol': ['AAPL'] * 100000
    })
    df_optimized = optimize_dataframe_memory(df)


if __name__ == '__main__':
    # Run benchmarks
    run_benchmarks()

    # Show examples
    print("\n\n")
    example_usage()

    # Uncomment to test parallel optimization
    # print("\n### Parallel Parameter Optimization ###")
    # optimize_parameters_parallel()
