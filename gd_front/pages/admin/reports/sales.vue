<template>
  <a-breadcrumb>
    <a-breadcrumb-item>
      <NuxtLink to="/admin">Admin</NuxtLink>
    </a-breadcrumb-item>
    <a-breadcrumb-item>
      <NuxtLink to="/admin/reports">Reports</NuxtLink>
    </a-breadcrumb-item>
    <a-breadcrumb-item>Sales Reports</a-breadcrumb-item>
  </a-breadcrumb>
  <div>Sales Reports here</div>
</template>
<script setup>
definePageMeta({
  layout: 'admin',
  middleware: 'auth-admin',
})
</script>