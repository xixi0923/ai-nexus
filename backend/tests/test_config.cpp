// 配置加载单元测试：默认值 + 环境变量覆盖 + 非法值回退。
#include "config.h"
#include <cassert>
#include <iostream>

#ifdef _WIN32
#include <stdlib.h>
static void set_env(const char* k, const char* v) { _putenv_s(k, v); }
#else
#include <stdlib.h>
static void set_env(const char* k, const char* v) { setenv(k, v, 1); }
#endif

int main() {
    // ---- 默认配置 ----
    {
        Config c = load_config();
        assert(c.port == 8080);
        assert(c.backend_token == "dev-backend-token");
        assert(c.llm_base_url == "https://api.openai.com/v1");
        assert(c.llm_model == "gpt-4o-mini");
        assert(c.llm_timeout_ms == 30000);
        assert(c.embed_dim == 1536);
        assert(c.vector_backend == "memory");
    }

    // ---- 环境变量覆盖 ----
    set_env("SERVER_PORT", "9090");
    set_env("LLM_MODEL", "llama-3.1-8b");
    set_env("VECTOR_BACKEND", "qdrant");
    set_env("EMBED_DIM", "768");
    {
        Config c = load_config();
        assert(c.port == 9090);
        assert(c.llm_model == "llama-3.1-8b");
        assert(c.vector_backend == "qdrant");
        assert(c.embed_dim == 768);
    }

    // ---- 非法整型回退默认 ----
    set_env("SERVER_PORT", "not-a-number");
    set_env("EMBED_DIM", "");   // 空值回退默认
    {
        Config c = load_config();
        assert(c.port == 8080);       // 解析失败 → 默认
        assert(c.embed_dim == 1536);  // 空值 → 默认
    }

    std::cout << "[test_config] ALL PASSED\n";
    return 0;
}
