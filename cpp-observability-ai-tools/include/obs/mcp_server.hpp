#pragma once

#include <string>
#include <memory>
#include <vector>
#include <functional>
#include <unordered_map>
#include <nlohmann/json.hpp>

namespace obs::mcp {

// Model Context Protocol (MCP) server implementation
// Enables AI assistants (ChatGPT, Claude, etc.) to interact with observability data

struct MCPRequest {
    std::string method;
    nlohmann::json params;
    std::string id;
};

struct MCPResponse {
    nlohmann::json result;
    std::string error;
    std::string id;
    bool success{true};
};

struct MCPTool {
    std::string name;
    std::string description;
    nlohmann::json input_schema;
    std::function<MCPResponse(const nlohmann::json&)> handler;
};

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

private:
    MCPServer();
    ~MCPServer();
    MCPServer(const MCPServer&) = delete;
    MCPServer& operator=(const MCPServer&) = delete;

    void accept_connections();
    void handle_client(int client_socket);
    MCPResponse handle_list_tools(const nlohmann::json& params);

    std::unordered_map<std::string, MCPTool> tools_;
    mutable std::mutex mutex_;
    std::atomic<bool> running_{false};
    int server_socket_{-1};
    int port_{8080};
    std::unique_ptr<std::thread> server_thread_;
};

} // namespace obs::mcp
