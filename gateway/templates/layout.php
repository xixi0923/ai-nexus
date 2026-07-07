<?php
// 生产环境：PHP 渲染 SPA 外壳，引导已构建的 Vue 资源（frontend/dist）。
// 开发环境请直接使用 Vite 开发服务器（frontend/，默认 :5173）。
?>
<!DOCTYPE html>
<html lang="zh-CN">
  <head>
    <meta charset="UTF-8" />
    <meta name="viewport" content="width=device-width, initial-scale=1.0" />
    <title>AI-Nexus</title>
    <link rel="stylesheet" href="/assets/index.css" />
  </head>
  <body>
    <div id="app"></div>
    <script type="module" src="/assets/index.js"></script>
  </body>
</html>
