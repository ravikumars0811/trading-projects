#include "obs/workflow_engine.hpp"
#include <random>
#include <sstream>
#include <future>

namespace obs::automation {

WorkflowEngine& WorkflowEngine::instance() {
    static WorkflowEngine instance;
    return instance;
}

std::string WorkflowEngine::generate_id() {
    static std::random_device rd;
    static std::mt19937_64 gen(rd());
    std::uniform_int_distribution<uint64_t> dis;

    std::ostringstream oss;
    oss << "wf_" << std::hex << dis(gen);
    return oss.str();
}

std::string WorkflowEngine::create_workflow(const std::string& name, bool parallel) {
    std::lock_guard<std::mutex> lock(mutex_);

    Workflow workflow;
    workflow.id = generate_id();
    workflow.name = name;
    workflow.parallel = parallel;
    workflow.status = TaskStatus::PENDING;

    workflows_[workflow.id] = workflow;
    return workflow.id;
}

void WorkflowEngine::add_task_to_workflow(const std::string& workflow_id,
                                         const std::string& task_name,
                                         const std::string& description,
                                         std::function<bool()> action) {
    std::lock_guard<std::mutex> lock(mutex_);

    auto it = workflows_.find(workflow_id);
    if (it == workflows_.end()) {
        return;
    }

    auto task = std::make_shared<Task>();
    task->id = generate_id();
    task->name = task_name;
    task->description = description;
    task->action = std::move(action);
    task->status = TaskStatus::PENDING;
    task->created_at = std::chrono::system_clock::now();

    it->second.tasks.push_back(task);
    tasks_[task->id] = task;
}

void WorkflowEngine::execute_workflow(const std::string& workflow_id) {
    std::unique_lock<std::mutex> lock(mutex_);

    auto it = workflows_.find(workflow_id);
    if (it == workflows_.end()) {
        return;
    }

    Workflow workflow = it->second;
    lock.unlock();

    if (workflow.parallel) {
        execute_parallel_workflow(workflow);
    } else {
        execute_sequential_workflow(workflow);
    }

    lock.lock();
    workflows_[workflow_id] = workflow;
}

void WorkflowEngine::execute_sequential_workflow(Workflow& workflow) {
    workflow.status = TaskStatus::RUNNING;

    for (auto& task : workflow.tasks) {
        task->status = TaskStatus::RUNNING;
        task->started_at = std::chrono::system_clock::now();

        try {
            bool success = task->action();
            task->status = success ? TaskStatus::COMPLETED : TaskStatus::FAILED;
            task->result = nlohmann::json{{"success", success}};
        } catch (const std::exception& e) {
            task->status = TaskStatus::FAILED;
            task->error_message = e.what();
            workflow.status = TaskStatus::FAILED;
            return;
        }

        task->completed_at = std::chrono::system_clock::now();

        if (task->status == TaskStatus::FAILED) {
            workflow.status = TaskStatus::FAILED;
            return;
        }
    }

    workflow.status = TaskStatus::COMPLETED;
}

void WorkflowEngine::execute_parallel_workflow(Workflow& workflow) {
    workflow.status = TaskStatus::RUNNING;

    std::vector<std::future<void>> futures;

    for (auto& task : workflow.tasks) {
        futures.push_back(std::async(std::launch::async, [&task]() {
            task->status = TaskStatus::RUNNING;
            task->started_at = std::chrono::system_clock::now();

            try {
                bool success = task->action();
                task->status = success ? TaskStatus::COMPLETED : TaskStatus::FAILED;
                task->result = nlohmann::json{{"success", success}};
            } catch (const std::exception& e) {
                task->status = TaskStatus::FAILED;
                task->error_message = e.what();
            }

            task->completed_at = std::chrono::system_clock::now();
        }));
    }

    // Wait for all tasks
    for (auto& future : futures) {
        future.wait();
    }

    // Check if any task failed
    bool any_failed = false;
    for (const auto& task : workflow.tasks) {
        if (task->status == TaskStatus::FAILED) {
            any_failed = true;
            break;
        }
    }

    workflow.status = any_failed ? TaskStatus::FAILED : TaskStatus::COMPLETED;
}

void WorkflowEngine::cancel_workflow(const std::string& workflow_id) {
    std::lock_guard<std::mutex> lock(mutex_);

    auto it = workflows_.find(workflow_id);
    if (it != workflows_.end()) {
        it->second.status = TaskStatus::CANCELLED;

        for (auto& task : it->second.tasks) {
            if (task->status == TaskStatus::PENDING || task->status == TaskStatus::RUNNING) {
                task->status = TaskStatus::CANCELLED;
            }
        }
    }
}

std::string WorkflowEngine::create_task(const std::string& name,
                                       const std::string& description,
                                       std::function<bool()> action) {
    std::lock_guard<std::mutex> lock(mutex_);

    auto task = std::make_shared<Task>();
    task->id = generate_id();
    task->name = name;
    task->description = description;
    task->action = std::move(action);
    task->status = TaskStatus::PENDING;
    task->created_at = std::chrono::system_clock::now();

    tasks_[task->id] = task;
    return task->id;
}

void WorkflowEngine::execute_task(const std::string& task_id) {
    std::unique_lock<std::mutex> lock(mutex_);

    auto it = tasks_.find(task_id);
    if (it == tasks_.end()) {
        return;
    }

    auto task = it->second;
    lock.unlock();

    task->status = TaskStatus::RUNNING;
    task->started_at = std::chrono::system_clock::now();

    try {
        bool success = task->action();
        task->status = success ? TaskStatus::COMPLETED : TaskStatus::FAILED;
        task->result = nlohmann::json{{"success", success}};
    } catch (const std::exception& e) {
        task->status = TaskStatus::FAILED;
        task->error_message = e.what();
    }

    task->completed_at = std::chrono::system_clock::now();
}

void WorkflowEngine::cancel_task(const std::string& task_id) {
    std::lock_guard<std::mutex> lock(mutex_);

    auto it = tasks_.find(task_id);
    if (it != tasks_.end()) {
        it->second->status = TaskStatus::CANCELLED;
    }
}

Workflow WorkflowEngine::get_workflow(const std::string& workflow_id) const {
    std::lock_guard<std::mutex> lock(mutex_);

    auto it = workflows_.find(workflow_id);
    if (it != workflows_.end()) {
        return it->second;
    }
    return Workflow{};
}

Task WorkflowEngine::get_task(const std::string& task_id) const {
    std::lock_guard<std::mutex> lock(mutex_);

    auto it = tasks_.find(task_id);
    if (it != tasks_.end()) {
        return *it->second;
    }
    return Task{};
}

std::vector<Workflow> WorkflowEngine::get_all_workflows() const {
    std::lock_guard<std::mutex> lock(mutex_);
    std::vector<Workflow> result;

    for (const auto& [id, workflow] : workflows_) {
        result.push_back(workflow);
    }

    return result;
}

std::vector<Task> WorkflowEngine::get_running_tasks() const {
    std::lock_guard<std::mutex> lock(mutex_);
    std::vector<Task> result;

    for (const auto& [id, task] : tasks_) {
        if (task->status == TaskStatus::RUNNING) {
            result.push_back(*task);
        }
    }

    return result;
}

nlohmann::json WorkflowEngine::export_json() const {
    std::lock_guard<std::mutex> lock(mutex_);
    nlohmann::json result;

    result["workflows"] = nlohmann::json::array();
    for (const auto& [id, workflow] : workflows_) {
        nlohmann::json wf;
        wf["id"] = workflow.id;
        wf["name"] = workflow.name;
        wf["parallel"] = workflow.parallel;
        wf["status"] = static_cast<int>(workflow.status);
        wf["task_count"] = workflow.tasks.size();

        result["workflows"].push_back(wf);
    }

    return result;
}

std::string WorkflowEngine::generate_workflow_from_description(const std::string& description) {
    // Placeholder for AI-assisted workflow generation
    // This would use LLM to parse the description and create appropriate tasks
    return create_workflow("AI Generated: " + description, false);
}

} // namespace obs::automation
