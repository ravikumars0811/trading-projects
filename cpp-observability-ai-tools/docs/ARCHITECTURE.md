# Architecture Documentation

## System Overview

The C++ Observability & AI Tools project is designed as a modular, layered architecture that provides comprehensive observability capabilities enhanced with AI-powered insights.

## Architecture Layers

```
┌─────────────────────────────────────────────────────────┐
│                   Application Layer                      │
│  (obs_server, examples, custom applications)            │
└─────────────────────────────────────────────────────────┘
                           │
┌─────────────────────────────────────────────────────────┐
│                    API/Interface Layer                   │
│  ┌────────────┐  ┌──────────────┐  ┌────────────────┐  │
│  │ MCP Server │  │ REST API     │  │ C++ Public API │  │
│  └────────────┘  └──────────────┘  └────────────────┘  │
└─────────────────────────────────────────────────────────┘
                           │
┌─────────────────────────────────────────────────────────┐
│                   Business Logic Layer                   │
│  ┌─────────────┐  ┌──────────┐  ┌──────────────────┐   │
│  │ Automation  │  │ AI       │  │ Analytics        │   │
│  │ Engine      │  │ Analyzer │  │ & Visualization  │   │
│  └─────────────┘  └──────────┘  └──────────────────┘   │
└─────────────────────────────────────────────────────────┘
                           │
┌─────────────────────────────────────────────────────────┐
│                Core Observability Layer                  │
│  ┌────────┐ ┌────────┐ ┌────────┐ ┌────────┐ ┌──────┐ │
│  │Metrics │ │Health  │ │Tracer  │ │Logger  │ │Alert │ │
│  │Collector│ │Monitor │ │        │ │        │ │Mgr   │ │
│  └────────┘ └────────┘ └────────┘ └────────┘ └──────┘ │
└─────────────────────────────────────────────────────────┘
                           │
┌─────────────────────────────────────────────────────────┐
│                  Infrastructure Layer                    │
│  ┌──────────────┐  ┌────────────┐  ┌────────────────┐  │
│  │ Thread Pool  │  │ Async      │  │ Data Stores    │  │
│  │              │  │ Executor   │  │ (In-Memory)    │  │
│  └──────────────┘  └────────────┘  └────────────────┘  │
└─────────────────────────────────────────────────────────┘
```

## Core Components

### 1. Metrics Collector

**Purpose**: Collect and manage application metrics

**Design Patterns**:
- Singleton pattern for global access
- Thread-safe with mutex protection
- Bounded buffer to prevent memory leaks

**Key Features**:
- Support for multiple metric types (counter, gauge, histogram, timer)
- Label-based metrics for multi-dimensional data
- Multiple export formats (JSON, Prometheus)
- RAII timer support

**Thread Safety**: All operations are protected by mutex

**Performance**: Lock-based, optimized for writes

### 2. Health Monitor

**Purpose**: Track system health across components

**Design Patterns**:
- Singleton pattern
- Observer pattern for health checks
- Background thread for periodic checks

**Key Features**:
- Pluggable health check registration
- Configurable check intervals
- Overall health status aggregation
- Automatic background monitoring

**Thread Safety**: Mutex-protected state

### 3. Distributed Tracer

**Purpose**: Implement distributed tracing

**Design Patterns**:
- Singleton pattern
- Span context propagation
- RAII span management

**Key Features**:
- Trace ID and span ID generation
- Parent-child span relationships
- Tag and log attachment
- Sampling support
- Jaeger-compatible export

**Thread Safety**: Mutex-protected span storage

### 4. Logger

**Purpose**: Structured logging with multiple outputs

**Design Patterns**:
- Singleton pattern
- Strategy pattern for formatters

**Key Features**:
- Multiple log levels
- Console and file output
- JSON formatting
- Context attachment
- Recent log buffering

**Thread Safety**: Mutex-protected writes

### 5. Alert Manager

**Purpose**: Manage alerts and notifications

**Design Patterns**:
- Singleton pattern
- Observer pattern for handlers
- Rule engine pattern

**Key Features**:
- Alert severity levels
- Pluggable alert handlers
- Rule-based alerting
- Alert lifecycle management
- Background rule evaluation

**Thread Safety**: Mutex-protected alert state

### 6. MCP Server

**Purpose**: Expose observability data via Model Context Protocol

**Design Patterns**:
- Singleton pattern
- Command pattern for tools
- Server-client architecture

**Key Features**:
- Tool registration system
- JSON-RPC-like protocol
- Built-in observability tools
- Extensible tool framework
- Multi-client support

**Thread Safety**: Mutex-protected tool registry, per-client threads

### 7. AI Analyzer

**Purpose**: AI-powered analysis of observability data

**Design Patterns**:
- Strategy pattern for analysis types
- Adapter pattern for LLM integration

**Key Features**:
- Anomaly detection
- Health insight generation
- Performance analysis
- Log pattern analysis
- Natural language Q&A

**Dependencies**: LLM Client (OpenAI API)

### 8. Workflow Engine

**Purpose**: Execute automated workflows

**Design Patterns**:
- Composite pattern for workflows
- Command pattern for tasks
- State pattern for execution

**Key Features**:
- Sequential and parallel execution
- Task dependencies
- Workflow lifecycle management
- AI-assisted workflow generation

**Thread Safety**: Mutex-protected workflow state

### 9. Visualizer

**Purpose**: Generate visualizations of observability data

**Design Patterns**:
- Singleton pattern
- Builder pattern for complex visualizations

**Key Features**:
- ASCII charts (sparklines, bar charts, histograms, heatmaps)
- HTML dashboard generation
- Multiple export formats
- Time series plotting

## Data Flow

### Metric Collection Flow

```
Application Code
      │
      ▼
MetricsCollector::increment()
      │
      ▼
[Mutex Lock]
      │
      ▼
Store Metric + Metadata
      │
      ▼
[Mutex Unlock]
      │
      ▼
Query/Export
      │
      ├──► Prometheus Format
      ├──► JSON Format
      └──► MCP Tools
```

### Health Monitoring Flow

```
Register Health Check
      │
      ▼
Background Thread Loop
      │
      ▼
Check Interval Elapsed?
      │
      ▼
Execute Health Check Function
      │
      ▼
Store Result
      │
      ▼
Update Overall Status
      │
      └──► MCP Tools / API
```

### AI Analysis Flow

```
Observability Data
      │
      ▼
AIAnalyzer::analyze_*()
      │
      ▼
Format Data for LLM
      │
      ▼
LLMClient::chat()
      │
      ▼
OpenAI API Request
      │
      ▼
Parse LLM Response
      │
      ▼
Return Structured Insights
```

### MCP Request Flow

```
AI Assistant (ChatGPT/Claude)
      │
      ▼
HTTP/Socket Request
      │
      ▼
MCP Server
      │
      ▼
Parse JSON Request
      │
      ▼
Route to Tool Handler
      │
      ├──► get_health_status
      ├──► query_metrics
      ├──► query_traces
      └──► query_logs
      │
      ▼
Execute Tool
      │
      ▼
Format JSON Response
      │
      ▼
Return to AI Assistant
```

## Concurrency Model

### Thread Pool Architecture

```
AsyncExecutor (Singleton)
      │
      ▼
ThreadPool (N worker threads)
      │
      ├──► Worker Thread 1
      ├──► Worker Thread 2
      ├──► Worker Thread 3
      └──► Worker Thread N
      │
      ▼
Task Queue (Thread-safe)
      │
      ▼
Tasks Executed via futures
```

### Background Threads

1. **Health Monitor Thread**: Periodic health check execution
2. **Alert Manager Thread**: Rule evaluation loop
3. **MCP Server Threads**: Accept thread + per-client threads
4. **Metrics Thread** (in main): Simulated metric generation

## Memory Management

### Bounded Buffers

- **Metrics**: Max 10,000 metrics (configurable)
- **Traces**: Max 10,000 spans (configurable)
- **Logs**: Max 1,000 recent entries (configurable)
- **Alerts**: Unbounded active, bounded history

### Automatic Cleanup

- Oldest entries removed when limits exceeded
- RAII patterns ensure resource cleanup
- No manual memory management required

## Extensibility Points

### 1. Custom MCP Tools

```cpp
obs::mcp::MCPTool custom_tool;
custom_tool.name = "my_tool";
custom_tool.handler = [](const nlohmann::json& params) {
    // Custom logic
    return response;
};
mcp.register_tool(custom_tool);
```

### 2. Custom Health Checks

```cpp
health.register_check("my_check", "component", []() {
    // Custom health logic
    return check_result;
});
```

### 3. Custom Alert Handlers

```cpp
alert_mgr.register_handler("my_handler", [](const Alert& alert) {
    // Custom alert handling
});
```

### 4. Custom Workflows

```cpp
auto workflow_id = engine.create_workflow("my_workflow");
engine.add_task_to_workflow(workflow_id, "task", "desc", []() {
    // Custom task logic
    return true;
});
```

## Deployment Considerations

### Scaling

- **Vertical**: Increase thread pool size
- **Horizontal**: Deploy multiple instances with shared backend
- **Distributed**: Implement remote metrics aggregation

### Performance Tuning

- Adjust buffer sizes based on traffic
- Configure health check intervals
- Tune thread pool size
- Enable sampling for high-volume traces

### Monitoring the Monitor

- Use metrics to track observability system performance
- Monitor memory usage and queue sizes
- Track API latency and error rates

## Security Considerations

1. **API Authentication**: Not implemented (add API keys/tokens)
2. **TLS/SSL**: Not implemented (add for production)
3. **Input Validation**: Basic validation present
4. **Rate Limiting**: Not implemented (consider for MCP server)
5. **Access Control**: Not implemented (add RBAC for sensitive data)

## Future Enhancements

1. **Persistent Storage**: Add database backend for metrics/traces
2. **Distributed Tracing**: Support for cross-service traces
3. **Metric Aggregation**: Time-series database integration
4. **Advanced AI**: More sophisticated anomaly detection
5. **UI Dashboard**: Web-based interactive dashboard
6. **Plugin System**: Dynamic library loading for extensions
7. **gRPC Support**: In addition to JSON-RPC
8. **Kubernetes Integration**: Service discovery and deployment

## Dependencies

### External Libraries

- **nlohmann/json**: JSON parsing and serialization
- **libcurl**: HTTP client for LLM API
- **OpenSSL**: TLS support for HTTPS
- **pthreads**: POSIX threads

### Optional Dependencies

- **Prometheus C++ client**: For Prometheus integration
- **Jaeger C++ client**: For Jaeger tracing
- **gRPC**: For gRPC-based MCP server

## Testing Strategy

### Unit Tests

- Test individual components in isolation
- Mock dependencies where needed
- Focus on core functionality

### Integration Tests

- Test component interactions
- Verify data flow between layers
- Test MCP server with real clients

### Performance Tests

- Benchmark metric collection rate
- Test concurrent health checks
- Measure memory usage under load

### AI Tests

- Test with mock LLM responses
- Verify parsing of AI outputs
- Test error handling

---

This architecture is designed for production use with emphasis on performance, reliability, and extensibility.
