<!-- pages/admin/campaigns/[id].vue -->
<template>
    <div v-if="loaded">
        <a-breadcrumb>
            <a-breadcrumb-item>
                <NuxtLink to="/admin">Admin</NuxtLink>
            </a-breadcrumb-item>
            <a-breadcrumb-item>
                <NuxtLink to="/admin/campaigns">Campaigns</NuxtLink>
            </a-breadcrumb-item>
            <a-breadcrumb-item>{{ campaign?.name }}</a-breadcrumb-item>
        </a-breadcrumb>
        <h1 style="margin: 20px 0 10px;">Campaign Details for: {{ campaign?.name }}</h1>
        <div v-if="campaign?.queues && campaign.queues.length > 0">
            <a-tabs default-active-key="1">
                <a-tab-pane key="1" tab="Filters">
                    <a-row v-for="(queue, qindex) in campaign.queues" :key="queue.id">
                        <h3 style="margin-bottom: 10px;">Server: {{ queue.server?.name }} [{{ queue.server?.host }}]
                        </h3>
                        <a-col :span="24" v-for="(filter, findex) in queue.filters" class="ert3" :key="filter.id">
                            <a-form-item class="ert5" label="Filter">
                                <textarea v-model="campaign.queues[qindex].filters[findex].filter"
                                    :placeholder="`Enter filter`" style="width: 100%;"
                                    :disabled="filterLoading[qindex][findex]" />
                            </a-form-item>
                            <a-form-item class="ert5" label="Enabled?">
                                <select v-model="campaign.queues[qindex].filters[findex].enabled" style="width: 100px;"
                                    :disabled="filterLoading[qindex][findex]">
                                    <option :value="1">Enabled</option>
                                    <option :value="0">Disabled</option>
                                </select>
                            </a-form-item>
                            <a-form-item class="ert5" label="Position">
                                <input v-model="campaign.queues[qindex].filters[findex].position"
                                    :placeholder="`Enter position`" style="width: 60px;"
                                    :disabled="filterLoading[qindex][findex]" />
                            </a-form-item>
                            <a-form-item class="ert6">
                                <a-popconfirm title="Sure delete?" @confirm="handleFilterDelete($event, filter.id)">
                                    <a-button type="primary" danger ghost>
                                        <DeleteOutlined /> Delete
                                    </a-button>
                                </a-popconfirm>
                                <a-button type="primary" ghost @click="handleFilterUpdate(qindex, findex)"
                                    :disabled="filterLoading[qindex][findex]" :loading="filterLoading[qindex][findex]">
                                    <SaveOutlined v-if="!filterLoading[qindex][findex]" /> Save
                                </a-button>
                            </a-form-item>
                        </a-col>
                        <a-col :span="24" class="ert3a">
                            <h3 style="margin-bottom: 10px;">Add filter for {{ campaign?.name }} on Server {{
                                queue.server?.name }} [{{
                                    queue.server?.host }}]
                            </h3>
                            <a-col :span="24" class="ert3">
                                <a-form-item class="ert5" label="Filter">
                                    <textarea v-model="newFilter.filter" :placeholder="`Enter filter`"
                                        style="width: 100%;" :disabled="newFilterLoading" />
                                </a-form-item>
                                <a-form-item class="ert5" label="Enabled?">
                                    <select v-model="newFilter.enabled" style="width: 100px;"
                                        :disabled="newFilterLoading">
                                        <option :value="1">Enabled</option>
                                        <option :value="0">Disabled</option>
                                    </select>
                                </a-form-item>
                                <a-form-item class="ert5" label="Position">
                                    <input v-model="newFilter.position" :placeholder="`Enter position`"
                                        style="width: 60px;" :disabled="newFilterLoading" />
                                </a-form-item>
                                <a-form-item class="ert6">
                                    <a-button type="primary" ghost @click="handleFilterCreate(queue.id)"
                                        :disabled="newFilterLoading">
                                        <SaveOutlined v-if="!newFilterLoading" /> Create
                                    </a-button>
                                </a-form-item>
                            </a-col>
                        </a-col>
                    </a-row>

                </a-tab-pane>
                <a-tab-pane key="2" tab="Settings">
                    <a-row v-for="(queue, qindex2) in campaign?.queues" :key="`e${queue.id}`">
                        <h3 style="margin-bottom: 10px;">Server: {{ queue.server?.name }} [{{ queue.server?.host }}]
                        </h3>

                        <a-col :span="24" v-for="(setting, sindex) in queue.settings" class="ert3b"
                            :key="`e${setting.id}t`">
                            <a-col :span=24 class="wqs1">
                                <a-form-item :label="setting.parameter"
                                    v-if="trueFalseSelectFields.includes(setting.parameter)">
                                    <select v-model="campaign.queues[qindex2].settings[sindex].value"
                                        style="width: 240px;" :disabled="settingLoading[qindex2][sindex]">
                                        <option>true</option>
                                        <option>false</option>
                                    </select>
                                </a-form-item>
                                <a-form-item :label="setting.parameter" v-else-if="setting.parameter === 'function'">
                                    <select v-model="campaign.queues[qindex2].settings[sindex].value"
                                        style="width: 200px;" :disabled="settingLoading[qindex2][sindex]">
                                        <option>predictive</option>
                                    </select>
                                </a-form-item>
                                <a-form-item :label="setting.parameter"
                                    v-else-if="!protectedFields.includes(setting.parameter)">
                                    <input v-model="campaign.queues[qindex2].settings[sindex].value"
                                        :placeholder="`Enter value`" style="width: 240px;"
                                        :disabled="settingLoading[qindex2][sindex]" />
                                </a-form-item>
                                <a-form-item v-else :label="setting.parameter">
                                    <span>{{ setting.value }}</span>
                                </a-form-item>
                                <a-form-item v-if="!protectedFields.includes(setting.parameter)">
                                    <a-button type="primary" ghost @click="handleSettingUpdate(qindex2, sindex)"
                                        :disabled="settingLoading[qindex2][sindex]"
                                        :loading="settingLoading[qindex2][sindex]">
                                        <SaveOutlined v-if="!settingLoading[qindex2][sindex]" /> Save
                                        <!-- Use the imported icon -->
                                    </a-button>
                                </a-form-item>
                            </a-col>
                        </a-col>
                        <a-col class="ert3a rtymt20">
                            <h3> Create a setting for {{ campaign?.name }} on Server: {{ queue.server?.name }} [{{
                                queue.server?.host }}]</h3>
                            <a-form-item>
                                <input v-model="newSetting.parameter" :placeholder="`Parameter`" style="width: 240px;"
                                    :disabled="newSettingLoading" />{{ ` : ` }}
                                <input v-model="newSetting.value" :placeholder="`Parameter`" style="width: 240px;"
                                    :disabled="newSettingLoading" />
                            </a-form-item>
                            <a-form-item>
                                <a-button type="primary" ghost @click="handleSettingCreate(queue.id)"
                                    :disabled="newSettingLoading" :loading="newSettingLoading">
                                    <SaveOutlined v-if="!newSettingLoading" /> Create
                                    <!-- Use the imported icon -->
                                </a-button>
                            </a-form-item>
                        </a-col>
                    </a-row>
                </a-tab-pane>
                <a-tab-pane key="3" tab="Agents">
                    <label style="display: flex;flex-direction: column;margin-bottom: 20px;">Select server
                        <select v-model="selectedServerId" style="width: 200px;">
                            <option :value="server.id" v-for="server in servers">{{ server.label }}</option>
                        </select>
                    </label>
                    <a-transfer v-model:target-keys="agentTargetKeys" v-model:selected-keys="agentSelectedKeys"
                        show-search :data-source="allAgents" :titles="['Source', 'Target']" :render="item => item.name"
                        :disabled="agentTransferDisabled" @change="handleAgentListChange"
                        @selectChange="handleAgentListSelectChange" />
                    <a-col class="ghi6">
                        <a-switch v-model:checked="agentTransferDisabled" un-checked-children="enabled"
                            checked-children="disabled" style="margin-top: 16px" />
                        <a-button type="primary" @click="showAgentModal = !showAgentModal">
                            <SaveOutlined /> Add New Agent
                        </a-button>
                    </a-col>
                </a-tab-pane>
            </a-tabs>
        </div>
        <a-modal :footer="null" title="Add New Agent" :visible="showAgentModal"
            @cancel="showAgentModal = !showAgentModal">
            <NewAgentForm :servers=servers :agents=allAgents
                @updated="() => { showAgentModal = false; fetchCampaign() }" :serverId="selectedServerId" />
        </a-modal>
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
const campaignId = route.params.id; // Access the dynamic route parameter
const campaign = ref(null)
const loading = ref(false)
const loaded = ref(false)
const protectedFields = ['calltoday', 'calls', 'filter', 'totalabandons', 'abandons', 'totalcalls', 'totaltalktime', 'talktime']
const trueFalseSelectFields = ['active', 'usecloser', 'usecallback', 'usetzfilter', 'debug']
const loaderPlaceholder = Array.from({ length: 100 }, () => Array(100).fill(false));
const settingLoading = ref(loaderPlaceholder.map(row => [...row]))
const filterLoading = ref(loaderPlaceholder.map(row => [...row]))
const newFilter = ref({
    filter: '',
    position: 0,
    enabled: 0
})
const newSetting = ref({
    parameter: '',
    value: 0,
    type: 'string'
})
const newFilterLoading = ref(false)
const newSettingLoading = ref(false)
const agentTransferDisabled = ref(true)
const allAgents = ref([])
const agentTargetKeys = ref([])
const agentSelectedKeys = ref([])
const selectedServerId = ref(null)
const servers = ref([])
const showAgentModal = ref(false)

definePageMeta({
    layout: 'admin',
    middleware: 'auth-admin',
})

const handleAgentListChange = async (nextTargetKeys, direction, moveKeys) => {
    console.log('targetKeys: ', nextTargetKeys);
    console.log('direction: ', direction);
    console.log('moveKeys: ', moveKeys);
    let id;
    campaign.value?.queues?.forEach(q => {
        if (q && q.server_id) {
            if (q.server && selectedServerId.value == q.server_id) {
                id = q.id
            }
        }
    })
    if (!id) {
        message.error('Please select a server');
    }
    const url = `/api/admin/queues/${id}/agents`;
    const { data, error } = await useFetch(url, {
        method: 'PUT',
        baseURL: config.public.apiBaseUrl,
        headers: {
            Accept: `application/json`,
            Authorization: `Bearer ${authToken}`
        },
        body: {
            agent_ids: moveKeys,
            action: direction === 'left' ? 'remove' : 'add'
        },
        onResponseError: ({ response }) => {
            // Access response data and handle error
            if (response && response._data && response._data.message) {
                message.error(response._data.message);
            } else {
                message.error('An unknown error occurred');
            }
        }
    })

    if (error.value) {
        console.error('Failed to update agent list:', error.value)
        return null
    } else {
        message.success("Updated")
    }

}

const handleAgentListSelectChange = (sourceSelectedKeys, targetSelectedKeys) => {
    console.log('sourceSelectedKeys: ', sourceSelectedKeys);
    console.log('targetSelectedKeys: ', targetSelectedKeys);
};


const handleFilterDelete = async (e, id) => {
    const url = `/api/admin/filters/${id}`;
    const { data, error } = await useFetch(url, {
        method: 'DELETE',
        baseURL: config.public.apiBaseUrl,
        headers: {
            Accept: `application/json`,
            Authorization: `Bearer ${authToken}`
        },
        onResponseError: ({ response }) => {
            // Access response data and handle error
            if (response && response._data && response._data.message) {
                message.error(response._data.message);
            } else {
                message.error('An unknown error occurred');
            }
        }
    })

    if (error.value) {
        console.error('Failed to fetch data:', error.value)
        return null
    } else {
        fetchCampaign()
        message.success("Deleted")
    }
}

const handleSettingCreate = async (queueId) => {
    newSettingLoading.value = true
    const url = `/api/admin/settings`;
    const { data, error } = await useFetch(url, {
        method: 'POST',
        baseURL: config.public.apiBaseUrl,
        headers: {
            Accept: `application/json`,
            Authorization: `Bearer ${authToken}`
        },
        body: JSON.stringify({ ...newSetting, queue_id: queueId }),
        onResponseError: ({ response }) => {
            // Access response data and handle error
            if (response && response._data && response._data.message) {
                message.error(response._data.message);
            } else {
                message.error('An unknown error occurred');
            }
            newSettingLoading.value = false
        }
    })

    if (error.value) {
        console.error('Failed to fetch data:', error.value)
        return null
    } else {
        fetchCampaign()
        message.success("Updated")
    }
    newSettingLoading.value = false
}


const handleSettingUpdate = async (qi, si) => {
    settingLoading.value[qi][si] = true
    const setting = campaign.value?.queues[qi].settings[si]
    const url = `/api/admin/settings/${setting.id}`;
    const { data, error } = await useFetch(url, {
        method: 'PUT',
        baseURL: config.public.apiBaseUrl,
        headers: {
            Accept: `application/json`,
            Authorization: `Bearer ${authToken}`
        },
        body: { ...setting },
        onResponseError: ({ response }) => {
            // Access response data and handle error
            if (response && response._data && response._data.message) {
                message.error(response._data.message);
            } else {
                message.error('An unknown error occurred');
            }
            settingLoading.value[qi][si] = false
        }
    })

    if (error.value) {
        console.error('Failed to fetch data:', error.value)
        return null
    } else {
        fetchCampaign()
        message.success("Updated")
    }
    settingLoading.value[qi][si] = false
}

const handleFilterCreate = async (queueId) => {
    newFilterLoading.value = true
    const url = `/api/admin/filters`;
    const { data, error } = await useFetch(url, {
        method: 'POST',
        baseURL: config.public.apiBaseUrl,
        headers: {
            Accept: `application/json`,
            Authorization: `Bearer ${authToken}`
        },
        body: { ...newFilter.value, queue_id: queueId },
        onResponseError: ({ response }) => {
            // Access response data and handle error
            if (response && response._data && response._data.message) {
                message.error(response._data.message);
            } else {
                message.error('An unknown error occurred');
            }
            newFilterLoading.value = false;
        }
    })

    if (error.value) {
        console.error('Failed to fetch data:', error.value)
    } else {
        fetchCampaign()
        message.success("Created")
        newFilter.value = {
            filter: '',
            position: 0,
            enabled: 0
        }
    }
    newFilterLoading.value = false
}

const handleFilterUpdate = async (qi, fi) => {
    filterLoading.value[qi][fi] = true
    const filter = campaign.value?.queues[qi].filters[fi]
    const url = `/api/admin/filters/${filter.id}`;
    const { data, error } = await useFetch(url, {
        method: 'PUT',
        baseURL: config.public.apiBaseUrl,
        headers: {
            Accept: `application/json`,
            Authorization: `Bearer ${authToken}`
        },
        body: { ...filter },
        onResponseError: ({ response }) => {
            // Access response data and handle error
            if (response && response._data && response._data.message) {
                message.error(response._data.message);
            } else {
                message.error('An unknown error occurred');
            }
            filterLoading.value[qi][fi] = false
        }
    })

    if (error.value) {
        console.error('Failed to fetch data:', error.value)
        return null
    } else {
        fetchCampaign()
        message.success("Updated")
    }
    filterLoading.value[qi][fi] = false
}

const fetchCampaign = async () => {
    const url = `/api/admin/campaigns/${campaignId}`;
    const { data, error } = await useFetch(url, {
        baseURL: config.public.apiBaseUrl,
        headers: {
            Accept: `application/json`,
            Authorization: `Bearer ${authToken}`
        },
        onResponseError: ({ response }) => {
            // Access response data and handle error
            if (response && response._data && response._data.message) {
                message.error(response._data.message);
            } else {
                message.error('An unknown error occurred');
            }
        }
    })

    loaded.value = true
    if (error.value) {
        console.error('Failed to fetch data:', error.value)
        return null
    } else {
        campaign.value = data.value?.data
        servers.value = []
        if (!selectedServerId.value) {
            selectedServerId.value = campaign.value?.queues[0].server.id
        }
        campaign.value?.queues?.forEach(q => {
            if (q && q.server) {
                servers.value.push({ id: q.server.id, label: `${q.server.name} (${q.server.host})` })
            }
            if (q && q.agents) {
                if (q.server && selectedServerId.value == q.server.id) {
                    q.agents.forEach(a => agentTargetKeys.value.push(String(a.id)))
                }
            }
        })
    }
}

const fetchAgents = async (serverId) => {
    const url = `/api/admin/agents?server_id=${serverId}&paginate=0&short=1`;
    const { data, error } = await useFetch(url, {
        baseURL: config.public.apiBaseUrl,
        headers: {
            Accept: `application/json`,
            Authorization: `Bearer ${authToken}`
        },
        onResponseError: ({ response }) => {
            // Access response data and handle error
            if (response && response._data && response._data.message) {
                message.error(response._data.message);
            } else {
                message.error('An unknown error occurred');
            }
        }
    })

    loaded.value = true
    if (error.value) {
        console.error('Failed to fetch data:', error.value)
        return null
    } else {
        allAgents.value = []
        data.value?.data?.forEach(a => allAgents.value.push({ key: String(a.id), name: a.name }))
    }
}

onMounted(async () => {
    await fetchCampaign()
})

watch(selectedServerId, (newVal) => {
    if (newVal && newVal > 0) {
        fetchAgents(newVal)
        agentTargetKeys.value = []
        campaign.value?.queues?.forEach(q => {
            if (q.server && selectedServerId.value == q.server.id) {
                q?.agents?.forEach(a => agentTargetKeys.value.push(String(a.id)))
            }
        })
    }
})
</script>

<style lang="scss">
.rtymt20 {
    margin-top: 30px;
}

.ert3a {
    margin-bottom: 100px;
}

.ert5 {
    margin-bottom: 5px;

    label {
        min-width: 80px;
    }
}

.ert3 {
    position: relative;
    margin-bottom: 0px;
    background-color: transparent;
    padding: 20px 20px;

    &:nth-of-type(odd) {
        background-color: #f2f7fe;
    }
}

.ert6 {
    display: flex;
    justify-content: flex-end;
    position: absolute;
    width: auto;
    bottom: 0;
    margin-bottom: 20px;
    right: 20px;
    min-width: 200px;
}

.wqs1 {
    margin-bottom: 0;
    display: flex;
    justify-content: space-between;

    .ant-form-item {
        margin-bottom: 0;

        label {
            min-width: 150px;
        }
    }

    button.ant-btn-primary {
        width: 100px;
    }
}

.ert3b {
    position: relative;
    margin-bottom: 0px;
    background-color: transparent;
    padding: 10px 20px;

    &:nth-of-type(odd) {
        background-color: #f2f7fe;
    }
}
.ghi6{
    display: flex;
    justify-content: space-between;
    align-items: center;
}
</style>