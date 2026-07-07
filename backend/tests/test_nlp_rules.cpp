// 规则型 NLP 单元测试（无需 LLM、零网络依赖）。
// 构建：cmake + make && ctest  或  g++ test_nlp_rules.cpp ../src/ai/nlp_rules.cpp
//       -I../include -I<json> -std=c++17
#include "ai/nlp_rules.h"
#include <cassert>
#include <iostream>

int main() {
    using namespace nlp_rules;

    // ---- 情感分析 ----
    assert(sentiment("这个产品太差了，很失望") == "negative");
    assert(sentiment("体验很好，我非常喜欢")   == "positive");
    assert(sentiment("今天天气多云")           == "neutral");
    assert(sentiment("terrible service, I hate it") == "negative");
    assert(sentiment("great, I love it")           == "positive");

    // ---- 意图识别（关键词优先级）----
    assert(intent("明天天气怎么样")           == "weather");
    assert(intent("我要重置登录密码")         == "reset_password");
    assert(intent("帮我订一张去北京机票")     == "book");
    assert(intent("查一下我的订单状态")       == "search");
    assert(intent("你好，在吗")               == "general_qa");
    // 同时含「天气」「查」时，靠前的 weather 优先
    assert(intent("查一下北京天气")           == "weather");

    // ---- 实体抽取 ----
    auto e = entities("联系 foo@bar.com 或 13812345678，地点北京，金额 ¥100，明天出发");
    assert(!e["emails"].empty());
    assert(e["emails"][0].get<std::string>() == "foo@bar.com");
    assert(!e["phones"].empty());
    assert(e["phones"][0].get<std::string>() == "13812345678");
    assert(!e["locations"].empty());
    assert(e["locations"][0].get<std::string>() == "北京");
    assert(!e["amounts"].empty());
    assert(e["amounts"][0].get<std::string>() == "100");
    assert(!e["dates"].empty());
    assert(e["dates"][0].get<std::string>() == "明天");
    // 无实体时返回空数组而非缺失字段
    auto e2 = entities("随便聊聊");
    assert(e2["emails"].empty());
    assert(e2["phones"].empty());

    std::cout << "[test_nlp_rules] ALL PASSED\n";
    return 0;
}
