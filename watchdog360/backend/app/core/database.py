"""
Database configuration and session management
"""
from sqlalchemy.ext.asyncio import AsyncSession, create_async_engine, async_sessionmaker
from sqlalchemy.orm import declarative_base
from sqlalchemy.pool import NullPool

from .config import settings

# Create async engine
engine = create_async_engine(
    settings.DATABASE_URL,
    echo=settings.DEBUG,
    poolclass=NullPool,
    pool_pre_ping=True,
)

# Create async session maker
AsyncSessionLocal = async_sessionmaker(
    engine,
    class_=AsyncSession,
    expire_on_commit=False,
    autocommit=False,
    autoflush=False,
)

# Base class for models
Base = declarative_base()


async def get_db() -> AsyncSession:
    """Get database session"""
    async with AsyncSessionLocal() as session:
        try:
            yield session
            await session.commit()
        except Exception:
            await session.rollback()
            raise
        finally:
            await session.close()


async def init_db():
    """Initialize database and create tables"""
    async with engine.begin() as conn:
        # Create TimescaleDB extension
        if settings.TIMESCALEDB_ENABLED:
            await conn.execute("CREATE EXTENSION IF NOT EXISTS timescaledb CASCADE;")

        # Create all tables
        await conn.run_sync(Base.metadata.create_all)

        # Convert metrics table to hypertable
        if settings.TIMESCALEDB_ENABLED:
            try:
                await conn.execute(
                    "SELECT create_hypertable('server_metrics', 'timestamp', if_not_exists => TRUE);"
                )
            except Exception as e:
                print(f"Note: {e}")  # Table might already be a hypertable
