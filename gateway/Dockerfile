FROM php:8.3-cli
# 中转层需要 curl 扩展访问 C++ 后端
RUN docker-php-ext-install curl
WORKDIR /var/www
COPY . .
RUN chmod +x docker-entrypoint.sh
EXPOSE 8000
ENTRYPOINT ["./docker-entrypoint.sh"]
