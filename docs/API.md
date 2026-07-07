# 前后端通信接口设计（API 契约）

本文定义 **前端 ⇄ PHP 中转层 ⇄ C++ 核心服务** 三层之间的数据交互规范。所有接口使用
`application/json`，时间统一为 RFC3339（UTC）。

## 1. 数据流与鉴权

```
Vue ──(1)──▶ PHP /api/*  ──(2)──▶ C++ :8080/api/*
  ◀──(4)─────────────────  ◀──(3)───────
```

1. 前端携带会话 Cookie 请求 `PHP /api/*`；
2. PHP 做**会话校验 + 限流**，再用服务端持有的 `BACKEND_TOKEN` 转发到 C++ 后端；
3. C++ 执行业务/AI 编排，返回 JSON；
4. PHP 透传（可做脱敏/缓存）回前端。

> **密钥不落前端**：LLM API Key、向量库密码等仅存在于 C++ 服务与 PHP 服务端的
> 环境变量中，前端永远拿不到。

### 鉴权
- 前端 → PHP：基于 Session Cookie（`PHPSESSID`）。未登录返回 `401`。
- PHP → C++：请求头 `X-Backend-Token: <BACKEND_TOKEN>`（在 `.env` 中配置）。

### 统一响应包络
```json
{ "code": 0, "message": "ok", "data": { }, "request_id": "uui" }
```
- `code = 0` 成功；非 0 见错误码表。
- 流式接口（如 `/api/chat` 开启 `stream`）返回 `text/event-stream`（SSE），
  逐片推送 `data: {...}\n\n`，结束发 `data: [DONE]\n\n`。

### 错误码
| code | HTTP | 含义 |
|------|------|------|
| 0 | 200 | 成功 |
| 40001 | 400 | 请求参数缺失/非法 |
| 40101 | 401 | 未认证（无会话） |
| 40301 | 403 | 后端令牌校验失败 |
| 42901 | 429 | 触发限流 |
| 50001 | 500 | 后端内部错误 |
| 50301 | 503 | AI 提供商不可用 / 超时 |

---

## 2. 接口清单

### 2.1 对话（LLM）
`POST /api/chat`
```jsonc
// 请求
{ "messages": [ {"role":"user","content":"你好"} ],
  "model": "gpt-4o-mini",   // 可选，缺省用服务端默认
  "stream": false,          // true=启用 SSE 流式
  "temperature": 0.7 }      // 可选
// 响应 data
{ "reply": "你好，我是 AI-Nexus 助手", "model": "gpt-4o-mini",
  "usage": {"prompt_tokens":9,"completion_tokens":12,"total_tokens":21} }
```

### 2.2 文档入库（切片 → 向量化 → 存储）
`POST /api/ingest`
```jsonc
// 请求
{ "doc_id": "doc-001", "title": "产品手册",
  "text": "很长的内容……", "chunk_size": 512, "metadata": {"src":"manual"} }
// 响应 data
{ "doc_id":"doc-001", "chunks": 14, "embedded": true }
```

### 2.3 知识检索（向量检索）
`POST /api/retrieve`
```jsonc
// 请求
{ "query": "如何重置密码？", "top_k": 5, "score_threshold": 0.2 }
// 响应 data
{ "results": [ {"doc_id":"doc-001","score":0.87,
                "chunk":"...相关片段...","metadata":{}} ] }
```

### 2.4 检索增强问答（RAG：检索 + 对话）
`POST /api/rag`
```jsonc
// 请求
{ "question":"如何重置密码？", "top_k":5, "stream":false }
// 响应 data
{ "answer":"...", "citations":[{"doc_id":"doc-001","score":0.87}] }
```

### 2.5 自然语言分析（NLP）
`POST /api/nlp/analyze`
```jsonc
// 请求
{ "text": "帮我订明天的机票去北京", "tasks":["intent","entities","sentiment"] }
// 响应 data
{ "intent":"book_flight", "entities":{"dest":"北京","date":"明天"},
  "sentiment":"neutral" }
```

### 2.6 健康检查
`GET /api/health` → `{ "status":"ok", "version":"1.0.0",
"components":{"llm":"up","vector":"up"} }`

---

## 3. 前端 → PHP 调用示例（Axios）

```js
// frontend/src/api/client.js
import axios from 'axios';
const http = axios.create({ baseURL: '/api', withCredentials: true });
export const sendChat = (messages, stream=false) =>
  http.post('/chat', { messages, stream });
export const ingestDoc = (payload) => http.post('/ingest', payload);
export const retrieve = (query, top_k=5) => http.post('/retrieve', { query, top_k });
export const analyzeText = (text) => http.post('/nlp/analyze', { text });
```

## 4. PHP → C++ 转发示例

```php
// gateway/src/Proxy/BackendProxy.php（节选）
$resp = $this->curl(
    $this->backendBase . '/api/chat',
    $body,
    [ 'X-Backend-Token: ' . $this->backendToken ]
);
```
