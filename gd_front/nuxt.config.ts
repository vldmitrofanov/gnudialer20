// https://nuxt.com/docs/api/configuration/nuxt-config
export default defineNuxtConfig({
  compatibilityDate: '2024-04-03',
  devtools: { enabled: true },
  pages: true,
  ssr: false,
  runtimeConfig: {
    public: {
      apiBaseUrl: 'http://127.0.0.1:8000', // Replace with your Laravel backend URL
    },
  },
  css: [
    '@/assets/styles/main.scss'
  ],
  modules: ["@ant-design-vue/nuxt"]
})