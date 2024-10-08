<template>
    <div>
        <a-breadcrumb>
            <a-breadcrumb-item>
                <NuxtLink to="/admin">Admin</NuxtLink>
            </a-breadcrumb-item>
            <a-breadcrumb-item>Campaigns</a-breadcrumb-item>
        </a-breadcrumb>
    
    <h3 style="margin: 10px 0 20px;">Campaign list</h3>
    <a-table :dataSource="dataSource?.data" :columns="columns">
        <template #bodyCell="{ column, record }">
            <template v-if="column.key === 'name'">
                <NuxtLink :to="`/admin/campaigns/${record.id}`">
                    {{ record.name }}
                </NuxtLink>
            </template>
            <template v-else-if="column.key === 'status'">
                <span>
                    <a-tag :color="parseInt(record.status) === 1 ? 'green' : 'grey'">
                        {{ `${parseInt(record.status) === 1 ? 'Active' : 'Inactive'}` }}
                    </a-tag>
                </span>
            </template>
            <template v-else-if="column.key === 'running'">
                <span v-for="q in record.queues">
                    {{ `ServerID:${q.server_id}` }} <a-tag  :color="parseInt(q.status) === 1 ? 'green' : 'volcano'">
                         {{ `${parseInt(q.status) === 1 ? 'Running' : 'Paused'}` }}
                    </a-tag>
                </span>
            </template>
            <template v-else-if="column.key === 'action'">
                <a-button :disabled="record.code == 'CLOSER'" @click.prevent="removeCampaign(record.id)">
                    <DeleteOutlined />
                </a-button>
            </template>
        </template>
    </a-table>
    <a-modal :open="isModalOpen"></a-modal>
</div>
</template>
<script setup>
import { message } from 'ant-design-vue';
const config = useRuntimeConfig()
const authToken = useCookie('auth_token').value
const isModalOpen = ref(false)
definePageMeta({
    layout: 'admin', 
    middleware: 'auth-admin',
})
const columns = [
    {
        title: 'Campaign',
        dataIndex: 'name',
        key: 'name',
    },
    {
        title: 'Active',
        dataIndex: 'status',
        key: 'status',
    },
    {
        title: 'Status',
        key: 'running',
    },
    {
        title: 'Agents',
        dataIndex: 'agents',
        key: 'agents',
    },
    {
        title: 'Action',
        key: 'action',
    },
]
const dataSource = ref({
    data: []
})
const requestProps = ref({
    perPage: 500,
    orderBy: 'name',
    order: 'asc'
})
const removeCampaign = async (id) => {
    if(!confirm('DeleteCampaign?')) {
        return
    }
    const url = `/api/admin/campaigns/${id}`;
    const { data, error } = await useFetch(url, {
        baseURL: config.public.apiBaseUrl,
        method: 'DELETE',
        headers: {
            Accept: `application/json`,
            Authorization: `Bearer ${authToken}`
        }
    })

    if (error.value) {
        console.error('Failed to delete campaign:', error.value)
        message.error(error.value?.message);
        return null
    } else {
        fetchCampaigns()
    }
}

const fetchCampaigns = async (page = 1) => {
    const params = new URLSearchParams({
        page: page,
        ...requestProps.value,
    });
    const url = `/api/admin/campaigns?${params.toString()}`;
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
        dataSource.value = data.value
    }
}
onMounted(async () => {
    await fetchCampaigns()
})
</script>