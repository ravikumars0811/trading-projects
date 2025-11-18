#include "obs/mcp_server.hpp"
#include <cassert>
#include <iostream>

void test_tool_registration() {
    std::cout << "Testing MCP tool registration... ";

    auto& mcp = obs::mcp::MCPServer::instance();

    obs::mcp::MCPTool test_tool;
    test_tool.name = "test_tool";
    test_tool.description = "A test tool";
    test_tool.input_schema = {{"type", "object"}};
    test_tool.handler = [](const nlohmann::json& params) -> obs::mcp::MCPResponse {
        obs::mcp::MCPResponse response;
        response.success = true;
        response.result = {{"status", "ok"}};
        return response;
    };

    mcp.register_tool(test_tool);

    auto tools = mcp.list_tools();
    bool found = false;
    for (const auto& tool : tools) {
        if (tool.name == "test_tool") {
            found = true;
            break;
        }
    }
    assert(found);

    std::cout << "PASSED\n";
}

void test_tool_execution() {
    std::cout << "Testing MCP tool execution... ";

    auto& mcp = obs::mcp::MCPServer::instance();

    auto response = mcp.execute_tool("test_tool", {});
    assert(response.success);
    assert(response.result.contains("status"));
    assert(response.result["status"] == "ok");

    std::cout << "PASSED\n";
}

void test_manifest() {
    std::cout << "Testing MCP tools manifest... ";

    auto& mcp = obs::mcp::MCPServer::instance();

    auto manifest = mcp.get_tools_manifest();
    assert(manifest.contains("tools"));
    assert(manifest["tools"].is_array());
    assert(!manifest["tools"].empty());

    std::cout << "PASSED\n";
}

int main() {
    std::cout << "Running MCP Server Tests\n";
    std::cout << "========================\n";

    test_tool_registration();
    test_tool_execution();
    test_manifest();

    std::cout << "\nAll tests passed!\n";
    return 0;
}
