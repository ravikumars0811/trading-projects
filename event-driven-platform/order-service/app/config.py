from pydantic_settings import BaseSettings
from functools import lru_cache

class Settings(BaseSettings):
    SERVICE_NAME: str = "order-service"
    DATABASE_URL: str = "postgresql+asyncpg://admin:admin123@localhost:5432/microservices_db"
    REDIS_URL: str = "redis://localhost:6379"
    KAFKA_BOOTSTRAP_SERVERS: str = "localhost:9092"
    USER_SERVICE_URL: str = "http://user-service:8000"
    PRODUCT_SERVICE_URL: str = "http://product-service:8000"

    class Config:
        env_file = ".env"
        case_sensitive = True

@lru_cache()
def get_settings() -> Settings:
    return Settings()

settings = get_settings()
