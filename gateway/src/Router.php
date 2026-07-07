<?php
namespace App;

// 前端控制器：区分「页面渲染」与「/api 接口中转」。
class Router
{
    public function dispatch(): void
    {
        $uri = parse_url($_SERVER['REQUEST_URI'] ?? '/', PHP_URL_PATH) ?: '/';
        $method = $_SERVER['REQUEST_METHOD'] ?? 'GET';

        // 健康检查：无需会话，直接转发到 C++ 后端
        if ($uri === '/api/health') {
            (new Controllers\HealthController())->handle();
            return;
        }

        // 非 /api 请求：由 PHP 渲染 SPA 外壳（生产）；开发请用 Vite
        if (strpos($uri, '/api/') !== 0) {
            if ($method === 'GET') {
                $this->renderPage();
                return;
            }
            http_response_code(404);
            header('Content-Type: application/json; charset=utf-8');
            echo json_encode(['code' => 40400, 'message' => 'not found']);
            return;
        }

        // /api 接口：需会话（演示为匿名会话）
        $session = new Auth\Session();
        if (!$session->isValid()) {
            http_response_code(401);
            header('Content-Type: application/json; charset=utf-8');
            echo json_encode(['code' => 40101, 'message' => 'unauthenticated', 'data' => null, 'request_id' => uniqid()]);
            return;
        }

        // 具体端点路由
        if ($uri === '/api/chat' || $uri === '/api/rag') {
            (new Controllers\ChatController())->handle($uri);
            return;
        }

        // 其余 /api/* 统一安全转发到 C++ 后端
        $proxy = new Proxy\BackendProxy();
        $body = file_get_contents('php://input');
        $resp = $proxy->forward($uri, $method, $body);
        http_response_code($resp['status']);
        header('Content-Type: application/json; charset=utf-8');
        echo $resp['body'];
    }

    private function renderPage(): void
    {
        require __DIR__ . '/../templates/layout.php';
    }
}
