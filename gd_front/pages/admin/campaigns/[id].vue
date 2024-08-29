<!-- pages/admin/campaigns/[id].vue -->
<template>
    <div>
        <h1>Campaign Details for ID: {{ campaignId }}</h1>
        <!-- Render campaign details here -->
    </div>
</template>

<script setup>
import { useRoute } from '#app';
const config = useRuntimeConfig()
const authToken = useCookie('auth_token').value
const route = useRoute();
const campaignId = route.params.id; // Access the dynamic route parameter
const campaign = ref(null)
definePageMeta({
    layout: 'admin', // Specify the layout here
})
const fetchCampaign = async () => {
    const url = `/api/admin/campaigns/${campaignId}`;
    const { data, error } = await useFetch(url, {
        baseURL: config.public.apiBaseUrl,
        headers: {
            Accept: `application/json`,
            Authorization: `Bearer ${authToken}`
        }
    })

    if (error.value) {
        console.error('Failed to fetch data:', error.value)
        message.error(error.value?.message);
        return null
    } else {
        campaign.value = data.value
    }
}
onMounted(async () => {
    await fetchCampaign()
})
</script>