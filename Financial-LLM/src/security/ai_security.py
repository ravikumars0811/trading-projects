"""
AI-Powered Security Layer for Financial LLM
Advanced security features using machine learning
"""

import hashlib
import hmac
import time
import jwt
import redis
import numpy as np
from typing import Dict, List, Optional, Tuple
from dataclasses import dataclass, field
from datetime import datetime, timedelta
from collections import defaultdict, deque
import threading
import logging
from enum import Enum

# ML libraries
from sklearn.ensemble import IsolationForest
from sklearn.preprocessing import StandardScaler
import pickle


logging.basicConfig(level=logging.INFO)
logger = logging.getLogger(__name__)


class ThreatLevel(Enum):
    """Threat severity levels"""
    LOW = 1
    MEDIUM = 2
    HIGH = 3
    CRITICAL = 4


@dataclass
class SecurityConfig:
    """Security configuration"""
    # JWT
    jwt_secret_key: str = "change-this-secret-key-in-production"
    jwt_algorithm: str = "HS256"
    jwt_expiration_hours: int = 24

    # Rate limiting
    enable_rate_limiting: bool = True
    rate_limit_requests_per_minute: int = 100
    rate_limit_requests_per_hour: int = 1000
    rate_limit_burst_size: int = 10

    # AI-based threat detection
    enable_ai_threat_detection: bool = True
    anomaly_detection_threshold: float = 0.5
    min_requests_for_detection: int = 100

    # IP filtering
    enable_ip_whitelist: bool = False
    whitelisted_ips: List[str] = field(default_factory=list)
    blacklisted_ips: List[str] = field(default_factory=list)

    # Redis for distributed rate limiting
    redis_host: str = "localhost"
    redis_port: int = 6379
    redis_db: int = 0

    # API key management
    enable_api_key_auth: bool = True
    api_key_header: str = "X-API-Key"


class APIKeyManager:
    """Manage API keys for authentication"""

    def __init__(self):
        self.api_keys: Dict[str, Dict] = {}
        self.lock = threading.Lock()

    def create_key(self, user_id: str, permissions: List[str],
                   rate_limit: Optional[int] = None,
                   expires_at: Optional[datetime] = None) -> str:
        """
        Create a new API key

        Args:
            user_id: User identifier
            permissions: List of allowed permissions
            rate_limit: Custom rate limit for this key
            expires_at: Expiration datetime

        Returns:
            Generated API key
        """
        # Generate secure random key
        timestamp = str(time.time()).encode()
        random_data = hashlib.sha256(timestamp).hexdigest()
        api_key = f"fllm_{random_data[:32]}"

        with self.lock:
            self.api_keys[api_key] = {
                'user_id': user_id,
                'permissions': permissions,
                'rate_limit': rate_limit,
                'created_at': datetime.utcnow(),
                'expires_at': expires_at,
                'is_active': True
            }

        logger.info(f"Created API key for user {user_id}")
        return api_key

    def validate_key(self, api_key: str) -> Tuple[bool, Optional[Dict]]:
        """
        Validate an API key

        Returns:
            (is_valid, key_info)
        """
        with self.lock:
            if api_key not in self.api_keys:
                return False, None

            key_info = self.api_keys[api_key]

            # Check if active
            if not key_info['is_active']:
                return False, None

            # Check expiration
            if key_info['expires_at'] and datetime.utcnow() > key_info['expires_at']:
                return False, None

            return True, key_info

    def revoke_key(self, api_key: str):
        """Revoke an API key"""
        with self.lock:
            if api_key in self.api_keys:
                self.api_keys[api_key]['is_active'] = False
                logger.info(f"Revoked API key: {api_key[:10]}...")


class JWTManager:
    """Manage JWT tokens for authentication"""

    def __init__(self, secret_key: str, algorithm: str = "HS256",
                 expiration_hours: int = 24):
        self.secret_key = secret_key
        self.algorithm = algorithm
        self.expiration_hours = expiration_hours

    def create_token(self, user_id: str, permissions: List[str],
                    custom_claims: Optional[Dict] = None) -> str:
        """
        Create a JWT token

        Args:
            user_id: User identifier
            permissions: List of permissions
            custom_claims: Additional claims to include

        Returns:
            JWT token
        """
        payload = {
            'user_id': user_id,
            'permissions': permissions,
            'exp': datetime.utcnow() + timedelta(hours=self.expiration_hours),
            'iat': datetime.utcnow(),
            'nbf': datetime.utcnow()
        }

        if custom_claims:
            payload.update(custom_claims)

        token = jwt.encode(payload, self.secret_key, algorithm=self.algorithm)
        return token

    def verify_token(self, token: str) -> Tuple[bool, Optional[Dict]]:
        """
        Verify a JWT token

        Returns:
            (is_valid, payload)
        """
        try:
            payload = jwt.decode(token, self.secret_key, algorithms=[self.algorithm])
            return True, payload
        except jwt.ExpiredSignatureError:
            logger.warning("Token expired")
            return False, None
        except jwt.InvalidTokenError as e:
            logger.warning(f"Invalid token: {e}")
            return False, None


class RateLimiter:
    """
    Advanced rate limiter with multiple strategies

    Implements:
    - Token bucket algorithm
    - Sliding window
    - Distributed rate limiting via Redis
    """

    def __init__(self, config: SecurityConfig, use_redis: bool = False):
        self.config = config
        self.use_redis = use_redis

        if use_redis:
            try:
                self.redis_client = redis.Redis(
                    host=config.redis_host,
                    port=config.redis_port,
                    db=config.redis_db,
                    decode_responses=True
                )
                self.redis_client.ping()
                logger.info("Connected to Redis for distributed rate limiting")
            except Exception as e:
                logger.error(f"Redis connection failed: {e}")
                self.use_redis = False
                self.redis_client = None

        # Local rate limiting (fallback or standalone)
        self.buckets: Dict[str, Dict] = defaultdict(lambda: {
            'tokens': config.rate_limit_burst_size,
            'last_update': time.time(),
            'request_times': deque(maxlen=1000)
        })
        self.lock = threading.Lock()

    def _refill_tokens(self, bucket: Dict, current_time: float):
        """Refill tokens based on elapsed time"""
        elapsed = current_time - bucket['last_update']
        tokens_to_add = elapsed * (self.config.rate_limit_requests_per_minute / 60.0)
        bucket['tokens'] = min(
            self.config.rate_limit_burst_size,
            bucket['tokens'] + tokens_to_add
        )
        bucket['last_update'] = current_time

    def check_rate_limit(self, identifier: str) -> Tuple[bool, Dict[str, any]]:
        """
        Check if request is within rate limit

        Args:
            identifier: User/IP identifier

        Returns:
            (is_allowed, metadata)
        """
        if not self.config.enable_rate_limiting:
            return True, {}

        current_time = time.time()

        if self.use_redis:
            return self._check_rate_limit_redis(identifier, current_time)
        else:
            return self._check_rate_limit_local(identifier, current_time)

    def _check_rate_limit_local(self, identifier: str, current_time: float) -> Tuple[bool, Dict]:
        """Local rate limiting implementation"""
        with self.lock:
            bucket = self.buckets[identifier]

            # Refill tokens
            self._refill_tokens(bucket, current_time)

            # Check sliding window
            bucket['request_times'].append(current_time)

            # Count requests in last minute
            minute_ago = current_time - 60
            requests_last_minute = sum(1 for t in bucket['request_times'] if t > minute_ago)

            # Count requests in last hour
            hour_ago = current_time - 3600
            requests_last_hour = sum(1 for t in bucket['request_times'] if t > hour_ago)

            # Check limits
            if bucket['tokens'] < 1:
                return False, {
                    'reason': 'burst_limit_exceeded',
                    'retry_after': 1.0 / (self.config.rate_limit_requests_per_minute / 60.0)
                }

            if requests_last_minute >= self.config.rate_limit_requests_per_minute:
                return False, {
                    'reason': 'minute_limit_exceeded',
                    'retry_after': 60
                }

            if requests_last_hour >= self.config.rate_limit_requests_per_hour:
                return False, {
                    'reason': 'hour_limit_exceeded',
                    'retry_after': 3600
                }

            # Consume token
            bucket['tokens'] -= 1

            return True, {
                'remaining_tokens': bucket['tokens'],
                'requests_last_minute': requests_last_minute,
                'requests_last_hour': requests_last_hour
            }

    def _check_rate_limit_redis(self, identifier: str, current_time: float) -> Tuple[bool, Dict]:
        """Distributed rate limiting using Redis"""
        try:
            # Use Redis for distributed rate limiting
            minute_key = f"ratelimit:{identifier}:minute"
            hour_key = f"ratelimit:{identifier}:hour"

            # Increment counters
            pipe = self.redis_client.pipeline()
            pipe.incr(minute_key)
            pipe.expire(minute_key, 60)
            pipe.incr(hour_key)
            pipe.expire(hour_key, 3600)
            results = pipe.execute()

            requests_last_minute = results[0]
            requests_last_hour = results[2]

            # Check limits
            if requests_last_minute > self.config.rate_limit_requests_per_minute:
                return False, {'reason': 'minute_limit_exceeded', 'retry_after': 60}

            if requests_last_hour > self.config.rate_limit_requests_per_hour:
                return False, {'reason': 'hour_limit_exceeded', 'retry_after': 3600}

            return True, {
                'requests_last_minute': requests_last_minute,
                'requests_last_hour': requests_last_hour
            }

        except Exception as e:
            logger.error(f"Redis rate limiting error: {e}")
            # Fallback to local
            return self._check_rate_limit_local(identifier, current_time)


class AIThreatDetector:
    """
    AI-powered threat detection system

    Uses machine learning to detect:
    - Anomalous request patterns
    - Potential DDoS attacks
    - Suspicious API usage
    - Data exfiltration attempts
    """

    def __init__(self, config: SecurityConfig):
        self.config = config
        self.request_features: Dict[str, List[List[float]]] = defaultdict(list)
        self.models: Dict[str, IsolationForest] = {}
        self.scalers: Dict[str, StandardScaler] = {}
        self.lock = threading.Lock()

        # Initialize model
        self.global_model = IsolationForest(
            contamination=config.anomaly_detection_threshold,
            random_state=42,
            n_estimators=100
        )
        self.global_scaler = StandardScaler()
        self.is_trained = False

    def extract_features(self, request_data: Dict) -> List[float]:
        """
        Extract features from request for anomaly detection

        Features:
        - Request size
        - Time of day
        - Request frequency
        - Response time
        - Error rate
        """
        features = []

        # Request size
        features.append(request_data.get('request_size', 0))

        # Time features
        now = datetime.utcnow()
        features.append(now.hour)
        features.append(now.weekday())

        # Frequency features
        features.append(request_data.get('requests_last_minute', 0))
        features.append(request_data.get('requests_last_hour', 0))

        # Performance features
        features.append(request_data.get('response_time_ms', 0))

        # Error rate
        features.append(request_data.get('error_count', 0))

        # Request type encoding
        request_type = request_data.get('request_type', 'unknown')
        type_encoding = hash(request_type) % 100
        features.append(type_encoding)

        return features

    def record_request(self, identifier: str, request_data: Dict):
        """Record request for training and detection"""
        if not self.config.enable_ai_threat_detection:
            return

        features = self.extract_features(request_data)

        with self.lock:
            self.request_features[identifier].append(features)

            # Global features
            if 'global' not in self.request_features:
                self.request_features['global'] = []
            self.request_features['global'].append(features)

            # Retrain periodically
            if len(self.request_features['global']) % 1000 == 0:
                self._retrain_model()

    def _retrain_model(self):
        """Retrain anomaly detection model"""
        if len(self.request_features['global']) < self.config.min_requests_for_detection:
            return

        try:
            X = np.array(self.request_features['global'])

            # Normalize features
            X_scaled = self.global_scaler.fit_transform(X)

            # Train model
            self.global_model.fit(X_scaled)
            self.is_trained = True

            logger.info(f"Retrained anomaly detection model on {len(X)} samples")

        except Exception as e:
            logger.error(f"Model retraining failed: {e}")

    def detect_threat(self, identifier: str, request_data: Dict) -> Tuple[ThreatLevel, float, str]:
        """
        Detect potential threats using AI

        Returns:
            (threat_level, anomaly_score, description)
        """
        if not self.config.enable_ai_threat_detection or not self.is_trained:
            return ThreatLevel.LOW, 0.0, "Detection not active"

        features = self.extract_features(request_data)

        try:
            # Scale features
            features_scaled = self.global_scaler.transform([features])

            # Get anomaly score
            anomaly_score = self.global_model.score_samples(features_scaled)[0]
            is_anomaly = self.global_model.predict(features_scaled)[0] == -1

            # Determine threat level
            if is_anomaly:
                if anomaly_score < -0.5:
                    threat_level = ThreatLevel.CRITICAL
                    description = "Highly anomalous request pattern detected"
                elif anomaly_score < -0.3:
                    threat_level = ThreatLevel.HIGH
                    description = "Suspicious request pattern detected"
                else:
                    threat_level = ThreatLevel.MEDIUM
                    description = "Potentially anomalous request"
            else:
                threat_level = ThreatLevel.LOW
                description = "Normal request pattern"

            return threat_level, float(anomaly_score), description

        except Exception as e:
            logger.error(f"Threat detection error: {e}")
            return ThreatLevel.LOW, 0.0, "Detection error"

    def save_model(self, path: str):
        """Save trained model"""
        if not self.is_trained:
            logger.warning("No trained model to save")
            return

        model_data = {
            'model': self.global_model,
            'scaler': self.global_scaler,
            'is_trained': self.is_trained
        }

        with open(path, 'wb') as f:
            pickle.dump(model_data, f)

        logger.info(f"Threat detection model saved to {path}")

    def load_model(self, path: str):
        """Load trained model"""
        try:
            with open(path, 'rb') as f:
                model_data = pickle.load(f)

            self.global_model = model_data['model']
            self.global_scaler = model_data['scaler']
            self.is_trained = model_data['is_trained']

            logger.info(f"Threat detection model loaded from {path}")

        except Exception as e:
            logger.error(f"Model loading failed: {e}")


class SecurityManager:
    """
    Comprehensive security manager

    Coordinates:
    - Authentication (API keys, JWT)
    - Rate limiting
    - AI threat detection
    - IP filtering
    """

    def __init__(self, config: SecurityConfig):
        self.config = config

        # Components
        self.api_key_manager = APIKeyManager()
        self.jwt_manager = JWTManager(
            config.jwt_secret_key,
            config.jwt_algorithm,
            config.jwt_expiration_hours
        )
        self.rate_limiter = RateLimiter(config)
        self.threat_detector = AIThreatDetector(config)

        # Statistics
        self.blocked_requests = 0
        self.total_requests = 0
        self.threat_counts = defaultdict(int)

        logger.info("Security manager initialized")

    def authenticate(self, api_key: Optional[str] = None,
                    jwt_token: Optional[str] = None) -> Tuple[bool, Optional[Dict], str]:
        """
        Authenticate request

        Returns:
            (is_authenticated, user_info, error_message)
        """
        # Try API key
        if api_key:
            is_valid, key_info = self.api_key_manager.validate_key(api_key)
            if is_valid:
                return True, key_info, ""
            return False, None, "Invalid API key"

        # Try JWT
        if jwt_token:
            is_valid, payload = self.jwt_manager.verify_token(jwt_token)
            if is_valid:
                return True, payload, ""
            return False, None, "Invalid or expired token"

        return False, None, "No authentication provided"

    def check_request(self, identifier: str, request_data: Dict,
                     api_key: Optional[str] = None,
                     jwt_token: Optional[str] = None,
                     ip_address: Optional[str] = None) -> Tuple[bool, Dict]:
        """
        Complete security check for a request

        Returns:
            (is_allowed, metadata)
        """
        self.total_requests += 1
        metadata = {}

        # IP filtering
        if ip_address:
            if ip_address in self.config.blacklisted_ips:
                self.blocked_requests += 1
                return False, {'reason': 'ip_blacklisted'}

            if self.config.enable_ip_whitelist:
                if ip_address not in self.config.whitelisted_ips:
                    self.blocked_requests += 1
                    return False, {'reason': 'ip_not_whitelisted'}

        # Authentication
        if self.config.enable_api_key_auth:
            is_authenticated, user_info, error = self.authenticate(api_key, jwt_token)
            if not is_authenticated:
                self.blocked_requests += 1
                return False, {'reason': 'authentication_failed', 'error': error}

            metadata['user_info'] = user_info

        # Rate limiting
        is_allowed, rate_limit_info = self.rate_limiter.check_rate_limit(identifier)
        if not is_allowed:
            self.blocked_requests += 1
            return False, {'reason': 'rate_limit_exceeded', **rate_limit_info}

        metadata.update(rate_limit_info)

        # AI threat detection
        self.threat_detector.record_request(identifier, request_data)
        threat_level, anomaly_score, description = self.threat_detector.detect_threat(
            identifier, request_data
        )

        metadata['threat_level'] = threat_level.name
        metadata['anomaly_score'] = anomaly_score
        metadata['threat_description'] = description

        # Block critical threats
        if threat_level == ThreatLevel.CRITICAL:
            self.blocked_requests += 1
            self.threat_counts[threat_level] += 1
            return False, {'reason': 'threat_detected', **metadata}

        # Log high threats
        if threat_level == ThreatLevel.HIGH:
            self.threat_counts[threat_level] += 1
            logger.warning(f"High threat detected for {identifier}: {description}")

        return True, metadata

    def get_statistics(self) -> Dict:
        """Get security statistics"""
        return {
            'total_requests': self.total_requests,
            'blocked_requests': self.blocked_requests,
            'block_rate': self.blocked_requests / max(self.total_requests, 1),
            'threat_counts': dict(self.threat_counts)
        }


if __name__ == "__main__":
    # Example usage
    config = SecurityConfig(
        enable_rate_limiting=True,
        enable_ai_threat_detection=True,
        rate_limit_requests_per_minute=100
    )

    security_manager = SecurityManager(config)

    # Create API key
    api_key = security_manager.api_key_manager.create_key(
        user_id="trader_123",
        permissions=["read", "write"],
        rate_limit=200
    )

    print(f"Created API key: {api_key}")

    # Check request
    request_data = {
        'request_size': 1024,
        'requests_last_minute': 5,
        'requests_last_hour': 50,
        'response_time_ms': 15.5,
        'error_count': 0,
        'request_type': 'classify'
    }

    is_allowed, metadata = security_manager.check_request(
        identifier="user_123",
        request_data=request_data,
        api_key=api_key,
        ip_address="192.168.1.100"
    )

    print(f"Request allowed: {is_allowed}")
    print(f"Metadata: {metadata}")

    # Get statistics
    stats = security_manager.get_statistics()
    print(f"Security stats: {stats}")
