"""
Tests for the Pricing Engine API
"""

import pytest
from fastapi.testclient import TestClient
from main import app

client = TestClient(app)


class TestHealthEndpoint:
    """Tests for health check endpoint"""

    def test_health_check(self):
        """Test health check returns 200"""
        response = client.get("/api/health")
        assert response.status_code == 200
        data = response.json()
        assert data["status"] == "healthy"
        assert "timestamp" in data
        assert "version" in data


class TestOptionPricing:
    """Tests for option pricing endpoints"""

    def test_black_scholes_call(self):
        """Test Black-Scholes call option pricing"""
        payload = {
            "spot_price": 100,
            "strike_price": 100,
            "risk_free_rate": 0.05,
            "volatility": 0.2,
            "time_to_maturity": 1.0,
            "dividend_yield": 0.0,
            "option_type": "call",
            "option_style": "european",
            "pricing_model": "black_scholes"
        }

        response = client.post("/api/pricing/option", json=payload)
        assert response.status_code == 200

        data = response.json()
        assert "price" in data
        assert "greeks" in data
        assert data["price"] > 0
        assert 0 <= data["greeks"]["delta"] <= 1  # Call delta range

    def test_black_scholes_put(self):
        """Test Black-Scholes put option pricing"""
        payload = {
            "spot_price": 100,
            "strike_price": 100,
            "risk_free_rate": 0.05,
            "volatility": 0.2,
            "time_to_maturity": 1.0,
            "dividend_yield": 0.0,
            "option_type": "put",
            "option_style": "european",
            "pricing_model": "black_scholes"
        }

        response = client.post("/api/pricing/option", json=payload)
        assert response.status_code == 200

        data = response.json()
        assert "price" in data
        assert data["price"] > 0
        assert -1 <= data["greeks"]["delta"] <= 0  # Put delta range

    def test_binomial_american_option(self):
        """Test binomial tree for American options"""
        payload = {
            "spot_price": 100,
            "strike_price": 100,
            "risk_free_rate": 0.05,
            "volatility": 0.2,
            "time_to_maturity": 1.0,
            "dividend_yield": 0.0,
            "option_type": "put",
            "option_style": "american",
            "pricing_model": "binomial",
            "num_steps": 100
        }

        response = client.post("/api/pricing/option", json=payload)
        assert response.status_code == 200

        data = response.json()
        assert "price" in data
        assert data["price"] > 0
        assert data["model_used"] == "binomial"

    def test_invalid_pricing_model_for_american(self):
        """Test that Black-Scholes is rejected for American options"""
        payload = {
            "spot_price": 100,
            "strike_price": 100,
            "risk_free_rate": 0.05,
            "volatility": 0.2,
            "time_to_maturity": 1.0,
            "dividend_yield": 0.0,
            "option_type": "call",
            "option_style": "american",
            "pricing_model": "black_scholes"
        }

        response = client.post("/api/pricing/option", json=payload)
        assert response.status_code == 422  # Validation error

    def test_negative_spot_price(self):
        """Test that negative spot price is rejected"""
        payload = {
            "spot_price": -100,
            "strike_price": 100,
            "risk_free_rate": 0.05,
            "volatility": 0.2,
            "time_to_maturity": 1.0,
            "dividend_yield": 0.0,
            "option_type": "call",
            "option_style": "european",
            "pricing_model": "black_scholes"
        }

        response = client.post("/api/pricing/option", json=payload)
        assert response.status_code == 422


class TestImpliedVolatility:
    """Tests for implied volatility calculation"""

    def test_calculate_implied_volatility(self):
        """Test implied volatility calculation"""
        # First, price an option to get a market price
        price_payload = {
            "spot_price": 100,
            "strike_price": 100,
            "risk_free_rate": 0.05,
            "volatility": 0.25,
            "time_to_maturity": 1.0,
            "dividend_yield": 0.0,
            "option_type": "call",
            "option_style": "european",
            "pricing_model": "black_scholes"
        }

        price_response = client.post("/api/pricing/option", json=price_payload)
        market_price = price_response.json()["price"]

        # Now calculate IV from this price
        iv_payload = {
            "spot_price": 100,
            "strike_price": 100,
            "risk_free_rate": 0.05,
            "time_to_maturity": 1.0,
            "dividend_yield": 0.0,
            "option_type": "call",
            "market_price": market_price
        }

        response = client.post("/api/pricing/implied-volatility", json=iv_payload)
        assert response.status_code == 200

        data = response.json()
        assert "implied_volatility" in data
        # Should recover the original volatility (0.25)
        assert abs(data["implied_volatility"] - 0.25) < 0.01


class TestBondPricing:
    """Tests for bond pricing endpoints"""

    def test_zero_coupon_bond(self):
        """Test zero coupon bond pricing"""
        payload = {
            "face_value": 1000,
            "maturity": 5,
            "coupon_rate": 0.0,
            "payment_frequency": "annual",
            "yield_curve": [
                {"maturity": 1, "rate": 0.03},
                {"maturity": 5, "rate": 0.04},
                {"maturity": 10, "rate": 0.045}
            ]
        }

        response = client.post("/api/pricing/bond", json=payload)
        assert response.status_code == 200

        data = response.json()
        assert "price" in data
        assert "yield_to_maturity" in data
        assert "duration" in data
        assert "convexity" in data
        assert data["price"] < 1000  # Should be discounted
        assert data["price"] > 0

    def test_coupon_bond(self):
        """Test coupon bearing bond pricing"""
        payload = {
            "face_value": 1000,
            "maturity": 5,
            "coupon_rate": 0.05,
            "payment_frequency": "semi_annual",
            "yield_curve": [
                {"maturity": 0.5, "rate": 0.03},
                {"maturity": 1, "rate": 0.035},
                {"maturity": 5, "rate": 0.045},
                {"maturity": 10, "rate": 0.05}
            ]
        }

        response = client.post("/api/pricing/bond", json=payload)
        assert response.status_code == 200

        data = response.json()
        assert data["price"] > 0
        assert data["duration"] > 0
        assert data["convexity"] > 0


class TestSwapPricing:
    """Tests for interest rate swap pricing"""

    def test_interest_rate_swap(self):
        """Test interest rate swap pricing"""
        payload = {
            "notional": 1000000,
            "fixed_rate": 0.04,
            "maturity": 5,
            "payment_frequency": "quarterly",
            "yield_curve": [
                {"maturity": 0.25, "rate": 0.03},
                {"maturity": 0.5, "rate": 0.032},
                {"maturity": 1, "rate": 0.035},
                {"maturity": 2, "rate": 0.038},
                {"maturity": 5, "rate": 0.042},
                {"maturity": 10, "rate": 0.045}
            ]
        }

        response = client.post("/api/pricing/swap", json=payload)
        assert response.status_code == 200

        data = response.json()
        assert "present_value" in data
        assert "fair_swap_rate" in data
        assert "duration" in data
        assert "dv01" in data
        assert data["fair_swap_rate"] > 0


if __name__ == "__main__":
    pytest.main([__file__, "-v"])
