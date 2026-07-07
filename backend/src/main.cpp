#include "server.h"
#include "config.h"
#include <iostream>

int main() {
    Config cfg = load_config();
    std::cout << "[ai-nexus] starting core service on port " << cfg.port << "\n";
    if (cfg.llm_api_key.empty()) {
        std::cerr << "[ai-nexus][warn] LLM_API_KEY 未设置，AI 调用将不可用。\n";
    }
    Server srv(cfg);
    srv.run();
    return 0;
}
