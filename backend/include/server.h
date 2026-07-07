#pragma once
#include "config.h"
#include "ai/llm_client.h"
#include "ai/nlp_engine.h"
#include "ai/vector_store.h"
#include <httplib.h>
#include <memory>

// C++ 核心服务：暴露 REST API，编排 LLM / NLP / 向量检索（RAG）。
class Server {
public:
    explicit Server(const Config& cfg);
    void run();

private:
    void register_routes();

    static nlohmann::json envelope(int code, const std::string& msg,
                                   const nlohmann::json& data,
                                   const std::string& rid);
    bool authed(const httplib::Request& req) const;
    static std::string new_rid();

    // 业务处理
    void handle_chat(const httplib::Request& req, httplib::Response& res);
    void handle_embed(const httplib::Request& req, httplib::Response& res);
    void handle_ingest(const httplib::Request& req, httplib::Response& res);
    void handle_retrieve(const httplib::Request& req, httplib::Response& res);
    void handle_rag(const httplib::Request& req, httplib::Response& res);
    void handle_nlp(const httplib::Request& req, httplib::Response& res);
    void handle_health(const httplib::Request& req, httplib::Response& res);

    // 文本切片（按字符长度，带简单重叠）
    static std::vector<std::string> chunk_text(const std::string& text,
                                               size_t chunk_size,
                                               size_t overlap = 0);

    Config       cfg_;
    LLMClient    llm_;
    NLPEngine    nlp_;
    VectorStore  store_;
    httplib::Server svr_;
};
