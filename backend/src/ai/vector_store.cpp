#include "ai/vector_store.h"
#include <cmath>
#include <algorithm>

void VectorStore::add(const std::string& id, const std::string& doc_id,
                      const std::string& chunk, const std::vector<float>& vec,
                      const nlohmann::json& metadata) {
    items_.push_back({id, doc_id, chunk, metadata, vec});
}

float VectorStore::cosine(const std::vector<float>& a, const std::vector<float>& b) {
    if (a.empty() || b.empty() || a.size() != b.size()) return 0.0f;
    double dot = 0.0, na = 0.0, nb = 0.0;
    for (size_t i = 0; i < a.size(); ++i) {
        dot += a[i] * b[i];
        na  += a[i] * a[i];
        nb  += b[i] * b[i];
    }
    double denom = std::sqrt(na) * std::sqrt(nb);
    return denom > 0.0 ? static_cast<float>(dot / denom) : 0.0f;
}

std::vector<std::pair<VectorItem, float>> VectorStore::search(
    const std::vector<float>& query_vec, size_t top_k, float threshold) const {
    std::vector<std::pair<VectorItem, float>> scored;
    scored.reserve(items_.size());
    for (const auto& it : items_) {
        float s = cosine(query_vec, it.vec);
        if (s >= threshold) scored.emplace_back(it, s);
    }
    std::partial_sort(scored.begin(),
                      scored.begin() + std::min(top_k, scored.size()),
                      scored.end(),
                      [](const auto& x, const auto& y) { return x.second > y.second; });
    if (scored.size() > top_k) scored.resize(top_k);
    return scored;
}
