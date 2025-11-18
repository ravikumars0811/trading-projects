#include "obs/tracer.hpp"
#include <random>
#include <sstream>
#include <iomanip>
#include <algorithm>

namespace obs {

Tracer& Tracer::instance() {
    static Tracer instance;
    return instance;
}

std::string Tracer::generate_trace_id() {
    static std::random_device rd;
    static std::mt19937_64 gen(rd());
    std::uniform_int_distribution<uint64_t> dis;

    std::ostringstream oss;
    oss << std::hex << std::setfill('0') << std::setw(16) << dis(gen);
    return oss.str();
}

std::string Tracer::generate_span_id() {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_int_distribution<uint32_t> dis;

    std::ostringstream oss;
    oss << std::hex << std::setfill('0') << std::setw(8) << dis(gen);
    return oss.str();
}

std::shared_ptr<Span> Tracer::start_span(const std::string& operation_name,
                                         const std::string& parent_span_id) {
    // Sampling check
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_real_distribution<> dis(0.0, 1.0);

    if (dis(gen) > sampling_rate_) {
        return nullptr; // Sampled out
    }

    auto span = std::make_shared<Span>();
    span->span_id = generate_span_id();
    span->operation_name = operation_name;
    span->parent_span_id = parent_span_id;
    span->start_time = std::chrono::system_clock::now();

    // Generate or inherit trace ID
    if (parent_span_id.empty()) {
        span->trace_id = generate_trace_id();
    } else {
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = std::find_if(spans_.begin(), spans_.end(),
                              [&parent_span_id](const Span& s) {
                                  return s.span_id == parent_span_id;
                              });
        if (it != spans_.end()) {
            span->trace_id = it->trace_id;
        } else {
            span->trace_id = generate_trace_id();
        }
    }

    return span;
}

void Tracer::finish_span(std::shared_ptr<Span> span) {
    if (!span || span->finished) return;

    span->end_time = std::chrono::system_clock::now();
    span->finished = true;

    std::lock_guard<std::mutex> lock(mutex_);
    spans_.push_back(*span);

    // Maintain max spans limit
    if (spans_.size() > max_spans_) {
        spans_.erase(spans_.begin(), spans_.begin() + (spans_.size() - max_spans_));
    }
}

void Tracer::add_tag(std::shared_ptr<Span> span,
                    const std::string& key,
                    const std::string& value) {
    if (span) {
        span->tags[key] = value;
    }
}

void Tracer::add_log(std::shared_ptr<Span> span,
                    const std::string& key,
                    const std::string& value) {
    if (span) {
        span->logs[key] = value;
    }
}

std::vector<Span> Tracer::get_spans_by_trace_id(const std::string& trace_id) const {
    std::lock_guard<std::mutex> lock(mutex_);
    std::vector<Span> result;

    std::copy_if(spans_.begin(), spans_.end(), std::back_inserter(result),
                [&trace_id](const Span& s) { return s.trace_id == trace_id; });

    return result;
}

std::vector<Span> Tracer::get_recent_spans(size_t limit) const {
    std::lock_guard<std::mutex> lock(mutex_);

    size_t start = spans_.size() > limit ? spans_.size() - limit : 0;
    return std::vector<Span>(spans_.begin() + start, spans_.end());
}

nlohmann::json Tracer::export_json() const {
    std::lock_guard<std::mutex> lock(mutex_);
    nlohmann::json result = nlohmann::json::array();

    for (const auto& span : spans_) {
        nlohmann::json span_json;
        span_json["trace_id"] = span.trace_id;
        span_json["span_id"] = span.span_id;
        span_json["parent_span_id"] = span.parent_span_id;
        span_json["operation_name"] = span.operation_name;

        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(
            span.end_time - span.start_time).count();
        span_json["duration_us"] = duration;

        span_json["tags"] = span.tags;
        span_json["logs"] = span.logs;

        result.push_back(span_json);
    }

    return result;
}

nlohmann::json Tracer::export_jaeger_format() const {
    std::lock_guard<std::mutex> lock(mutex_);
    nlohmann::json result;
    result["data"] = nlohmann::json::array();

    // Group spans by trace_id
    std::unordered_map<std::string, std::vector<Span>> traces;
    for (const auto& span : spans_) {
        traces[span.trace_id].push_back(span);
    }

    for (const auto& [trace_id, spans] : traces) {
        nlohmann::json trace;
        trace["traceID"] = trace_id;
        trace["spans"] = nlohmann::json::array();

        for (const auto& span : spans) {
            nlohmann::json jaeger_span;
            jaeger_span["spanID"] = span.span_id;
            jaeger_span["operationName"] = span.operation_name;
            jaeger_span["references"] = nlohmann::json::array();

            if (!span.parent_span_id.empty()) {
                jaeger_span["references"].push_back({
                    {"refType", "CHILD_OF"},
                    {"traceID", trace_id},
                    {"spanID", span.parent_span_id}
                });
            }

            auto start_us = std::chrono::duration_cast<std::chrono::microseconds>(
                span.start_time.time_since_epoch()).count();
            auto duration_us = std::chrono::duration_cast<std::chrono::microseconds>(
                span.end_time - span.start_time).count();

            jaeger_span["startTime"] = start_us;
            jaeger_span["duration"] = duration_us;
            jaeger_span["tags"] = nlohmann::json::array();

            for (const auto& [key, value] : span.tags) {
                jaeger_span["tags"].push_back({
                    {"key", key},
                    {"type", "string"},
                    {"value", value}
                });
            }

            trace["spans"].push_back(jaeger_span);
        }

        result["data"].push_back(trace);
    }

    return result;
}

void Tracer::set_sampling_rate(double rate) {
    sampling_rate_ = std::clamp(rate, 0.0, 1.0);
}

void Tracer::set_max_spans(size_t max_spans) {
    std::lock_guard<std::mutex> lock(mutex_);
    max_spans_ = max_spans;
}

} // namespace obs
