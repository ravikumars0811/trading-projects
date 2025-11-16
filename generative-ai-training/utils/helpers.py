"""
Utility Helper Functions for AI-Powered Financial Applications

Common functions used across modules.
"""

import os
import time
import hashlib
import json
from typing import Any, Callable, Dict, List, Optional
from functools import wraps
from datetime import datetime, timedelta
import tiktoken


class TokenCounter:
    """Utility for counting and estimating token costs."""

    def __init__(self, model: str = "gpt-4"):
        """
        Initialize token counter.

        Args:
            model: Model name for tokenization
        """
        self.encoder = tiktoken.encoding_for_model(model)
        self.pricing = self._get_pricing()

    def _get_pricing(self) -> Dict[str, Dict[str, float]]:
        """Get current API pricing (update regularly!)."""
        return {
            "gpt-4": {"input": 0.03 / 1000, "output": 0.06 / 1000},
            "gpt-4-turbo-preview": {"input": 0.01 / 1000, "output": 0.03 / 1000},
            "gpt-3.5-turbo": {"input": 0.0015 / 1000, "output": 0.002 / 1000},
            "claude-3-opus": {"input": 0.015 / 1000, "output": 0.075 / 1000},
            "claude-3-sonnet": {"input": 0.003 / 1000, "output": 0.015 / 1000},
            "claude-3-haiku": {"input": 0.00025 / 1000, "output": 0.00125 / 1000},
        }

    def count_tokens(self, text: str) -> int:
        """Count tokens in text."""
        return len(self.encoder.encode(text))

    def estimate_cost(
        self,
        prompt: str,
        completion: str,
        model: str
    ) -> Dict[str, float]:
        """
        Estimate API call cost.

        Args:
            prompt: Input prompt
            completion: Model completion
            model: Model name

        Returns:
            Cost breakdown
        """
        prompt_tokens = self.count_tokens(prompt)
        completion_tokens = self.count_tokens(completion)

        pricing = self.pricing.get(model, self.pricing["gpt-4"])

        prompt_cost = prompt_tokens * pricing["input"]
        completion_cost = completion_tokens * pricing["output"]

        return {
            "prompt_tokens": prompt_tokens,
            "completion_tokens": completion_tokens,
            "total_tokens": prompt_tokens + completion_tokens,
            "prompt_cost": prompt_cost,
            "completion_cost": completion_cost,
            "total_cost": prompt_cost + completion_cost,
            "model": model
        }


class SimpleCache:
    """Simple in-memory cache with TTL."""

    def __init__(self, ttl_seconds: int = 3600):
        """
        Initialize cache.

        Args:
            ttl_seconds: Time to live in seconds
        """
        self.cache = {}
        self.ttl = ttl_seconds

    def get(self, key: str) -> Optional[Any]:
        """Get value from cache."""
        if key in self.cache:
            value, timestamp = self.cache[key]
            if time.time() - timestamp < self.ttl:
                return value
            else:
                del self.cache[key]
        return None

    def set(self, key: str, value: Any):
        """Set value in cache."""
        self.cache[key] = (value, time.time())

    def clear(self):
        """Clear all cache."""
        self.cache.clear()

    def get_stats(self) -> Dict[str, int]:
        """Get cache statistics."""
        return {
            "size": len(self.cache),
            "ttl_seconds": self.ttl
        }


def hash_text(text: str) -> str:
    """Generate hash of text for caching."""
    return hashlib.md5(text.encode()).hexdigest()


def retry_with_exponential_backoff(
    max_retries: int = 3,
    initial_delay: float = 1.0,
    exponential_base: float = 2.0,
    exceptions: tuple = (Exception,)
):
    """
    Decorator for retrying functions with exponential backoff.

    Args:
        max_retries: Maximum number of retries
        initial_delay: Initial delay in seconds
        exponential_base: Base for exponential backoff
        exceptions: Tuple of exceptions to catch

    Returns:
        Decorated function
    """
    def decorator(func: Callable) -> Callable:
        @wraps(func)
        def wrapper(*args, **kwargs):
            delay = initial_delay

            for attempt in range(max_retries):
                try:
                    return func(*args, **kwargs)
                except exceptions as e:
                    if attempt == max_retries - 1:
                        raise

                    print(f"Attempt {attempt + 1} failed: {e}")
                    print(f"Retrying in {delay:.1f}s...")
                    time.sleep(delay)
                    delay *= exponential_base

            return func(*args, **kwargs)

        return wrapper
    return decorator


def time_execution(func: Callable) -> Callable:
    """
    Decorator to time function execution.

    Args:
        func: Function to time

    Returns:
        Decorated function
    """
    @wraps(func)
    def wrapper(*args, **kwargs):
        start_time = time.time()
        result = func(*args, **kwargs)
        end_time = time.time()

        execution_time = (end_time - start_time) * 1000  # milliseconds

        # Add timing info to result if it's a dict
        if isinstance(result, dict):
            result["_execution_time_ms"] = execution_time
        else:
            print(f"{func.__name__} executed in {execution_time:.2f}ms")

        return result

    return wrapper


class PerformanceTracker:
    """Track API performance metrics."""

    def __init__(self):
        """Initialize tracker."""
        self.metrics = {
            "requests": [],
            "latencies": [],
            "costs": [],
            "errors": [],
        }

    def record_request(
        self,
        model: str,
        latency_ms: float,
        cost: float,
        success: bool = True,
        error: Optional[str] = None
    ):
        """Record API request metrics."""
        self.metrics["requests"].append({
            "model": model,
            "timestamp": datetime.now().isoformat(),
            "latency_ms": latency_ms,
            "cost": cost,
            "success": success,
            "error": error
        })

        if success:
            self.metrics["latencies"].append(latency_ms)
            self.metrics["costs"].append(cost)
        else:
            self.metrics["errors"].append(error)

    def get_summary(self) -> Dict[str, Any]:
        """Get performance summary."""
        if not self.metrics["requests"]:
            return {"message": "No requests recorded"}

        latencies = self.metrics["latencies"]
        costs = self.metrics["costs"]

        return {
            "total_requests": len(self.metrics["requests"]),
            "successful_requests": len(latencies),
            "failed_requests": len(self.metrics["errors"]),
            "success_rate": f"{(len(latencies) / len(self.metrics['requests']) * 100):.1f}%",
            "avg_latency_ms": sum(latencies) / len(latencies) if latencies else 0,
            "min_latency_ms": min(latencies) if latencies else 0,
            "max_latency_ms": max(latencies) if latencies else 0,
            "p95_latency_ms": sorted(latencies)[int(len(latencies) * 0.95)] if latencies else 0,
            "total_cost": sum(costs),
            "avg_cost_per_request": sum(costs) / len(costs) if costs else 0,
        }

    def reset(self):
        """Reset all metrics."""
        self.metrics = {
            "requests": [],
            "latencies": [],
            "costs": [],
            "errors": [],
        }


def format_financial_number(number: float, prefix: str = "$") -> str:
    """
    Format number for financial display.

    Args:
        number: Number to format
        prefix: Prefix (e.g., '$', '£')

    Returns:
        Formatted string
    """
    if abs(number) >= 1e9:
        return f"{prefix}{number/1e9:.2f}B"
    elif abs(number) >= 1e6:
        return f"{prefix}{number/1e6:.2f}M"
    elif abs(number) >= 1e3:
        return f"{prefix}{number/1e3:.2f}K"
    else:
        return f"{prefix}{number:.2f}"


def validate_api_keys() -> Dict[str, bool]:
    """
    Validate that required API keys are set.

    Returns:
        Dictionary of key availability
    """
    keys = {
        "OPENAI_API_KEY": bool(os.getenv("OPENAI_API_KEY")),
        "ANTHROPIC_API_KEY": bool(os.getenv("ANTHROPIC_API_KEY")),
        "ALPHA_VANTAGE_API_KEY": bool(os.getenv("ALPHA_VANTAGE_API_KEY")),
        "FINNHUB_API_KEY": bool(os.getenv("FINNHUB_API_KEY")),
    }

    return keys


def load_json_file(file_path: str) -> Dict[str, Any]:
    """
    Safely load JSON file.

    Args:
        file_path: Path to JSON file

    Returns:
        Parsed JSON data
    """
    try:
        with open(file_path, 'r') as f:
            return json.load(f)
    except FileNotFoundError:
        print(f"File not found: {file_path}")
        return {}
    except json.JSONDecodeError as e:
        print(f"Invalid JSON in {file_path}: {e}")
        return {}


def save_json_file(data: Dict[str, Any], file_path: str):
    """
    Safely save JSON file.

    Args:
        data: Data to save
        file_path: Output file path
    """
    try:
        with open(file_path, 'w') as f:
            json.dump(data, f, indent=2)
    except Exception as e:
        print(f"Error saving to {file_path}: {e}")


class RateLimiter:
    """Simple rate limiter for API calls."""

    def __init__(self, max_requests: int, time_window_seconds: int):
        """
        Initialize rate limiter.

        Args:
            max_requests: Maximum requests allowed
            time_window_seconds: Time window in seconds
        """
        self.max_requests = max_requests
        self.time_window = time_window_seconds
        self.requests = []

    def can_make_request(self) -> bool:
        """Check if request can be made."""
        now = time.time()

        # Remove old requests
        self.requests = [
            req_time for req_time in self.requests
            if now - req_time < self.time_window
        ]

        return len(self.requests) < self.max_requests

    def record_request(self):
        """Record a new request."""
        self.requests.append(time.time())

    def wait_if_needed(self):
        """Wait if rate limit is reached."""
        while not self.can_make_request():
            wait_time = 1.0  # Check every second
            print(f"Rate limit reached. Waiting {wait_time}s...")
            time.sleep(wait_time)

        self.record_request()


def main():
    """Test utility functions."""

    print("Testing Utility Functions")
    print("="*80)

    # Test token counter
    print("\n1. Token Counter")
    counter = TokenCounter()
    text = "The S&P 500 rose 2.3% on strong economic data."
    tokens = counter.count_tokens(text)
    print(f"Text: {text}")
    print(f"Tokens: {tokens}")

    cost = counter.estimate_cost(text, "This is positive for markets.", "gpt-4")
    print(f"Estimated cost: ${cost['total_cost']:.6f}")

    # Test cache
    print("\n2. Simple Cache")
    cache = SimpleCache(ttl_seconds=5)
    cache.set("test_key", "test_value")
    print(f"Cached value: {cache.get('test_key')}")
    time.sleep(6)
    print(f"After TTL: {cache.get('test_key')}")

    # Test retry decorator
    print("\n3. Retry Decorator")

    @retry_with_exponential_backoff(max_retries=3)
    def flaky_function(fail_count=2):
        if not hasattr(flaky_function, 'attempts'):
            flaky_function.attempts = 0

        flaky_function.attempts += 1
        if flaky_function.attempts <= fail_count:
            raise Exception("Simulated failure")
        return "Success!"

    try:
        result = flaky_function(fail_count=2)
        print(f"Result: {result}")
    except Exception as e:
        print(f"Error: {e}")

    # Test performance tracker
    print("\n4. Performance Tracker")
    tracker = PerformanceTracker()
    tracker.record_request("gpt-4", 150.5, 0.002, success=True)
    tracker.record_request("gpt-3.5", 50.2, 0.0005, success=True)
    tracker.record_request("gpt-4", 0, 0, success=False, error="Timeout")

    summary = tracker.get_summary()
    print("Performance Summary:")
    for key, value in summary.items():
        print(f"  {key}: {value}")

    # Test number formatting
    print("\n5. Financial Number Formatting")
    numbers = [1234.56, 1234567.89, 1234567890.12]
    for num in numbers:
        print(f"  {num} -> {format_financial_number(num)}")

    # Test API key validation
    print("\n6. API Key Validation")
    keys = validate_api_keys()
    for key, available in keys.items():
        status = "✓" if available else "✗"
        print(f"  {status} {key}")


if __name__ == "__main__":
    main()
