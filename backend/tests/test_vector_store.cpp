#include "ai/vector_store.h"
#include <cassert>
#include <iostream>

int main() {
    VectorStore store;

    // 正交向量：点积应接近 0
    store.add("a", "d1", "alpha", {1.0f, 0.0f, 0.0f});
    store.add("b", "d1", "beta",  {0.0f, 1.0f, 0.0f});
    store.add("c", "d2", "gamma", {0.0f, 0.0f, 1.0f});
    // 与 a 同向，应排第一
    store.add("d", "d2", "alpha2",{1.0f, 0.0f, 0.0f});

    assert(store.size() == 4);

    auto hits = store.search({1.0f, 0.0f, 0.0f}, 2);
    assert(hits.size() == 2);
    // 第一个必须是与 a 同向的（a 或 d，score=1.0）
    assert(hits[0].second > 0.99f);
    // 第二个也应相关（a/d 中另一个）
    assert(hits[1].second > 0.99f);

    // 阈值过滤：正交向量 score≈0 应被排除
    auto filtered = store.search({1.0f, 0.0f, 0.0f}, 5, 0.5f);
    assert(filtered.size() == 2);

    std::cout << "[test_vector_store] ALL PASSED (" << store.size() << " items)\n";
    return 0;
}
