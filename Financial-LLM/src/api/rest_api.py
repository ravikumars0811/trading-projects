"""
REST API for Financial LLM
High-performance API for HFT and investment banking applications
"""

from fastapi import FastAPI, HTTPException, Depends, Header, Request, BackgroundTasks
from fastapi.middleware.cors import CORSMiddleware
from fastapi.middleware.gzip import GZipMiddleware
from fastapi.responses import JSONResponse, StreamingResponse
from pydantic import BaseModel, Field, validator
from typing import List, Dict, Optional, Any
import asyncio
import time
import uvicorn
from datetime import datetime
import logging
from prometheus_client import Counter, Histogram, Gauge, generate_latest
import json

import sys
import os
sys.path.append(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))

from inference.engine import FinancialLLMInferenceEngine, InferenceConfig


# Configure logging
logging.basicConfig(level=logging.INFO)
logger = logging.getLogger(__name__)


# Prometheus metrics
REQUEST_COUNT = Counter('api_requests_total', 'Total API requests', ['endpoint', 'method', 'status'])
REQUEST_LATENCY = Histogram('api_request_latency_seconds', 'Request latency', ['endpoint'])
ACTIVE_REQUESTS = Gauge('api_active_requests', 'Number of active requests')
INFERENCE_LATENCY = Histogram('inference_latency_ms', 'Inference latency in milliseconds', ['operation'])


# Pydantic models

class ClassifyRequest(BaseModel):
    """Request for text classification"""
    text: str = Field(..., description="Text to classify", min_length=1, max_length=10000)
    return_probabilities: bool = Field(True, description="Return probability scores")

    @validator('text')
    def validate_text(cls, v):
        if not v.strip():
            raise ValueError("Text cannot be empty")
        return v


class ClassifyResponse(BaseModel):
    """Response for text classification"""
    sentiment: Dict[str, float] = Field(..., description="Sentiment scores")
    latency_ms: float = Field(..., description="Inference latency in milliseconds")
    timestamp: str = Field(..., description="Response timestamp")


class GenerateRequest(BaseModel):
    """Request for text generation"""
    prompt: str = Field(..., description="Generation prompt", min_length=1, max_length=5000)
    max_new_tokens: Optional[int] = Field(100, description="Maximum tokens to generate", ge=1, le=1000)
    temperature: Optional[float] = Field(0.8, description="Sampling temperature", ge=0.1, le=2.0)
    top_k: Optional[int] = Field(50, description="Top-k sampling", ge=1, le=100)
    top_p: Optional[float] = Field(0.95, description="Nucleus sampling", ge=0.0, le=1.0)
    stream: bool = Field(False, description="Stream response")


class GenerateResponse(BaseModel):
    """Response for text generation"""
    text: str = Field(..., description="Generated text")
    num_tokens: int = Field(..., description="Number of tokens generated")
    latency_ms: float = Field(..., description="Generation latency in milliseconds")
    tokens_per_second: float = Field(..., description="Generation speed")
    timestamp: str = Field(..., description="Response timestamp")


class BatchClassifyRequest(BaseModel):
    """Request for batch classification"""
    texts: List[str] = Field(..., description="List of texts to classify", min_items=1, max_items=100)


class BatchClassifyResponse(BaseModel):
    """Response for batch classification"""
    results: List[Dict[str, Any]] = Field(..., description="Classification results")
    total_latency_ms: float = Field(..., description="Total processing latency")
    throughput: float = Field(..., description="Texts processed per second")


class HealthResponse(BaseModel):
    """Health check response"""
    status: str = Field(..., description="Service status")
    version: str = Field(..., description="API version")
    model_loaded: bool = Field(..., description="Whether model is loaded")
    uptime_seconds: float = Field(..., description="Service uptime")
    latency_stats: Optional[Dict[str, float]] = Field(None, description="Latency statistics")


class MarketAnalysisRequest(BaseModel):
    """Request for market analysis"""
    ticker: str = Field(..., description="Stock ticker symbol")
    data: Dict[str, Any] = Field(..., description="Market data")
    analysis_type: str = Field(..., description="Type of analysis: sentiment, trend, risk")


class MarketAnalysisResponse(BaseModel):
    """Response for market analysis"""
    ticker: str
    analysis: Dict[str, Any]
    recommendation: Optional[str] = None
    confidence: Optional[float] = None
    latency_ms: float


# API Application

class FinancialLLMAPI:
    """Financial LLM REST API"""

    def __init__(self, engine: FinancialLLMInferenceEngine):
        self.engine = engine
        self.app = FastAPI(
            title="Financial LLM API",
            description="High-performance API for financial language model inference",
            version="1.0.0",
            docs_url="/docs",
            redoc_url="/redoc"
        )

        self.start_time = time.time()
        self.setup_middleware()
        self.setup_routes()

    def setup_middleware(self):
        """Setup API middleware"""
        # CORS
        self.app.add_middleware(
            CORSMiddleware,
            allow_origins=["*"],  # Configure appropriately for production
            allow_credentials=True,
            allow_methods=["*"],
            allow_headers=["*"],
        )

        # GZip compression
        self.app.add_middleware(GZipMiddleware, minimum_size=1000)

        # Request tracking
        @self.app.middleware("http")
        async def track_requests(request: Request, call_next):
            ACTIVE_REQUESTS.inc()
            start_time = time.perf_counter()

            try:
                response = await call_next(request)
                status = response.status_code
            except Exception as e:
                logger.error(f"Request failed: {e}")
                status = 500
                raise
            finally:
                latency = time.perf_counter() - start_time
                REQUEST_LATENCY.labels(endpoint=request.url.path).observe(latency)
                REQUEST_COUNT.labels(
                    endpoint=request.url.path,
                    method=request.method,
                    status=status
                ).inc()
                ACTIVE_REQUESTS.dec()

            return response

    def setup_routes(self):
        """Setup API routes"""

        @self.app.get("/", response_model=Dict[str, str])
        async def root():
            """Root endpoint"""
            return {
                "service": "Financial LLM API",
                "version": "1.0.0",
                "status": "operational"
            }

        @self.app.get("/health", response_model=HealthResponse)
        async def health():
            """Health check endpoint"""
            uptime = time.time() - self.start_time
            latency_stats = self.engine.get_latency_stats()

            return HealthResponse(
                status="healthy",
                version="1.0.0",
                model_loaded=True,
                uptime_seconds=uptime,
                latency_stats=latency_stats
            )

        @self.app.post("/v1/classify", response_model=ClassifyResponse)
        async def classify(request: ClassifyRequest):
            """
            Classify text for sentiment/market regime

            Fast endpoint optimized for low-latency classification
            Typical latency: < 10ms
            """
            start_time = time.perf_counter()

            try:
                result = self.engine.classify(request.text)

                INFERENCE_LATENCY.labels(operation='classify').observe(result['latency_ms'])

                return ClassifyResponse(
                    sentiment=result,
                    latency_ms=result['latency_ms'],
                    timestamp=datetime.utcnow().isoformat()
                )

            except Exception as e:
                logger.error(f"Classification failed: {e}")
                raise HTTPException(status_code=500, detail=str(e))

        @self.app.post("/v1/generate", response_model=GenerateResponse)
        async def generate(request: GenerateRequest):
            """
            Generate text from prompt

            Use for market commentary, trade rationale, etc.
            Typical latency: 100-500ms depending on length
            """
            try:
                result = self.engine.generate(
                    prompt=request.prompt,
                    max_new_tokens=request.max_new_tokens,
                    temperature=request.temperature,
                    top_k=request.top_k,
                    top_p=request.top_p
                )

                INFERENCE_LATENCY.labels(operation='generate').observe(result['latency_ms'])

                return GenerateResponse(
                    text=result['text'],
                    num_tokens=result['num_tokens'],
                    latency_ms=result['latency_ms'],
                    tokens_per_second=result['tokens_per_second'],
                    timestamp=datetime.utcnow().isoformat()
                )

            except Exception as e:
                logger.error(f"Generation failed: {e}")
                raise HTTPException(status_code=500, detail=str(e))

        @self.app.post("/v1/batch/classify", response_model=BatchClassifyResponse)
        async def batch_classify(request: BatchClassifyRequest):
            """
            Batch classification for higher throughput

            Process multiple texts in a single request
            """
            start_time = time.perf_counter()

            try:
                results = self.engine.batch_classify(request.texts)

                total_latency_ms = (time.perf_counter() - start_time) * 1000
                throughput = len(request.texts) / (total_latency_ms / 1000)

                return BatchClassifyResponse(
                    results=results,
                    total_latency_ms=total_latency_ms,
                    throughput=throughput
                )

            except Exception as e:
                logger.error(f"Batch classification failed: {e}")
                raise HTTPException(status_code=500, detail=str(e))

        @self.app.post("/v1/market/analyze", response_model=MarketAnalysisResponse)
        async def analyze_market(request: MarketAnalysisRequest):
            """
            Analyze market data for a specific ticker

            Provides sentiment, trend, and risk analysis
            """
            start_time = time.perf_counter()

            try:
                # Format market data for analysis
                data_text = f"Ticker: {request.ticker}\n"
                data_text += f"Data: {json.dumps(request.data)}\n"
                data_text += f"Analysis type: {request.analysis_type}"

                # Classify sentiment
                sentiment = self.engine.classify(data_text)

                # Generate recommendation
                prompt = f"Based on {request.analysis_type} analysis for {request.ticker}, recommendation:"
                recommendation = self.engine.generate(prompt, max_new_tokens=100, temperature=0.5)

                latency_ms = (time.perf_counter() - start_time) * 1000

                return MarketAnalysisResponse(
                    ticker=request.ticker,
                    analysis={
                        'sentiment': sentiment,
                        'type': request.analysis_type
                    },
                    recommendation=recommendation['text'],
                    confidence=max(sentiment.values()) if sentiment else None,
                    latency_ms=latency_ms
                )

            except Exception as e:
                logger.error(f"Market analysis failed: {e}")
                raise HTTPException(status_code=500, detail=str(e))

        @self.app.get("/metrics")
        async def metrics():
            """Prometheus metrics endpoint"""
            return generate_latest()

        @self.app.get("/stats")
        async def stats():
            """Get inference statistics"""
            return self.engine.get_latency_stats()

        @self.app.post("/stats/reset")
        async def reset_stats():
            """Reset inference statistics"""
            self.engine.reset_latency_stats()
            return {"status": "statistics reset"}


def create_app(model_path: str, tokenizer_path: str, device: str = 'cuda') -> FastAPI:
    """
    Create and configure the FastAPI application

    Args:
        model_path: Path to model checkpoint
        tokenizer_path: Path to tokenizer
        device: Device for inference ('cuda' or 'cpu')

    Returns:
        Configured FastAPI app
    """
    # Initialize inference engine
    config = InferenceConfig(
        model_path=model_path,
        tokenizer_path=tokenizer_path,
        device=device,
        batch_size=1,
        use_torch_compile=True,
        use_mixed_precision=True,
        track_latency=True
    )

    engine = FinancialLLMInferenceEngine(config)

    # Create API
    api = FinancialLLMAPI(engine)

    return api.app


def run_server(
    model_path: str,
    tokenizer_path: str,
    host: str = "0.0.0.0",
    port: int = 8000,
    workers: int = 1,
    device: str = "cuda"
):
    """
    Run the API server

    Args:
        model_path: Path to model checkpoint
        tokenizer_path: Path to tokenizer
        host: Server host
        port: Server port
        workers: Number of worker processes
        device: Device for inference
    """
    app = create_app(model_path, tokenizer_path, device)

    uvicorn.run(
        app,
        host=host,
        port=port,
        workers=workers,
        log_level="info",
        access_log=True
    )


if __name__ == "__main__":
    import argparse

    parser = argparse.ArgumentParser(description="Financial LLM REST API Server")
    parser.add_argument("--model-path", type=str, required=True, help="Path to model checkpoint")
    parser.add_argument("--tokenizer-path", type=str, required=True, help="Path to tokenizer")
    parser.add_argument("--host", type=str, default="0.0.0.0", help="Server host")
    parser.add_argument("--port", type=int, default=8000, help="Server port")
    parser.add_argument("--workers", type=int, default=1, help="Number of workers")
    parser.add_argument("--device", type=str, default="cuda", help="Device (cuda/cpu)")

    args = parser.parse_args()

    run_server(
        model_path=args.model_path,
        tokenizer_path=args.tokenizer_path,
        host=args.host,
        port=args.port,
        workers=args.workers,
        device=args.device
    )
