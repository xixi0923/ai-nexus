# AI 模块集成方案

AI-Nexus 的核心服务（C++）通过**统一适配层**接入三类前沿能力：大语言模型（LLM）、
自然语言处理（NLP）、向量检索（Vector Retrieval）。设计目标是「可插拔、可替换、可本地化」。

## 1. 能力矩阵

| 能力 | 默认实现 | 可替换方案 |
|------|----------|------------|
| LLM 生成 | OpenAI 兼容 `/v1/chat/completions` | 本地 llama.cpp / vLLM / 国产大模型（通义/文心/智谱） |
| Embedding | OpenAI 兼容 `/v1/embeddings` | bge / m3e 本地模型、Cohere |
| 向量检索 | 进程内近似检索（余弦） | Qdrant / Milvus / Chroma（生产推荐） |
| NLP | LLM 提示工程 + 轻量规则回退 | 专用 NER/意图模型（如 spaCy、HanLP） |

## 2. LLM 调用（流式优先）

- 协议：OpenAI Chat Completions（业界事实标准，绝大多数国内外模型兼容）。
- 流式：`stream=true` 时解析 SSE，逐 token 推给前端，首字延迟最低。
- 配置（环境变量，绝不进前端）：
  ```
  LLM_BASE_URL=https://api.openai.com/v1
  LLM_API_KEY=sk-***
  LLM_MODEL=gpt-4o-mini
  LLM_TIMEOUT_MS=30000
  ```
- 失败策略：超时 → 503；限流(429) → 退避重试 1 次 → 仍失败返回 503 + 友好文案。

## 3. 向量检索与 RAG 管线

标准的**检索增强生成（RAG）**流程：

```
入库: 文档 → 切片(chunk_size) → Embedding → 写入向量库(带 doc_id/metadata)
检索: 用户问句 → Embedding → 向量库 top_k 余弦 → 相关片段
生成: 片段拼接为 context + 问句 → LLM → 答案 + 引用(citations)
```

- `vector_store` 默认实现内存版余弦相似度（便于零依赖演示）；
  生产将 `VECTOR_BACKEND=qdrant` 切换为 Qdrant（`http://qdrant:6333`），
  接口不变（`add` / `search`）。
- 维度约定：默认 1536（text-embedding-3-small），由 `EMBED_DIM` 控制。
- 距离度量：余弦相似度；`score_threshold` 过滤低相关片段，抑制幻觉。

## 4. NLP 模块

`nlp_engine` 提供 `analyze(text, tasks)`：
- `intent`：让 LLM 从候选意图集中分类（或规则回退）。
- `entities`：抽取关键槽位（地点/时间/金额…），结构化输出。
- `sentiment`：正向/中性/负向。
- 无 LLM 时退化为正则/词典粗分类，保证服务不降级为全失败。

## 5. 安全与成本

- **密钥隔离**：所有 AI 密钥仅存于 C++ / PHP 服务端环境变量，前端经 PHP 中转，永不直接持 Key。
- **审计**：每次 LLM 调用记录 `request_id`、token 用量，便于成本归因。
- **内容安全**：PHP 层做输入/输出敏感词与长度护栏；可接入内容审核 API。

## 6. 扩展路线

1. 多模态：新增 `/api/vision`（图片理解），复用同一 LLM 适配层。
2. Agent/工具调用：在 `llm_client` 增加 function-calling，编排外部工具。
3. 评测：对 RAG 答案做自动评测（faithfulness / relevance），闭环优化切片与 prompt。
