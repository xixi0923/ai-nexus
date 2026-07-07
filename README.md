# AI-Nexus

> 前后端分离的 AI 应用基座：Vue 前端 + PHP 中转/渲染层 + C++ 核心服务，原生集成大语言模型（LLM）、自然语言处理（NLP）与向量检索能力。

## 架构总览

```
┌────────────┐     HTTPS/JSON      ┌──────────────────┐    HTTP/JSON     ┌──────────────────────┐
│   Browser  │ ──────────────────▶ │   PHP Gateway    │ ───────────────▶ │   C++ Core Service   │
│  (Vue SPA) │ ◀────────────────── │ (渲染 + 接口中转) │ ◀─────────────── │  (LLM / NLP / Vector) │
└────────────┘                     └──────────────────┘                  └───────────┬──────────┘
                                            │ 会话/鉴权、限流、缓存                              │
                                            │                                            ▼
                                                                          ┌────────────────────┐
                                                                          │  AI Providers       │
                                                                          │  - LLM (OpenAI兼容) │
                                                                          │  - Embedding 模型   │
                                                                          │  - 向量库(Qdrant等) │
                                                                          └────────────────────┘
```

**分层职责**

| 层 | 技术 | 职责 |
|----|------|------|
| 前端 | Vue 3 + Vite | 交互界面（对话 / 知识库问答），通过 Axios 调 PHP 中转层 |
| 中转层 | PHP 8 | 服务端页面渲染、会话与鉴权、把 `/api/*` 安全代理到 C++ 后端、限流/缓存 |
| 核心服务 | C++17 (cpp-httplib) | 暴露 REST API，编排 LLM 调用、NLP 解析、向量检索（RAG） |
| AI 能力 | LLM / Embedding / 向量库 | 经 OpenAI 兼容协议或本地模型接入，详见 `docs/AI_INTEGRATION.md` |

## 目录结构

```
ai-nexus/
├── README.md
├── docs/
│   ├── API.md                 # 前后端通信接口设计（契约、错误码、数据流）
│   └── AI_INTEGRATION.md      # AI 模块集成方案（LLM/NLP/向量检索/RAG）
├── backend/                   # C++ 核心服务
│   ├── CMakeLists.txt
│   ├── include/               # 头文件（server / config / ai/*）
│   ├── src/                   # 实现（main / server / config / ai/*）
│   └── tests/                 # 单元测试（vector_store 等）
├── frontend/                  # Vue 3 + Vite
│   ├── src/api/               # Axios 客户端（调 PHP 中转）
│   ├── src/views/             # 页面（对话 / 知识库）
│   ├── src/components/        # 组件
│   └── src/router/            # 前端路由
├── gateway/                   # PHP 中转层 / 页面渲染
│   ├── public/index.php       # 入口
│   ├── src/                   # Router / Auth / Proxy / Controllers
│   └── templates/             # 服务端渲染模板
└── scripts/dev-up.sh          # 本地一键启动
```

## 快速开始

```bash
# 1) 后端：构建并启动 C++ 核心服务（默认 :8080）
cd backend && cmake -B build && cmake --build build && ./build/ai-nexus-server

# 2) 中转层：配置并启动 PHP（需 PHP 8 + ext-curl）
cd gateway && cp .env.example .env && php -S 0.0.0.0:8000 -t public

# 3) 前端：开发模式（Vite 代理 /api 到 PHP）
cd frontend && npm install && npm run dev
```

> 生产部署建议：前端构建为静态资源由 PHP/Nginx 托管；PHP 与 C++ 服务之间走内网，仅 PHP 暴露公网。

详见 `docs/API.md` 与 `docs/AI_INTEGRATION.md`。
