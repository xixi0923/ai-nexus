<?php
namespace App\Auth;

// 会话与鉴权。演示环境自动发放匿名会话；
// 生产请将 isValid() 换成真实登录校验（如 JWT / OAuth）。
class Session
{
    public function __construct()
    {
        if (session_status() === PHP_SESSION_NONE) {
            session_start();
        }
    }

    public function isValid(): bool
    {
        if (empty($_SESSION['uid'])) {
            $_SESSION['uid'] = 'anon-' . bin2hex(random_bytes(6));
        }
        return true;
    }

    public function uid(): string
    {
        return $_SESSION['uid'] ?? 'anon';
    }
}
