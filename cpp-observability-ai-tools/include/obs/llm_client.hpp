#pragma once

#include <string>
#include <vector>
#include <memory>
#include <nlohmann/json.hpp>

namespace obs::ai {

struct Message {
    std::string role; // "system", "user", "assistant"
    std::string content;
};

struct LLMRequest {
    std::string model{"gpt-4"};
    std::vector<Message> messages;
    double temperature{0.7};
    int max_tokens{2000};
    std::vector<std::string> stop_sequences;
    nlohmann::json tools; // For function calling
};

struct LLMResponse {
    std::string content;
    std::string finish_reason;
    int total_tokens{0};
    int prompt_tokens{0};
    int completion_tokens{0};
    nlohmann::json function_call; // If function calling is used
};

class LLMClient {
public:
    explicit LLMClient(const std::string& api_key,
                      const std::string& base_url = "https://api.openai.com/v1");

    // Chat completions
    LLMResponse chat(const LLMRequest& request);
    LLMResponse chat(const std::string& prompt,
                    const std::string& system_prompt = "");

    // Streaming (for future implementation)
    using StreamCallback = std::function<void(const std::string&)>;
    void chat_stream(const LLMRequest& request, StreamCallback callback);

    // Embeddings
    std::vector<double> get_embedding(const std::string& text,
                                     const std::string& model = "text-embedding-ada-002");

    // Configuration
    void set_api_key(const std::string& api_key);
    void set_base_url(const std::string& base_url);
    void set_timeout(int seconds);

private:
    std::string make_request(const std::string& endpoint,
                            const nlohmann::json& payload);

    std::string api_key_;
    std::string base_url_;
    int timeout_seconds_{30};
};

} // namespace obs::ai
