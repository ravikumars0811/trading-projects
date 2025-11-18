#pragma once

#include <string>
#include <functional>
#include <vector>
#include <memory>
#include <chrono>
#include <mutex>
#include <nlohmann/json.hpp>

namespace obs {

enum class AlertSeverity {
    INFO,
    WARNING,
    CRITICAL
};

enum class AlertStatus {
    ACTIVE,
    RESOLVED,
    ACKNOWLEDGED
};

struct Alert {
    std::string id;
    std::string name;
    std::string description;
    AlertSeverity severity;
    AlertStatus status;
    std::chrono::system_clock::time_point triggered_at;
    std::chrono::system_clock::time_point resolved_at;
    std::unordered_map<std::string, std::string> labels;
    std::string source;
};

using AlertHandler = std::function<void(const Alert&)>;
using AlertCondition = std::function<bool()>;

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

private:
    AlertManager() = default;
    ~AlertManager();
    AlertManager(const AlertManager&) = delete;
    AlertManager& operator=(const AlertManager&) = delete;

    struct AlertRule {
        std::string name;
        AlertCondition condition;
        std::string description;
        AlertSeverity severity;
        std::chrono::seconds interval;
        std::chrono::steady_clock::time_point last_check;
        std::string active_alert_id;
    };

    void notify_handlers(const Alert& alert);
    void evaluation_loop();
    std::string generate_alert_id();

    mutable std::mutex mutex_;
    std::unordered_map<std::string, Alert> alerts_;
    std::vector<Alert> alert_history_;
    std::vector<AlertRule> rules_;
    std::unordered_map<std::string, AlertHandler> handlers_;
    std::atomic<bool> evaluation_active_{false};
    std::unique_ptr<std::thread> evaluation_thread_;
};

} // namespace obs
