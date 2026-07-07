<?php
// 极简引导：PSR-4 风格自动加载 + 载入 .env

spl_autoload_register(function ($class) {
    $prefix = 'App\\';
    if (strncmp($class, $prefix, strlen($prefix)) !== 0) return;
    $rel = substr($class, strlen($prefix));
    $file = __DIR__ . '/src/' . str_replace('\\', '/', $rel) . '.php';
    if (file_exists($file)) require $file;
});

$envFile = __DIR__ . '/.env';
if (file_exists($envFile)) {
    foreach (file($envFile) as $line) {
        $line = trim($line);
        if ($line === '' || $line[0] === '#') continue;
        if (strpos($line, '=') === false) continue;
        [$k, $v] = explode('=', $line, 2);
        $_ENV[trim($k)] = trim($v, "\"' ");
    }
}
