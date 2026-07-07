import axios from 'axios';

// 所有请求走 PHP 中转层（/api），由 PHP 做会话校验与后端转发。
const http = axios.create({
  baseURL: '/api',
  withCredentials: true,
  timeout: 60000,
});

export const health = () => http.get('/health');

export const sendChat = (messages, stream = false) =>
  http.post('/chat', { messages, stream });

export const ragAsk = (question, top_k = 5) =>
  http.post('/rag', { question, top_k });

export const ingestDoc = (payload) => http.post('/ingest', payload);

export const retrieve = (query, top_k = 5) =>
  http.post('/retrieve', { query, top_k });

export const analyzeText = (text, tasks = ['intent', 'entities', 'sentiment']) =>
  http.post('/nlp/analyze', { text, tasks });

// SSE 流式对话：直接消费 PHP 透传的 text/event-stream。
// C++ 端每段推送 `data: <增量文本>\n\n`，结束发 `data: [DONE]\n\n`。
export async function streamChat(messages, onDelta) {
  const resp = await fetch('/api/chat', {
    method: 'POST',
    headers: { 'Content-Type': 'application/json' },
    credentials: 'include',
    body: JSON.stringify({ messages, stream: true }),
  });
  if (!resp.ok) throw new Error('stream request failed: ' + resp.status);
  const reader = resp.body.getReader();
  const decoder = new TextDecoder();
  let buf = '';
  while (true) {
    const { done, value } = await reader.read();
    if (done) break;
    buf += decoder.decode(value, { stream: true });
    let idx;
    while ((idx = buf.indexOf('\n\n')) !== -1) {
      const raw = buf.slice(0, idx);
      buf = buf.slice(idx + 2);
      const line = raw.split('\n').find((l) => l.startsWith('data:'));
      if (!line) continue;
      const data = line.slice(5).trim();
      if (data === '[DONE]') return;
      onDelta(data);
    }
  }
}
