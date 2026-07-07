#include "ai/llm_client.h"
#include <httplib.h>
#include <algorithm>
#include <stdexcept>

LLMClient::LLMClient(const std::string& base_url, const std::string& api_key,
                     const std::string& model, int timeout_ms,
                     const std::string& embed_base_url,
                     const std::string& embed_api_key,
                     const std::string& embed_model)
    : base_url_(base_url), api_key_(api_key), model_(model), timeout_ms_(timeout_ms),
      embed_base_url_(embed_base_url.empty() ? base_url : embed_base_url),
      embed_api_key_(embed_api_key.empty() ? api_key : embed_api_key),
      embed_model_(embed_model.empty() ? model : embed_model) {}

std::pair<int, std::string> LLMClient::post_json(const std::string& path,
                                                 const nlohmann::json& payload,
                                                 bool stream) {
    httplib::Client cli(base_url_.c_str());
    cli.set_connection_timeout(10);
    int sec = std::max(1, timeout_ms_ / 1000);
    cli.set_read_timeout(sec, 0);
    httplib::Headers headers = {
        {"Authorization", "Bearer " + api_key_},
        {"Content-Type", "application/json"},
        {"Accept", stream ? "text/event-stream" : "application/json"}};
    auto res = cli.Post(path.c_str(), headers, payload.dump(), "application/json");
    if (!res) return {0, ""};
    return {res->status, res->body};
}

nlohmann::json LLMClient::chat(const std::vector<ChatMessage>& messages,
                               double temperature) {
    nlohmann::json payload = build_chat_request(model_, messages, false, temperature);

    auto [status, body] = post_json("/chat/completions", payload, false);
    if (status != 200)
        return {{"error", true}, {"http_status", status}, {"body", body}};

    try {
        std::string reply = parse_chat_content(body);
        auto j = nlohmann::json::parse(body);
        nlohmann::json usage = j.value("usage", nlohmann::json::object());
        return {{"reply", reply}, {"model", j.value("model", model_)}, {"usage", usage}};
    } catch (...) {
        return {{"error", true}, {"http_status", status}, {"body", body}};
    }
}

bool LLMClient::chat_stream(const std::vector<ChatMessage>& messages,
                            const std::function<void(const std::string&)>& on_delta,
                            double temperature) {
    nlohmann::json msg_arr = nlohmann::json::array();
    for (const auto& m : messages)
        msg_arr.push_back({{"role", m.role}, {"content", m.content}});
    nlohmann::json payload = {
        {"model", model_}, {"messages", msg_arr},
        {"temperature", temperature}, {"stream", true}};

    httplib::Client cli(base_url_.c_str());
    cli.set_connection_timeout(10);
    int sec = std::max(1, timeout_ms_ / 1000);
    cli.set_read_timeout(sec, 0);
    httplib::Headers headers = {
        {"Authorization", "Bearer " + api_key_},
        {"Content-Type", "application/json"},
        {"Accept", "text/event-stream"}};

    std::string buf;
    auto flush_line = [&](const std::string& raw) {
        std::string line = raw;
        if (!line.empty() && line.back() == '\r') line.pop_back();
        if (line.rfind("data:", 0) != 0) return;
        std::string data = line.substr(5);
        size_t p = data.find_first_not_of(" ");
        if (p != std::string::npos) data = data.substr(p);
        if (data == "[DONE]") return;
        try {
            auto j = nlohmann::json::parse(data);
            std::string delta = j["choices"][0]["delta"]["content"].get<std::string>();
            if (!delta.empty()) on_delta(delta);
        } catch (...) { /* 跳过 keep-alive 或非 JSON 行 */ }
    };

    auto res = cli.Post("/chat/completions", headers, payload.dump(), "application/json",
        [&](const char* data, size_t len) {
            buf.append(data, len);
            size_t nl;
            while ((nl = buf.find('\n')) != std::string::npos) {
                flush_line(buf.substr(0, nl));
                buf.erase(0, nl + 1);
            }
            return true;
        });
    return res && res->status == 200;
}

std::vector<float> LLMClient::embed(const std::string& text) {
    httplib::Client cli(embed_base_url_.c_str());
    cli.set_connection_timeout(10);
    cli.set_read_timeout(std::max(1, timeout_ms_ / 1000), 0);
    httplib::Headers headers = {
        {"Authorization", "Bearer " + embed_api_key_},
        {"Content-Type", "application/json"}};
    nlohmann::json payload = {{"model", embed_model_}, {"input", text}};
    auto res = cli.Post("/embeddings", headers, payload.dump(), "application/json");
    if (!res || res->status != 200)
        throw std::runtime_error("embedding failed, http=" +
                                 std::to_string(res ? res->status : 0));
    auto j = nlohmann::json::parse(res->body);
    std::vector<float> vec;
    for (const auto& v : j["data"][0]["embedding"])
        vec.push_back(v.get<float>());
    return vec;
}

std::vector<std::vector<float>> LLMClient::embed_batch(
    const std::vector<std::string>& texts) {
    std::vector<std::vector<float>> out;
    out.reserve(texts.size());
    for (const auto& t : texts) out.push_back(embed(t));
    return out;
}
