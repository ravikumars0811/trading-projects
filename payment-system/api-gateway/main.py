"""
API Gateway
Central entry point for all microservices with rate limiting and authentication
"""
from fastapi import FastAPI, Request, HTTPException, status, Depends
from fastapi.middleware.cors import CORSMiddleware
from fastapi.responses import JSONResponse
import httpx
import time
import sys
import os

sys.path.append(os.path.join(os.path.dirname(__file__), '..'))
from shared.utils import (
    setup_logging,
    RedisClient,
    RateLimiter,
    decode_token
)
from prometheus_client import Counter, Histogram, make_asgi_app

logger = setup_logging("api-gateway")

# Service URLs
SERVICES = {
    "user": os.getenv("USER_SERVICE_URL", "http://user-service:8001"),
    "wallet": os.getenv("WALLET_SERVICE_URL", "http://wallet-service:8002"),
    "transaction": os.getenv("TRANSACTION_SERVICE_URL", "http://transaction-service:8003"),
    "payment": os.getenv("PAYMENT_GATEWAY_URL", "http://payment-gateway:8004"),
}

# Prometheus metrics
REQUEST_COUNT = Counter(
    'api_gateway_requests_total',
    'Total request count',
    ['method', 'service', 'status']
)
REQUEST_LATENCY = Histogram(
    'api_gateway_request_duration_seconds',
    'Request latency',
    ['method', 'service']
)

app = FastAPI(
    title="Payment System API Gateway",
    description="Central API Gateway for Payment System Microservices",
    version="1.0.0"
)

app.add_middleware(
    CORSMiddleware,
    allow_origins=["*"],
    allow_credentials=True,
    allow_methods=["*"],
    allow_headers=["*"],
)

# Initialize Redis and Rate Limiter
redis_client = RedisClient()
rate_limiter = RateLimiter(redis_client)


# Rate limiting middleware
@app.middleware("http")
async def rate_limit_middleware(request: Request, call_next):
    """Rate limiting middleware"""
    # Get client IP
    client_ip = request.client.host

    # Check rate limit (100 requests per minute)
    if not rate_limiter.check_rate_limit(client_ip, max_requests=100, window_seconds=60):
        return JSONResponse(
            status_code=status.HTTP_429_TOO_MANY_REQUESTS,
            content={
                "error": "Rate limit exceeded",
                "message": "Too many requests. Please try again later."
            }
        )

    response = await call_next(request)
    return response


# Metrics middleware
@app.middleware("http")
async def metrics_middleware(request: Request, call_next):
    """Collect metrics for each request"""
    start_time = time.time()

    response = await call_next(request)

    process_time = time.time() - start_time
    response.headers["X-Process-Time"] = str(process_time)

    # Extract service from path
    path_parts = request.url.path.split("/")
    service = path_parts[1] if len(path_parts) > 1 else "unknown"

    REQUEST_COUNT.labels(
        method=request.method,
        service=service,
        status=response.status_code
    ).inc()

    REQUEST_LATENCY.labels(
        method=request.method,
        service=service
    ).observe(process_time)

    return response


async def verify_token(request: Request):
    """Verify JWT token from Authorization header"""
    auth_header = request.headers.get("Authorization")

    # Skip auth for public endpoints
    public_endpoints = ["/", "/health", "/docs", "/redoc", "/openapi.json"]
    if request.url.path in public_endpoints:
        return None

    # Skip auth for login and register
    if "login" in request.url.path or "register" in request.url.path:
        return None

    if not auth_header or not auth_header.startswith("Bearer "):
        raise HTTPException(
            status_code=status.HTTP_401_UNAUTHORIZED,
            detail="Missing or invalid authorization header"
        )

    token = auth_header.split(" ")[1]
    return decode_token(token)


async def proxy_request(service_url: str, path: str, request: Request):
    """Proxy request to microservice"""
    # Build target URL
    target_url = f"{service_url}{path}"

    # Get request body if present
    body = None
    if request.method in ["POST", "PUT", "PATCH"]:
        body = await request.body()

    # Forward headers
    headers = dict(request.headers)
    headers.pop("host", None)  # Remove host header

    try:
        async with httpx.AsyncClient(timeout=30.0) as client:
            response = await client.request(
                method=request.method,
                url=target_url,
                headers=headers,
                params=request.query_params,
                content=body
            )

            return JSONResponse(
                status_code=response.status_code,
                content=response.json() if response.text else None,
                headers=dict(response.headers)
            )

    except httpx.TimeoutException:
        raise HTTPException(
            status_code=status.HTTP_504_GATEWAY_TIMEOUT,
            detail="Service request timed out"
        )
    except httpx.RequestError as e:
        logger.error(f"Error proxying request: {e}")
        raise HTTPException(
            status_code=status.HTTP_503_SERVICE_UNAVAILABLE,
            detail="Service temporarily unavailable"
        )


# User Service Routes
@app.api_route("/users/{path:path}", methods=["GET", "POST", "PUT", "DELETE"])
async def user_service_proxy(path: str, request: Request):
    """Proxy requests to User Service"""
    await verify_token(request)
    return await proxy_request(SERVICES["user"], f"/api/v1/users/{path}", request)


# Wallet Service Routes
@app.api_route("/wallets/{path:path}", methods=["GET", "POST", "PUT", "DELETE"])
async def wallet_service_proxy(path: str, request: Request):
    """Proxy requests to Wallet Service"""
    await verify_token(request)
    return await proxy_request(SERVICES["wallet"], f"/api/v1/wallets/{path}", request)


# Transaction Service Routes
@app.api_route("/transactions/{path:path}", methods=["GET", "POST", "PUT", "DELETE"])
async def transaction_service_proxy(path: str, request: Request):
    """Proxy requests to Transaction Service"""
    await verify_token(request)
    return await proxy_request(SERVICES["transaction"], f"/api/v1/transactions/{path}", request)


# Payment Gateway Routes
@app.api_route("/payment/{path:path}", methods=["GET", "POST", "PUT", "DELETE"])
async def payment_service_proxy(path: str, request: Request):
    """Proxy requests to Payment Gateway"""
    await verify_token(request)
    return await proxy_request(SERVICES["payment"], f"/api/v1/payment/{path}", request)


@app.get("/")
async def root():
    """Root endpoint"""
    return {
        "service": "api-gateway",
        "version": "1.0.0",
        "status": "running",
        "services": list(SERVICES.keys())
    }


@app.get("/health")
async def health_check():
    """Health check endpoint"""
    service_status = {}

    async with httpx.AsyncClient(timeout=5.0) as client:
        for name, url in SERVICES.items():
            try:
                response = await client.get(f"{url}/health")
                service_status[name] = "healthy" if response.status_code == 200 else "unhealthy"
            except Exception:
                service_status[name] = "unavailable"

    return {
        "status": "healthy",
        "services": service_status
    }


# Metrics endpoint
metrics_app = make_asgi_app()
app.mount("/metrics", metrics_app)


if __name__ == "__main__":
    import uvicorn
    uvicorn.run("main:app", host="0.0.0.0", port=8000, reload=True)
