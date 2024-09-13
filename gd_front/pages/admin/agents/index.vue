<template>
  <div v-if="loaded">
    <a-breadcrumb>
      <a-breadcrumb-item>
        <NuxtLink to="/admin">Admin</NuxtLink>
      </a-breadcrumb-item>
      <a-breadcrumb-item>Agents</a-breadcrumb-item>
    </a-breadcrumb>
    <h1 style="margin: 20px 0 10px;">Agent list</h1>
    <a-table :dataSource="dataSource?.data" :columns="columns">
      <template #bodyCell="{ column, record }">
        <template v-if="column.key === 'name'">
          <NuxtLink :to="`/admin/agents/${record.id}`">
            {{ record.user?.first_name }} {{ record.user?.last_name }}
          </NuxtLink>
        </template>
        <template v-else-if="column.key === 'status'">
          <span>
            <a-tag :color="parseInt(record.status) === 1 ? 'green' : 'grey'">
              {{ `${parseInt(record.status) === 1 ? 'Active' : 'Inactive'}` }}
            </a-tag>
          </span>
        </template>
        <template v-else-if="column.key === 'server'">
          <div>
            {{ record.server?.name }} [{{ record.server?.host }}]
          </div>
        </template>
        <template v-else-if="column.key === 'sip'">
          <div>
            <pre v-if="showSip === record.id">{{ record.settings?.pjsip?.content }}</pre>
            <a href="#" @click.stop.prevent="toggleShowSip(record.id)">{{ `${showSip === record.id?'Hide':'Show'}` }}</a>
          </div>
        </template>
        <template v-else-if="column.key === 'running'">
          <span v-for="q in record.queues">
            {{ `ServerID:${q.server_id}` }} <a-tag :color="parseInt(q.status) === 1 ? 'green' : 'volcano'">
              {{ `${parseInt(q.status) === 1 ? 'Running' : 'Paused'}` }}
            </a-tag>
          </span>
        </template>
      </template>
    </a-table>
  </div>
  <div v-else>
    <a-skeleton active />
  </div>
</template>
<script setup>
const config = useRuntimeConfig()
const authToken = useCookie('auth_token').value
const isModalOpen = ref(false)
const loaded = ref(false)
const showSip = ref(0)
definePageMeta({
  layout: 'admin', 
  middleware: 'auth-admin',
})

const columns = [
  {
    title: 'Extension',
    dataIndex: 'id',
    key: 'id',
  },
  {
    title: 'Name',
    key: 'name',
  },
  {
    title: 'Server',
    key: 'server',
  },
  {
    title: 'SIP',
    key: 'sip',
  },
  {
    title: 'Action',
    key: 'action',
  },
]
const toggleShowSip = (id) => {
  if (showSip.value === id) {
    showSip.value = 0
  } else {
    showSip.value = id
  }
}
const dataSource = ref({
  data: []
})
const requestProps = ref({
  perPage: 30,
  orderBy: 'name',
  order: 'asc'
})
const fetchAgents = async (page = 1) => {
  const params = new URLSearchParams({
    page: page,
    ...requestProps.value,
  });
  const url = `/api/admin/agents?${params.toString()}`;
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
    loaded.value = true
    return null
  } else {
    dataSource.value = data.value
    loaded.value = true
  }
}
onMounted(async () => {
  await fetchAgents()
})
</script>