#include "obs/visualizer.hpp"
#include <algorithm>
#include <cmath>
#include <sstream>
#include <iomanip>

namespace obs::analytics {

Visualizer& Visualizer::instance() {
    static Visualizer instance;
    return instance;
}

char Visualizer::get_bar_char(double ratio) const {
    if (ratio >= 0.875) return '█';
    if (ratio >= 0.75) return '▇';
    if (ratio >= 0.625) return '▆';
    if (ratio >= 0.5) return '▅';
    if (ratio >= 0.375) return '▄';
    if (ratio >= 0.25) return '▃';
    if (ratio >= 0.125) return '▂';
    if (ratio > 0) return '▁';
    return ' ';
}

std::string Visualizer::normalize_value(double value, double min, double max, int scale) const {
    if (max == min) return std::string(scale / 2, '█');

    double normalized = (value - min) / (max - min);
    int bar_length = static_cast<int>(normalized * scale);
    return std::string(std::max(0, std::min(bar_length, scale)), '█');
}

std::string Visualizer::sparkline(const std::vector<double>& values) {
    if (values.empty()) return "";

    double min_val = *std::min_element(values.begin(), values.end());
    double max_val = *std::max_element(values.begin(), values.end());

    std::ostringstream oss;
    for (double val : values) {
        double ratio = (max_val == min_val) ? 0.5 : (val - min_val) / (max_val - min_val);
        oss << get_bar_char(ratio);
    }

    return oss.str();
}

std::string Visualizer::plot_time_series(const std::vector<double>& values,
                                        const std::vector<std::string>& labels,
                                        int width,
                                        int height) {
    if (values.empty()) return "No data to plot";

    double min_val = *std::min_element(values.begin(), values.end());
    double max_val = *std::max_element(values.begin(), values.end());

    std::ostringstream oss;
    oss << "Time Series Plot (range: " << std::fixed << std::setprecision(2)
        << min_val << " - " << max_val << ")\n";
    oss << std::string(width + 10, '─') << "\n";

    // Create plot matrix
    std::vector<std::string> lines(height);
    for (int i = 0; i < height; ++i) {
        lines[i] = std::string(width, ' ');
    }

    // Plot points
    for (size_t i = 0; i < values.size() && i < static_cast<size_t>(width); ++i) {
        double normalized = (max_val == min_val) ? 0.5 :
                           (values[i] - min_val) / (max_val - min_val);
        int y = height - 1 - static_cast<int>(normalized * (height - 1));
        y = std::max(0, std::min(y, height - 1));

        lines[y][i] = '●';

        // Connect with line
        if (i > 0) {
            double prev_normalized = (max_val == min_val) ? 0.5 :
                                    (values[i-1] - min_val) / (max_val - min_val);
            int prev_y = height - 1 - static_cast<int>(prev_normalized * (height - 1));
            prev_y = std::max(0, std::min(prev_y, height - 1));

            int start_y = std::min(y, prev_y);
            int end_y = std::max(y, prev_y);

            for (int j = start_y; j <= end_y; ++j) {
                if (lines[j][i-1] == ' ') {
                    lines[j][i-1] = '│';
                }
            }
        }
    }

    // Output plot
    for (const auto& line : lines) {
        oss << "│ " << line << "\n";
    }
    oss << "└" << std::string(width, '─') << "\n";

    return oss.str();
}

std::string Visualizer::bar_chart(const std::vector<std::pair<std::string, double>>& data,
                                 int width) {
    if (data.empty()) return "No data to display";

    double max_val = 0.0;
    for (const auto& [label, value] : data) {
        max_val = std::max(max_val, value);
    }

    std::ostringstream oss;
    oss << "Bar Chart\n";
    oss << std::string(width + 30, '─') << "\n";

    for (const auto& [label, value] : data) {
        int bar_length = (max_val > 0) ? static_cast<int>((value / max_val) * width) : 0;
        oss << std::setw(20) << std::left << label << " │ "
            << std::string(bar_length, '█') << " " << std::fixed << std::setprecision(2)
            << value << "\n";
    }

    return oss.str();
}

std::string Visualizer::histogram(const std::vector<double>& values, int bins, int width) {
    if (values.empty()) return "No data to display";

    double min_val = *std::min_element(values.begin(), values.end());
    double max_val = *std::max_element(values.begin(), values.end());
    double bin_width = (max_val - min_val) / bins;

    std::vector<int> bin_counts(bins, 0);

    for (double val : values) {
        int bin = static_cast<int>((val - min_val) / bin_width);
        bin = std::max(0, std::min(bin, bins - 1));
        bin_counts[bin]++;
    }

    int max_count = *std::max_element(bin_counts.begin(), bin_counts.end());

    std::ostringstream oss;
    oss << "Histogram (" << bins << " bins)\n";
    oss << std::string(width + 30, '─') << "\n";

    for (int i = 0; i < bins; ++i) {
        double bin_start = min_val + i * bin_width;
        double bin_end = bin_start + bin_width;

        int bar_length = (max_count > 0) ?
                        static_cast<int>((static_cast<double>(bin_counts[i]) / max_count) * width) : 0;

        oss << std::fixed << std::setprecision(1) << std::setw(8) << bin_start
            << "-" << std::setw(8) << bin_end << " │ "
            << std::string(bar_length, '█') << " (" << bin_counts[i] << ")\n";
    }

    return oss.str();
}

std::string Visualizer::heatmap(const std::vector<std::vector<double>>& matrix,
                               const std::vector<std::string>& row_labels,
                               const std::vector<std::string>& col_labels) {
    if (matrix.empty()) return "No data to display";

    // Find min and max values
    double min_val = matrix[0][0];
    double max_val = matrix[0][0];

    for (const auto& row : matrix) {
        for (double val : row) {
            min_val = std::min(min_val, val);
            max_val = std::max(max_val, val);
        }
    }

    std::ostringstream oss;
    oss << "Heatmap (range: " << std::fixed << std::setprecision(2)
        << min_val << " - " << max_val << ")\n\n";

    // Column headers
    if (!col_labels.empty()) {
        oss << std::setw(15) << " ";
        for (const auto& label : col_labels) {
            oss << std::setw(10) << label;
        }
        oss << "\n";
    }

    // Rows
    for (size_t i = 0; i < matrix.size(); ++i) {
        if (!row_labels.empty() && i < row_labels.size()) {
            oss << std::setw(15) << row_labels[i];
        } else {
            oss << std::setw(15) << ("Row " + std::to_string(i));
        }

        for (double val : matrix[i]) {
            double ratio = (max_val == min_val) ? 0.5 : (val - min_val) / (max_val - min_val);
            char heat_char = get_bar_char(ratio);
            oss << std::setw(10) << std::string(8, heat_char);
        }
        oss << "\n";
    }

    return oss.str();
}

nlohmann::json Visualizer::export_plotly_format(const std::string& chart_type,
                                                const nlohmann::json& data) {
    nlohmann::json plotly;
    plotly["data"] = nlohmann::json::array();
    plotly["layout"] = {
        {"title", "Observability Chart"},
        {"xaxis", {{"title", "X Axis"}}},
        {"yaxis", {{"title", "Y Axis"}}}
    };

    nlohmann::json trace;
    trace["type"] = chart_type;

    if (data.contains("x")) trace["x"] = data["x"];
    if (data.contains("y")) trace["y"] = data["y"];
    if (data.contains("values")) trace["values"] = data["values"];
    if (data.contains("labels")) trace["labels"] = data["labels"];

    plotly["data"].push_back(trace);

    return plotly;
}

nlohmann::json Visualizer::export_chartjs_format(const std::string& chart_type,
                                                 const nlohmann::json& data) {
    nlohmann::json chartjs;
    chartjs["type"] = chart_type;
    chartjs["data"] = {
        {"datasets", nlohmann::json::array()}
    };

    if (data.contains("labels")) {
        chartjs["data"]["labels"] = data["labels"];
    }

    nlohmann::json dataset;
    if (data.contains("data")) dataset["data"] = data["data"];
    if (data.contains("label")) dataset["label"] = data["label"];

    chartjs["data"]["datasets"].push_back(dataset);

    return chartjs;
}

std::string Visualizer::generate_html_dashboard(const nlohmann::json& metrics,
                                               const nlohmann::json& health,
                                               const nlohmann::json& alerts) {
    std::ostringstream html;

    html << R"(
<!DOCTYPE html>
<html>
<head>
    <title>Observability Dashboard</title>
    <script src="https://cdn.plot.ly/plotly-latest.min.js"></script>
    <style>
        body { font-family: Arial, sans-serif; margin: 20px; background: #f5f5f5; }
        .container { max-width: 1200px; margin: 0 auto; }
        .card { background: white; padding: 20px; margin: 20px 0; border-radius: 8px; box-shadow: 0 2px 4px rgba(0,0,0,0.1); }
        h1, h2 { color: #333; }
        .status-healthy { color: green; font-weight: bold; }
        .status-warning { color: orange; font-weight: bold; }
        .status-critical { color: red; font-weight: bold; }
        .metric { display: inline-block; margin: 10px 20px; }
        .metric-label { font-size: 12px; color: #666; }
        .metric-value { font-size: 24px; font-weight: bold; color: #333; }
    </style>
</head>
<body>
    <div class="container">
        <h1>📊 Observability Dashboard</h1>

        <div class="card">
            <h2>System Health</h2>
            <div id="health-status"></div>
        </div>

        <div class="card">
            <h2>Key Metrics</h2>
            <div id="metrics"></div>
        </div>

        <div class="card">
            <h2>Active Alerts</h2>
            <div id="alerts"></div>
        </div>

        <div class="card">
            <h2>Metrics Chart</h2>
            <div id="chart"></div>
        </div>
    </div>

    <script>
        const healthData = )" << health.dump() << R"(;
        const metricsData = )" << metrics.dump() << R"(;
        const alertsData = )" << alerts.dump() << R"(;

        // Render health status
        document.getElementById('health-status').innerHTML =
            '<p class="status-healthy">System Status: ' + (healthData.overall_status || 'Unknown') + '</p>';

        // Render metrics
        let metricsHtml = '';
        if (Array.isArray(metricsData)) {
            metricsData.slice(0, 6).forEach(metric => {
                metricsHtml += `<div class="metric">
                    <div class="metric-label">${metric.name || 'Unknown'}</div>
                    <div class="metric-value">${(metric.value || 0).toFixed(2)}</div>
                </div>`;
            });
        }
        document.getElementById('metrics').innerHTML = metricsHtml || 'No metrics available';

        // Render alerts
        let alertsHtml = '';
        if (alertsData.active_alerts && alertsData.active_alerts.length > 0) {
            alertsData.active_alerts.forEach(alert => {
                alertsHtml += `<p class="status-critical">⚠️ ${alert.name}: ${alert.description}</p>`;
            });
        } else {
            alertsHtml = '<p class="status-healthy">No active alerts</p>';
        }
        document.getElementById('alerts').innerHTML = alertsHtml;

        // Create chart
        if (Array.isArray(metricsData) && metricsData.length > 0) {
            const chartData = [{
                x: metricsData.map(m => m.name || 'Unknown'),
                y: metricsData.map(m => m.value || 0),
                type: 'bar'
            }];

            Plotly.newPlot('chart', chartData, {
                title: 'Metrics Overview',
                xaxis: { title: 'Metric Name' },
                yaxis: { title: 'Value' }
            });
        }
    </script>
</body>
</html>
)";

    return html.str();
}

} // namespace obs::analytics
