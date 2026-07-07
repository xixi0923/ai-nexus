<template>
  <section>
    <h2>知识库问答（RAG）</h2>
    <p class="desc">文档切片向量化入库 → 检索相关片段 → 大模型生成带引用的答案。</p>

    <div class="panel">
      <h3>① 文档入库</h3>
      <input v-model="docId" placeholder="doc_id，如 doc-001" />
      <textarea v-model="docText" placeholder="粘贴文档内容…" rows="6"></textarea>
      <button @click="ingest" :disabled="busy">入库</button>
      <span v-if="ingestMsg" class="msg">{{ ingestMsg }}</span>
    </div>

    <div class="panel">
      <h3>② 基于知识库提问</h3>
      <input v-model="question" @keyup.enter="ask" placeholder="例如：如何重置密码？" />
      <button @click="ask" :disabled="busy">提问</button>
      <div v-if="answer" class="answer">{{ answer }}</div>
      <div v-if="citations.length" class="citations">
        引用：<span v-for="c in citations" :key="c.doc_id">{{ c.doc_id }}({{ c.score.toFixed(2) }}) </span>
      </div>
    </div>
  </section>
</template>

<script setup>
import { ref } from 'vue';
import { ingestDoc, ragAsk } from '../api/client';

const docId = ref('');
const docText = ref('');
const ingestMsg = ref('');
const question = ref('');
const answer = ref('');
const citations = ref([]);
const busy = ref(false);

async function ingest() {
  if (!docId.value || !docText.value) { ingestMsg.value = '请填写 doc_id 与内容'; return; }
  busy.value = true;
  ingestMsg.value = '';
  try {
    const r = await ingestDoc({ doc_id: docId.value, text: docText.value, chunk_size: 512 });
    const d = r.data?.data || {};
    ingestMsg.value = `已入库：chunks=${d.chunks}, embedded=${d.embedded}`;
  } catch (e) {
    ingestMsg.value = '入库失败';
  } finally {
    busy.value = false;
  }
}

async function ask() {
  if (!question.value) return;
  busy.value = true;
  answer.value = '';
  citations.value = [];
  try {
    const r = await ragAsk(question.value, 5);
    const d = r.data?.data || {};
    answer.value = d.reply || '无回答';
    citations.value = d.citations || [];
  } catch (e) {
    answer.value = '提问失败';
  } finally {
    busy.value = false;
  }
}
</script>

<style scoped>
.desc { opacity: .6; font-size: 13px; }
.panel { background: var(--panel); padding: 16px; border-radius: 12px; margin-bottom: 16px; max-width: 640px; }
.panel input, .panel textarea { width: 100%; margin: 6px 0; padding: 10px; border-radius: 8px; border: 1px solid #2c3350; background: #11162a; color: var(--text); }
.panel button { padding: 8px 18px; border: none; border-radius: 8px; background: var(--accent); color: #fff; cursor: pointer; }
.msg { margin-left: 10px; font-size: 13px; opacity: .8; }
.answer { margin-top: 10px; white-space: pre-wrap; line-height: 1.6; }
.citations { margin-top: 8px; font-size: 12px; opacity: .7; }
</style>
