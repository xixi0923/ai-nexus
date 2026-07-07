#!/bin/sh
set -e

# bootstrap.php 只读取 .env 文件（不读真实环境变量），
# 这里把 docker-compose 注入的环境变量落成 .env，再启动 PHP 内置服务器。
: "${BACKEND_BASE:=http://backend:8080}"
: "${BACKEND_TOKEN:=dev-backend-token}"
: "${REQUIRE_LOGIN:=false}"
: "${APP_ENV:=production}"

cat > /var/www/.env <<EOF
BACKEND_BASE=${BACKEND_BASE}
BACKEND_TOKEN=${BACKEND_TOKEN}
REQUIRE_LOGIN=${REQUIRE_LOGIN}
APP_ENV=${APP_ENV}
EOF

exec php -S 0.0.0.0:8000 -t public
