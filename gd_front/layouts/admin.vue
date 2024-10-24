<!-- layouts/admin.vue -->
<template>
  <a-layout style="min-height: 100vh">
    <!-- Header: Full width -->
    <a-layout-header class="site-layout-background"
      style="padding: 0; background: #001529; width: 100vw; position: fixed; z-index: 1; top: 0; left: 0; display: flex;justify-content: space-between;">
      <div style="color: white; text-align: center; font-size: 20px; line-height: 64px;">
        <NuxtLink to="/admin">Admin Panel</NuxtLink>
      </div>
      <div style="color: white;">
        <a href="#" @click.stop.prevent="handleLogout()">Log out</a>
      </div>
    </a-layout-header>

    <!-- Content layout with sidebar and main content -->
    <a-layout style="margin-top: 64px;"> <!-- Push down by header height -->
      <!-- Sidebar -->
      <a-layout-sider collapsible v-model:collapsed="collapsed" width="200" class="site-layout-background">
        <SideBarMenu :collapsed="collapsed" />
      </a-layout-sider>

      <!-- Main Content -->
      <a-layout-content style="margin: 24px 16px 0; overflow: initial">
        <div style="padding: 24px; background: #fff; min-height: 360px">
          <slot />
        </div>
      </a-layout-content>
    </a-layout>
  </a-layout>
</template>

<script setup>
import { ref } from 'vue';
import { useFetch, useCookie, useRuntimeConfig } from '#app'
const authToken = useCookie('auth_token').value
const router = useRouter();
const collapsed = ref(false);
const handleLogout = async () => {
  try {
    // Call the Laravel API to revoke the token
    await $fetch('/api/logout', {
      method: 'POST',
      headers: {
        Authorization: `Bearer ${authToken}`, // Replace with the actual token
      },
    });

    // Clear the local authentication state (remove token)
    auth.logout(); // Assuming your composable manages token removal

    // Redirect to the login page
    router.push('/login');
  } catch (error) {
    console.error('Error logging out:', error);
  }
}
</script>

<style scoped>
.site-layout-background {
  background: #fff;
}
</style>
