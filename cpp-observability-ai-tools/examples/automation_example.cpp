#include "obs/workflow_engine.hpp"
#include "obs/logger.hpp"
#include <iostream>
#include <thread>
#include <chrono>

int main() {
    auto& logger = obs::Logger::instance();
    logger.set_level(obs::LogLevel::INFO);

    LOG_INFO("Workflow Automation Example");
    LOG_INFO("============================\n");

    auto& engine = obs::automation::WorkflowEngine::instance();

    // Example 1: Sequential Workflow
    LOG_INFO("1. Creating Sequential Workflow:");
    std::string workflow1_id = engine.create_workflow("Data Processing Pipeline", false);

    engine.add_task_to_workflow(workflow1_id, "Fetch Data",
        "Retrieve data from source", []() -> bool {
            LOG_INFO("   Executing: Fetch Data");
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
            return true;
        });

    engine.add_task_to_workflow(workflow1_id, "Validate Data",
        "Check data integrity", []() -> bool {
            LOG_INFO("   Executing: Validate Data");
            std::this_thread::sleep_for(std::chrono::milliseconds(300));
            return true;
        });

    engine.add_task_to_workflow(workflow1_id, "Process Data",
        "Transform and analyze data", []() -> bool {
            LOG_INFO("   Executing: Process Data");
            std::this_thread::sleep_for(std::chrono::milliseconds(700));
            return true;
        });

    engine.add_task_to_workflow(workflow1_id, "Store Results",
        "Save processed data", []() -> bool {
            LOG_INFO("   Executing: Store Results");
            std::this_thread::sleep_for(std::chrono::milliseconds(400));
            return true;
        });

    LOG_INFO("   Executing workflow...");
    engine.execute_workflow(workflow1_id);

    auto workflow1 = engine.get_workflow(workflow1_id);
    LOG_INFO("   Workflow status: " + std::to_string(static_cast<int>(workflow1.status)));

    // Example 2: Parallel Workflow
    LOG_INFO("\n2. Creating Parallel Workflow:");
    std::string workflow2_id = engine.create_workflow("Parallel Health Checks", true);

    engine.add_task_to_workflow(workflow2_id, "Check Database",
        "Verify database connectivity", []() -> bool {
            LOG_INFO("   Checking Database...");
            std::this_thread::sleep_for(std::chrono::milliseconds(300));
            LOG_INFO("   Database: OK");
            return true;
        });

    engine.add_task_to_workflow(workflow2_id, "Check Cache",
        "Verify cache availability", []() -> bool {
            LOG_INFO("   Checking Cache...");
            std::this_thread::sleep_for(std::chrono::milliseconds(200));
            LOG_INFO("   Cache: OK");
            return true;
        });

    engine.add_task_to_workflow(workflow2_id, "Check API",
        "Verify API endpoints", []() -> bool {
            LOG_INFO("   Checking API...");
            std::this_thread::sleep_for(std::chrono::milliseconds(400));
            LOG_INFO("   API: OK");
            return true;
        });

    LOG_INFO("   Executing parallel workflow...");
    engine.execute_workflow(workflow2_id);

    auto workflow2 = engine.get_workflow(workflow2_id);
    LOG_INFO("   Workflow status: " + std::to_string(static_cast<int>(workflow2.status)));

    // Example 3: Standalone Task
    LOG_INFO("\n3. Creating Standalone Task:");
    std::string task_id = engine.create_task("Generate Report",
        "Create observability report", []() -> bool {
            LOG_INFO("   Generating report...");
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
            LOG_INFO("   Report generated successfully");
            return true;
        });

    LOG_INFO("   Executing task...");
    engine.execute_task(task_id);

    auto task = engine.get_task(task_id);
    LOG_INFO("   Task status: " + std::to_string(static_cast<int>(task.status)));

    // Display all workflows
    LOG_INFO("\n4. All Workflows:");
    auto all_workflows = engine.get_all_workflows();
    for (const auto& wf : all_workflows) {
        LOG_INFO("   - " + wf.name + " (" + std::to_string(wf.tasks.size()) + " tasks)");
    }

    LOG_INFO("\nAutomation examples complete!");

    return 0;
}
