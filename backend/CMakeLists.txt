cmake_minimum_required(VERSION 3.16)
project(ai-nexus-backend CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ---- 第三方依赖（头文件，构建时自动拉取）----
include(FetchContent)
FetchContent_Declare(httplib
  GIT_REPOSITORY https://github.com/yhirose/cpp-httplib.git
  GIT_TAG v0.15.3)
FetchContent_Declare(json
  GIT_REPOSITORY https://github.com/nlohmann/json.git
  GIT_TAG v3.11.3)
FetchContent_MakeAvailable(httplib json)

# HTTPS 出站调用（访问 OpenAI 兼容端点）需要 OpenSSL
find_package(OpenSSL REQUIRED)

# ---- 核心服务可执行文件 ----
add_executable(ai-nexus-server
  src/main.cpp
  src/config.cpp
  src/server.cpp
  src/ai/llm_client.cpp
  src/ai/nlp_engine.cpp
  src/ai/vector_store.cpp
  src/ai/nlp_rules.cpp)

target_include_directories(ai-nexus-server PRIVATE include)
target_compile_definitions(ai-nexus-server PRIVATE CPPHTTPLIB_OPENSSL_SUPPORT)
target_link_libraries(ai-nexus-server
  PRIVATE httplib OpenSSL::SSL OpenSSL::Crypto nlohmann_json::nlohmann_json pthread)

# ---- 单元测试（零额外依赖，assert 驱动，注册为 ctest）----
enable_testing()

add_executable(test_vector_store
  tests/test_vector_store.cpp
  src/ai/vector_store.cpp)
target_include_directories(test_vector_store PRIVATE include)
target_link_libraries(test_vector_store PRIVATE nlohmann_json::nlohmann_json)
add_test(NAME test_vector_store COMMAND test_vector_store)

add_executable(test_nlp_rules
  tests/test_nlp_rules.cpp
  src/ai/nlp_rules.cpp)
target_include_directories(test_nlp_rules PRIVATE include)
target_link_libraries(test_nlp_rules PRIVATE nlohmann_json::nlohmann_json)
add_test(NAME test_nlp_rules COMMAND test_nlp_rules)

add_executable(test_llm_protocol
  tests/test_llm_protocol.cpp)
target_include_directories(test_llm_protocol PRIVATE include)
target_link_libraries(test_llm_protocol PRIVATE nlohmann_json::nlohmann_json)
add_test(NAME test_llm_protocol COMMAND test_llm_protocol)

add_executable(test_config
  tests/test_config.cpp
  src/config.cpp)
target_include_directories(test_config PRIVATE include)
target_link_libraries(test_config PRIVATE nlohmann_json::nlohmann_json)
add_test(NAME test_config COMMAND test_config)

