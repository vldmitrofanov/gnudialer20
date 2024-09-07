<template>
    <div>
        <a-breadcrumb>
            <a-breadcrumb-item>
                <NuxtLink to="/admin">Admin</NuxtLink>
            </a-breadcrumb-item>
            <a-breadcrumb-item>
                <NuxtLink to="/admin/leads">Leads</NuxtLink>
            </a-breadcrumb-item>
            <a-breadcrumb-item>Leads in "{{ campaignName }}"</a-breadcrumb-item>
        </a-breadcrumb>
        <div class="ant-table-wrapper">
            <div class="ant-table">
                <div class="ant-table-container">
                    <div class="ant-table-content">
                        <table style="table-layout: auto;">
                            <thead>
                                <tr>
                                    <th v-for="nf in fieldNames" class="ant-table-cell mft6__th">
                                        {{ nf }}
                                    </th>
                                </tr>
                            </thead>
                            <tbody>
                                <tr v-for="record in dataSource.data">
                                    <td v-for="nf in fieldNames">{{ record[nf] }}</td>
                                </tr>
                            </tbody>
                        </table>
                    </div>
                </div>
            </div>
            <a-pagination :current="pagination.current" :pageSize="pagination.per_page" :total="pagination.total"
                @change="fetchLeads" />
        </div>
    </div>
</template>
<script setup>
import { useRoute } from '#app';
definePageMeta({
    layout: 'admin', // Specify the layout here
})
const config = useRuntimeConfig()
const authToken = useCookie('auth_token').value
const route = useRoute();
const campaignName = route.params.name; // Access the dynamic route parameter
const fieldsToRemove = ['tzl', 'tzh', 'wdayl', 'wdayh', 'satl', 'sath', 'sunl', 'sunh', 'holl', 'holh'];
const fieldNames = ref([])
const dataSource = ref({
    data: []
})
const requestProps = ref({
    perPage: 30,
    orderBy: 'name',
    order: 'asc'
})
const pagination = computed(() => dataSource.value)
const fetchLeads = async (page = 1) => {
    const params = new URLSearchParams({
        campaign: campaignName,
        page: page,
        ...requestProps.value,
    });
    const url = `/api/admin/leads?${params.toString()}`;
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
        dataSource.value = data.value.leads
        fieldNames.value = Object.keys(data.value?.leads?.data[0]);
        fieldNames.value = fieldNames.value.filter(field => !fieldsToRemove.includes(field));
        return
    }
}
onMounted(async () => {
    await fetchLeads(1)
})
</script>

<style lang="scss">
.mft6__th {
    position: relative;
    color: rgba(0, 0, 0, 0.88);
    font-weight: 600;
    text-align: start;
    background: #fafafa;
    border-bottom: 1px solid #f0f0f0;
    transition: background 0.2s ease;
    padding: 16px 16px;
    overflow-wrap: break-word;
}
</style>