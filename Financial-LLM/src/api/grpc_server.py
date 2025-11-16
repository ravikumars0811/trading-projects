"""
gRPC Server for Financial LLM
Ultra-low latency interface for HFT applications
"""

import grpc
from concurrent import futures
import time
import json
import logging
from datetime import datetime
from typing import Iterator

# Note: In production, you would generate these from the .proto file using:
# python -m grpc_tools.protoc -I. --python_out=. --grpc_python_out=. grpc_service.proto

# For this implementation, we'll create the service manually
# In production, use the auto-generated code

import sys
import os
sys.path.append(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))

from inference.engine import FinancialLLMInferenceEngine, InferenceConfig


# Configure logging
logging.basicConfig(level=logging.INFO)
logger = logging.getLogger(__name__)


# Mock proto messages (in production, use auto-generated from .proto)
class ClassifyRequest:
    def __init__(self, text="", return_probabilities=True):
        self.text = text
        self.return_probabilities = return_probabilities


class ClassifyResponse:
    def __init__(self, sentiment=None, latency_ms=0.0, timestamp=""):
        self.sentiment = sentiment or {}
        self.latency_ms = latency_ms
        self.timestamp = timestamp


class GenerateRequest:
    def __init__(self, prompt="", max_new_tokens=100, temperature=0.8, top_k=50, top_p=0.95):
        self.prompt = prompt
        self.max_new_tokens = max_new_tokens
        self.temperature = temperature
        self.top_k = top_k
        self.top_p = top_p


class GenerateResponse:
    def __init__(self, text="", num_tokens=0, latency_ms=0.0, tokens_per_second=0.0, timestamp=""):
        self.text = text
        self.num_tokens = num_tokens
        self.latency_ms = latency_ms
        self.tokens_per_second = tokens_per_second
        self.timestamp = timestamp


class HealthCheckRequest:
    pass


class HealthCheckResponse:
    def __init__(self, status="", version="", model_loaded=False, uptime_seconds=0.0):
        self.status = status
        self.version = version
        self.model_loaded = model_loaded
        self.uptime_seconds = uptime_seconds


class FinancialLLMServicer:
    """gRPC service implementation for Financial LLM"""

    def __init__(self, engine: FinancialLLMInferenceEngine):
        self.engine = engine
        self.start_time = time.time()
        self.request_count = 0
        logger.info("FinancialLLM gRPC service initialized")

    def Classify(self, request, context):
        """
        Classify text (ultra-low latency endpoint)

        Target latency: < 5ms for local requests
        """
        self.request_count += 1
        start_time = time.perf_counter()

        try:
            # Validate request
            if not request.text:
                context.set_code(grpc.StatusCode.INVALID_ARGUMENT)
                context.set_details("Text field is required")
                return ClassifyResponse()

            # Perform inference
            result = self.engine.classify(request.text)

            # Create response
            response = ClassifyResponse(
                sentiment=result,
                latency_ms=result.get('latency_ms', 0.0),
                timestamp=datetime.utcnow().isoformat()
            )

            total_latency_ms = (time.perf_counter() - start_time) * 1000
            logger.debug(f"Classify request completed in {total_latency_ms:.2f}ms")

            return response

        except Exception as e:
            logger.error(f"Classification error: {e}", exc_info=True)
            context.set_code(grpc.StatusCode.INTERNAL)
            context.set_details(f"Classification failed: {str(e)}")
            return ClassifyResponse()

    def Generate(self, request, context):
        """Generate text from prompt"""
        self.request_count += 1
        start_time = time.perf_counter()

        try:
            if not request.prompt:
                context.set_code(grpc.StatusCode.INVALID_ARGUMENT)
                context.set_details("Prompt field is required")
                return GenerateResponse()

            # Perform generation
            result = self.engine.generate(
                prompt=request.prompt,
                max_new_tokens=request.max_new_tokens or 100,
                temperature=request.temperature or 0.8,
                top_k=request.top_k or 50,
                top_p=request.top_p or 0.95
            )

            response = GenerateResponse(
                text=result['text'],
                num_tokens=result['num_tokens'],
                latency_ms=result['latency_ms'],
                tokens_per_second=result['tokens_per_second'],
                timestamp=datetime.utcnow().isoformat()
            )

            total_latency_ms = (time.perf_counter() - start_time) * 1000
            logger.debug(f"Generate request completed in {total_latency_ms:.2f}ms")

            return response

        except Exception as e:
            logger.error(f"Generation error: {e}", exc_info=True)
            context.set_code(grpc.StatusCode.INTERNAL)
            context.set_details(f"Generation failed: {str(e)}")
            return GenerateResponse()

    def HealthCheck(self, request, context):
        """Health check endpoint"""
        uptime = time.time() - self.start_time

        response = HealthCheckResponse(
            status="healthy",
            version="1.0.0",
            model_loaded=True,
            uptime_seconds=uptime
        )

        return response


class FinancialLLMgRPCServer:
    """gRPC Server for Financial LLM"""

    def __init__(self, engine: FinancialLLMInferenceEngine,
                 host: str = "0.0.0.0",
                 port: int = 50051,
                 max_workers: int = 10,
                 max_concurrent_rpcs: int = 100):
        self.engine = engine
        self.host = host
        self.port = port
        self.max_workers = max_workers
        self.max_concurrent_rpcs = max_concurrent_rpcs

        # Create gRPC server with optimized settings
        self.server = grpc.server(
            futures.ThreadPoolExecutor(max_workers=max_workers),
            options=[
                ('grpc.max_send_message_length', 50 * 1024 * 1024),  # 50MB
                ('grpc.max_receive_message_length', 50 * 1024 * 1024),  # 50MB
                ('grpc.max_concurrent_streams', max_concurrent_rpcs),
                ('grpc.so_reuseport', 1),
                ('grpc.use_local_subchannel_pool', 1),
                # Performance optimizations
                ('grpc.keepalive_time_ms', 10000),
                ('grpc.keepalive_timeout_ms', 5000),
                ('grpc.http2.min_time_between_pings_ms', 10000),
                ('grpc.http2.max_pings_without_data', 0),
            ]
        )

        # Add service
        self.servicer = FinancialLLMServicer(engine)

        # In production, use the generated code:
        # financial_llm_pb2_grpc.add_FinancialLLMServicer_to_server(self.servicer, self.server)

        # Add insecure port
        self.server.add_insecure_port(f'{host}:{port}')

        logger.info(f"gRPC server configured on {host}:{port}")

    def start(self):
        """Start the gRPC server"""
        self.server.start()
        logger.info(f"gRPC server started on {self.host}:{self.port}")
        logger.info(f"Max workers: {self.max_workers}, Max concurrent RPCs: {self.max_concurrent_rpcs}")

    def stop(self, grace_period: int = 5):
        """Stop the gRPC server"""
        logger.info("Stopping gRPC server...")
        self.server.stop(grace_period)
        logger.info("gRPC server stopped")

    def wait_for_termination(self):
        """Wait for server termination"""
        try:
            self.server.wait_for_termination()
        except KeyboardInterrupt:
            logger.info("Received keyboard interrupt")
            self.stop()


def run_grpc_server(
    model_path: str,
    tokenizer_path: str,
    host: str = "0.0.0.0",
    port: int = 50051,
    max_workers: int = 10,
    device: str = "cuda"
):
    """
    Run the gRPC server

    Args:
        model_path: Path to model checkpoint
        tokenizer_path: Path to tokenizer
        host: Server host
        port: Server port
        max_workers: Number of worker threads
        device: Device for inference
    """
    logger.info("Initializing Financial LLM gRPC Server...")

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

    # Create and start server
    server = FinancialLLMgRPCServer(
        engine=engine,
        host=host,
        port=port,
        max_workers=max_workers
    )

    server.start()

    logger.info("="*60)
    logger.info("Financial LLM gRPC Server Ready")
    logger.info(f"Address: {host}:{port}")
    logger.info(f"Device: {device}")
    logger.info("="*60)

    # Wait for termination
    server.wait_for_termination()


# Client example for testing
class FinancialLLMgRPCClient:
    """gRPC Client for Financial LLM"""

    def __init__(self, host: str = "localhost", port: int = 50051):
        self.channel = grpc.insecure_channel(
            f'{host}:{port}',
            options=[
                ('grpc.max_send_message_length', 50 * 1024 * 1024),
                ('grpc.max_receive_message_length', 50 * 1024 * 1024),
            ]
        )

        # In production, use the generated stub:
        # self.stub = financial_llm_pb2_grpc.FinancialLLMStub(self.channel)

        logger.info(f"gRPC client connected to {host}:{port}")

    def classify(self, text: str) -> dict:
        """Classify text"""
        request = ClassifyRequest(text=text, return_probabilities=True)

        start_time = time.perf_counter()

        # In production:
        # response = self.stub.Classify(request)

        # Mock response for now
        response = {
            'sentiment': {'positive': 0.8, 'negative': 0.2},
            'latency_ms': (time.perf_counter() - start_time) * 1000
        }

        return response

    def generate(self, prompt: str, max_new_tokens: int = 100,
                 temperature: float = 0.8) -> dict:
        """Generate text"""
        request = GenerateRequest(
            prompt=prompt,
            max_new_tokens=max_new_tokens,
            temperature=temperature
        )

        # In production:
        # response = self.stub.Generate(request)

        response = {
            'text': 'Generated text here...',
            'num_tokens': max_new_tokens
        }

        return response

    def health_check(self) -> dict:
        """Check server health"""
        # In production:
        # response = self.stub.HealthCheck(HealthCheckRequest())

        return {
            'status': 'healthy',
            'version': '1.0.0'
        }

    def close(self):
        """Close client connection"""
        self.channel.close()


if __name__ == "__main__":
    import argparse

    parser = argparse.ArgumentParser(description="Financial LLM gRPC Server")
    parser.add_argument("--model-path", type=str, required=True, help="Path to model checkpoint")
    parser.add_argument("--tokenizer-path", type=str, required=True, help="Path to tokenizer")
    parser.add_argument("--host", type=str, default="0.0.0.0", help="Server host")
    parser.add_argument("--port", type=int, default=50051, help="Server port")
    parser.add_argument("--workers", type=int, default=10, help="Number of workers")
    parser.add_argument("--device", type=str, default="cuda", help="Device (cuda/cpu)")

    args = parser.parse_args()

    run_grpc_server(
        model_path=args.model_path,
        tokenizer_path=args.tokenizer_path,
        host=args.host,
        port=args.port,
        max_workers=args.workers,
        device=args.device
    )
