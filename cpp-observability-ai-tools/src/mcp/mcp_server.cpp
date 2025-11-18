#include "obs/mcp_server.hpp"
#include "obs/metrics_collector.hpp"
#include "obs/health_monitor.hpp"
#include "obs/tracer.hpp"
#include "obs/logger.hpp"
#include "obs/alert_manager.hpp"
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <cstring>

namespace obs::mcp {

MCPServer& MCPServer::instance() {
    static MCPServer instance;
    return instance;
}

MCPServer::MCPServer() {
    register_default_tools();
}

MCPServer::~MCPServer() {
    stop();
}

void MCPServer::start(int port) {
    if (running_.exchange(true)) {
        return; // Already running
    }

    port_ = port;

    server_socket_ = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket_ < 0) {
        LOG_ERROR("Failed to create server socket");
        running_ = false;
        return;
    }

    int opt = 1;
    setsockopt(server_socket_, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    sockaddr_in address{};
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port_);

    if (bind(server_socket_, (struct sockaddr*)&address, sizeof(address)) < 0) {
        LOG_ERROR("Failed to bind server socket");
        close(server_socket_);
        running_ = false;
        return;
    }

    if (listen(server_socket_, 10) < 0) {
        LOG_ERROR("Failed to listen on server socket");
        close(server_socket_);
        running_ = false;
        return;
    }

    LOG_INFO("MCP Server started on port " + std::to_string(port_));

    server_thread_ = std::make_unique<std::thread>([this]() {
        accept_connections();
    });
}

void MCPServer::stop() {
    if (!running_.exchange(false)) {
        return; // Not running
    }

    if (server_socket_ >= 0) {
        close(server_socket_);
        server_socket_ = -1;
    }

    if (server_thread_ && server_thread_->joinable()) {
        server_thread_->join();
    }

    LOG_INFO("MCP Server stopped");
}

bool MCPServer::is_running() const {
    return running_.load();
}

void MCPServer::register_tool(const MCPTool& tool) {
    std::lock_guard<std::mutex> lock(mutex_);
    tools_[tool.name] = tool;
    LOG_INFO("Registered MCP tool: " + tool.name);
}

void MCPServer::unregister_tool(const std::string& name) {
    std::lock_guard<std::mutex> lock(mutex_);
    tools_.erase(name);
    LOG_INFO("Unregistered MCP tool: " + name);
}

MCPResponse MCPServer::handle_request(const MCPRequest& request) {
    if (request.method == "tools/list") {
        return handle_list_tools(request.params);
    } else if (request.method == "tools/call") {
        std::string tool_name = request.params.value("name", "");
        nlohmann::json params = request.params.value("arguments", nlohmann::json::object());
        return execute_tool(tool_name, params);
    }

    MCPResponse response;
    response.success = false;
    response.error = "Unknown method: " + request.method;
    response.id = request.id;
    return response;
}

MCPResponse MCPServer::execute_tool(const std::string& tool_name,
                                    const nlohmann::json& params) {
    std::lock_guard<std::mutex> lock(mutex_);

    auto it = tools_.find(tool_name);
    if (it == tools_.end()) {
        MCPResponse response;
        response.success = false;
        response.error = "Tool not found: " + tool_name;
        return response;
    }

    try {
        return it->second.handler(params);
    } catch (const std::exception& e) {
        MCPResponse response;
        response.success = false;
        response.error = std::string("Tool execution failed: ") + e.what();
        return response;
    }
}

std::vector<MCPTool> MCPServer::list_tools() const {
    std::lock_guard<std::mutex> lock(mutex_);
    std::vector<MCPTool> result;

    for (const auto& [name, tool] : tools_) {
        result.push_back(tool);
    }

    return result;
}

nlohmann::json MCPServer::get_tools_manifest() const {
    std::lock_guard<std::mutex> lock(mutex_);
    nlohmann::json manifest;
    manifest["tools"] = nlohmann::json::array();

    for (const auto& [name, tool] : tools_) {
        nlohmann::json tool_json;
        tool_json["name"] = tool.name;
        tool_json["description"] = tool.description;
        tool_json["inputSchema"] = tool.input_schema;

        manifest["tools"].push_back(tool_json);
    }

    return manifest;
}

void MCPServer::register_default_tools() {
    // Health check tool
    MCPTool health_tool;
    health_tool.name = "get_health_status";
    health_tool.description = "Get current health status of all monitored components";
    health_tool.input_schema = {
        {"type", "object"},
        {"properties", {
            {"component", {
                {"type", "string"},
                {"description", "Optional: specific component name to query"}
            }}
        }}
    };
    health_tool.handler = [](const nlohmann::json& params) -> MCPResponse {
        MCPResponse response;
        response.success = true;
        response.result = obs::HealthMonitor::instance().export_json();
        return response;
    };
    register_tool(health_tool);

    // Metrics query tool
    MCPTool metrics_tool;
    metrics_tool.name = "query_metrics";
    metrics_tool.description = "Query metrics data with optional filtering";
    metrics_tool.input_schema = {
        {"type", "object"},
        {"properties", {
            {"metric_name", {
                {"type", "string"},
                {"description", "Optional: specific metric name to query"}
            }}
        }}
    };
    metrics_tool.handler = [](const nlohmann::json& params) -> MCPResponse {
        MCPResponse response;
        response.success = true;
        response.result = obs::MetricsCollector::instance().export_json();
        return response;
    };
    register_tool(metrics_tool);

    // Traces query tool
    MCPTool traces_tool;
    traces_tool.name = "query_traces";
    traces_tool.description = "Query distributed traces";
    traces_tool.input_schema = {
        {"type", "object"},
        {"properties", {
            {"trace_id", {
                {"type", "string"},
                {"description", "Optional: specific trace ID to query"}
            }},
            {"limit", {
                {"type", "integer"},
                {"description", "Maximum number of traces to return"}
            }}
        }}
    };
    traces_tool.handler = [](const nlohmann::json& params) -> MCPResponse {
        MCPResponse response;
        response.success = true;
        int limit = params.value("limit", 100);
        response.result = obs::Tracer::instance().export_json();
        return response;
    };
    register_tool(traces_tool);

    // Alerts query tool
    MCPTool alerts_tool;
    alerts_tool.name = "query_alerts";
    alerts_tool.description = "Query active alerts and alert history";
    alerts_tool.input_schema = {
        {"type", "object"},
        {"properties", {
            {"active_only", {
                {"type", "boolean"},
                {"description", "Return only active alerts"}
            }}
        }}
    };
    alerts_tool.handler = [](const nlohmann::json& params) -> MCPResponse {
        MCPResponse response;
        response.success = true;
        response.result = obs::AlertManager::instance().export_json();
        return response;
    };
    register_tool(alerts_tool);

    // Logs query tool
    MCPTool logs_tool;
    logs_tool.name = "query_logs";
    logs_tool.description = "Query application logs";
    logs_tool.input_schema = {
        {"type", "object"},
        {"properties", {
            {"level", {
                {"type", "string"},
                {"description", "Optional: filter by log level (TRACE, DEBUG, INFO, WARN, ERROR, FATAL)"}
            }},
            {"limit", {
                {"type", "integer"},
                {"description", "Maximum number of log entries to return"}
            }}
        }}
    };
    logs_tool.handler = [](const nlohmann::json& params) -> MCPResponse {
        MCPResponse response;
        response.success = true;
        int limit = params.value("limit", 100);
        response.result = obs::Logger::instance().export_json();
        return response;
    };
    register_tool(logs_tool);
}

MCPResponse MCPServer::handle_list_tools(const nlohmann::json& params) {
    MCPResponse response;
    response.success = true;
    response.result = get_tools_manifest();
    return response;
}

void MCPServer::accept_connections() {
    while (running_.load()) {
        sockaddr_in client_addr{};
        socklen_t client_len = sizeof(client_addr);

        int client_socket = accept(server_socket_, (struct sockaddr*)&client_addr, &client_len);
        if (client_socket < 0) {
            if (running_.load()) {
                LOG_ERROR("Failed to accept client connection");
            }
            continue;
        }

        std::thread([this, client_socket]() {
            handle_client(client_socket);
        }).detach();
    }
}

void MCPServer::handle_client(int client_socket) {
    char buffer[4096];
    while (running_.load()) {
        ssize_t bytes_read = recv(client_socket, buffer, sizeof(buffer) - 1, 0);
        if (bytes_read <= 0) {
            break;
        }

        buffer[bytes_read] = '\0';

        try {
            nlohmann::json request_json = nlohmann::json::parse(buffer);

            MCPRequest request;
            request.method = request_json.value("method", "");
            request.params = request_json.value("params", nlohmann::json::object());
            request.id = request_json.value("id", "");

            MCPResponse response = handle_request(request);
            response.id = request.id;

            nlohmann::json response_json;
            if (response.success) {
                response_json["result"] = response.result;
            } else {
                response_json["error"] = response.error;
            }
            response_json["id"] = response.id;

            std::string response_str = response_json.dump() + "\n";
            send(client_socket, response_str.c_str(), response_str.length(), 0);

        } catch (const std::exception& e) {
            LOG_ERROR(std::string("Error handling client request: ") + e.what());
        }
    }

    close(client_socket);
}

} // namespace obs::mcp
