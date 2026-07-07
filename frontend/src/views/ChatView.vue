<template>
  <section>
    <h2>智能对话</h2>
    <p class="desc">基于大语言模型，支持流式输出。对话请求经 PHP 中转层转发至 C++ 核心服务。</p>
    <ChatWindow />
  </section>
</template>

<script setup>
import ChatWindow from '../components/ChatWindow.vue';
</script>

<style scoped>
.desc { opacity: .6; font-size: 13px; }
</style>
