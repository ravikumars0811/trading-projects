"""
Pricing models wrapper
Uses C++ implementation if available, otherwise falls back to pure Python
"""

import logging
from typing import Dict, List, Tuple, Any
import math
from enum import Enum

logger = logging.getLogger(__name__)

# Try to import C++ module
try:
    import sys
    sys.path.insert(0, '../python/src')
    from pricing_engine_py import (
        MarketData,
        BlackScholesPricer,
        BinomialTreePricer,
        MonteCarloOptionPricer,
        YieldCurve,
        YieldCurvePoint,
        ZeroCouponBond,
        CouponBond,
        InterestRateSwap,
        OptionType,
        OptionStyle,
        CompoundingFrequency
    )
    CPP_AVAILABLE = True
    logger.info("Using C++ pricing engine")
except ImportError as e:
    CPP_AVAILABLE = False
    logger.warning(f"C++ module not available, using Python fallback: {e}")

    # Define fallback enums
    class OptionType(Enum):
        CALL = "CALL"
        PUT = "PUT"

    class OptionStyle(Enum):
        EUROPEAN = "EUROPEAN"
        AMERICAN = "AMERICAN"

    class CompoundingFrequency(Enum):
        CONTINUOUS = "CONTINUOUS"
        ANNUAL = "ANNUAL"
        SEMI_ANNUAL = "SEMI_ANNUAL"
        QUARTERLY = "QUARTERLY"
        MONTHLY = "MONTHLY"


# Python fallback implementations

class PythonBlackScholes:
    """Pure Python Black-Scholes implementation"""

    @staticmethod
    def norm_cdf(x: float) -> float:
        """Standard normal cumulative distribution function"""
        return 0.5 * (1.0 + math.erf(x / math.sqrt(2.0)))

    @staticmethod
    def norm_pdf(x: float) -> float:
        """Standard normal probability density function"""
        return math.exp(-0.5 * x * x) / math.sqrt(2.0 * math.pi)

    @staticmethod
    def calculate_d1(S: float, K: float, r: float, sigma: float, T: float, q: float = 0.0) -> float:
        """Calculate d1 parameter"""
        numerator = math.log(S / K) + (r - q + 0.5 * sigma * sigma) * T
        denominator = sigma * math.sqrt(T)
        return numerator / denominator

    @staticmethod
    def calculate_d2(d1: float, sigma: float, T: float) -> float:
        """Calculate d2 parameter"""
        return d1 - sigma * math.sqrt(T)

    @staticmethod
    def price(S: float, K: float, r: float, sigma: float, T: float,
              option_type: str, q: float = 0.0) -> float:
        """Calculate option price"""
        if T <= 0:
            if option_type.upper() == "CALL":
                return max(0.0, S - K)
            else:
                return max(0.0, K - S)

        d1 = PythonBlackScholes.calculate_d1(S, K, r, sigma, T, q)
        d2 = PythonBlackScholes.calculate_d2(d1, sigma, T)

        discount = math.exp(-q * T)
        pv_strike = K * math.exp(-r * T)

        if option_type.upper() == "CALL":
            return S * discount * PythonBlackScholes.norm_cdf(d1) - \
                   pv_strike * PythonBlackScholes.norm_cdf(d2)
        else:
            return pv_strike * PythonBlackScholes.norm_cdf(-d2) - \
                   S * discount * PythonBlackScholes.norm_cdf(-d1)

    @staticmethod
    def greeks(S: float, K: float, r: float, sigma: float, T: float,
               option_type: str, q: float = 0.0) -> Dict[str, float]:
        """Calculate Greeks"""
        if T <= 0:
            return {"delta": 0, "gamma": 0, "theta": 0, "vega": 0, "rho": 0}

        d1 = PythonBlackScholes.calculate_d1(S, K, r, sigma, T, q)
        d2 = PythonBlackScholes.calculate_d2(d1, sigma, T)
        sqrt_t = math.sqrt(T)
        discount = math.exp(-q * T)

        greeks_dict = {}

        # Delta
        if option_type.upper() == "CALL":
            greeks_dict["delta"] = discount * PythonBlackScholes.norm_cdf(d1)
        else:
            greeks_dict["delta"] = -discount * PythonBlackScholes.norm_cdf(-d1)

        # Gamma
        greeks_dict["gamma"] = discount * PythonBlackScholes.norm_pdf(d1) / \
                               (S * sigma * sqrt_t)

        # Vega (per 1% change)
        greeks_dict["vega"] = S * discount * PythonBlackScholes.norm_pdf(d1) * sqrt_t / 100.0

        # Theta (per day)
        term1 = -(S * discount * PythonBlackScholes.norm_pdf(d1) * sigma) / (2.0 * sqrt_t)
        term2 = q * S * discount
        term3 = r * K * math.exp(-r * T)

        if option_type.upper() == "CALL":
            greeks_dict["theta"] = (term1 + term2 * PythonBlackScholes.norm_cdf(d1) -
                                   term3 * PythonBlackScholes.norm_cdf(d2)) / 365.0
        else:
            greeks_dict["theta"] = (term1 - term2 * PythonBlackScholes.norm_cdf(-d1) +
                                   term3 * PythonBlackScholes.norm_cdf(-d2)) / 365.0

        # Rho (per 1% change)
        pv_strike = K * T * math.exp(-r * T)
        if option_type.upper() == "CALL":
            greeks_dict["rho"] = pv_strike * PythonBlackScholes.norm_cdf(d2) / 100.0
        else:
            greeks_dict["rho"] = -pv_strike * PythonBlackScholes.norm_cdf(-d2) / 100.0

        return greeks_dict


# High-level pricing functions

def price_option_black_scholes(
    spot_price: float,
    strike_price: float,
    risk_free_rate: float,
    volatility: float,
    time_to_maturity: float,
    dividend_yield: float,
    option_type: str
) -> Dict[str, Any]:
    """Price option using Black-Scholes model"""

    if CPP_AVAILABLE:
        market_data = MarketData(
            spot_price, strike_price, risk_free_rate,
            volatility, time_to_maturity, dividend_yield
        )
        opt_type = OptionType.CALL if option_type.lower() == "call" else OptionType.PUT
        pricer = BlackScholesPricer(market_data, opt_type)

        price = pricer.price()
        greeks_obj = pricer.calculate_greeks()

        return {
            "price": price,
            "greeks": {
                "delta": greeks_obj.delta,
                "gamma": greeks_obj.gamma,
                "theta": greeks_obj.theta,
                "vega": greeks_obj.vega,
                "rho": greeks_obj.rho
            }
        }
    else:
        # Python fallback
        price = PythonBlackScholes.price(
            spot_price, strike_price, risk_free_rate,
            volatility, time_to_maturity, option_type, dividend_yield
        )
        greeks = PythonBlackScholes.greeks(
            spot_price, strike_price, risk_free_rate,
            volatility, time_to_maturity, option_type, dividend_yield
        )

        return {"price": price, "greeks": greeks}


def price_option_binomial(
    spot_price: float,
    strike_price: float,
    risk_free_rate: float,
    volatility: float,
    time_to_maturity: float,
    dividend_yield: float,
    option_type: str,
    option_style: str,
    num_steps: int = 100
) -> Dict[str, Any]:
    """Price option using Binomial tree model"""

    if CPP_AVAILABLE:
        market_data = MarketData(
            spot_price, strike_price, risk_free_rate,
            volatility, time_to_maturity, dividend_yield
        )
        opt_type = OptionType.CALL if option_type.lower() == "call" else OptionType.PUT
        opt_style = OptionStyle.EUROPEAN if option_style.lower() == "european" else OptionStyle.AMERICAN

        pricer = BinomialTreePricer(market_data, opt_type, opt_style, num_steps)

        price = pricer.price()
        greeks_obj = pricer.calculate_greeks()

        return {
            "price": price,
            "greeks": {
                "delta": greeks_obj.delta,
                "gamma": greeks_obj.gamma,
                "theta": greeks_obj.theta,
                "vega": greeks_obj.vega,
                "rho": greeks_obj.rho
            }
        }
    else:
        # Simplified Python binomial tree
        raise NotImplementedError("Python fallback for binomial tree not implemented")


def price_option_monte_carlo(
    spot_price: float,
    strike_price: float,
    risk_free_rate: float,
    volatility: float,
    time_to_maturity: float,
    dividend_yield: float,
    option_type: str,
    num_simulations: int = 100000
) -> Dict[str, Any]:
    """Price option using Monte Carlo simulation"""

    if CPP_AVAILABLE:
        market_data = MarketData(
            spot_price, strike_price, risk_free_rate,
            volatility, time_to_maturity, dividend_yield
        )
        opt_type = OptionType.CALL if option_type.lower() == "call" else OptionType.PUT

        pricer = MonteCarloOptionPricer(
            market_data, opt_type, OptionStyle.EUROPEAN, num_simulations
        )

        price = pricer.price()
        greeks_obj = pricer.calculate_greeks()

        return {
            "price": price,
            "greeks": {
                "delta": greeks_obj.delta,
                "gamma": greeks_obj.gamma,
                "theta": greeks_obj.theta,
                "vega": greeks_obj.vega,
                "rho": greeks_obj.rho
            }
        }
    else:
        raise NotImplementedError("Python fallback for Monte Carlo not implemented")


def calculate_implied_volatility(
    spot_price: float,
    strike_price: float,
    risk_free_rate: float,
    time_to_maturity: float,
    dividend_yield: float,
    option_type: str,
    market_price: float
) -> float:
    """Calculate implied volatility"""

    if CPP_AVAILABLE:
        market_data = MarketData(
            spot_price, strike_price, risk_free_rate,
            0.5, time_to_maturity, dividend_yield  # Initial volatility guess
        )
        opt_type = OptionType.CALL if option_type.lower() == "call" else OptionType.PUT

        return BlackScholesPricer.implied_volatility(market_data, opt_type, market_price)
    else:
        # Newton-Raphson in Python
        sigma = 0.5
        for _ in range(100):
            price = PythonBlackScholes.price(
                spot_price, strike_price, risk_free_rate,
                sigma, time_to_maturity, option_type, dividend_yield
            )
            diff = price - market_price

            if abs(diff) < 1e-6:
                return sigma

            greeks = PythonBlackScholes.greeks(
                spot_price, strike_price, risk_free_rate,
                sigma, time_to_maturity, option_type, dividend_yield
            )
            vega = greeks["vega"] * 100.0

            if abs(vega) < 1e-10:
                raise ValueError("Vega too small")

            sigma = sigma - diff / vega

            if sigma <= 0:
                sigma = 0.01

        raise ValueError("Implied volatility did not converge")


def price_bond(
    face_value: float,
    maturity: float,
    coupon_rate: float,
    yield_curve: List[Tuple[float, float]],
    payment_frequency: str
) -> Dict[str, Any]:
    """Price a bond"""

    if CPP_AVAILABLE:
        freq_map = {
            "annual": CompoundingFrequency.ANNUAL,
            "semi_annual": CompoundingFrequency.SEMI_ANNUAL,
            "quarterly": CompoundingFrequency.QUARTERLY,
            "monthly": CompoundingFrequency.MONTHLY,
        }
        freq = freq_map.get(payment_frequency.lower(), CompoundingFrequency.SEMI_ANNUAL)

        curve_points = [YieldCurvePoint(m, r) for m, r in yield_curve]
        curve = YieldCurve(curve_points)

        if coupon_rate == 0:
            bond = ZeroCouponBond(face_value, maturity, curve)
        else:
            bond = CouponBond(face_value, maturity, coupon_rate, curve, freq)

        price = bond.price()
        ytm = bond.yield_to_maturity(price)
        duration = bond.duration()
        convexity = bond.convexity()

        return {
            "price": price,
            "ytm": ytm,
            "duration": duration,
            "convexity": convexity
        }
    else:
        raise NotImplementedError("Python fallback for bond pricing not implemented")


def price_swap(
    notional: float,
    fixed_rate: float,
    maturity: float,
    yield_curve: List[Tuple[float, float]],
    payment_frequency: str
) -> Dict[str, Any]:
    """Price an interest rate swap"""

    if CPP_AVAILABLE:
        freq_map = {
            "annual": CompoundingFrequency.ANNUAL,
            "semi_annual": CompoundingFrequency.SEMI_ANNUAL,
            "quarterly": CompoundingFrequency.QUARTERLY,
            "monthly": CompoundingFrequency.MONTHLY,
        }
        freq = freq_map.get(payment_frequency.lower(), CompoundingFrequency.QUARTERLY)

        curve_points = [YieldCurvePoint(m, r) for m, r in yield_curve]
        curve = YieldCurve(curve_points)

        swap = InterestRateSwap(notional, fixed_rate, maturity, curve, freq)

        return {
            "present_value": swap.present_value(),
            "fair_swap_rate": swap.fair_swap_rate(),
            "duration": swap.duration(),
            "dv01": swap.dv01()
        }
    else:
        raise NotImplementedError("Python fallback for swap pricing not implemented")
