<template>
  <div id="app">
    <header class="topbar">
      <span class="logo">AI-Nexus</span>
      <nav>
        <router-link to="/chat">对话</router-link>
        <router-link to="/knowledge">知识库</router-link>
      </nav>
    </header>
    <main class="content">
      <router-view />
    </main>
  </div>
</template>

<script setup></script>

<style>
:root { --bg: #0f1220; --panel: #1a1f33; --accent: #6c8cff; --text: #e6e9f5; }
* { box-sizing: border-box; }
body { margin: 0; font-family: system-ui, -apple-system, "Segoe UI", sans-serif; background: var(--bg); color: var(--text); }
#app { min-height: 100vh; display: flex; flex-direction: column; }
.topbar { display: flex; align-items: center; gap: 24px; padding: 12px 20px; background: var(--panel); }
.logo { font-weight: 700; color: var(--accent); }
nav a { color: var(--text); text-decoration: none; margin-right: 16px; opacity: .8; }
nav a.router-link-active { opacity: 1; color: var(--accent); }
.content { flex: 1; padding: 20px; }
</style>
