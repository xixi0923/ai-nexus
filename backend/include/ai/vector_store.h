#pragma once
#include <string>
#include <vector>
#include <nlohmann/json.hpp>

// 单条向量记录
struct VectorItem {
    std::string id;
    std::string doc_id;
    std::string chunk;
    nlohmann::json metadata;
    std::vector<float> vec;
};

// 进程内向量检索（余弦相似度）。
// 零依赖演示实现；生产可令 VECTOR_BACKEND=qdrant，接口保持 add/search 不变。
class VectorStore {
public:
    void add(const std::string& id,
             const std::string& doc_id,
             const std::string& chunk,
             const std::vector<float>& vec,
             const nlohmann::json& metadata = nlohmann::json::object());

    // 返回 top_k 个 (item, score)，score 为余弦相似度；低于 threshold 的过滤掉。
    std::vector<std::pair<VectorItem, float>> search(
        const std::vector<float>& query_vec,
        size_t top_k = 5,
        float threshold = 0.0f) const;

    size_t size() const { return items_.size(); }

private:
    static float cosine(const std::vector<float>& a, const std::vector<float>& b);

    std::vector<VectorItem> items_;
};
