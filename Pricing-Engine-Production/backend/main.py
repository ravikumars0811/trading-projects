"""
Production-ready Pricing Engine API
Provides REST endpoints for pricing financial instruments
"""

from fastapi import FastAPI, HTTPException, Depends
from fastapi.middleware.cors import CORSMiddleware
from fastapi.responses import JSONResponse
from pydantic import BaseModel, Field, validator
from typing import List, Optional, Dict, Any
import logging
from datetime import datetime
import uvicorn

# Configure logging
logging.basicConfig(
    level=logging.INFO,
    format='%(asctime)s - %(name)s - %(levelname)s - %(message)s'
)
logger = logging.getLogger(__name__)

# Initialize FastAPI app
app = FastAPI(
    title="Pricing Engine API",
    description="Production-ready pricing engine for investment banks",
    version="1.0.0",
    docs_url="/api/docs",
    redoc_url="/api/redoc"
)

# CORS middleware for frontend access
app.add_middleware(
    CORSMiddleware,
    allow_origins=["*"],  # Configure appropriately for production
    allow_credentials=True,
    allow_methods=["*"],
    allow_headers=["*"],
)

# Pydantic models for request/response validation

class OptionPricingRequest(BaseModel):
    """Request model for option pricing"""
    spot_price: float = Field(..., gt=0, description="Current stock price")
    strike_price: float = Field(..., gt=0, description="Strike price")
    risk_free_rate: float = Field(..., ge=0, description="Risk-free interest rate")
    volatility: float = Field(..., ge=0, le=2, description="Implied volatility")
    time_to_maturity: float = Field(..., gt=0, description="Time to maturity in years")
    dividend_yield: float = Field(0.0, ge=0, description="Continuous dividend yield")
    option_type: str = Field(..., pattern="^(call|put)$", description="Option type: call or put")
    option_style: str = Field(..., pattern="^(european|american)$", description="Option style")
    pricing_model: str = Field(
        "black_scholes",
        pattern="^(black_scholes|binomial|monte_carlo)$",
        description="Pricing model to use"
    )
    num_steps: Optional[int] = Field(100, gt=0, description="Number of steps for binomial tree")
    num_simulations: Optional[int] = Field(100000, gt=0, description="Number of Monte Carlo simulations")

    @validator('pricing_model')
    def validate_model_style(cls, v, values):
        if 'option_style' in values:
            if v == 'black_scholes' and values['option_style'] == 'american':
                raise ValueError("Black-Scholes model only supports European options")
        return v

class GreeksResponse(BaseModel):
    """Response model for Greeks"""
    delta: float
    gamma: float
    theta: float
    vega: float
    rho: float

class OptionPricingResponse(BaseModel):
    """Response model for option pricing"""
    price: float
    greeks: GreeksResponse
    model_used: str
    timestamp: str

class ImpliedVolatilityRequest(BaseModel):
    """Request model for implied volatility calculation"""
    spot_price: float = Field(..., gt=0)
    strike_price: float = Field(..., gt=0)
    risk_free_rate: float = Field(..., ge=0)
    time_to_maturity: float = Field(..., gt=0)
    dividend_yield: float = Field(0.0, ge=0)
    option_type: str = Field(..., pattern="^(call|put)$")
    market_price: float = Field(..., gt=0, description="Market price of the option")

class ImpliedVolatilityResponse(BaseModel):
    """Response model for implied volatility"""
    implied_volatility: float
    timestamp: str

class YieldCurvePoint(BaseModel):
    """Yield curve point"""
    maturity: float = Field(..., ge=0)
    rate: float

class BondPricingRequest(BaseModel):
    """Request model for bond pricing"""
    face_value: float = Field(..., gt=0, description="Face value of the bond")
    maturity: float = Field(..., gt=0, description="Time to maturity in years")
    coupon_rate: float = Field(..., ge=0, description="Annual coupon rate (0 for zero-coupon)")
    yield_curve: List[YieldCurvePoint] = Field(..., min_items=1, description="Yield curve points")
    payment_frequency: str = Field(
        "semi_annual",
        pattern="^(annual|semi_annual|quarterly|monthly)$",
        description="Coupon payment frequency"
    )

class BondPricingResponse(BaseModel):
    """Response model for bond pricing"""
    price: float
    yield_to_maturity: float
    duration: float
    convexity: float
    timestamp: str

class SwapPricingRequest(BaseModel):
    """Request model for interest rate swap pricing"""
    notional: float = Field(..., gt=0, description="Notional amount")
    fixed_rate: float = Field(..., description="Fixed rate")
    maturity: float = Field(..., gt=0, description="Swap maturity in years")
    yield_curve: List[YieldCurvePoint] = Field(..., min_items=1)
    payment_frequency: str = Field(
        "quarterly",
        pattern="^(annual|semi_annual|quarterly|monthly)$"
    )

class SwapPricingResponse(BaseModel):
    """Response model for swap pricing"""
    present_value: float
    fair_swap_rate: float
    duration: float
    dv01: float
    timestamp: str

class HealthCheckResponse(BaseModel):
    """Health check response"""
    status: str
    timestamp: str
    version: str

# Utility function to convert enum strings
def get_option_type(option_type_str: str):
    """Convert string to OptionType enum"""
    try:
        from pricing_models import OptionType
        return OptionType.CALL if option_type_str.lower() == "call" else OptionType.PUT
    except ImportError:
        # Fallback if C++ module not available
        return option_type_str.upper()

def get_option_style(option_style_str: str):
    """Convert string to OptionStyle enum"""
    try:
        from pricing_models import OptionStyle
        return OptionStyle.EUROPEAN if option_style_str.lower() == "european" else OptionStyle.AMERICAN
    except ImportError:
        return option_style_str.upper()

def get_compounding_frequency(freq_str: str):
    """Convert string to CompoundingFrequency enum"""
    try:
        from pricing_models import CompoundingFrequency
        freq_map = {
            "annual": CompoundingFrequency.ANNUAL,
            "semi_annual": CompoundingFrequency.SEMI_ANNUAL,
            "quarterly": CompoundingFrequency.QUARTERLY,
            "monthly": CompoundingFrequency.MONTHLY,
        }
        return freq_map.get(freq_str.lower(), CompoundingFrequency.SEMI_ANNUAL)
    except ImportError:
        return freq_str.upper()

# API Endpoints

@app.get("/", response_model=Dict[str, str])
async def root():
    """Root endpoint"""
    return {
        "message": "Pricing Engine API",
        "version": "1.0.0",
        "docs": "/api/docs"
    }

@app.get("/api/health", response_model=HealthCheckResponse)
async def health_check():
    """Health check endpoint"""
    return HealthCheckResponse(
        status="healthy",
        timestamp=datetime.utcnow().isoformat(),
        version="1.0.0"
    )

@app.post("/api/pricing/option", response_model=OptionPricingResponse)
async def price_option(request: OptionPricingRequest):
    """
    Price an option using the specified model

    - **Black-Scholes**: For European options only
    - **Binomial Tree**: For European and American options
    - **Monte Carlo**: For European options (American requires LSM method)
    """
    try:
        logger.info(f"Pricing option: {request.dict()}")

        # Import pricing models
        from pricing_models import (
            price_option_black_scholes,
            price_option_binomial,
            price_option_monte_carlo
        )

        # Select pricing model
        if request.pricing_model == "black_scholes":
            result = price_option_black_scholes(
                spot_price=request.spot_price,
                strike_price=request.strike_price,
                risk_free_rate=request.risk_free_rate,
                volatility=request.volatility,
                time_to_maturity=request.time_to_maturity,
                dividend_yield=request.dividend_yield,
                option_type=request.option_type
            )
        elif request.pricing_model == "binomial":
            result = price_option_binomial(
                spot_price=request.spot_price,
                strike_price=request.strike_price,
                risk_free_rate=request.risk_free_rate,
                volatility=request.volatility,
                time_to_maturity=request.time_to_maturity,
                dividend_yield=request.dividend_yield,
                option_type=request.option_type,
                option_style=request.option_style,
                num_steps=request.num_steps
            )
        else:  # monte_carlo
            result = price_option_monte_carlo(
                spot_price=request.spot_price,
                strike_price=request.strike_price,
                risk_free_rate=request.risk_free_rate,
                volatility=request.volatility,
                time_to_maturity=request.time_to_maturity,
                dividend_yield=request.dividend_yield,
                option_type=request.option_type,
                num_simulations=request.num_simulations
            )

        return OptionPricingResponse(
            price=result["price"],
            greeks=GreeksResponse(**result["greeks"]),
            model_used=request.pricing_model,
            timestamp=datetime.utcnow().isoformat()
        )

    except Exception as e:
        logger.error(f"Error pricing option: {str(e)}")
        raise HTTPException(status_code=500, detail=str(e))

@app.post("/api/pricing/implied-volatility", response_model=ImpliedVolatilityResponse)
async def calculate_implied_volatility(request: ImpliedVolatilityRequest):
    """
    Calculate implied volatility from market price using Black-Scholes model
    """
    try:
        logger.info(f"Calculating implied volatility: {request.dict()}")

        from pricing_models import calculate_implied_volatility

        iv = calculate_implied_volatility(
            spot_price=request.spot_price,
            strike_price=request.strike_price,
            risk_free_rate=request.risk_free_rate,
            time_to_maturity=request.time_to_maturity,
            dividend_yield=request.dividend_yield,
            option_type=request.option_type,
            market_price=request.market_price
        )

        return ImpliedVolatilityResponse(
            implied_volatility=iv,
            timestamp=datetime.utcnow().isoformat()
        )

    except Exception as e:
        logger.error(f"Error calculating implied volatility: {str(e)}")
        raise HTTPException(status_code=500, detail=str(e))

@app.post("/api/pricing/bond", response_model=BondPricingResponse)
async def price_bond(request: BondPricingRequest):
    """
    Price a bond (zero-coupon or coupon-bearing)
    """
    try:
        logger.info(f"Pricing bond: {request.dict()}")

        from pricing_models import price_bond

        result = price_bond(
            face_value=request.face_value,
            maturity=request.maturity,
            coupon_rate=request.coupon_rate,
            yield_curve=[(p.maturity, p.rate) for p in request.yield_curve],
            payment_frequency=request.payment_frequency
        )

        return BondPricingResponse(
            price=result["price"],
            yield_to_maturity=result["ytm"],
            duration=result["duration"],
            convexity=result["convexity"],
            timestamp=datetime.utcnow().isoformat()
        )

    except Exception as e:
        logger.error(f"Error pricing bond: {str(e)}")
        raise HTTPException(status_code=500, detail=str(e))

@app.post("/api/pricing/swap", response_model=SwapPricingResponse)
async def price_swap(request: SwapPricingRequest):
    """
    Price an interest rate swap
    """
    try:
        logger.info(f"Pricing swap: {request.dict()}")

        from pricing_models import price_swap

        result = price_swap(
            notional=request.notional,
            fixed_rate=request.fixed_rate,
            maturity=request.maturity,
            yield_curve=[(p.maturity, p.rate) for p in request.yield_curve],
            payment_frequency=request.payment_frequency
        )

        return SwapPricingResponse(
            present_value=result["present_value"],
            fair_swap_rate=result["fair_swap_rate"],
            duration=result["duration"],
            dv01=result["dv01"],
            timestamp=datetime.utcnow().isoformat()
        )

    except Exception as e:
        logger.error(f"Error pricing swap: {str(e)}")
        raise HTTPException(status_code=500, detail=str(e))

# Error handlers
@app.exception_handler(ValueError)
async def value_error_handler(request, exc):
    return JSONResponse(
        status_code=400,
        content={"detail": str(exc)}
    )

@app.exception_handler(Exception)
async def general_exception_handler(request, exc):
    logger.error(f"Unhandled exception: {str(exc)}")
    return JSONResponse(
        status_code=500,
        content={"detail": "Internal server error"}
    )

if __name__ == "__main__":
    uvicorn.run(
        "main:app",
        host="0.0.0.0",
        port=8000,
        reload=True,
        log_level="info"
    )
