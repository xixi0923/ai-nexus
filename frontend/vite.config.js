import { defineConfig } from 'vite';
import vue from '@vitejs/plugin-vue';

// 开发模式下，将 /api 代理到 PHP 中转层（PHP 再转发到 C++ 后端）。
export default defineConfig({
  plugins: [vue()],
  server: {
    port: 5173,
    proxy: {
      '/api': {
        target: 'http://localhost:8000',
        changeOrigin: true,
      },
    },
  },
  build: {
    outDir: 'dist',
  },
});
