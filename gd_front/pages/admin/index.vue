<template>
  <div>
    <h1>Admin Dashboard</h1>
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
            {{ `ServerID:${q.server_id}` }} <a-tag :color="parseInt(q.status) === 1 ? 'green' : 'volcano'">
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
  </div>
</template>

<script setup>
import { useFetch, useCookie, useRuntimeConfig } from '#app'
const config = useRuntimeConfig()
const authToken = useCookie('auth_token').value
const dataSource = ref({
  data: []
})
definePageMeta({
  layout: 'admin',
  middleware: 'auth-admin',
})
const columns = [
  {
    title: 'AgentID',
    dataIndex: 'agent_id',
    key: 'agent_id',
  },
  {
    title: 'Name',
    key: 'name',
  },
  {
    title: 'Status',
    key: 'status',
  },
  {
    title: 'Campaign',
    key: 'campaign',
  },
  {
    title: 'Lead',
    key: 'lead',
  },
  {
    title: 'Active Campaigns',
    key: 'active_campaigns',
  },
  {
    title: 'Action',
    key: 'action',
  },
];
const requestProps = ref({
  perPage: 500,
  orderBy: 'name',
  order: 'asc'
})
const fetchConfBridges = async (page=1) => {
  const params = new URLSearchParams({
    page: page,
    ...requestProps.value,
  });
  const url = `/api/admin/agents/active-conf-bridges?${params.toString()}`;
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
onMounted(() => {
  fetchConfBridges()
})
</script>