<?php
namespace App\Controllers;

use App\Proxy\BackendProxy;

// 对话 / RAG 端点。识别 stream 标志，对 SSE 响应原样透传。
class ChatController
{
    public function handle(string $uri): void
    {
        $body = file_get_contents('php://input');
        $isStream = $this->isStreamRequested($body);

        $proxy = new BackendProxy();
        $resp = $proxy->forward($uri, 'POST', $body);

        http_response_code($resp['status']);
        if ($isStream) {
            header('Content-Type: text/event-stream');
            header('Cache-Control: no-cache');
        } else {
            header('Content-Type: application/json; charset=utf-8');
        }
        echo $resp['body'];
    }

    private function isStreamRequested(string $reqBody): bool
    {
        $j = json_decode($reqBody, true);
        return isset($j['stream']) && $j['stream'] === true;
    }
}
