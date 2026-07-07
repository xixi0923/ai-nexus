<?php
namespace App\Controllers;

use App\Proxy\BackendProxy;

// 健康检查：直接转发到 C++ 后端的 /api/health。
class HealthController
{
    public function handle(): void
    {
        $proxy = new BackendProxy();
        $resp = $proxy->forward('/api/health', 'GET', '');
        http_response_code($resp['status']);
        header('Content-Type: application/json; charset=utf-8');
        echo $resp['body'];
    }
}
