#include "config.h"
#include <cstdlib>
#include <string>

namespace {

// 读取环境变量，缺失则回退 default_val
std::string env_or(const char* name, const std::string& default_val) {
    const char* v = std::getenv(name);
    return (v && v[0] != '\0') ? std::string(v) : default_val;
}

int env_int_or(const char* name, int default_val) {
    const char* v = std::getenv(name);
    if (v && v[0] != '\0') {
        try { return std::stoi(v); } catch (...) { return default_val; }
    }
    return default_val;
}

} // namespace

Config load_config() {
    Config c;
    c.port          = env_int_or("SERVER_PORT", c.port);
    c.backend_token = env_or("BACKEND_TOKEN", c.backend_token);

    c.llm_base_url   = env_or("LLM_BASE_URL", c.llm_base_url);
    c.llm_api_key    = env_or("LLM_API_KEY", c.llm_api_key);
    c.llm_model      = env_or("LLM_MODEL", c.llm_model);
    c.llm_timeout_ms = env_int_or("LLM_TIMEOUT_MS", c.llm_timeout_ms);

    c.embed_base_url = env_or("EMBED_BASE_URL", c.embed_base_url);
    c.embed_api_key  = env_or("EMBED_API_KEY", c.embed_api_key);
    c.embed_model    = env_or("EMBED_MODEL", c.embed_model);
    c.embed_dim      = env_int_or("EMBED_DIM", c.embed_dim);

    c.vector_backend = env_or("VECTOR_BACKEND", c.vector_backend);
    c.qdrant_url     = env_or("QDRANT_URL", c.qdrant_url);
    return c;
}
