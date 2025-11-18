# API Reference

## Core Observability APIs

### MetricsCollector

```cpp
namespace obs {
    class MetricsCollector {
    public:
        static MetricsCollector& instance();

        // Increment a counter metric
        void increment(const std::string& name, double value = 1.0,
                      const std::unordered_map<std::string, std::string>& labels = {});

        // Set a gauge metric
        void set_gauge(const std::string& name, double value,
                      const std::unordered_map<std::string, std::string>& labels = {});

        // Record histogram value
        void record_histogram(const std::string& name, double value,
                             const std::unordered_map<std::string, std::string>& labels = {});

        // Timer operations
        void start_timer(const std::string& name);
        void stop_timer(const std::string& name,
                       const std::unordered_map<std::string, std::string>& labels = {});

        // Query operations
        std::vector<Metric> get_all_metrics() const;
        std::vector<Metric> get_metrics_by_name(const std::string& name) const;
        nlohmann::json export_json() const;
        nlohmann::json export_prometheus_format() const;

        // Statistics
        Statistics get_statistics() const;
        void reset();
    };

    // RAII Timer
    class ScopedTimer {
    public:
        explicit ScopedTimer(const std::string& name,
                            const std::unordered_map<std::string, std::string>& labels = {});
        ~ScopedTimer();
    };
}
```

### HealthMonitor

```cpp
namespace obs {
    class HealthMonitor {
    public:
        static HealthMonitor& instance();

        // Register health check
        void register_check(const std::string& name,
                           const std::string& component,
                           HealthCheckFunction check_fn,
                           std::chrono::seconds interval = std::chrono::seconds(30));

        // Manual execution
        HealthCheck execute_check(const std::string& name);
        std::vector<HealthCheck> execute_all_checks();

        // Query operations
        HealthStatus get_overall_status() const;
        std::vector<HealthCheck> get_all_checks() const;
        std::vector<HealthCheck> get_checks_by_component(const std::string& component) const;
        std::vector<HealthCheck> get_unhealthy_checks() const;

        // Export
        nlohmann::json export_json() const;

        // Background monitoring
        void start_monitoring();
        void stop_monitoring();
    };
}
```

### Tracer

```cpp
namespace obs {
    class Tracer {
    public:
        static Tracer& instance();

        // Span operations
        std::shared_ptr<Span> start_span(const std::string& operation_name,
                                         const std::string& parent_span_id = "");
        void finish_span(std::shared_ptr<Span> span);
        void add_tag(std::shared_ptr<Span> span, const std::string& key, const std::string& value);
        void add_log(std::shared_ptr<Span> span, const std::string& key, const std::string& value);

        // Query operations
        std::vector<Span> get_spans_by_trace_id(const std::string& trace_id) const;
        std::vector<Span> get_recent_spans(size_t limit = 100) const;
        nlohmann::json export_json() const;
        nlohmann::json export_jaeger_format() const;

        // Configuration
        void set_sampling_rate(double rate);
        void set_max_spans(size_t max_spans);
    };

    // RAII Span
    class ScopedSpan {
    public:
        explicit ScopedSpan(const std::string& operation_name,
                           const std::string& parent_span_id = "");
        ~ScopedSpan();

        void add_tag(const std::string& key, const std::string& value);
        void add_log(const std::string& key, const std::string& value);
        std::string get_trace_id() const;
        std::string get_span_id() const;
    };
}
```

### Logger

```cpp
namespace obs {
    class Logger {
    public:
        static Logger& instance();

        void log(LogLevel level, const std::string& message,
                const std::string& logger_name = "default",
                const std::string& file = "", int line = 0,
                const std::string& function = "");

        void log_with_context(LogLevel level, const std::string& message,
                             const std::unordered_map<std::string, std::string>& context,
                             const std::string& logger_name = "default");

        // Configuration
        void set_level(LogLevel level);
        void set_output_file(const std::string& filename);
        void enable_console(bool enable);
        void enable_json_format(bool enable);

        // Query
        std::vector<LogEntry> get_recent_logs(size_t limit = 100) const;
        std::vector<LogEntry> get_logs_by_level(LogLevel level) const;
        nlohmann::json export_json() const;
    };
}

// Convenience macros
#define LOG_TRACE(msg)
#define LOG_DEBUG(msg)
#define LOG_INFO(msg)
#define LOG_WARN(msg)
#define LOG_ERROR(msg)
#define LOG_FATAL(msg)
```

### AlertManager

```cpp
namespace obs {
    class AlertManager {
    public:
        static AlertManager& instance();

        // Alert operations
        std::string trigger_alert(const std::string& name,
                                 const std::string& description,
                                 AlertSeverity severity,
                                 const std::unordered_map<std::string, std::string>& labels = {},
                                 const std::string& source = "");
        void resolve_alert(const std::string& alert_id);
        void acknowledge_alert(const std::string& alert_id);

        // Alert rules
        void register_alert_rule(const std::string& name,
                                AlertCondition condition,
                                const std::string& description,
                                AlertSeverity severity,
                                std::chrono::seconds check_interval = std::chrono::seconds(60));

        // Alert handlers
        void register_handler(const std::string& name, AlertHandler handler);
        void unregister_handler(const std::string& name);

        // Query operations
        std::vector<Alert> get_active_alerts() const;
        std::vector<Alert> get_alerts_by_severity(AlertSeverity severity) const;
        std::vector<Alert> get_alert_history(size_t limit = 100) const;
        nlohmann::json export_json() const;

        // Background monitoring
        void start_rule_evaluation();
        void stop_rule_evaluation();
    };
}
```

## MCP APIs

### MCPServer

```cpp
namespace obs::mcp {
    class MCPServer {
    public:
        static MCPServer& instance();

        // Server operations
        void start(int port = 8080);
        void stop();
        bool is_running() const;

        // Tool registration
        void register_tool(const MCPTool& tool);
        void unregister_tool(const std::string& name);

        // Request handling
        MCPResponse handle_request(const MCPRequest& request);
        MCPResponse execute_tool(const std::string& tool_name,
                                const nlohmann::json& params);

        // Tool discovery
        std::vector<MCPTool> list_tools() const;
        nlohmann::json get_tools_manifest() const;

        // Built-in tools
        void register_default_tools();
    };
}
```

## AI APIs

### LLMClient

```cpp
namespace obs::ai {
    class LLMClient {
    public:
        explicit LLMClient(const std::string& api_key,
                          const std::string& base_url = "https://api.openai.com/v1");

        // Chat completions
        LLMResponse chat(const LLMRequest& request);
        LLMResponse chat(const std::string& prompt,
                        const std::string& system_prompt = "");

        // Streaming
        void chat_stream(const LLMRequest& request, StreamCallback callback);

        // Embeddings
        std::vector<double> get_embedding(const std::string& text,
                                         const std::string& model = "text-embedding-ada-002");

        // Configuration
        void set_api_key(const std::string& api_key);
        void set_base_url(const std::string& base_url);
        void set_timeout(int seconds);
    };
}
```

### AIAnalyzer

```cpp
namespace obs::ai {
    class AIAnalyzer {
    public:
        explicit AIAnalyzer(std::shared_ptr<LLMClient> llm_client);

        // Analysis methods
        AnomalyReport analyze_metrics(const nlohmann::json& metrics_data);
        std::vector<HealthInsight> analyze_health(const nlohmann::json& health_data);
        std::vector<PerformanceInsight> analyze_traces(const nlohmann::json& trace_data);
        LogInsight analyze_logs(const nlohmann::json& log_data);

        // Q&A
        std::string answer_question(const std::string& question,
                                   const nlohmann::json& context_data);

        // Reports
        std::string generate_summary_report(const nlohmann::json& all_observability_data);
    };
}
```

## Automation APIs

### WorkflowEngine

```cpp
namespace obs::automation {
    class WorkflowEngine {
    public:
        static WorkflowEngine& instance();

        // Workflow operations
        std::string create_workflow(const std::string& name, bool parallel = false);
        void add_task_to_workflow(const std::string& workflow_id,
                                 const std::string& task_name,
                                 const std::string& description,
                                 std::function<bool()> action);
        void execute_workflow(const std::string& workflow_id);
        void cancel_workflow(const std::string& workflow_id);

        // Task operations
        std::string create_task(const std::string& name,
                               const std::string& description,
                               std::function<bool()> action);
        void execute_task(const std::string& task_id);
        void cancel_task(const std::string& task_id);

        // Query operations
        Workflow get_workflow(const std::string& workflow_id) const;
        Task get_task(const std::string& task_id) const;
        std::vector<Workflow> get_all_workflows() const;
        std::vector<Task> get_running_tasks() const;
        nlohmann::json export_json() const;

        // AI-assisted
        std::string generate_workflow_from_description(const std::string& description);
    };
}
```

## Analytics APIs

### Visualizer

```cpp
namespace obs::analytics {
    class Visualizer {
    public:
        static Visualizer& instance();

        // ASCII visualizations
        std::string plot_time_series(const std::vector<double>& values,
                                    const std::vector<std::string>& labels = {},
                                    int width = 80, int height = 20);
        std::string bar_chart(const std::vector<std::pair<std::string, double>>& data,
                             int width = 80);
        std::string histogram(const std::vector<double>& values,
                             int bins = 10, int width = 80);
        std::string heatmap(const std::vector<std::vector<double>>& matrix,
                           const std::vector<std::string>& row_labels = {},
                           const std::vector<std::string>& col_labels = {});
        std::string sparkline(const std::vector<double>& values);

        // Export formats
        nlohmann::json export_plotly_format(const std::string& chart_type,
                                            const nlohmann::json& data);
        nlohmann::json export_chartjs_format(const std::string& chart_type,
                                             const nlohmann::json& data);

        // Dashboard
        std::string generate_html_dashboard(const nlohmann::json& metrics,
                                           const nlohmann::json& health,
                                           const nlohmann::json& alerts);
    };
}
```

## Utilities APIs

### AsyncExecutor

```cpp
namespace obs::utils {
    class AsyncExecutor {
    public:
        static AsyncExecutor& instance();

        // Execute task asynchronously
        template<typename F>
        std::future<typename std::invoke_result<F>::type> execute(F&& f);

        // Schedule task for later execution
        template<typename F>
        void schedule(F&& f, std::chrono::milliseconds delay);

        // Execute task periodically
        template<typename F>
        void schedule_periodic(F&& f, std::chrono::milliseconds interval);

        // Configuration
        void resize_pool(size_t num_threads);
        size_t get_queue_size() const;
    };
}
```

---

For detailed usage examples, see the `examples/` directory.
