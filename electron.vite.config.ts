import { defineConfig, externalizeDepsPlugin } from 'electron-vite'
import { resolve } from 'path'

export default defineConfig({
  main: {
    plugins: [externalizeDepsPlugin()]
  },
  preload: {
    plugins: [externalizeDepsPlugin()]
  },
  renderer: {
    // Stub config - actual renderer runs from SvelteKit
    root: resolve('src/renderer-stub'),
    server: {
      port: 5174 // Use different port to avoid conflict with SvelteKit
    },
    build: {
      outDir: resolve('out/renderer-stub'),
      rollupOptions: {
        input: resolve('src/renderer-stub/index.html')
      }
    }
  }
})
