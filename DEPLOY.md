# 本地部署与测试指南

> 本仓库已包含一套 `docker-compose` 配置，**一条命令起全栈**。也提供手动三步部署，适合你本机已装好 g++/php/node 的情况。

## 一、环境说明（重要）

本项目为**前后端分离 + C++ 核心服务**，运行需要三类运行时：

| 层 | 运行时 |
|----|--------|
| 前端 | Node.js ≥ 18（`npm`） |
| 中转层 | PHP ≥ 8.1 + `ext-curl` |
| 核心服务 | C++17 编译器（`g++`/`cmake`）+ `libssl-dev` |

> ⚠️ 当前 WorkBuddy 沙箱**未安装 g++ / PHP / Docker，且无法访问 npm 源**，因此无法在沙箱内直接运行。请在你**自己的机器**上按下面步骤部署测试（仓库已推送到 GitHub，直接 clone 即可）。

## 二、方式一：Docker Compose（推荐，一条命令）

 prerequisites：已安装 Docker 与 Docker Compose v2。

```bash
# 1. 拉取代码
git clone https://github.com/xixi0923/ai-nexus.git
cd ai-nexus

# 2.（可选）启用真实对话：编辑 docker-compose.yml 中 backend.environment，
#    取消注释并填入 LLM_API_KEY / EMBED_API_KEY。
#    不填也能跑：健康检查和规则型 NLP 立即可用，仅对话/向量检索需密钥。

# 3. 构建并启动全栈
docker compose up --build

# 4. 浏览器打开
open http://localhost:8080
```

启动后三个容器在同一网络内互通：

```
Browser ──:8080──▶ frontend(nginx) ──/api──▶ gateway(php:8000) ──▶ backend(cpp:8080)
```

常用命令：

```bash
docker compose ps                     # 查看状态
docker compose logs -f backend        # 看 C++ 服务日志
docker compose down                   # 停止并移除容器
```

## 三、方式二：手动部署（本机已有工具链）

```bash
# 终端 1：C++ 核心服务
cd backend
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
./build/ai-nexus-server                # 监听 :8080

# 终端 2：PHP 中转层
cd gateway
cp .env.example .env                   # 按需改 BACKEND_BASE / BACKEND_TOKEN
php -S 0.0.0.0:8000 -t public          # 监听 :8000

# 终端 3：Vue 前端（开发模式，已内置 /api 代理到 :8000）
cd frontend
npm install
npm run dev                            # 默认 :5173
```

开发模式访问 `http://localhost:5173`；生产构建用 `npm run build`，再用任意静态服务器托管 `dist/` 并反代 `/api` 到 `:8000`。

## 四、测试清单

| 功能 | 是否需密钥 | 验证方式 |
|------|-----------|----------|
| 健康检查 | 否 | `GET /api/health` → `{"code":0,"data":{"status":"ok",...}}` |
| 规则型 NLP | 否 | 聊天页切到「分析」或调 `POST /api/nlp/analyze`（情感/意图/实体） |
| 对话（LLM） | 是 | `POST /api/chat`，或在聊天页输入 |
| 流式对话 | 是 | 聊天页开启流式 |
| RAG | 是 | 先 `POST /api/ingest` 灌文档，再 `POST /api/rag` 问答 |

快速冒烟：

```bash
curl http://localhost:8080/api/health
curl -X POST http://localhost:8080/api/nlp/analyze \
  -H 'Content-Type: application/json' \
  -d '{"text":"明天北京天气怎么样，13812345678","tasks":["intent","entities","sentiment"]}'
```

> 注：浏览器经 `:8080`（前端）访问时，路径同样是 `/api/...`，由 nginx 反代到 PHP 层。

## 五、常见问题

- **对话返回 503 / `llm provider error`**：未配置 `LLM_API_KEY`，或 `LLM_BASE_URL` 不可达。先填密钥再试。
- **403 `backend token mismatch`**：`backend` 与 `gateway` 的 `BACKEND_TOKEN` 不一致。两者默认均为 `dev-backend-token`，保持一致即可。
- **SSE 不流式**：PHP 内置服务器 / curl 可能缓冲响应，仍能正常返回完整结果，只是非逐字流式。生产建议用 Nginx + PHP-FPM。
- **`npm install` 慢/失败**：检查本机网络与 npm 源（国内可换 `npm config set registry https://registry.npmmirror.com`）。
