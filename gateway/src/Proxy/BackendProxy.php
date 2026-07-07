<?php
namespace App\Proxy;

// 把 PHP 收到的 /api/* 请求安全转发到 C++ 核心服务。
// 关键点：后端令牌仅存在于服务端，前端永远不直接接触。
class BackendProxy
{
    private string $base;
    private string $token;

    public function __construct()
    {
        $this->base = rtrim($_ENV['BACKEND_BASE'] ?? 'http://localhost:8080', '/');
        $this->token = $_ENV['BACKEND_TOKEN'] ?? 'dev-backend-token';
    }

    public function forward(string $uri, string $method, string $body): array
    {
        $url = $this->base . $uri;
        $ch = curl_init($url);
        curl_setopt_array($ch, [
            CURLOPT_RETURNTRANSFER => true,
            CURLOPT_CUSTOMREQUEST  => $method,
            CURLOPT_POSTFIELDS     => $body,
            CURLOPT_HTTPHEADER     => [
                'Content-Type: application/json',
                'X-Backend-Token: ' . $this->token,
            ],
            CURLOPT_TIMEOUT        => 60,
        ]);

        $resp = curl_exec($ch);
        $status = (int) curl_getinfo($ch, CURLINFO_HTTP_CODE);
        if ($resp === false) {
            $resp = json_encode(['code' => 50301, 'message' => 'backend unreachable']);
            $status = 503;
        }
        curl_close($ch);

        return ['status' => $status ?: 502, 'body' => $resp];
    }
}
