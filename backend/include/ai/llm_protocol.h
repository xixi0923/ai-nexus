#pragma once
#include <string>
#include <vector>
#include <nlohmann/json.hpp>

// 与 LLM 提供方（OpenAI 兼容）交互的纯函数协议层。
// 设计为 header-only、零网络依赖，便于单元测试直接引用，
// 同时被 llm_client.cpp 复用，避免请求/响应解析逻辑重复。

// 单条对话消息
struct ChatMessage {
    std::string role;    // "system" | "user" | "assistant"
    std::string content;
};

// 构造 /chat/completions 请求体。max_tokens<=0 时不写入该字段。
inline nlohmann::json build_chat_request(const std::string& model,
                                         const std::vector<ChatMessage>& messages,
                                         bool stream,
                                         double temperature,
                                         int max_tokens = 0) {
    nlohmann::json arr = nlohmann::json::array();
    for (const auto& m : messages)
        arr.push_back({{"role", m.role}, {"content", m.content}});
    nlohmann::json p = {
        {"model", model},
        {"messages", arr},
        {"temperature", temperature},
        {"stream", stream}};
    if (max_tokens > 0) p["max_tokens"] = max_tokens;
    return p;
}

// 构造 /embeddings 请求体。input 可为字符串或字符串数组。
inline nlohmann::json build_embed_request(const std::string& model,
                                          const nlohmann::json& input) {
    return {{"model", model}, {"input", input}};
}

// 从 /chat/completions 的完整响应体解析助手回复文本。
// 解析失败（结构不符）时抛出 nlohmann::json 异常，便于上层捕获降级。
inline std::string parse_chat_content(const std::string& body) {
    auto j = nlohmann::json::parse(body);
    return j["choices"][0]["message"]["content"].get<std::string>();
}

// 从流式 SSE 的单个 `data:` 载荷（已去除 "data: " 前缀）解析增量文本。
// 遇到 [DONE] / 非 JSON / 无 content 字段时返回空串（不抛异常）。
inline std::string parse_stream_delta(const std::string& data_payload) {
    if (data_payload == "[DONE]") return "";
    try {
        auto j = nlohmann::json::parse(data_payload);
        if (j.is_array()) {            // 兼容极少数返回数组的提供商
            if (j.empty()) return "";
            j = j[0];
        }
        if (j.contains("choices") && j["choices"].is_array() && !j["choices"].empty()) {
            const auto& c = j["choices"][0];
            if (c.contains("delta") && c["delta"].contains("content"))
                return c["delta"]["content"].get<std::string>();
        }
    } catch (...) { /* 跳过 keep-alive / 非 JSON 行 */ }
    return "";
}

// 从 /embeddings 响应体解析向量（取 data[0].embedding）。
inline std::vector<float> parse_embed_response(const std::string& body) {
    auto j = nlohmann::json::parse(body);
    std::vector<float> vec;
    for (const auto& v : j["data"][0]["embedding"])
        vec.push_back(v.get<float>());
    return vec;
}
