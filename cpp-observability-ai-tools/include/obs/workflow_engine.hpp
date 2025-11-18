#pragma once

#include <string>
#include <vector>
#include <functional>
#include <memory>
#include <chrono>
#include <nlohmann/json.hpp>

namespace obs::automation {

enum class TaskStatus {
    PENDING,
    RUNNING,
    COMPLETED,
    FAILED,
    CANCELLED
};

struct Task {
    std::string id;
    std::string name;
    std::string description;
    std::function<bool()> action;
    TaskStatus status{TaskStatus::PENDING};
    std::chrono::system_clock::time_point created_at;
    std::chrono::system_clock::time_point started_at;
    std::chrono::system_clock::time_point completed_at;
    std::string error_message;
    nlohmann::json result;
};

struct Workflow {
    std::string id;
    std::string name;
    std::vector<std::shared_ptr<Task>> tasks;
    bool parallel{false}; // Execute tasks in parallel or sequential
    TaskStatus status{TaskStatus::PENDING};
};

class WorkflowEngine {
public:
    static WorkflowEngine& instance();

    // Workflow operations
    std::string create_workflow(const std::string& name, bool parallel = false);
    void add_task_to_workflow(const std::string& workflow_id,
                             const std::string& task_name,
                             const std::string& description,
                             std::function<bool()> action);

    void execute_workflow(const std::string& workflow_id);
    void cancel_workflow(const std::string& workflow_id);

    // Task operations
    std::string create_task(const std::string& name,
                           const std::string& description,
                           std::function<bool()> action);

    void execute_task(const std::string& task_id);
    void cancel_task(const std::string& task_id);

    // Query operations
    Workflow get_workflow(const std::string& workflow_id) const;
    Task get_task(const std::string& task_id) const;
    std::vector<Workflow> get_all_workflows() const;
    std::vector<Task> get_running_tasks() const;
    nlohmann::json export_json() const;

    // AI-assisted workflow generation
    std::string generate_workflow_from_description(const std::string& description);

private:
    WorkflowEngine() = default;
    ~WorkflowEngine() = default;
    WorkflowEngine(const WorkflowEngine&) = delete;
    WorkflowEngine& operator=(const WorkflowEngine&) = delete;

    void execute_sequential_workflow(Workflow& workflow);
    void execute_parallel_workflow(Workflow& workflow);
    std::string generate_id();

    mutable std::mutex mutex_;
    std::unordered_map<std::string, Workflow> workflows_;
    std::unordered_map<std::string, std::shared_ptr<Task>> tasks_;
};

} // namespace obs::automation
