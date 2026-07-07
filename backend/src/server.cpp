#include "server.h"
#include <ctime>
#include <random>
#include <sstream>
#include <iomanip>

namespace {

// 解析请求体为 JSON；失败返回空 object 并在 *ok 置 false
nlohmann::json parse_body(const std::string& body, bool& ok) {
    ok = true;
    if (body.empty()) { ok = false; return {}; }
    try { return nlohmann::json::parse(body); }
    catch (...) { ok = false; return {}; }
}

std::vector<ChatMessage> to_messages(const nlohmann::json& arr) {
    std::vector<ChatMessage> out;
    if (arr.is_array()) {
        for (const auto& m : arr) {
            if (m.contains("role") && m.contains("content"))
                out.push_back({m["role"].get<std::string>(),
                               m["content"].get<std::string>()});
        }
    }
    return out;
}

} // namespace

Server::Server(const Config& cfg)
    : cfg_(cfg),
      llm_(cfg.llm_base_url, cfg.llm_api_key, cfg.llm_model, cfg.llm_timeout_ms,
           cfg.embed_base_url, cfg.embed_api_key, cfg.embed_model),
      nlp_(llm_) {
    register_routes();
}

void Server::register_routes() {
    svr_.Post("/api/chat",     [this](auto& q, auto& s){ handle_chat(q, s); });
    svr_.Post("/api/embed",    [this](auto& q, auto& s){ handle_embed(q, s); });
    svr_.Post("/api/ingest",   [this](auto& q, auto& s){ handle_ingest(q, s); });
    svr_.Post("/api/retrieve", [this](auto& q, auto& s){ handle_retrieve(q, s); });
    svr_.Post("/api/rag",      [this](auto& q, auto& s){ handle_rag(q, s); });
    svr_.Post("/api/nlp/analyze", [this](auto& q, auto& s){ handle_nlp(q, s); });
    svr_.Get("/api/health",    [this](auto& q, auto& s){ handle_health(q, s); });
    svr_.Get("/health",        [this](auto& q, auto& s){ handle_health(q, s); });
}

nlohmann::json Server::envelope(int code, const std::string& msg,
                                const nlohmann::json& data, const std::string& rid) {
    return {{"code", code}, {"message", msg}, {"data", data}, {"request_id", rid}};
}

bool Server::authed(const httplib::Request& req) const {
    auto it = req.headers.find("X-Backend-Token");
    if (it == req.headers.end()) return false;
    return it->second == cfg_.backend_token;
}

std::string Server::new_rid() {
    static std::mt19937 rng(std::random_device{}());
    std::uniform_int_distribution<int> d(0, 15);
    std::stringstream ss;
    ss << std::hex;
    for (int i = 0; i < 16; ++i) ss << d(rng);
    return ss.str();
}

std::vector<std::string> Server::chunk_text(const std::string& text,
                                            size_t chunk_size, size_t overlap) {
    std::vector<std::string> chunks;
    if (text.empty()) return chunks;
    if (chunk_size == 0) chunk_size = 512;
    size_t step = chunk_size > overlap ? chunk_size - overlap : chunk_size;
    for (size_t i = 0; i < text.size(); i += step) {
        chunks.push_back(text.substr(i, chunk_size));
        if (i + chunk_size >= text.size()) break;
    }
    return chunks;
}

void Server::handle_health(const httplib::Request&, httplib::Response& res) {
    nlohmann::json comps = {
        {"llm",   cfg_.llm_api_key.empty() ? "down" : "up"},
        {"embed", cfg_.embed_api_key.empty() ? "down" : "up"},
        {"vector", cfg_.vector_backend}};
    nlohmann::json data = {{"status", "ok"},
                           {"version", "1.0.0"},
                           {"components", comps},
                           {"docs", "see docs/API.md"}};
    res.set_header("Content-Type", "application/json");
    res.set_content(envelope(0, "ok", data, new_rid()).dump(), "application/json");
}

void Server::handle_chat(const httplib::Request& req, httplib::Response& res) {
    std::string rid = new_rid();
    if (!authed(req)) {
        res.status = 403;
        res.set_content(envelope(40301, "backend token mismatch", {}, rid).dump(),
                        "application/json");
        return;
    }
    bool ok; auto j = parse_body(req.body, ok);
    if (!ok || !j.contains("messages")) {
        res.status = 400;
        res.set_content(envelope(40001, "missing 'messages'", {}, rid).dump(),
                        "application/json");
        return;
    }
    auto messages = to_messages(j["messages"]);
    double temp = j.value("temperature", 0.7);
    bool stream = j.value("stream", false);

    if (stream) {
        // 收集增量后按 SSE 分块回放（scaffold 简化：先完整拉取再流式下发）
        struct StreamState { std::vector<std::string> deltas; size_t idx = 0; bool done = false; };
        auto st = std::make_shared<StreamState>();
        llm_.chat_stream(messages, [&](const std::string& d){ st->deltas.push_back(d); }, temp);
        res.set_header("Content-Type", "text/event-stream");
        res.set_header("Cache-Control", "no-cache");
        res.set_chunked_content_provider("text/event-stream",
            [st](size_t, httplib::DataSink& sink) -> bool {
                if (st->idx < st->deltas.size()) {
                    std::string s = "data: " + st->deltas[st->idx++] + "\n\n";
                    sink.write(s.data(), s.size());
                    return true;
                }
                if (!st->done) {
                    st->done = true;
                    std::string s = "data: [DONE]\n\n";
                    sink.write(s.data(), s.size());
                    return true;
                }
                return false;
            });
        return;
    }

    nlohmann::json r = llm_.chat(messages, temp);
    if (r.contains("error")) {
        res.status = 503;
        res.set_content(envelope(50301, "llm provider error",
                                 {{"detail", r.value("body", "")}}, rid).dump(),
                        "application/json");
        return;
    }
    res.set_content(envelope(0, "ok", r, rid).dump(), "application/json");
}

void Server::handle_embed(const httplib::Request& req, httplib::Response& res) {
    std::string rid = new_rid();
    if (!authed(req)) { res.status = 403; res.set_content(envelope(40301,"token",{},rid).dump(),"application/json"); return; }
    bool ok; auto j = parse_body(req.body, ok);
    if (!ok || !j.contains("text")) { res.status = 400; res.set_content(envelope(40001,"missing text",{},rid).dump(),"application/json"); return; }
    try {
        auto vec = llm_.embed(j["text"].get<std::string>());
        res.set_content(envelope(0, "ok",
            {{"embedding", vec}, {"dim", vec.size()}}, rid).dump(), "application/json");
    } catch (const std::exception& e) {
        res.status = 503;
        res.set_content(envelope(50301, std::string("embed failed: ") + e.what(), {}, rid).dump(), "application/json");
    }
}

void Server::handle_ingest(const httplib::Request& req, httplib::Response& res) {
    std::string rid = new_rid();
    if (!authed(req)) { res.status = 403; res.set_content(envelope(40301,"token",{},rid).dump(),"application/json"); return; }
    bool ok; auto j = parse_body(req.body, ok);
    if (!ok || !j.contains("text") || !j.contains("doc_id")) {
        res.status = 400; res.set_content(envelope(40001,"missing text/doc_id",{},rid).dump(),"application/json"); return;
    }
    std::string doc_id = j["doc_id"].get<std::string>();
    std::string text   = j["text"].get<std::string>();
    size_t chunk_size  = j.value("chunk_size", 512);
    nlohmann::json metadata = j.value("metadata", nlohmann::json::object());

    auto chunks = chunk_text(text, chunk_size);
    size_t stored = 0;
    for (size_t i = 0; i < chunks.size(); ++i) {
        try {
            auto vec = llm_.embed(chunks[i]);
            std::string cid = doc_id + "#" + std::to_string(i);
            store_.add(cid, doc_id, chunks[i], vec, metadata);
            ++stored;
        } catch (...) { /* 单块失败跳过，继续后续 */ }
    }
    res.set_content(envelope(0, "ok",
        {{"doc_id", doc_id}, {"chunks", chunks.size()}, {"embedded", stored}}, rid).dump(),
        "application/json");
}

void Server::handle_retrieve(const httplib::Request& req, httplib::Response& res) {
    std::string rid = new_rid();
    if (!authed(req)) { res.status = 403; res.set_content(envelope(40301,"token",{},rid).dump(),"application/json"); return; }
    bool ok; auto j = parse_body(req.body, ok);
    if (!ok || !j.contains("query")) { res.status = 400; res.set_content(envelope(40001,"missing query",{},rid).dump(),"application/json"); return; }
    size_t top_k = j.value("top_k", 5);
    float thr = j.value("score_threshold", 0.0f);
    try {
        auto qv = llm_.embed(j["query"].get<std::string>());
        auto hits = store_.search(qv, top_k, thr);
        nlohmann::json results = nlohmann::json::array();
        for (const auto& [item, score] : hits) {
            results.push_back({{"doc_id", item.doc_id},
                               {"score", score},
                               {"chunk", item.chunk},
                               {"metadata", item.metadata}});
        }
        res.set_content(envelope(0, "ok", {{"results", results}}, rid).dump(), "application/json");
    } catch (const std::exception& e) {
        res.status = 503; res.set_content(envelope(50301, std::string("retrieve failed: ")+e.what(), {}, rid).dump(), "application/json");
    }
}

void Server::handle_rag(const httplib::Request& req, httplib::Response& res) {
    std::string rid = new_rid();
    if (!authed(req)) { res.status = 403; res.set_content(envelope(40301,"token",{},rid).dump(),"application/json"); return; }
    bool ok; auto j = parse_body(req.body, ok);
    if (!ok || !j.contains("question")) { res.status = 400; res.set_content(envelope(40001,"missing question",{},rid).dump(),"application/json"); return; }
    std::string question = j["question"].get<std::string>();
    size_t top_k = j.value("top_k", 5);
    try {
        auto qv = llm_.embed(question);
        auto hits = store_.search(qv, top_k, 0.1f);
        std::string context;
        nlohmann::json citations = nlohmann::json::array();
        for (const auto& [item, score] : hits) {
            context += "[来源 " + item.doc_id + "]\n" + item.chunk + "\n\n";
            citations.push_back({{"doc_id", item.doc_id}, {"score", score}});
        }
        std::vector<ChatMessage> msgs = {
            {"system", "你是基于检索内容的助手。只依据下方<context>作答，"
                       "不知道就说不知道，并在答案后附上引用来源。\n<context>\n" + context + "</context>"},
            {"user", question}};
        nlohmann::json r = llm_.chat(msgs, 0.3);
        if (r.contains("error")) { res.status = 503; res.set_content(envelope(50301,"llm error",{},rid).dump(),"application/json"); return; }
        r["citations"] = citations;
        res.set_content(envelope(0, "ok", r, rid).dump(), "application/json");
    } catch (const std::exception& e) {
        res.status = 503; res.set_content(envelope(50301, std::string("rag failed: ")+e.what(), {}, rid).dump(), "application/json");
    }
}

void Server::handle_nlp(const httplib::Request& req, httplib::Response& res) {
    std::string rid = new_rid();
    if (!authed(req)) { res.status = 403; res.set_content(envelope(40301,"token",{},rid).dump(),"application/json"); return; }
    bool ok; auto j = parse_body(req.body, ok);
    if (!ok || !j.contains("text")) { res.status = 400; res.set_content(envelope(40001,"missing text",{},rid).dump(),"application/json"); return; }
    std::vector<std::string> tasks;
    if (j.contains("tasks") && j["tasks"].is_array())
        for (const auto& t : j["tasks"]) tasks.push_back(t.get<std::string>());
    nlohmann::json r = nlp_.analyze(j["text"].get<std::string>(), tasks);
    r["request_id"] = rid;
    res.set_content(envelope(0, "ok", r, rid).dump(), "application/json");
}

void Server::run() {
    svr_.listen("0.0.0.0", cfg_.port);
}
