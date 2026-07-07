# AI-Nexus

> 前后端分离的 AI 应用基座：Vue 前端 + PHP 中转层 + C++ 核心服务，集成 LLM、NLP 与向量检索（RAG）。

AI-Nexus 是一套**可落地、可扩展**的 AI 应用脚手架。它把“前沿 AI 能力”封装在一个高性能 C++ 核心服务里，通过 PHP 中转层做页面渲染、会话管理与接口鉴权，再用 Vue 提供现代交互界面。所有 AI 密钥（LLM / Embedding / 向量库）**只存在于服务端环境变量**，前端永远不直接接触密钥。

```
┌────────────┐      HTTP/JSON       ┌─────────────────┐   X-Backend-Token   ┌──────────────────────┐
│   Browser   │ ──── /api/* ───────▶ │   PHP 中转层     │ ───────────────────▶│   C++ 核心服务        │
│  (Vue 3)   │ ◀─── 页面/JSON ───── │ (渲染+代理+鉴权) │ ◀───────────────────│ (LLM/NLP/向量检索)   │
└────────────┘                      └─────────────────┘                      └───────────┬──────────┘
                                                                                        │
                                                                          ┌─────────────┼─────────────┐
                                                                       ┌──▼───┐    ┌─────▼─────┐   ┌────▼────┐
                                                                       │ LLM  │    │  Embedding │   │ 向量库  │
                                                                       │(OpenAI│    │ (OpenAI    │   │(内存/   │
                                                                       │兼容) │    │  兼容)     │   │ Qdrant) │
                                                                       └──────┘    └───────────┘   └─────────┘
```

## ✨ 特性

- **前后端分离**：Vue 3 + Vite 前端，PHP 中转层负责服务端渲染与 API 代理，C++17 核心服务承载 AI 计算。
- **AI 三件套**：
  - **LLM 调用**：OpenAI 兼容协议，支持非流式 / 流式（SSE）对话，可一键切换到本地 llama.cpp / vLLM / 国产模型。
  - **NLP**：意图识别、实体抽取、情感分析，优先 LLM 提示工程，缺失密钥时自动降级到规则引擎。
  - **向量检索（RAG）**：轻量内存向量库（余弦相似度）开箱即用，可无缝切换 Qdrant。
- **RAG 管线**：`/api/rag` = 向量检索 → 拼装上下文 → LLM 生成 → 带引用返回。
- **安全隔离**：密钥仅服务端持有；PHP 与 C++ 之间用 `X-Backend-Token` 互信校验。
- **工程化**：CMake 构建、零外部依赖的单元测试（ctest）、GitHub Actions CI（C++ / 前端 / PHP 三路）。

## 🧱 技术栈

| 层 | 技术 | 职责 |
|----|------|------|
| 前端 | Vue 3 + Vue Router + Vite + Axios | 对话 / 知识库界面，调用 PHP 中转层 |
| 中转层 | PHP 8（原生 + Composer 可选） | 页面渲染、会话管理、后端代理、鉴权 |
| 核心服务 | C++17 + cpp-httplib + nlohmann/json + OpenSSL | REST API、AI 模块、向量检索 |
| 文档 | Markdown | 接口契约、AI 集成方案 |

## 📂 目录结构

```
ai-nexus/
├── backend/                    # C++ 核心服务
│   ├── CMakeLists.txt
│   ├── include/
│   │   ├── config.h            # 配置结构 + load_config()
│   │   ├── server.h            # HTTP 路由声明
│   │   └── ai/
│   │       ├── llm_client.h     # LLM / Embedding 客户端
│   │       ├── llm_protocol.h   # 请求构造/响应解析（header-only，可单测）
│   │       ├── nlp_engine.h     # NLP 引擎（LLM 优先，规则降级）
│   │       ├── nlp_rules.h      # 规则型 NLP（可独立单测）
│   │       └── vector_store.h   # 向量存储 + 余弦检索
│   ├── src/
│   │   ├── main.cpp
│   │   ├── config.cpp
│   │   ├── server.cpp           # 路由实现（含 SSE 流式）
│   │   └── ai/
│   │       ├── llm_client.cpp
│   │       ├── nlp_engine.cpp
│   │       ├── nlp_rules.cpp
│   │       └── vector_store.cpp
│   └── tests/                   # 单元测试（assert + ctest）
│       ├── test_vector_store.cpp
│       ├── test_nlp_rules.cpp
│       ├── test_llm_protocol.cpp
│       └── test_config.cpp
├── gateway/                    # PHP 中转层
│   ├── composer.json
│   ├── .env.example
│   ├── bootstrap.php
│   ├── public/index.php         # 唯一入口
│   ├── src/
│   │   ├── Router.php
│   │   ├── Auth/Session.php
│   │   ├── Proxy/BackendProxy.php   # 转发到 C++ 并附 X-Backend-Token
│   │   └── Controllers/{ChatController,HealthController}.php
│   └── templates/layout.php
├── frontend/                   # Vue 前端
│   ├── package.json
│   ├── vite.config.js
│   ├── index.html
│   └── src/
│       ├── main.js
│       ├── App.vue
│       ├── router/index.js
│       ├── api/client.js        # 调用 PHP /api/*
│       ├── components/{ChatWindow,MessageBubble}.vue
│       └── views/{ChatView,KnowledgeView}.vue
├── docs/
│   ├── API.md                   # 接口契约 + 错误码 + 数据流
│   └── AI_INTEGRATION.md        # RAG 管线 / 模型配置 / 可替换方案
├── scripts/dev-up.sh            # 一键启动脚本
├── .github/workflows/ci.yml     # CI
├── .gitignore
└── LICENSE
```

## 🚀 快速开始

### 前置依赖

- C++：`cmake >= 3.16`、`g++`（C++17）、`libssl-dev`（HTTPS 出站）
- PHP：`php >= 8.0` + `ext-curl`
- 前端：`node >= 18` + `npm`
- （可选）向量库：Qdrant（使用内存向量库时不需要）

### 1. 配置密钥

复制示例并填入你的密钥（**切勿提交 `.env`**）：

```bash
cp gateway/.env.example gateway/.env
```

核心环境变量（C++ 服务读取，`docs/AI_INTEGRATION.md` 有完整说明）：

| 变量 | 说明 | 默认 |
|------|------|------|
| `SERVER_PORT` | C++ 服务端口 | `8080` |
| `BACKEND_TOKEN` | PHP↔C++ 互信令牌 | `dev-backend-token` |
| `LLM_BASE_URL` | LLM 端点（OpenAI 兼容） | `https://api.openai.com/v1` |
| `LLM_API_KEY` | LLM 密钥 | _(空)_ |
| `LLM_MODEL` | 模型名 | `gpt-4o-mini` |
| `EMBED_BASE_URL` | Embedding 端点 | `https://api.openai.com/v1` |
| `EMBED_API_KEY` | Embedding 密钥 | _(空)_ |
| `EMBED_MODEL` | Embedding 模型 | `text-embedding-3-small` |
| `EMBED_DIM` | 向量维度 | `1536` |
| `VECTOR_BACKEND` | `memory` 或 `qdrant` | `memory` |
| `QDRANT_URL` | Qdrant 地址 | `http://localhost:6333` |

### 2. 启动 C++ 核心服务

```bash
cd backend
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
./build/ai-nexus-server
```

### 3. 启动 PHP 中转层

```bash
cd gateway
php -S 0.0.0.0:8000 -t public
```

### 4. 启动前端（开发模式）

```bash
cd frontend
npm install
npm run dev      # 生产构建：npm run build
```

打开浏览器访问 PHP 入口（默认 `http://localhost:8000`），即可使用对话与知识库页面。

## 🧪 测试

C++ 服务包含零外部依赖的单元测试，使用 `ctest` 运行：

```bash
cd backend
cmake -S . -B build
cmake --build build --parallel
ctest --test-dir build --output-on-failure
```

覆盖：

- **vector_store**：余弦相似度排序、阈值过滤、正交向量排除。
- **nlp_rules**：情感 / 意图 / 实体抽取（邮箱、手机号、金额、地点、日期）。
- **llm_protocol**：请求体构造、回复与流式增量解析、Embedding 编解码。
- **config**：默认值、环境变量覆盖、非法值回退。

## 🔌 接口与 AI 集成

- **接口契约、错误码、前后端数据流** → 见 [`docs/API.md`](docs/API.md)
- **RAG 管线、模型配置、可替换方案** → 见 [`docs/AI_INTEGRATION.md`](docs/AI_INTEGRATION.md)

核心端点（经 PHP 中转层暴露给前端）：

| 方法 | 路径 | 说明 |
|------|------|------|
| `POST` | `/api/chat` | 非流式对话 |
| `POST` | `/api/chat/stream` | SSE 流式对话 |
| `POST` | `/api/rag` | 检索增强生成 |
| `POST` | `/api/nlp` | 意图/实体/情感分析 |
| `GET`  | `/api/health` | 健康检查 |

## ⚙️ CI

`.github/workflows/ci.yml` 在 `push` / `pull_request` 到 `main` 时运行三路作业：

1. **cpp**：`ubuntu-latest` 上 `cmake` 构建 C++ 服务并执行 `ctest`。
2. **frontend**：`node 20` 安装依赖并 `npm run build`。
3. **php**：对所有 `.php` 文件执行 `php -l` 语法检查。

## 📄 许可证

[MIT](LICENSE) —— 自由使用、修改与分发，请保留版权声明。
