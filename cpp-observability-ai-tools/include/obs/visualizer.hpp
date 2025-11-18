#pragma once

#include <string>
#include <vector>
#include <nlohmann/json.hpp>

namespace obs::analytics {

// Generates ASCII/Unicode visualizations and exports for web dashboards
class Visualizer {
public:
    static Visualizer& instance();

    // Time series visualization
    std::string plot_time_series(const std::vector<double>& values,
                                const std::vector<std::string>& labels = {},
                                int width = 80,
                                int height = 20);

    // Bar chart
    std::string bar_chart(const std::vector<std::pair<std::string, double>>& data,
                         int width = 80);

    // Histogram
    std::string histogram(const std::vector<double>& values,
                         int bins = 10,
                         int width = 80);

    // Heat map (ASCII representation)
    std::string heatmap(const std::vector<std::vector<double>>& matrix,
                       const std::vector<std::string>& row_labels = {},
                       const std::vector<std::string>& col_labels = {});

    // Sparkline (compact inline visualization)
    std::string sparkline(const std::vector<double>& values);

    // Export formats for web dashboards
    nlohmann::json export_plotly_format(const std::string& chart_type,
                                        const nlohmann::json& data);

    nlohmann::json export_chartjs_format(const std::string& chart_type,
                                         const nlohmann::json& data);

    // Dashboard generation
    std::string generate_html_dashboard(const nlohmann::json& metrics,
                                       const nlohmann::json& health,
                                       const nlohmann::json& alerts);

private:
    Visualizer() = default;
    ~Visualizer() = default;
    Visualizer(const Visualizer&) = delete;
    Visualizer& operator=(const Visualizer&) = delete;

    char get_bar_char(double ratio) const;
    std::string normalize_value(double value, double min, double max, int scale) const;
};

} // namespace obs::analytics
