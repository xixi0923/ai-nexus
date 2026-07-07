<template>
  <div class="bubble" :class="role">
    <div class="role">{{ role === 'user' ? '你' : 'AI' }}</div>
    <div class="text">{{ text }}</div>
  </div>
</template>

<script setup>
defineProps({
  role: { type: String, default: 'user' },
  text: { type: String, default: '' },
});
</script>

<style scoped>
.bubble { margin: 10px 0; max-width: 78%; padding: 10px 14px; border-radius: 12px; line-height: 1.5; white-space: pre-wrap; }
.bubble.user { margin-left: auto; background: var(--accent); color: #fff; }
.bubble.assistant { margin-right: auto; background: var(--panel); }
.role { font-size: 12px; opacity: .6; margin-bottom: 4px; }
</style>
