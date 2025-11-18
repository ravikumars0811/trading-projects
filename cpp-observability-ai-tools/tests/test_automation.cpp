#include "obs/workflow_engine.hpp"
#include <cassert>
#include <iostream>

void test_task_creation() {
    std::cout << "Testing task creation... ";

    auto& engine = obs::automation::WorkflowEngine::instance();

    std::string task_id = engine.create_task("test_task", "A test task", []() -> bool {
        return true;
    });

    assert(!task_id.empty());

    auto task = engine.get_task(task_id);
    assert(task.name == "test_task");
    assert(task.status == obs::automation::TaskStatus::PENDING);

    std::cout << "PASSED\n";
}

void test_task_execution() {
    std::cout << "Testing task execution... ";

    auto& engine = obs::automation::WorkflowEngine::instance();

    bool executed = false;
    std::string task_id = engine.create_task("exec_test", "Execute test", [&executed]() -> bool {
        executed = true;
        return true;
    });

    engine.execute_task(task_id);

    assert(executed);

    auto task = engine.get_task(task_id);
    assert(task.status == obs::automation::TaskStatus::COMPLETED);

    std::cout << "PASSED\n";
}

void test_workflow_creation() {
    std::cout << "Testing workflow creation... ";

    auto& engine = obs::automation::WorkflowEngine::instance();

    std::string workflow_id = engine.create_workflow("test_workflow", false);
    assert(!workflow_id.empty());

    engine.add_task_to_workflow(workflow_id, "task1", "Task 1", []() -> bool {
        return true;
    });

    engine.add_task_to_workflow(workflow_id, "task2", "Task 2", []() -> bool {
        return true;
    });

    auto workflow = engine.get_workflow(workflow_id);
    assert(workflow.name == "test_workflow");
    assert(workflow.tasks.size() == 2);

    std::cout << "PASSED\n";
}

void test_workflow_execution() {
    std::cout << "Testing workflow execution... ";

    auto& engine = obs::automation::WorkflowEngine::instance();

    int execution_count = 0;
    std::string workflow_id = engine.create_workflow("exec_workflow", false);

    engine.add_task_to_workflow(workflow_id, "task1", "Task 1", [&execution_count]() -> bool {
        execution_count++;
        return true;
    });

    engine.add_task_to_workflow(workflow_id, "task2", "Task 2", [&execution_count]() -> bool {
        execution_count++;
        return true;
    });

    engine.execute_workflow(workflow_id);

    assert(execution_count == 2);

    auto workflow = engine.get_workflow(workflow_id);
    assert(workflow.status == obs::automation::TaskStatus::COMPLETED);

    std::cout << "PASSED\n";
}

int main() {
    std::cout << "Running Automation Tests\n";
    std::cout << "========================\n";

    test_task_creation();
    test_task_execution();
    test_workflow_creation();
    test_workflow_execution();

    std::cout << "\nAll tests passed!\n";
    return 0;
}
