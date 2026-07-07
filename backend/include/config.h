#pragma once
#include <string>

// 运行时配置：优先读环境变量，缺失时回退到默认值。
// 所有 AI 密钥仅存在于服务端环境变量，绝不进入前端。
struct Config {
    // 服务
    int         port = 8080;
    std::string backend_token = "dev-backend-token"; // 校验 PHP 中转层带来的 X-Backend-Token

    // LLM（OpenAI 兼容协议）
    std::string llm_base_url  = "https://api.openai.com/v1";
    std::string llm_api_key   = "";
    std::string llm_model     = "gpt-4o-mini";
    int         llm_timeout_ms = 30000;

    // Embedding
    std::string embed_base_url = "https://api.openai.com/v1";
    std::string embed_api_key  = "";
    std::string embed_model    = "text-embedding-3-small";
    int         embed_dim      = 1536;

    // 向量检索
    std::string vector_backend = "memory";          // memory | qdrant
    std::string qdrant_url     = "http://localhost:6333";
};

// 从环境变量加载配置（线程安全：仅启动时调用一次）
Config load_config();
