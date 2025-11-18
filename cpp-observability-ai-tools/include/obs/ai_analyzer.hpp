#pragma once

#include "llm_client.hpp"
#include <string>
#include <vector>
#include <memory>
#include <nlohmann/json.hpp>

namespace obs::ai {

// AI-powered analysis of observability data
class AIAnalyzer {
public:
    explicit AIAnalyzer(std::shared_ptr<LLMClient> llm_client);

    // Analyze metrics and detect anomalies
    struct AnomalyReport {
        bool has_anomaly;
        std::string description;
        std::vector<std::string> affected_metrics;
        double confidence;
        std::string recommendation;
    };

    AnomalyReport analyze_metrics(const nlohmann::json& metrics_data);

    // Analyze health checks and provide insights
    struct HealthInsight {
        std::string component;
        std::string status;
        std::string analysis;
        std::vector<std::string> recommendations;
        std::string root_cause;
    };

    std::vector<HealthInsight> analyze_health(const nlohmann::json& health_data);

    // Analyze traces and identify performance issues
    struct PerformanceInsight {
        std::string operation;
        double avg_duration_ms;
        std::string bottleneck;
        std::vector<std::string> optimization_suggestions;
    };

    std::vector<PerformanceInsight> analyze_traces(const nlohmann::json& trace_data);

    // Log analysis
    struct LogInsight {
        std::vector<std::string> error_patterns;
        std::vector<std::string> warning_patterns;
        std::string summary;
        std::vector<std::string> action_items;
    };

    LogInsight analyze_logs(const nlohmann::json& log_data);

    // Answer operational questions
    std::string answer_question(const std::string& question,
                               const nlohmann::json& context_data);

    // Generate reports
    std::string generate_summary_report(const nlohmann::json& all_observability_data);

private:
    std::shared_ptr<LLMClient> llm_client_;

    std::string format_metrics_for_llm(const nlohmann::json& metrics);
    std::string format_health_for_llm(const nlohmann::json& health);
    std::string format_traces_for_llm(const nlohmann::json& traces);
};

} // namespace obs::ai
