#include "ai/nlp_rules.h"
#include <algorithm>
#include <regex>

namespace {

const std::vector<std::string>& pos_words() {
    static const std::vector<std::string> w = {
        "好", "赞", "喜欢", "棒", "优秀", "完美", "满意",
        "great", "good", "love", "nice", "awesome"};
    return w;
}
const std::vector<std::string>& neg_words() {
    static const std::vector<std::string> w = {
        "差", "烂", "讨厌", "糟", "坏", "失望", "生气",
        "bad", "hate", "worst", "terrible"};
    return w;
}

// 意图关键词（顺序即优先级，靠前者优先匹配）
const std::vector<std::pair<std::string, std::string>>& intent_rules() {
    static const std::vector<std::pair<std::string, std::string>> r = {
        {"天气", "weather"},
        {"重置", "reset_password"},
        {"密码", "reset_password"},
        {"订",   "book"},
        {"买",   "book"},
        {"机票", "book"},
        {"票",   "book"},
        {"查",   "search"},
        {"搜索", "search"},
        {"找",   "search"},
        {"查询", "search"}};
    return r;
}

const std::vector<std::string>& locations() {
    static const std::vector<std::string> c = {
        "北京", "上海", "广州", "深圳", "杭州", "成都", "武汉",
        "南京", "西安", "重庆", "苏州", "天津",
        "纽约", "伦敦", "东京", "巴黎", "新加坡"};
    return c;
}

const std::vector<std::string>& date_words() {
    static const std::vector<std::string> d = {
        "今天", "明天", "后天", "大后天", "本周", "下周", "周末",
        "周一", "周二", "周三", "周四", "周五", "周六", "周日",
        "下周一", "下周二", "下周三", "下周四", "下周五", "下周六", "下周日"};
    return d;
}

} // namespace

namespace nlp_rules {

std::string sentiment(const std::string& text) {
    for (const auto& w : neg_words())
        if (text.find(w) != std::string::npos) return "negative";
    for (const auto& w : pos_words())
        if (text.find(w) != std::string::npos) return "positive";
    return "neutral";
}

std::string intent(const std::string& text) {
    for (const auto& [kw, label] : intent_rules())
        if (text.find(kw) != std::string::npos) return label;
    return "general_qa";
}

nlohmann::json entities(const std::string& text) {
    nlohmann::json out = nlohmann::json::object();
    out["emails"]    = nlohmann::json::array();
    out["phones"]    = nlohmann::json::array();
    out["amounts"]   = nlohmann::json::array();
    out["locations"] = nlohmann::json::array();
    out["dates"]     = nlohmann::json::array();

    // 邮箱
    std::regex email_re(R"([A-Za-z0-9._%+-]+@[A-Za-z0-9.-]+\.[A-Za-z]{2,})");
    for (auto it = std::sregex_iterator(text.begin(), text.end(), email_re);
         it != std::sregex_iterator(); ++it)
        out["emails"].push_back(it->str());

    // 手机号（中国大陆 11 位）
    std::regex phone_re(R"(1[3-9][0-9]{9})");
    for (auto it = std::sregex_iterator(text.begin(), text.end(), phone_re);
         it != std::sregex_iterator(); ++it)
        out["phones"].push_back(it->str());

    // 金额（整数或小数）
    std::regex amount_re(R"([0-9]+(?:\.[0-9]+)?)");
    for (auto it = std::sregex_iterator(text.begin(), text.end(), amount_re);
         it != std::sregex_iterator(); ++it)
        out["amounts"].push_back(it->str());

    // 地点（词典匹配）
    for (const auto& loc : locations())
        if (text.find(loc) != std::string::npos) out["locations"].push_back(loc);

    // 日期/时间（词典匹配）
    for (const auto& dw : date_words())
        if (text.find(dw) != std::string::npos) out["dates"].push_back(dw);

    return out;
}

} // namespace nlp_rules
