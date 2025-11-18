#include "obs/llm_client.hpp"
#include <curl/curl.h>
#include <sstream>
#include <stdexcept>

namespace obs::ai {

static size_t write_callback(void* contents, size_t size, size_t nmemb, std::string* userp) {
    userp->append((char*)contents, size * nmemb);
    return size * nmemb;
}

LLMClient::LLMClient(const std::string& api_key, const std::string& base_url)
    : api_key_(api_key), base_url_(base_url) {
    curl_global_init(CURL_GLOBAL_DEFAULT);
}

LLMResponse LLMClient::chat(const LLMRequest& request) {
    nlohmann::json payload;
    payload["model"] = request.model;
    payload["temperature"] = request.temperature;
    payload["max_tokens"] = request.max_tokens;

    payload["messages"] = nlohmann::json::array();
    for (const auto& msg : request.messages) {
        payload["messages"].push_back({
            {"role", msg.role},
            {"content", msg.content}
        });
    }

    if (!request.stop_sequences.empty()) {
        payload["stop"] = request.stop_sequences;
    }

    if (!request.tools.empty()) {
        payload["tools"] = request.tools;
    }

    std::string response_str = make_request("/chat/completions", payload);

    // Parse response
    nlohmann::json response_json = nlohmann::json::parse(response_str);

    LLMResponse response;
    if (response_json.contains("choices") && !response_json["choices"].empty()) {
        auto& choice = response_json["choices"][0];
        response.content = choice["message"].value("content", "");
        response.finish_reason = choice.value("finish_reason", "");

        if (choice["message"].contains("function_call")) {
            response.function_call = choice["message"]["function_call"];
        }
    }

    if (response_json.contains("usage")) {
        response.total_tokens = response_json["usage"].value("total_tokens", 0);
        response.prompt_tokens = response_json["usage"].value("prompt_tokens", 0);
        response.completion_tokens = response_json["usage"].value("completion_tokens", 0);
    }

    return response;
}

LLMResponse LLMClient::chat(const std::string& prompt, const std::string& system_prompt) {
    LLMRequest request;
    if (!system_prompt.empty()) {
        request.messages.push_back({"system", system_prompt});
    }
    request.messages.push_back({"user", prompt});

    return chat(request);
}

void LLMClient::chat_stream(const LLMRequest& request, StreamCallback callback) {
    // Streaming implementation placeholder
    auto response = chat(request);
    callback(response.content);
}

std::vector<double> LLMClient::get_embedding(const std::string& text, const std::string& model) {
    nlohmann::json payload;
    payload["model"] = model;
    payload["input"] = text;

    std::string response_str = make_request("/embeddings", payload);
    nlohmann::json response_json = nlohmann::json::parse(response_str);

    std::vector<double> embedding;
    if (response_json.contains("data") && !response_json["data"].empty()) {
        auto& data = response_json["data"][0];
        if (data.contains("embedding")) {
            for (const auto& val : data["embedding"]) {
                embedding.push_back(val.get<double>());
            }
        }
    }

    return embedding;
}

void LLMClient::set_api_key(const std::string& api_key) {
    api_key_ = api_key;
}

void LLMClient::set_base_url(const std::string& base_url) {
    base_url_ = base_url;
}

void LLMClient::set_timeout(int seconds) {
    timeout_seconds_ = seconds;
}

std::string LLMClient::make_request(const std::string& endpoint, const nlohmann::json& payload) {
    CURL* curl = curl_easy_init();
    if (!curl) {
        throw std::runtime_error("Failed to initialize CURL");
    }

    std::string url = base_url_ + endpoint;
    std::string response_string;
    std::string payload_str = payload.dump();

    struct curl_slist* headers = nullptr;
    headers = curl_slist_append(headers, "Content-Type: application/json");
    std::string auth_header = "Authorization: Bearer " + api_key_;
    headers = curl_slist_append(headers, auth_header.c_str());

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, payload_str.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response_string);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, timeout_seconds_);

    CURLcode res = curl_easy_perform(curl);

    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);

    if (res != CURLE_OK) {
        throw std::runtime_error(std::string("CURL request failed: ") + curl_easy_strerror(res));
    }

    return response_string;
}

} // namespace obs::ai
