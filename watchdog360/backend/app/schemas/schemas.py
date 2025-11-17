"""
Pydantic schemas for request/response validation
"""
from datetime import datetime
from typing import Optional, List, Dict, Any
from pydantic import BaseModel, EmailStr, Field


# User schemas
class UserBase(BaseModel):
    email: EmailStr
    full_name: Optional[str] = None


class UserCreate(UserBase):
    password: str


class UserLogin(BaseModel):
    email: EmailStr
    password: str


class User(UserBase):
    id: int
    is_active: bool
    created_at: datetime

    class Config:
        from_attributes = True


class Token(BaseModel):
    access_token: str
    token_type: str = "bearer"


# Server schemas
class ServerBase(BaseModel):
    hostname: str
    ip_address: Optional[str] = None
    os_type: Optional[str] = None
    os_version: Optional[str] = None


class ServerCreate(ServerBase):
    pass


class Server(ServerBase):
    id: int
    user_id: int
    is_active: bool
    created_at: datetime
    last_seen_at: Optional[datetime] = None

    class Config:
        from_attributes = True


# Metrics schemas
class CPUMetrics(BaseModel):
    cpu_percent: float
    cpu_count: int
    cpu_freq_current: Optional[float] = 0
    cpu_freq_min: Optional[float] = 0
    cpu_freq_max: Optional[float] = 0
    load_average: List[float] = [0, 0, 0]


class MemoryMetrics(BaseModel):
    memory_total: float
    memory_available: float
    memory_used: float
    memory_percent: float
    swap_total: float
    swap_used: float
    swap_percent: float


class DiskMetric(BaseModel):
    device: str
    mountpoint: str
    fstype: str
    total: float
    used: float
    free: float
    percent: float


class IOMetrics(BaseModel):
    read_count: float
    write_count: float
    read_bytes: float
    write_bytes: float
    read_time: Optional[float] = 0
    write_time: Optional[float] = 0


class NetworkMetrics(BaseModel):
    bytes_sent: float
    bytes_recv: float
    packets_sent: float
    packets_recv: float
    errin: Optional[float] = 0
    errout: Optional[float] = 0
    dropin: Optional[float] = 0
    dropout: Optional[float] = 0


class ProcessInfo(BaseModel):
    pid: int
    name: str
    cpu_percent: float
    memory_percent: float
    username: str


class MetricsData(BaseModel):
    timestamp: datetime
    hostname: str
    cpu: CPUMetrics
    memory: MemoryMetrics
    disk: List[DiskMetric]
    io: IOMetrics
    network: NetworkMetrics
    processes: List[ProcessInfo]


class MetricsResponse(BaseModel):
    status: str
    message: str
    server_id: Optional[int] = None


class ServerMetricResponse(BaseModel):
    id: int
    server_id: int
    timestamp: datetime
    cpu_percent: float
    memory_percent: float
    disk_usage_avg: Optional[float] = None
    anomaly_score: Optional[float] = None
    is_anomaly: bool

    class Config:
        from_attributes = True


# Alert schemas
class AlertBase(BaseModel):
    alert_type: str
    severity: str
    title: str
    message: Optional[str] = None
    metric_value: Optional[float] = None
    threshold_value: Optional[float] = None


class AlertCreate(AlertBase):
    server_id: int


class Alert(AlertBase):
    id: int
    server_id: int
    is_resolved: bool
    is_prediction: bool
    created_at: datetime
    resolved_at: Optional[datetime] = None
    predicted_at: Optional[datetime] = None

    class Config:
        from_attributes = True


# Dashboard schemas
class DashboardStats(BaseModel):
    total_servers: int
    active_servers: int
    critical_alerts: int
    warning_alerts: int
    total_metrics_today: int


class ServerStats(BaseModel):
    server: Server
    latest_metrics: Optional[ServerMetricResponse] = None
    cpu_trend: List[float] = []
    memory_trend: List[float] = []
    active_alerts: int = 0


class TimeSeriesData(BaseModel):
    timestamps: List[datetime]
    values: List[float]


class AnomalyDetectionResult(BaseModel):
    is_anomaly: bool
    anomaly_score: float
    explanation: str
    affected_metrics: List[str]


class PredictionResult(BaseModel):
    metric_name: str
    predicted_value: float
    predicted_at: datetime
    confidence: float
    trend: str  # increasing, decreasing, stable
