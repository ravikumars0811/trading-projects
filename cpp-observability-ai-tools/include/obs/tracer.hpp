#pragma once

#include <string>
#include <chrono>
#include <memory>
#include <vector>
#include <unordered_map>
#include <mutex>
#include <nlohmann/json.hpp>

namespace obs {

struct Span {
    std::string trace_id;
    std::string span_id;
    std::string parent_span_id;
    std::string operation_name;
    std::chrono::system_clock::time_point start_time;
    std::chrono::system_clock::time_point end_time;
    std::unordered_map<std::string, std::string> tags;
    std::unordered_map<std::string, std::string> logs;
    bool finished{false};
};

class Tracer {
public:
    static Tracer& instance();

    // Span operations
    std::shared_ptr<Span> start_span(const std::string& operation_name,
                                     const std::string& parent_span_id = "");

    void finish_span(std::shared_ptr<Span> span);

    void add_tag(std::shared_ptr<Span> span,
                const std::string& key,
                const std::string& value);

    void add_log(std::shared_ptr<Span> span,
                const std::string& key,
                const std::string& value);

    // Query operations
    std::vector<Span> get_spans_by_trace_id(const std::string& trace_id) const;
    std::vector<Span> get_recent_spans(size_t limit = 100) const;
    nlohmann::json export_json() const;
    nlohmann::json export_jaeger_format() const;

    // Configuration
    void set_sampling_rate(double rate); // 0.0 to 1.0
    void set_max_spans(size_t max_spans);

private:
    Tracer() = default;
    ~Tracer() = default;
    Tracer(const Tracer&) = delete;
    Tracer& operator=(const Tracer&) = delete;

    std::string generate_trace_id();
    std::string generate_span_id();

    mutable std::mutex mutex_;
    std::vector<Span> spans_;
    size_t max_spans_{10000};
    double sampling_rate_{1.0};
};

// RAII span helper
class ScopedSpan {
public:
    explicit ScopedSpan(const std::string& operation_name,
                       const std::string& parent_span_id = "")
        : span_(Tracer::instance().start_span(operation_name, parent_span_id)) {}

    ~ScopedSpan() {
        if (span_ && !span_->finished) {
            Tracer::instance().finish_span(span_);
        }
    }

    void add_tag(const std::string& key, const std::string& value) {
        Tracer::instance().add_tag(span_, key, value);
    }

    void add_log(const std::string& key, const std::string& value) {
        Tracer::instance().add_log(span_, key, value);
    }

    std::string get_trace_id() const { return span_->trace_id; }
    std::string get_span_id() const { return span_->span_id; }

private:
    std::shared_ptr<Span> span_;
};

} // namespace obs
