#include "ai/nlp_engine.h"
#include "ai/llm_client.h"
#include <algorithm>

NLPEngine::NLPEngine(LLMClient& llm) : llm_(llm) {}

nlohmann::json NLPEngine::analyze(const std::string& text,
                                  const std::vector<std::string>& tasks) {
    // 无 API Key 时退化为规则实现，保证服务可用
    if (tasks.empty()) return {{"text", text}};

    try {
        return analyze_by_llm(text, tasks);
    } catch (...) {
        nlohmann::json fallback = {{"text", text}, {"mode", "rule"}};
        if (std::find(tasks.begin(), tasks.end(), "sentiment") != tasks.end())
            fallback["sentiment"] = rule_sentiment(text);
        return fallback;
    }
}

nlohmann::json NLPEngine::analyze_by_llm(const std::string& text,
                                         const std::vector<std::string>& tasks) {
    nlohmann::json task_obj = nlohmann::json::object();
    for (const auto& t : tasks) task_obj[t] = true;

    std::string sys = "你是一个 NLP 分析引擎。仅输出 JSON，不要解释。"
                      "字段包括: intent(意图英文标识), entities(对象→值), sentiment(positive/neutral/negative)。";
    ChatMessage m1{"system", sys};
    ChatMessage m2{"user", "分析文本: " + text +
                          "\n需要的字段: " + task_obj.dump()};
    auto r = llm_.chat({m1, m2}, 0.2);
    if (r.contains("error")) throw std::runtime_error("llm nlp failed");
    std::string reply = r["reply"].get<std::string>();
    // 容错：截取首个 { 到末个 }
    auto a = reply.find('{'), b = reply.rfind('}');
    if (a == std::string::npos || b == std::string::npos)
        throw std::runtime_error("llm returned no json");
    return nlohmann::json::parse(reply.substr(a, b - a + 1));
}

// 极简规则情感：命中正向/负向词典即判定，否则 neutral
std::string NLPEngine::rule_sentiment(const std::string& text) const {
    static const std::vector<std::string> pos = {"好", "赞", "喜欢", "棒", "优秀", "great", "good", "love"};
    static const std::vector<std::string> neg = {"差", "烂", "讨厌", "糟", "坏", "bad", "hate", "worst"};
    for (const auto& w : neg) if (text.find(w) != std::string::npos) return "negative";
    for (const auto& w : pos) if (text.find(w) != std::string::npos) return "positive";
    return "neutral";
}
