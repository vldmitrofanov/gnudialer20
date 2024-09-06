<template>
    <div v-if="loaded">
        <a-breadcrumb>
            <a-breadcrumb-item>
                <NuxtLink to="/admin">Admin</NuxtLink>
            </a-breadcrumb-item>
            <a-breadcrumb-item>
                <NuxtLink to="/admin/agents">Agents</NuxtLink>
            </a-breadcrumb-item>
            <a-breadcrumb-item>{{ agent?.name }}</a-breadcrumb-item>
        </a-breadcrumb>
        <h1 style="margin: 20px 0 10px;">Aent: {{ agent?.name }}</h1>
    </div>
    <div v-else>
        <a-skeleton active />
    </div>
</template>
<script setup>
import { useRoute } from '#app';
const config = useRuntimeConfig()
const authToken = useCookie('auth_token').value
const route = useRoute();
const agentId = route.params.id; // Access the dynamic route parameter
</script>