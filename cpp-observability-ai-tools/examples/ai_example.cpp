#include "obs/llm_client.hpp"
#include "obs/ai_analyzer.hpp"
#include "obs/metrics_collector.hpp"
#include "obs/logger.hpp"
#include <iostream>
#include <cstdlib>

int main() {
    auto& logger = obs::Logger::instance();
    logger.set_level(obs::LogLevel::INFO);

    LOG_INFO("AI-Powered Observability Analysis Example");
    LOG_INFO("==========================================\n");

    // Check for API key
    const char* api_key = std::getenv("OPENAI_API_KEY");
    if (!api_key) {
        LOG_WARN("OPENAI_API_KEY environment variable not set");
        LOG_INFO("This example requires an OpenAI API key to function");
        LOG_INFO("Set it with: export OPENAI_API_KEY='your-api-key'");
        LOG_INFO("\nShowing example structure without actual API calls...\n");

        // Show example usage without actual API calls
        LOG_INFO("Example Usage:");
        LOG_INFO("-------------");
        LOG_INFO("auto llm_client = std::make_shared<obs::ai::LLMClient>(api_key);");
        LOG_INFO("obs::ai::AIAnalyzer analyzer(llm_client);");
        LOG_INFO("");
        LOG_INFO("// Analyze metrics for anomalies");
        LOG_INFO("auto anomaly_report = analyzer.analyze_metrics(metrics_json);");
        LOG_INFO("");
        LOG_INFO("// Get health insights");
        LOG_INFO("auto health_insights = analyzer.analyze_health(health_json);");
        LOG_INFO("");
        LOG_INFO("// Answer operational questions");
        LOG_INFO("auto answer = analyzer.answer_question(");
        LOG_INFO("    \"Why is latency increasing?\",");
        LOG_INFO("    observability_data");
        LOG_INFO(");");

        return 0;
    }

    // Create LLM client
    auto llm_client = std::make_shared<obs::ai::LLMClient>(api_key);
    LOG_INFO("Created LLM client");

    // Create AI analyzer
    obs::ai::AIAnalyzer analyzer(llm_client);
    LOG_INFO("Created AI analyzer\n");

    // Setup sample metrics
    auto& metrics = obs::MetricsCollector::instance();
    metrics.set_gauge("cpu_usage", 85.5);
    metrics.set_gauge("memory_usage", 92.3);
    metrics.set_gauge("disk_usage", 45.0);
    metrics.record_histogram("request_latency_ms", 250);

    // Example 1: Analyze metrics for anomalies
    LOG_INFO("1. Analyzing metrics for anomalies...");
    try {
        auto metrics_json = metrics.export_json();
        auto anomaly_report = analyzer.analyze_metrics(metrics_json);

        LOG_INFO("   Anomaly detected: " + std::string(anomaly_report.has_anomaly ? "Yes" : "No"));
        if (anomaly_report.has_anomaly) {
            LOG_INFO("   Description: " + anomaly_report.description);
            LOG_INFO("   Confidence: " + std::to_string(anomaly_report.confidence));
            LOG_INFO("   Recommendation: " + anomaly_report.recommendation);
        }
    } catch (const std::exception& e) {
        LOG_ERROR("   Failed: " + std::string(e.what()));
    }

    // Example 2: Answer operational question
    LOG_INFO("\n2. Answering operational question...");
    try {
        nlohmann::json context;
        context["metrics"] = metrics.export_json();

        auto answer = analyzer.answer_question(
            "What is the current system resource usage?",
            context
        );

        LOG_INFO("   Answer: " + answer);
    } catch (const std::exception& e) {
        LOG_ERROR("   Failed: " + std::string(e.what()));
    }

    // Example 3: Generate summary report
    LOG_INFO("\n3. Generating observability summary report...");
    try {
        nlohmann::json all_data;
        all_data["metrics"] = metrics.export_json();

        auto report = analyzer.generate_summary_report(all_data);
        LOG_INFO("   Report:\n" + report);
    } catch (const std::exception& e) {
        LOG_ERROR("   Failed: " + std::string(e.what()));
    }

    LOG_INFO("\nAI analysis complete!");

    return 0;
}
