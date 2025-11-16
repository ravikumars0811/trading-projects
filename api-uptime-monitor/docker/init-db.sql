-- Initialize TimescaleDB extension
CREATE EXTENSION IF NOT EXISTS timescaledb;

-- Create hypertable for health_checks (optimized for time-series queries)
-- This will be run after tables are created by SQLAlchemy
-- We'll handle this in a migration script instead
