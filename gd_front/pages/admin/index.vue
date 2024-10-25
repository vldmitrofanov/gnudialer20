<template>
  <div>
    <h1>Admin Dashboard</h1>
    <div class="lmy658d">
      <div>
        <select v-model="server_id">
          <option :value="null" disabled>Select Server</option>
          <option v-for="s in servers" :value="s.id">{{ s.name }} [{{ s.host }}]</option>
        </select>
      </div>
      <div><span v-if="connected > 0">Connected</span><span v-else>Offline</span></div>
    </div>
    <a-table :dataSource="dataSource?.data" :columns="columns">
      <template #bodyCell="{ column, record }">
        <template v-if="column.key === 'agent_id'">
          <NuxtLink :to="`/admin/agents/${record.id}`">
            {{ record.agent_id }}
          </NuxtLink>
        </template>
        <template v-else-if="column.key === 'name'">
          {{ record?.agent?.user?.first_name }} {{ record?.agent?.user?.last_name }}
        </template>
        <template v-else-if="column.key === 'status'">
          <span>
            <a-tag
              :color="parseInt(record.online) === 1 && parseInt(record.pause) === 0 && parseInt(record.available) === 1 ? 'green' : parseInt(record.online) === 1 && parseInt(record.available) === 0 && parseInt(record.pause) === 0 ? 'pink' : 'grey'"
              v-if="record.online > 0">
              {{ `${parseInt(record.available) === 1 && parseInt(record.pause) === 0 ? 'Online' : parseInt(record.pause)
                === 1 ? 'Paused' : 'On Call'}` }}
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
        <template v-else-if="column.key === 'campaign'">
          {{ record.campaign }}
        </template>
        <template v-else-if="column.key === 'lead'">
          {{ record.lead }}
        </template>
        <template v-else-if="column.key === 'action'"></template>
      </template>
    </a-table>
  </div>
</template>

<script setup>
const DEBUG = true;
let INTERVAL;
import { useFetch, useCookie, useRuntimeConfig } from '#app'
const config = useRuntimeConfig()
const authToken = useCookie('auth_token').value
const servers = ref([])
const server_id = ref(null)
const connected = ref(false)
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
const initiateWebsocket = (server) => {
  if (DEBUG) {
    console.log('initiateWebsocket(server)', server?.data.ws.proto)
  }
  const agentRegex = /PJSIP\/\d+-/;
  const proto = server?.data?.ws?.proto
  const host = server?.data?.ws?.host
  const app_name = server?.data?.ws?.app_name
  const user = server?.data?.ws?.user
  const password = server?.data?.ws?.password
  if (!host) {
    message.error('A critical error occured while connecting to the server')
    return
  }
  const ws = new WebSocket(`${proto}://${host}/ari/events?api_key=${user}:${password}&app=${app_name}&subscribeAll=true`);
  ws.onopen = () => {
    if (DEBUG) {
      console.log('WebSocket connection opened');
    }
    connected.value = true
  };

  // Listen for incoming messages (events)
  ws.onmessage = (event) => {
    const data = JSON.parse(event.data);  // Assuming the event data is JSON
    //console.log('Received event:', data);

    // Handle different events based on the event type
    switch (data.variable) {
      case 'BRIDGEPEER':
        if (DEBUG) {
          console.log('BRIDGEPEER event:', data);
        }
        if (agentRegex.test(data.value)) {
          onBringePeer(data)
        }
        break;
    }
    switch (data.dialplan_app) {
      case 'AddQueueMember':
        console.log('AddQueueMember event:', data);
        if (agentRegex.test(data.value)) {
          console.log(data)
        }
        break;
    }
    switch (data.type) {
      case "ChannelDestroyed":
        if (agentRegex.test(data.value)) {
          onChannelDestroyed(data)
        }
        break;
      case "ChannelEnteredBridge":
        if (agentRegex.test(data.value)) {
          if (DEBUG) {
            console.log('ENTERED_BRIDGE', data.bridge)
          }
          onChannelEnteredBridge(data)
        }
        break;
      case "Dial":
        if (data.dialstatus === "CONGESTION") {
          if (DEBUG) {
            console.log('congestion', data)
          }
          onCongestion(data)
        }
        break;
    }
  };

  // Handle WebSocket errors
  ws.onerror = (error) => {
    console.error('WebSocket error:', error);
    connected.value = false;  // Update connection status on error
  };

  // Handle WebSocket close
  ws.onclose = (event) => {
    if (DEBUG) {
      console.log('WebSocket connection closed:', event);
    }
    connected.value = false;  // Update connection status on close
  };
}
const requestProps = ref({
  perPage: 500,
  orderBy: 'name',
  order: 'asc'
})
const fetchConfBridges = async (page = 1) => {
  const params = new URLSearchParams({
    page: page,
    server_id: server_id.value,
    ...requestProps.value,
  });
  const url = `/api/admin/agents/active-conf-bridges?${params.toString()}`;
  const { data, error } = await $fetch(url, {
    baseURL: config.public.apiBaseUrl,
    headers: {
      Accept: `application/json`,
      Authorization: `Bearer ${authToken}`
    }
  })

  if (error) {
    console.error('Failed to fetch data:', error)
    message.error(error.message);
    return null
  } else {
    console.log('DATA', data)
    dataSource.value.data = data
    return 1
  }
}
const fetchServers = async () => {
  const { data, error } = await $fetch('/api/admin/servers', {
    baseURL: config.public.apiBaseUrl,
    headers: {
      Accept: `application/json`,
      Authorization: `Bearer ${authToken}`
    }
  })

  if (error) {
    console.error('Failed to fetch data:', error)
    return null
  } else {
    if (DEBUG) {
      console.log('Fetched data:', data)
    }
    return data
  }
}

const updateDataSource = (newData) => {
  const existingData = dataSource.value.data;

  // Create a map of the existing data for quick lookup
  const existingMap = new Map(existingData.map(item => [item.id, item]));

  // Loop through new data to update or add entries
  newData.forEach(newItem => {
    const existingItem = existingMap.get(newItem.id);

    if (existingItem) {
      // If the item exists, update active, pause, online
      existingItem.active = newItem.active;
      existingItem.pause = newItem.pause;
      existingItem.online = newItem.online;
    } else {
      // If the item doesn't exist, add it to the data source
      dataSource.value.data.push(newItem);
    }
  });

  // Remove entries from the existing data that are not present in new data
  dataSource.value.data = existingData.filter(existingItem => {
    return newData.some(newItem => newItem.id === existingItem.id);
  });
};
const updateConfBridges = async (page = 1) => {
  const params = new URLSearchParams({
    page: page,
    server_id: server_id.value,
    ...requestProps.value,
  });
  const url = `/api/admin/agents/active-conf-bridges?${params.toString()}`;
  const { data, error } = await $fetch(url, {
    baseURL: config.public.apiBaseUrl,
    headers: {
      Accept: `application/json`,
      Authorization: `Bearer ${authToken}`
    }
  })

  if (error) {
    console.error('Failed to fetch data:', error)
    message.error(error.message);
    return null
  } else {
    updateDataSource(data)
  }
}
onMounted(async () => {
  servers.value = await fetchServers()
  if (servers.value?.length > 0) {
    server_id.value = servers.value[0].id
    fetchConfBridges()
    initiateWebsocket(servers.value[0])
    INTERVAL = setInterval(() => {
      updateConfBridges();  // Pass your `server_id` here
    }, 5000);
  }

})
</script>
<style lang="scss">
.lmy658d {
  margin: 20px 0;
  display: flex;
  justify-content: space-between;

  select {
    padding: 4px 8px;
  }
}
</style>