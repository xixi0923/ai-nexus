// LLM 协议层（请求构造 / 响应解析）单元测试。
// 这些是纯函数、零网络依赖，可直接验证 OpenAI 兼容协议的编解码正确性。
#include "ai/llm_protocol.h"
#include <cassert>
#include <iostream>

int main() {
    using namespace nlohmann;

    // ---- 构造对话请求 ----
    auto req = build_chat_request("gpt-4o-mini",
        {{"system", "你是助手"}, {"user", "你好"}},
        /*stream=*/false, /*temperature=*/0.7);
    assert(req["model"] == "gpt-4o-mini");
    assert(req["stream"] == false);
    assert(req["temperature"] == 0.7);
    assert(req["messages"].size() == 2);
    assert(req["messages"][0]["role"] == "system");
    assert(req["messages"][1]["content"] == "你好");
    // max_tokens<=0 时不写入该字段
    assert(!req.contains("max_tokens"));

    auto req2 = build_chat_request("m", {}, /*stream=*/true, 0.2, /*max_tokens=*/256);
    assert(req2["stream"] == true);
    assert(req2["max_tokens"] == 256);

    // ---- 解析非流式回复 ----
    std::string body = R"({"choices":[{"message":{"content":"你好，有什么可以帮你？"}}],)"
                        R"("model":"gpt-4o-mini"})";
    assert(parse_chat_content(body) == "你好，有什么可以帮你？");

    // 结构不符时应抛异常（交由上层捕获降级）
    bool threw = false;
    try { parse_chat_content(R"({"foo":1})"); }
    catch (...) { threw = true; }
    assert(threw);

    // ---- 解析流式增量 ----
    assert(parse_stream_delta("[DONE]") == "");
    assert(parse_stream_delta("keep-alive") == "");             // 非 JSON
    assert(parse_stream_delta(R"({"choices":[{"delta":{"content":"你好"}}]})") == "你好");
    // 无 content 字段（如 role 事件）返回空串
    assert(parse_stream_delta(R"({"choices":[{"delta":{"role":"assistant"}}]})") == "");

    // ---- Embedding 请求/响应 ----
    auto ereq = build_embed_request("text-embedding-3-small", "hello world");
    assert(ereq["model"] == "text-embedding-3-small");
    assert(ereq["input"] == "hello world");

    std::string ebody = R"({"data":[{"embedding":[0.1,0.2,-0.3]}]})";
    auto vec = parse_embed_response(ebody);
    assert(vec.size() == 3);
    assert(vec[0] == 0.1f && vec[2] == -0.3f);

    std::cout << "[test_llm_protocol] ALL PASSED\n";
    return 0;
}
