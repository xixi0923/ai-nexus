#pragma once
#include <string>
#include <vector>
#include <functional>
#include <nlohmann/json.hpp>

#include "ai/llm_protocol.h"   // ChatMessage 与请求/响应协议纯函数（header-only）

// 统一的大模型调用适配层。
// 默认对接 OpenAI 兼容 /v1/chat/completions 与 /v1/embeddings，
// 替换 base_url/api_key 即可切换到本地 llama.cpp / vLLM / 国产大模型。
class LLMClient {
public:
    LLMClient(const std::string& base_url,
              const std::string& api_key,
              const std::string& model,
              int timeout_ms,
              const std::string& embed_base_url = "",
              const std::string& embed_api_key  = "",
              const std::string& embed_model    = "");

    // 非流式对话。返回完整响应 JSON（含 reply / model / usage）。
    nlohmann::json chat(const std::vector<ChatMessage>& messages,
                        double temperature = 0.7);

    // 流式对话。on_delta 逐片回调增量文本。返回 true 表示成功。
    bool chat_stream(const std::vector<ChatMessage>& messages,
                     const std::function<void(const std::string&)>& on_delta,
                     double temperature = 0.7);

    // 单条文本向量化。
    std::vector<float> embed(const std::string& text);

    // 批量向量化。
    std::vector<std::vector<float>> embed_batch(const std::vector<std::string>& texts);

private:
    // 执行 HTTPS POST，返回 (http_status, body)
    std::pair<int, std::string> post_json(const std::string& path,
                                          const nlohmann::json& payload,
                                          bool stream = false);

    std::string base_url_;
    std::string api_key_;
    std::string model_;
    int         timeout_ms_;

    // Embedding 配置（缺省时复用 LLM 的 base/key）
    std::string embed_base_url_;
    std::string embed_api_key_;
    std::string embed_model_;
};
