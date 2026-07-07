import { createRouter, createWebHistory } from 'vue-router';
import ChatView from '../views/ChatView.vue';
import KnowledgeView from '../views/KnowledgeView.vue';

const router = createRouter({
  history: createWebHistory(),
  routes: [
    { path: '/', redirect: '/chat' },
    { path: '/chat', component: ChatView, meta: { title: '智能对话' } },
    { path: '/knowledge', component: KnowledgeView, meta: { title: '知识库问答' } },
  ],
});

export default router;
