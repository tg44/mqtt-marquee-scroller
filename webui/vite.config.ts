import { defineConfig } from 'vite';
import { svelte } from '@sveltejs/vite-plugin-svelte';
import { viteSingleFile } from 'vite-plugin-singlefile';

export default defineConfig({
  plugins: [svelte(), viteSingleFile()],
  build: {
    outDir: 'dist',
    minify: true,
  },
  server: {
    proxy: {
      '/api': {
        target: 'http://192.168.1.100',
        changeOrigin: true,
      },
      '/update': {
        target: 'http://192.168.1.100',
        changeOrigin: true,
      },
    },
  },
});
