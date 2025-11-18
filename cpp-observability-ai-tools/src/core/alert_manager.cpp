#include "obs/alert_manager.hpp"
#include <random>
#include <sstream>
#include <iomanip>

namespace obs {

AlertManager& AlertManager::instance() {
    static AlertManager instance;
    return instance;
}

AlertManager::~AlertManager() {
    stop_rule_evaluation();
}

std::string AlertManager::generate_alert_id() {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_int_distribution<uint64_t> dis;

    std::ostringstream oss;
    oss << "alert_" << std::hex << dis(gen);
    return oss.str();
}

std::string AlertManager::trigger_alert(const std::string& name,
                                       const std::string& description,
                                       AlertSeverity severity,
                                       const std::unordered_map<std::string, std::string>& labels,
                                       const std::string& source) {
    std::lock_guard<std::mutex> lock(mutex_);

    Alert alert;
    alert.id = generate_alert_id();
    alert.name = name;
    alert.description = description;
    alert.severity = severity;
    alert.status = AlertStatus::ACTIVE;
    alert.triggered_at = std::chrono::system_clock::now();
    alert.labels = labels;
    alert.source = source;

    alerts_[alert.id] = alert;
    alert_history_.push_back(alert);

    // Notify handlers asynchronously
    auto alert_copy = alert;
    std::thread([this, alert_copy]() {
        notify_handlers(alert_copy);
    }).detach();

    return alert.id;
}

void AlertManager::resolve_alert(const std::string& alert_id) {
    std::lock_guard<std::mutex> lock(mutex_);

    auto it = alerts_.find(alert_id);
    if (it != alerts_.end()) {
        it->second.status = AlertStatus::RESOLVED;
        it->second.resolved_at = std::chrono::system_clock::now();

        // Update in history
        for (auto& alert : alert_history_) {
            if (alert.id == alert_id) {
                alert.status = AlertStatus::RESOLVED;
                alert.resolved_at = it->second.resolved_at;
                break;
            }
        }

        alerts_.erase(it);
    }
}

void AlertManager::acknowledge_alert(const std::string& alert_id) {
    std::lock_guard<std::mutex> lock(mutex_);

    auto it = alerts_.find(alert_id);
    if (it != alerts_.end()) {
        it->second.status = AlertStatus::ACKNOWLEDGED;

        // Update in history
        for (auto& alert : alert_history_) {
            if (alert.id == alert_id) {
                alert.status = AlertStatus::ACKNOWLEDGED;
                break;
            }
        }
    }
}

void AlertManager::register_alert_rule(const std::string& name,
                                      AlertCondition condition,
                                      const std::string& description,
                                      AlertSeverity severity,
                                      std::chrono::seconds check_interval) {
    std::lock_guard<std::mutex> lock(mutex_);

    AlertRule rule;
    rule.name = name;
    rule.condition = std::move(condition);
    rule.description = description;
    rule.severity = severity;
    rule.interval = check_interval;
    rule.last_check = std::chrono::steady_clock::now();

    rules_.push_back(std::move(rule));
}

void AlertManager::register_handler(const std::string& name, AlertHandler handler) {
    std::lock_guard<std::mutex> lock(mutex_);
    handlers_[name] = std::move(handler);
}

void AlertManager::unregister_handler(const std::string& name) {
    std::lock_guard<std::mutex> lock(mutex_);
    handlers_.erase(name);
}

std::vector<Alert> AlertManager::get_active_alerts() const {
    std::lock_guard<std::mutex> lock(mutex_);
    std::vector<Alert> result;

    for (const auto& [id, alert] : alerts_) {
        if (alert.status == AlertStatus::ACTIVE) {
            result.push_back(alert);
        }
    }

    return result;
}

std::vector<Alert> AlertManager::get_alerts_by_severity(AlertSeverity severity) const {
    std::lock_guard<std::mutex> lock(mutex_);
    std::vector<Alert> result;

    for (const auto& [id, alert] : alerts_) {
        if (alert.severity == severity) {
            result.push_back(alert);
        }
    }

    return result;
}

std::vector<Alert> AlertManager::get_alert_history(size_t limit) const {
    std::lock_guard<std::mutex> lock(mutex_);
    size_t start = alert_history_.size() > limit ? alert_history_.size() - limit : 0;
    return std::vector<Alert>(alert_history_.begin() + start, alert_history_.end());
}

nlohmann::json AlertManager::export_json() const {
    std::lock_guard<std::mutex> lock(mutex_);
    nlohmann::json result;

    result["active_alerts"] = nlohmann::json::array();
    for (const auto& [id, alert] : alerts_) {
        nlohmann::json alert_json;
        alert_json["id"] = alert.id;
        alert_json["name"] = alert.name;
        alert_json["description"] = alert.description;
        alert_json["severity"] = static_cast<int>(alert.severity);
        alert_json["status"] = static_cast<int>(alert.status);
        alert_json["labels"] = alert.labels;
        alert_json["source"] = alert.source;

        result["active_alerts"].push_back(alert_json);
    }

    return result;
}

void AlertManager::start_rule_evaluation() {
    if (evaluation_active_.exchange(true)) {
        return; // Already running
    }

    evaluation_thread_ = std::make_unique<std::thread>([this]() {
        evaluation_loop();
    });
}

void AlertManager::stop_rule_evaluation() {
    if (!evaluation_active_.exchange(false)) {
        return; // Not running
    }

    if (evaluation_thread_ && evaluation_thread_->joinable()) {
        evaluation_thread_->join();
    }
}

void AlertManager::evaluation_loop() {
    while (evaluation_active_.load()) {
        auto now = std::chrono::steady_clock::now();

        std::unique_lock<std::mutex> lock(mutex_);
        auto rules_copy = rules_;
        lock.unlock();

        for (auto& rule : rules_copy) {
            if (now - rule.last_check >= rule.interval) {
                try {
                    bool condition_met = rule.condition();

                    if (condition_met && rule.active_alert_id.empty()) {
                        // Trigger alert
                        std::string alert_id = trigger_alert(
                            rule.name,
                            rule.description,
                            rule.severity,
                            {},
                            "rule_engine"
                        );
                        rule.active_alert_id = alert_id;
                    } else if (!condition_met && !rule.active_alert_id.empty()) {
                        // Resolve alert
                        resolve_alert(rule.active_alert_id);
                        rule.active_alert_id.clear();
                    }
                } catch (...) {
                    // Ignore evaluation errors
                }

                rule.last_check = now;
            }
        }

        // Update rules in the actual storage
        std::lock_guard<std::mutex> update_lock(mutex_);
        rules_ = rules_copy;

        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}

void AlertManager::notify_handlers(const Alert& alert) {
    std::lock_guard<std::mutex> lock(mutex_);

    for (const auto& [name, handler] : handlers_) {
        try {
            handler(alert);
        } catch (...) {
            // Ignore handler errors
        }
    }
}

} // namespace obs
