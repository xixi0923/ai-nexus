#!/usr/bin/env bash
# AI-Nexus 本地一键启动（需：cmake + g++、PHP 8、Node 18+）
set -e

echo "==> 1) 构建并启动 C++ 核心服务 (:8080)"
cd backend
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j"$(nproc 2>/dev/null || echo 4)"
./build/ai-nexus-server &
CPP_PID=$!
cd ..

echo "==> 2) 启动 PHP 中转层 (:8000)"
cd gateway
cp -n .env.example .env
php -S 0.0.0.0:8000 -t public &
PHP_PID=$!
cd ..

echo "==> 3) 启动 Vue 前端开发服务器 (:5173)"
cd frontend
npm install
npm run dev

# 前端退出后清理后台进程
kill "$CPP_PID" "$PHP_PID" 2>/dev/null || true
