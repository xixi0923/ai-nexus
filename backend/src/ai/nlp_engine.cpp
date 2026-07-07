#include "ai/nlp_engine.h"
#include "ai/llm_client.h"
#include "ai/nlp_rules.h"
#include <algorithm>

NLPEngine::NLPEngine(LLMClient& llm) : llm_(llm) {}

nlohmann::json NLPEngine::analyze(const std::string& text,
                                  const std::vector<std::string>& tasks) {
    // 无 API Key / LLM 失败时退化为规则引擎，保证服务可用
    if (tasks.empty()) return {{"text", text}};

    try {
        return analyze_by_llm(text, tasks);
    } catch (...) {
        nlohmann::json fallback = {{"text", text}, {"mode", "rule"}};
        for (const auto& t : tasks) {
            if (t == "sentiment")      fallback["sentiment"] = nlp_rules::sentiment(text);
            else if (t == "intent")    fallback["intent"]    = nlp_rules::intent(text);
            else if (t == "entities")  fallback["entities"]  = nlp_rules::entities(text);
        }
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
