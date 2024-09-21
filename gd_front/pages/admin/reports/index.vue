<template>
  <div>
    <a-breadcrumb>
      <a-breadcrumb-item>
        <NuxtLink to="/admin">Admin</NuxtLink>
      </a-breadcrumb-item>
      <a-breadcrumb-item>
        <NuxtLink to="/admin/reports">Reports</NuxtLink>
      </a-breadcrumb-item>
      <a-breadcrumb-item>{{ `${activeTab == "agent" ? 'Agent' : 'Campaign'}` }} Reports</a-breadcrumb-item>
    </a-breadcrumb>
    <div class="lntw474">
      <h3>{{ `${activeTab == "agent" ? 'Agent' : 'Campaign'}` }} Reports</h3>
      <div class="lntw474_item"><label>Select dates</label>
        <a-range-picker v-model:value="dateRange" show-time valueFormat="YYYY-MM-DD HH:mm:ss" />
      </div>
      <a-tabs v-model:activeKey="activeTab">
        <a-tab-pane key="campaign" tab="Campaign Reports">
          <div class="gyue5">
            <label>Select Campaign</label>
            <select v-model="dispoRequestProps.campaign_id">
              <option :value="0">Please Selest</option>
              <option :value="c.id" v-for="c in campaigns.data">{{ c.name }}</option>
            </select>
          </div>
        </a-tab-pane>
        <a-tab-pane key="agent" tab="Agent Reports">
          <div class="lntw474_item"><label>Select Agents</label>
            <a-transfer v-model:target-keys="agentTargetKeys" v-model:selected-keys="agentSelectedKeys" show-search
              :data-source="allAgents" :titles="['Source', 'Target']" :render="item => item.name"
              :disabled="agentTransferDisabled" @change="handleAgentListChange"
              @selectChange="handleAgentListSelectChange" />
          </div>
        </a-tab-pane>
      </a-tabs>
      <div style="display: grid;grid-template-columns: repeat(4,1fr);gap: 10px;padding: 20px 0;">
      <label style="font-weight: 500;">
        <input type="checkbox" v-model="checkAll" @change="toggleAllDispo" />
        Check All
      </label>

      <div v-for="(value, index) in dispoValues" :key="index">
        <label>
          <input type="checkbox" v-model="dispoRequestProps.dispositions" :value="value" />
          {{ getDispo(value) }}
        </label>
      </div>
    </div>
      <div><a-button @click="fetchDispositions(1)">Get results</a-button></div>
    </div>
    <div class="ant-table-wrapper">
      <div class="ant-table">
        <div class="ant-table-container">
          <div class="ant-table-content">
            <table style="table-layout: auto;" class="it7uer-table">
              <thead>
                <tr>
                  <th>LeadID</th>
                  <th>Campaign</th>
                  <th>Agent</th>
                  <th>Call Date</th>
                  <th>Call Duration</th>
                  <th>Disposition</th>
                </tr>
              </thead>
              <tbody>
                <tr v-for="dispo in dispositions.data">
                  <td>{{ dispo.lead_id }}</td>
                  <td>{{ dispo.campaign?.name }} [{{ dispo.campaign?.code }}]</td>
                  <td>{{ dispo.agent?.name }} [{{ dispo.agent?.id }}]</td>
                  <td>{{ dispo.start }}</td>
                  <td>{{ dispo.call_duration }}</td>
                  <td>{{ getDispo(dispo.disposition) }}</td>
                </tr>
              </tbody>
            </table>
          </div>
        </div>
      </div>
    </div>

    <a-pagination :current="pagination.current" :pageSize="pagination.per_page" :total="pagination.total"
      @showSizeChange="handlePageSizeChange" @change="fetchDispositions" />
  </div>
</template>
<script setup>
const config = useRuntimeConfig()
const authToken = useCookie('auth_token').value
definePageMeta({
  layout: 'admin',
  middleware: 'auth-admin',
})
const activeTab = ref('agent')
const route = useRoute()
const dateRange = ref(null)
const allAgents = ref([])
const agentRequestProps = ref({
  perPage: 500,
  orderBy: 'last_name',
  order: 'asc'
})
const agentTransferDisabled = ref(false)
const agentTargetKeys = ref([])
const agentSelectedKeys = ref([])
const dispositions = ref({
  data: []
})
const campaigns = ref({
  data: []
})
const pagination = computed(() => dispositions.value)
const fetchAgents = async (page = 1) => {
  const params = new URLSearchParams({
    page: page,
    ...agentRequestProps.value,
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
    return null
  } else {
    //agentSource.value = data.value
    data.value?.data?.forEach(a => allAgents.value.push({ key: String(a.id), name: `${a.user.first_name} ${a.user.last_name}` }))
  }
}

const dispoRequestProps = ref({
  perPage: 30,
  orderBy: 'id',
  order: 'desc',
  agents: [],
  start_date: '',
  end_date: '',
  dispositions: [12],
  campaign_id: 0
})
const handlePageSizeChange = (currentPage, newPageSize) => {
  dispoRequestProps.value.perPage = newPageSize; // Update perPage in requestProps
  fetchDispositions(); // Fetch leads with new page size
};
const fetchDispositions = async (page = 1) => {
  if (!dateRange.value || dateRange.value.length < 2) {
    message.error("Please select date")
    return
  } else {
    dispoRequestProps.value.start_date = dateRange.value[0]
    dispoRequestProps.value.end_date = dateRange.value[1]
  }
  console.log('agentTargetKeys', agentTargetKeys.value)
  if (activeTab.value === 'agent') {
    if (!agentTargetKeys.value || agentTargetKeys.value.length === 0) {
      message.error("Please select agents")
      return
    } else {
      dispoRequestProps.value.campaign_id = 0
      dispoRequestProps.value.agents = agentTargetKeys.value
    }
  } else {
    if (dispoRequestProps.value.campaign_id === 0) {
      message.error("Please select the campaign")
      return
    } else {
      dispoRequestProps.value.agents = null
    }
  }

  const params = new URLSearchParams({
    page: page,
    ...dispoRequestProps.value,
  });
  const url = `/api/admin/dispositions?${params.toString()}`;
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
    //agentSource.value = data.value
    dispositions.value = data.value
  }
}
const campaignRequestProps = ref({
  perPage: 500,
  orderBy: 'name',
  order: 'asc'
})
const fetchCampaigns = async (page = 1) => {
  const params = new URLSearchParams({
    page: page,
    ...campaignRequestProps.value,
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
    campaigns.value = data.value
  }
}
const handleAgentListSelectChange = (sourceSelectedKeys, targetSelectedKeys) => {
  console.log('sourceSelectedKeys: ', sourceSelectedKeys);
  console.log('targetSelectedKeys: ', targetSelectedKeys);
};
const handleAgentListChange = async (nextTargetKeys, direction, moveKeys) => {
  console.log('targetKeys: ', nextTargetKeys);
  console.log('direction: ', direction);
  console.log('moveKeys: ', moveKeys);
}
const dispoValues = Array.from({ length: 13 }, (_, i) => i);
const checkAll = ref(false);
const toggleAllDispo = () => {
  dispoRequestProps.value
  if (checkAll.value) {
    dispoRequestProps.value.dispositions = [...dispoValues]; // Select all
  } else {
    dispoRequestProps.value.dispositions = []; // Deselect all
  }
}
onMounted(async () => {
  fetchCampaigns()
  await fetchAgents()
})
watch(() => route.query.mode, (mode) => {
  if (mode === 'campaign' || mode === 'agent') {
    activeTab.value = mode
  }
}, { immediate: true })
</script>
<style lang="scss">
.lntw474 {
  max-width: 512px;
  padding-top: 20px;
  padding-bottom: 60px;
  display: flex;
  flex-direction: column;
  gap: 10px;

  .lntw474_item {
    display: flex;
    flex-direction: column;
  }
}

.it7uer-table {
  width: 100%;

  td {
    text-align: center;
    padding: 4px;
  }

  tr:nth-child(even) {
    background-color: #f2f7fe;

  }

  td:not(:last-child),
  th:not(:last-child) {
    border-right: solid #d3deee 1px;
  }

  th {
    position: relative;
    color: rgba(0, 0, 0, 0.58);
    font-weight: 600;
    text-align: center;
    background: #fafafa;
    border-bottom: 1px solid #f0f0f0;
    transition: background 0.2s ease;
    padding: 6px 4px;
  }
}
.gyue5{
  display: flex;
  flex-direction: column;
  padding: 20px 0 20px;
  select{
    height: 40px;
    border-radius: 5px;
  }
}
</style>