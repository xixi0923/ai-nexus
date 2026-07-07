<template>
  <div class="chat">
    <div class="messages">
      <MessageBubble
        v-for="(m, i) in messages"
        :key="i"
        :role="m.role"
        :text="m.text"
      />
      <div v-if="busy" class="hint">AI 正在输入…</div>
    </div>
    <div class="composer">
      <input
        v-model="input"
        @keyup.enter="send"
        :disabled="busy"
        placeholder="说点什么，回车发送…"
      />
      <button @click="send" :disabled="busy">发送</button>
    </div>
  </div>
</template>

<script setup>
import { ref } from 'vue';
import MessageBubble from './MessageBubble.vue';
import { streamChat } from '../api/client';

const messages = ref([]);
const input = ref('');
const busy = ref(false);

async function send() {
  const text = input.value.trim();
  if (!text || busy.value) return;
  input.value = '';
  messages.value.push({ role: 'user', text });
  const assistant = { role: 'assistant', text: '' };
  messages.value.push(assistant);
  busy.value = true;

  // 仅把已完成的 user/assistant 消息作为上下文传给后端
  const history = messages.value
    .filter((m) => m.text || m.role === 'user')
    .map((m) => ({ role: m.role, content: m.text }));

  try {
    await streamChat(history, (delta) => {
      assistant.text += delta;
    });
  } catch (e) {
    assistant.text += '\n[出错了，请稍后重试]';
  } finally {
    busy.value = false;
  }
}
</script>

<style scoped>
.chat { display: flex; flex-direction: column; height: calc(100vh - 140px); }
.messages { flex: 1; overflow-y: auto; padding: 8px 0; }
.hint { opacity: .5; font-size: 13px; margin: 6px 0; }
.composer { display: flex; gap: 10px; padding-top: 10px; }
.composer input { flex: 1; padding: 10px 14px; border-radius: 10px; border: 1px solid #2c3350; background: var(--panel); color: var(--text); }
.composer button { padding: 10px 20px; border: none; border-radius: 10px; background: var(--accent); color: #fff; cursor: pointer; }
.composer button:disabled { opacity: .5; cursor: not-allowed; }
</style>
