#pragma once
#include <string>
#include <nlohmann/json.hpp>

// 规则型 NLP（无需 LLM）。用作 LLM 不可用时的降级实现，亦可独立单测。
namespace nlp_rules {

// 极简情感分析：命中负向/正向词典即判定，否则 neutral。
std::string sentiment(const std::string& text);

// 意图识别：基于关键词的轻量分类，返回英文意图标识。
// 例如 weather / reset_password / book / search / general_qa。
std::string intent(const std::string& text);

// 实体抽取：邮箱、手机号、金额、地点、日期时间。返回分类后的对象。
// { "emails":[...], "phones":[...], "amounts":[...], "locations":[...], "dates":[...] }
nlohmann::json entities(const std::string& text);

} // namespace nlp_rules
