<template>

</template>
<script setup>
definePageMeta({
  layout: 'admin', // Specify the layout here
})
const campaigns = ref([])
const fetchCampaigns = async() => {

}
onMounted(() => {
    fetchCampaigns()
})
</script>