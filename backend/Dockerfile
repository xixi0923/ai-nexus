# 多阶段构建：构建阶段编译 C++ 服务，运行阶段仅保留二进制 + 运行时库
FROM ubuntu:22.04 AS build
ENV DEBIAN_FRONTEND=noninteractive
RUN apt-get update && apt-get install -y --no-install-recommends \
      build-essential cmake git libssl-dev ca-certificates \
    && rm -rf /var/lib/apt/lists/*
WORKDIR /src
COPY . .
RUN cmake -S . -B build -DCMAKE_BUILD_TYPE=Release \
    && cmake --build build --parallel

FROM ubuntu:22.04 AS runtime
ENV DEBIAN_FRONTEND=noninteractive
RUN apt-get update && apt-get install -y --no-install-recommends \
      libssl3 ca-certificates \
    && rm -rf /var/lib/apt/lists/*
WORKDIR /app
COPY --from=build /src/build/ai-nexus-server /app/ai-nexus-server
# 密钥等通过环境变量注入（见 docker-compose.yml），不要写死在镜像里
EXPOSE 8080
CMD ["/app/ai-nexus-server"]
