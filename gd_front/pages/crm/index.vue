<template>
    <a-row>
        <!-- Left Pane: Dial Pad and Disposition Buttons -->
        <a-col :span="5" class="left-pane">
            <a-card title="Disposition" class="disposition">
                <a-col :span="24">
                    <span>Server: {{ `${connected ? 'Connected' : 'Disconnected'}` }}</span>
                </a-col>
                <a-col :span="24">
                    <span>Agent Status: {{ `${agentStatus?.status?.Paused == 1 ? 'Paused' : agentStatus?.status?.Status >
                        1 ? 'Active' : 'Inactive'}` }}</span>
                </a-col>
                <a-row :gutter="{ xs: 8, sm: 8, md: 8, lg: 8 }">
                    <a-col :span="12">
                        <a-button block class="disposition-button" :disabled="allButtonsDisabled">
                            Callback <small>0</small>
                        </a-button>
                    </a-col>
                    <a-col :span="12">
                        <a-button block class="disposition-button" :disabled="allButtonsDisabled">
                            Fresh <small>1</small>
                        </a-button>
                    </a-col>
                    <a-col :span="12">
                        <a-button block class="disposition-button" :disabled="allButtonsDisabled">
                            NoAns <small>2</small>
                        </a-button>
                    </a-col>
                    <a-col :span="12">
                        <a-button block class="disposition-button" :disabled="allButtonsDisabled">
                            AnsMach <small>3</small>
                        </a-button>
                    </a-col>
                    <a-col :span="12">
                        <a-button block class="disposition-button" :disabled="allButtonsDisabled">
                            Busy <small>4</small>
                        </a-button>
                    </a-col>
                    <a-col :span="12">
                        <a-button block class="disposition-button" :disabled="allButtonsDisabled">
                            FBusy <small>5</small>
                        </a-button>
                    </a-col>
                    <a-col :span="12">
                        <a-button block class="disposition-button" :disabled="allButtonsDisabled">
                            Fax <small>6</small>
                        </a-button>
                    </a-col>
                    <a-col :span="12">
                        <a-button block class="disposition-button" :disabled="allButtonsDisabled">
                            Disco <small>7</small>
                        </a-button>
                    </a-col>
                    <a-col :span="12">
                        <a-button block class="disposition-button dnc-button" :disabled="allButtonsDisabled">
                            DNC <small>8</small>
                        </a-button>
                    </a-col>
                    <a-col :span="12">
                        <a-button block class="disposition-button" :disabled="allButtonsDisabled">
                            Invalid <small>9</small>
                        </a-button>
                    </a-col>
                    <a-col :span="12">
                        <a-button block class="disposition-button" :disabled="allButtonsDisabled">
                            Other <small>10</small>
                        </a-button>
                    </a-col>
                    <a-col :span="12">
                        <a-button block class="disposition-button sale-button" :disabled="allButtonsDisabled">
                            Sale <small>12</small>
                        </a-button>
                    </a-col>
                    <a-col :span="12">
                        <a-button block class="disposition-button" :disabled="allButtonsDisabled">
                            NoSale <small>11</small>
                        </a-button>
                    </a-col>
                    <a-col :span="24">
                        <a-dropdown class="qsb-etytrui" :disabled="queueButtonDisabled">
                            <template #overlay>
                                <a-menu>
                                    <a-menu-item v-for="ql in queues" :key="ql" :value="ql"
                                        @click="handleSelectQueue(ql)">
                                        {{ ql.campaign?.name }}
                                    </a-menu-item>
                                </a-menu>
                            </template>
                            <a-button>
                                {{ `${queue ? queue.campaign?.name : 'Select Queue'}` }}
                                <DownOutlined />
                            </a-button>
                        </a-dropdown>
                    </a-col>
                    <a-col :span="24" v-if="queue">
                        <a-button block class="start-button" :disabled="startButtonDisabled" @click="togglePause">
                            <template #icon v-if="running">Pause <pause-circle-filled /></template>
                            <template #icon v-else>Start <play-circle-filled /></template>
                        </a-button>
                    </a-col>
                </a-row>
            </a-card>
        </a-col>

        <!-- Right Pane: Form with Tabs -->
        <a-col :span="19" class="right-pane">
            <a-tabs default-active-key="1">
                <a-tab-pane key="1" tab="Main">
                    <a-form layout="vertical">
                        <a-row :gutter=24 v-if="lead">
                            <a-col :span="8" v-for="(value, key) in lead" :key="key">
                                <a-form-item :label="key">
                                    <a-input :placeholder="key" v-model="lead[key]"/>
                                </a-form-item>
                            </a-col>
                        </a-row>
                    </a-form>
                </a-tab-pane>
                <a-tab-pane key="2" tab="Script">
                    <!-- Content for the Notes tab -->
                    <a-textarea rows="4" placeholder="Add notes about the lead" />
                </a-tab-pane>
                <a-tab-pane key="3" tab="History">
                    <!-- Content for the History tab -->
                    <p>Call history and other interactions with the lead.</p>
                </a-tab-pane>agentStatus
            </a-tabs>
        </a-col>
    </a-row>
</template>

<script setup>
import { message } from 'ant-design-vue';
import { useFetch, useCookie, useRuntimeConfig } from '#app'
const config = useRuntimeConfig()
const authToken = useCookie('auth_token').value
const name = 'DialerCRM'
const serverData = ref(null)
const allButtonsDisabled = ref(true)
const running = computed(() => parseInt(agentStatus.value?.status?.Status) > 1 && parseInt(agentStatus.value?.status?.Paused) === 0)
const user = ref(null)
const agent = ref(null)
const connected = ref(false)
const startButtonDisabled = ref(true)
const agentStatus = ref(null)
const queues = ref([])
const queue = ref(null)
const queueButtonDisabled = ref(true)
const loggedIn = ref(false)
const channel = ref(null)
const lead = ref(null)
const callerId = ref(null)

const initiateWebsocket = (server) => {
    const proto = server?.data?.ws?.proto
    const host = server?.data?.ws?.host
    const app_name = server?.data?.ws?.app_name
    const user = server?.data?.ws?.user
    const password = server?.data?.ws?.password
    const ws = new WebSocket(`${proto}://${host}/ari/events?api_key=${user}:${password}&app=${app_name}&subscribeAll=true`);
    ws.onopen = () => {
        console.log('WebSocket connection opened');
        connected.value = true
    };

    // Listen for incoming messages (events)
    ws.onmessage = (event) => {
        const data = JSON.parse(event.data);  // Assuming the event data is JSON
        //console.log('Received event:', data);

        // Handle different events based on the event type
        switch (data.variable) {
            case 'BRIDGEPEER':
                console.log('BRIDGEPEER event:', data);
                if (data.value.includes(`PJSIP/${agent.value?.id}-`)) {
                    onBringePeer(data)
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
        console.log('WebSocket connection closed:', event);
        connected.value = false;  // Update connection status on close
    };
}

const onBringePeer = (data) => {
    if(data.channel) {
        channel.value = data.channel.name
        const str = data.channel.connected.name
        callerId.value = data.channel.caller
        const trimmedStr = str.slice(1, -1);  // Removes the first and last characters

        // Step 2: Split the string by "-"
        const parts = trimmedStr.split('-');

        // Retrieve the values
        const campaign = parts[0];  // "test1"
        const leadId = parts[1]; // "111"
        getLead(campaign,leadId)
        allButtonsDisabled.value = false
    }
}

const getLead = async(campaign,leadId) => {
    if(!campaign || !leadId) {
      message.error('No campaign found');
      return;
    }
    lead.value = null 
    const { data, error } = await useFetch(`/api/leads?campaign=${campaign}&lead_id=${leadId}`, {
        baseURL: config.public.apiBaseUrl,
        headers: {
            Accept: `application/json`,
            Authorization: `Bearer ${authToken}`
        }
    })

    if (error.value) {
        console.error('Failed to fetch data:', error.value)
        message.error(error.value);
        return null
    } else {
        //console.log('Fetched data:', data.value)
        lead.value = data.value?.lead     
    }
}

const handleSelectQueue = (ql) => {
    console.log(ql)
    queue.value = ql
    if (queue.value) {
        getAgentStatus()
    }
}

const parseAsteriskResponse = (response) => {
    if (typeof response !== 'string') {
        response = String(response);
    }

    const events = response.trim().split("\r\n\r\n");
    const parsedData = {};

    events.forEach(event => {
        const lines = event.split("\r\n");

        lines.forEach(line => {
            const [key, value] = line.split(": ");
            if (key && value !== undefined) {
                parsedData[key] = value;
            }
        });
    });

    return parsedData;
}


const togglePause = async () => {
    const serverId = serverData.value.id
    const agentId = agent.value?.id
    const queueCode = queue.value?.campaign?.code
    const pause = running.value
    const { data, error } = await useFetch(`/api/asterisk/agent/pause`, {
        method: 'POST',
        baseURL: config.public.apiBaseUrl,
        headers: {
            Accept: `application/json`,
            Authorization: `Bearer ${authToken}`
        },
        body: {
            server_id: serverId,
            agent: agentId,
            queue: queueCode,
            pause: pause
        }
    })

    if (error.value) {
        console.error('Failed to fetch data:', error.value)
        return null
    } else {
        //console.log('Fetched data:', data.value)
        const resObj = parseAsteriskResponse(data.value?.response)
        if (parseInt(resObj?.Status) === 2) {
            agentStatus.value = {
                status: {
                    Status: "2",
                    Paused: resObj?.Paused,
                    InCall: resObj?.Incall,
                }
            }
        }
    }
}

const getAgentQueues = async () => {
    const serverId = serverData.value.id
    const agentId = agent.value?.id
    const { data, error } = await useFetch(`/api/queue?server_id=${serverId}&agent_id=${agentId}`, {
        baseURL: config.public.apiBaseUrl,
        headers: {
            Accept: `application/json`,
            Authorization: `Bearer ${authToken}`
        }
    })

    if (error.value) {
        console.error('Failed to fetch data:', error.value)
        return null
    } else {
        console.log('Fetched data:', data.value)
        queues.value = data.value.data
        if (!loggedIn.value) {
            queueButtonDisabled.value = false
        }
    }
}

const getAgentStatus = async () => {
    startButtonDisabled.value = true
    const serverId = serverData.value.id
    const queueName = queue?.value.campaign?.code
    const agentId = agent.value?.id
    const { data, error } = await useFetch(`/api/asterisk/agent/status?server_id=${serverId}&queue=${queueName}&agent=${agentId}`, {
        baseURL: config.public.apiBaseUrl,
        headers: {
            Accept: `application/json`,
            Authorization: `Bearer ${authToken}`
        }
    })

    if (error.value) {
        console.error('Failed to fetch data:', error.value)
        return null
    } else {
        console.log('Fetched data:', data.value)
        agentStatus.value = data.value
        startButtonDisabled.value = false
    }
}

const fetchServerData = async (id) => {
    const { data, error } = await useFetch('/api/server/' + id, {
        baseURL: config.public.apiBaseUrl,
        headers: {
            Accept: `application/json`,
            Authorization: `Bearer ${authToken}`
        }
    })

    if (error.value) {
        console.error('Failed to fetch data:', error.value)
        return null
    } else {
        console.log('Fetched data:', data.value)
        return data.value
    }
}

onMounted(async () => {
    const chunk = localStorage.getItem('user')
    if (!chunk) {
        //no user, throw some error
        console.log('no user')
    } else {
        user.value = JSON.parse(chunk)
        if (!user.value.agents || user.value.agents.length === 0) {
            // no agents, throw some error
            console.log('no agents')
        } else {
            if (user.value.agents.length > 1) {
                // we supposed to give user option
                // to select agent
                // but for meantime
                // we pick the first agent
                agent.value = user.value.agents[0]
                const d1 = await fetchServerData(agent.value.server_id)
                serverData.value = d1
                initiateWebsocket(serverData.value)
            } else {
                agent.value = user.value.agents[0]
                const d2 = await fetchServerData(agent.value.server_id)
                serverData.value = d2
                initiateWebsocket(serverData.value)
                await getAgentQueues()
            }
        }
    }
})
</script>

<style scoped>
.qsb-etytrui {
    width: 100%;
}

.left-pane {
    background-color: #f9f9f9;
    padding: 16px;
    min-height: 100vh;
}

.right-pane {
    padding: 16px;
}

.dial-pad {
    margin-bottom: 24px;
}

.dial-button {
    margin-bottom: 8px;
}

.disposition-button {
    margin-bottom: 8px;
    display: flex;
    justify-content: space-between;
    align-items: center;
    color: #777;
    font-size: 14px;
    flex-direction: column;
    line-height: 1;
    padding: 8px 20px;
    min-height: 44px;
}

.dnc-button {
    background-color: #ffc0cb;
    /* Light pink */
    border-color: #ffc0cb;
}

.sale-button {
    background-color: #52c41a;
    /* Green for Sale */
    border-color: #52c41a;
    color: white;
}

.start-button {
    min-height: 44px;
    margin-top: 18px;
    display: flex;
    justify-content: center;
    align-items: center;
    gap: 8px;
    color: #777;
}
</style>
