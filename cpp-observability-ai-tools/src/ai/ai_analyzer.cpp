#include "obs/ai_analyzer.hpp"
#include <sstream>

namespace obs::ai {

AIAnalyzer::AIAnalyzer(std::shared_ptr<LLMClient> llm_client)
    : llm_client_(std::move(llm_client)) {}

std::string AIAnalyzer::format_metrics_for_llm(const nlohmann::json& metrics) {
    std::ostringstream oss;
    oss << "Metrics Data:\n";

    if (metrics.is_array()) {
        for (const auto& metric : metrics) {
            oss << "- " << metric.value("name", "unknown") << ": "
                << metric.value("value", 0.0) << "\n";
        }
    }

    return oss.str();
}

std::string AIAnalyzer::format_health_for_llm(const nlohmann::json& health) {
    std::ostringstream oss;
    oss << "Health Check Data:\n";
    oss << "Overall Status: " << health.value("overall_status", -1) << "\n\n";

    if (health.contains("checks")) {
        for (const auto& check : health["checks"]) {
            oss << "- " << check.value("name", "unknown") << ": "
                << check.value("message", "") << "\n";
        }
    }

    return oss.str();
}

std::string AIAnalyzer::format_traces_for_llm(const nlohmann::json& traces) {
    std::ostringstream oss;
    oss << "Trace Data:\n";

    if (traces.is_array()) {
        for (const auto& span : traces) {
            oss << "- Operation: " << span.value("operation_name", "unknown")
                << ", Duration: " << span.value("duration_us", 0) << "us\n";
        }
    }

    return oss.str();
}

AIAnalyzer::AnomalyReport AIAnalyzer::analyze_metrics(const nlohmann::json& metrics_data) {
    std::string context = format_metrics_for_llm(metrics_data);

    std::string prompt = "Analyze the following metrics data and identify any anomalies, "
                        "unusual patterns, or concerning trends:\n\n" + context +
                        "\n\nProvide a JSON response with: has_anomaly (bool), description (string), "
                        "affected_metrics (array), confidence (0-1), and recommendation (string).";

    try {
        auto response = llm_client_->chat(prompt, "You are an expert system observability analyst.");

        // Parse LLM response
        nlohmann::json result = nlohmann::json::parse(response.content);

        AnomalyReport report;
        report.has_anomaly = result.value("has_anomaly", false);
        report.description = result.value("description", "");
        report.confidence = result.value("confidence", 0.0);
        report.recommendation = result.value("recommendation", "");

        if (result.contains("affected_metrics")) {
            for (const auto& m : result["affected_metrics"]) {
                report.affected_metrics.push_back(m.get<std::string>());
            }
        }

        return report;
    } catch (const std::exception& e) {
        AnomalyReport report;
        report.has_anomaly = false;
        report.description = std::string("Analysis failed: ") + e.what();
        report.confidence = 0.0;
        return report;
    }
}

std::vector<AIAnalyzer::HealthInsight> AIAnalyzer::analyze_health(const nlohmann::json& health_data) {
    std::string context = format_health_for_llm(health_data);

    std::string prompt = "Analyze the following health check data and provide insights:\n\n" + context +
                        "\n\nFor each unhealthy component, provide: component, status, analysis, "
                        "recommendations (array), and potential root_cause.";

    std::vector<HealthInsight> insights;

    try {
        auto response = llm_client_->chat(prompt, "You are an expert system health analyst.");

        // Parse LLM response
        nlohmann::json result = nlohmann::json::parse(response.content);

        if (result.is_array()) {
            for (const auto& item : result) {
                HealthInsight insight;
                insight.component = item.value("component", "");
                insight.status = item.value("status", "");
                insight.analysis = item.value("analysis", "");
                insight.root_cause = item.value("root_cause", "");

                if (item.contains("recommendations")) {
                    for (const auto& rec : item["recommendations"]) {
                        insight.recommendations.push_back(rec.get<std::string>());
                    }
                }

                insights.push_back(insight);
            }
        }
    } catch (const std::exception& e) {
        // Return empty insights on error
    }

    return insights;
}

std::vector<AIAnalyzer::PerformanceInsight> AIAnalyzer::analyze_traces(const nlohmann::json& trace_data) {
    std::string context = format_traces_for_llm(trace_data);

    std::string prompt = "Analyze the following distributed trace data and identify performance issues:\n\n" +
                        context + "\n\nProvide insights about slow operations, bottlenecks, and "
                        "optimization suggestions.";

    std::vector<PerformanceInsight> insights;

    try {
        auto response = llm_client_->chat(prompt, "You are an expert performance analyst.");

        nlohmann::json result = nlohmann::json::parse(response.content);

        if (result.is_array()) {
            for (const auto& item : result) {
                PerformanceInsight insight;
                insight.operation = item.value("operation", "");
                insight.avg_duration_ms = item.value("avg_duration_ms", 0.0);
                insight.bottleneck = item.value("bottleneck", "");

                if (item.contains("optimization_suggestions")) {
                    for (const auto& sug : item["optimization_suggestions"]) {
                        insight.optimization_suggestions.push_back(sug.get<std::string>());
                    }
                }

                insights.push_back(insight);
            }
        }
    } catch (const std::exception& e) {
        // Return empty insights on error
    }

    return insights;
}

AIAnalyzer::LogInsight AIAnalyzer::analyze_logs(const nlohmann::json& log_data) {
    std::ostringstream context;
    context << "Log Data:\n";

    if (log_data.is_array()) {
        for (const auto& log : log_data) {
            context << "[" << log.value("level", "") << "] "
                   << log.value("message", "") << "\n";
        }
    }

    std::string prompt = "Analyze the following log data:\n\n" + context.str() +
                        "\n\nIdentify error patterns, warning patterns, provide a summary, "
                        "and suggest action items.";

    LogInsight insight;

    try {
        auto response = llm_client_->chat(prompt, "You are an expert log analyst.");

        nlohmann::json result = nlohmann::json::parse(response.content);

        insight.summary = result.value("summary", "");

        if (result.contains("error_patterns")) {
            for (const auto& p : result["error_patterns"]) {
                insight.error_patterns.push_back(p.get<std::string>());
            }
        }

        if (result.contains("warning_patterns")) {
            for (const auto& p : result["warning_patterns"]) {
                insight.warning_patterns.push_back(p.get<std::string>());
            }
        }

        if (result.contains("action_items")) {
            for (const auto& a : result["action_items"]) {
                insight.action_items.push_back(a.get<std::string>());
            }
        }
    } catch (const std::exception& e) {
        insight.summary = std::string("Analysis failed: ") + e.what();
    }

    return insight;
}

std::string AIAnalyzer::answer_question(const std::string& question,
                                       const nlohmann::json& context_data) {
    std::ostringstream context;
    context << "Observability Data:\n" << context_data.dump(2);

    std::string prompt = "Based on the following observability data, answer this question:\n\n" +
                        question + "\n\nContext:\n" + context.str();

    try {
        auto response = llm_client_->chat(prompt,
            "You are an expert observability assistant. Provide clear, actionable answers "
            "based on the observability data.");
        return response.content;
    } catch (const std::exception& e) {
        return std::string("Failed to answer question: ") + e.what();
    }
}

std::string AIAnalyzer::generate_summary_report(const nlohmann::json& all_observability_data) {
    std::string prompt = "Generate a comprehensive observability summary report from this data:\n\n" +
                        all_observability_data.dump(2) +
                        "\n\nInclude: overall system health, key metrics, active alerts, "
                        "performance insights, and actionable recommendations.";

    try {
        auto response = llm_client_->chat(prompt,
            "You are an expert observability analyst creating executive summaries.");
        return response.content;
    } catch (const std::exception& e) {
        return std::string("Failed to generate report: ") + e.what();
    }
}

} // namespace obs::ai
