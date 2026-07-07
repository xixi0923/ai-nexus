#pragma once
#include <string>
#include <vector>
#include <nlohmann/json.hpp>

class LLMClient;

// 自然语言处理引擎：意图识别 / 实体抽取 / 情感分析。
// 优先用 LLM 提示工程实现，无 LLM 密钥时退化为轻量规则，保证不整体降级。
class NLPEngine {
public:
    explicit NLPEngine(LLMClient& llm);

    // tasks 可为 ["intent","entities","sentiment"] 的子集
    nlohmann::json analyze(const std::string& text,
                           const std::vector<std::string>& tasks);

private:
    nlohmann::json analyze_by_llm(const std::string& text,
                                  const std::vector<std::string>& tasks);
    std::string rule_sentiment(const std::string& text) const;

    LLMClient& llm_;
};
