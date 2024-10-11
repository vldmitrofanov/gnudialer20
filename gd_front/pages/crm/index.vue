<template>
    <a-row>
        <!-- Left Pane: Dial Pad and Disposition Buttons -->
        <a-col :span="5" class="left-pane">
            <a-card :title="null" class="disposition">
                <div class="agent-status-wrapper">
                    <DatabaseOutlined :style="{ color: connected ? 'green' : 'lightgray' }" />

                    <span class="agent-status-text">Agent Status: <span :style="{
                        color: parseInt(agentStatus?.status?.online)
                            >
                            0 ? 'green' : '#999'
                    }">{{ `${parseInt(agentStatus?.status?.pause) === 1 ? 'Paused' :
                        agentStatus?.status?.online
                            ==
                            1 ? 'Active' : 'Inactive'}` }}</span></span>
                </div>
                <a-row :gutter="{ xs: 8, sm: 8, md: 8, lg: 8 }">
                    <a-col :span="12">
                        <a-button block class="disposition-button" :disabled="allButtonsDisabled"
                            @click=handleDisposition(0)>
                            Callback <small>0</small>
                        </a-button>
                    </a-col>
                    <a-col :span="12">
                        <a-button block class="disposition-button" :disabled="allButtonsDisabled"
                            @click=handleDisposition(1)>
                            Fresh <small>1</small>
                        </a-button>
                    </a-col>
                    <a-col :span="12">
                        <a-button block class="disposition-button" :disabled="allButtonsDisabled"
                            @click=handleDisposition(2)>
                            NoAns <small>2</small>
                        </a-button>
                    </a-col>
                    <a-col :span="12">
                        <a-button block class="disposition-button" :disabled="allButtonsDisabled"
                            @click=handleDisposition(3)>
                            AnsMach <small>3</small>
                        </a-button>
                    </a-col>
                    <a-col :span="12">
                        <a-button block class="disposition-button" :disabled="allButtonsDisabled"
                            @click=handleDisposition(4)>
                            Busy <small>4</small>
                        </a-button>
                    </a-col>
                    <a-col :span="12">
                        <a-button block class="disposition-button" :disabled="allButtonsDisabled"
                            @click=handleDisposition(5)>
                            FBusy <small>5</small>
                        </a-button>
                    </a-col>
                    <a-col :span="12">
                        <a-button block class="disposition-button" :disabled="allButtonsDisabled"
                            @click=handleDisposition(6)>
                            Fax <small>6</small>
                        </a-button>
                    </a-col>
                    <a-col :span="12">
                        <a-button block class="disposition-button" :disabled="allButtonsDisabled"
                            @click=handleDisposition(7)>
                            Disco <small>7</small>
                        </a-button>
                    </a-col>
                    <a-col :span="12">
                        <a-button block class="disposition-button dnc-button" :disabled="allButtonsDisabled"
                            @click=handleDisposition(8)>
                            DNC <small>8</small>
                        </a-button>
                    </a-col>
                    <a-col :span="12">
                        <a-button block class="disposition-button" :disabled="allButtonsDisabled"
                            @click=handleDisposition(9)>
                            Invalid <small>9</small>
                        </a-button>
                    </a-col>
                    <a-col :span="12">
                        <a-button block class="disposition-button" :disabled="allButtonsDisabled"
                            @click=handleDisposition(10)>
                            Other <small>10</small>
                        </a-button>
                    </a-col>
                    <a-col :span="12">
                        <a-button block class="disposition-button sale-button" :disabled="allButtonsDisabled"
                            @click=handleDisposition(12)>
                            Sale <small>12</small>
                        </a-button>
                    </a-col>
                    <a-col :span="12">
                        <a-button block class="disposition-button" :disabled="allButtonsDisabled"
                            @click=handleDisposition(11)>
                            NoSale <small>11</small>
                        </a-button>
                    </a-col>
                    <a-col :span="12" v-if="customerChannel">
                        <a-button block class="hold-button" :disabled="allButtonsDisabled" @click=toggleHold>
                            {{ isChannel2OnHold ? 'Unhold' : 'Hold' }}
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
            <a-card :title="null"
                v-if="queue?.settings?.find(item => item.parameter === 'allow3way' && item.value === '1')">
                <a-col :span="24">3Way transfer:</a-col>
                <div v-if="!threeWayStatus">
                    <a-col :span="12" v-for="threeWay in queue?.campaign?.three_ways">
                        <a-button block class="three-way-button" :disabled="allButtonsDisabled"
                            @click=handle3WayTransfer(threeWay.id)>
                            {{ threeWay.name }}
                        </a-button>
                    </a-col>
                </div>
                <div v-else>
                    <a-col :span="12"><a-button>Leave 3way</a-button></a-col>
                    <a-col :span="12"><a-button>Hangup 3way Line</a-button></a-col>
                </div>
            </a-card>
        </a-col>

        <!-- Right Pane: Form with Tabs -->
        <a-col :span="19" class="right-pane">
            <a-tabs default-active-key="1">
                <a-tab-pane key="1" tab="Main">
                    <div class="sticky-form">
                        <a-form layout="inline">
                            <a-form-item>
                                <a-input-group compact>
                                    <a-select v-model:value="searchBy" style="width: 100px;" :disabled="running">
                                        <a-select-option value="id">ID</a-select-option>
                                        <a-select-option value="phone">Phone</a-select-option>
                                    </a-select>
                                    <a-input v-model:value="searchTerm" :placeholder="`Enter ${searchBy}`"
                                        style="width: 200px;" @keypress.enter="handleSearch" :disabled="running" />
                                    <a-button type="primary" @click="handleSearch" :disabled="running">
                                        <SearchOutlined /> <!-- Use the imported icon -->
                                    </a-button>
                                </a-input-group>
                            </a-form-item>
                        </a-form>
                    </div>
                    <LeadForm ref="leadFormRef" v-if="lead" :lead="lead" :schema="leadSchema"
                        @onSave="handleLeadSave" />
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
            <div v-if="DEBUG" class="debug-panel">Agent channel: {{ agentChannel?.name }} | customerChannel {{ customerChannel.name }} |
                onHold: {{ isChannel2OnHold }} | bridge: {{ bridge?.id }} | leadCampaign {{ leadCampaign }}
            </div>
        </a-col>
    </a-row>
    <a-modal :open="isCBModalVisible" title="Select Date and Time" @ok="handleCBDateSelected"
        @cancel="isCBModalVisible = false">
        <h3>Pleas select CB date</h3>
        <a-date-picker v-model:value="cb_datetime" show-time :default-value="defaultDate" format="YYYY-MM-DD hh:mm A"
            style="width: 100%;" />
    </a-modal>
</template>

<script setup>
import { message } from 'ant-design-vue';
import { useFetch, useCookie, useRuntimeConfig } from '#app'
import { SearchOutlined } from '@ant-design/icons-vue';
const DEBUG = true
const config = useRuntimeConfig()
const authToken = useCookie('auth_token').value
const isCBModalVisible = ref(false)
const serverData = ref(null)
const allButtonsDisabled = ref(true)
const running = computed(() => parseInt(agentStatus.value?.online) === 1 && parseInt(agentStatus.value?.pause) !== 0 )
const user = ref(null)
const agent = ref(null)
const connected = ref(false)
const startButtonDisabled = ref(true)
const agentStatus = ref(null)
const queues = ref([])
const queue = ref(null)
const queueButtonDisabled = ref(true)
const loggedIn = ref(false)
const agentChannel = ref(null)
const customerChannel = ref(null)
const isChannel2OnHold = ref(false)
const lead = ref(null)
const leadSchema = ref(null)
const callerId = ref(null)
const leadCampaign = ref(null)
const onCall = ref(false)
const searchBy = ref('phone')
const searchTerm = ref('')
const disposition = ref(10000)
const leadFormRef = ref(null)
const pauseAfterCall = ref(false)
const cb_datetime = ref(null)
const defaultDate = ref(new Date());
const bridge = ref(null)
const threeWayStatus = ref(null)

defaultDate.value.setDate(defaultDate.value.getDate() + 1);
defaultDate.value.setHours(12, 0, 0, 0); // Noon (12:00)

const toggleHold = async () => {
    if (!customerChannel.value) {
        message.error("No channel found to put on hold")
        return
    }
    let actionCommand = ''
    if (!isChannel2OnHold.value) {
        actionCommand = "Action: Redirect\r\n";
        actionCommand += `Channel: ${customerChannel.value?.name}\r\n`;
        actionCommand += "Context: gnudialer_hold\r\n";
        actionCommand += "Exten: s\r\n";
        actionCommand += "Priority: 1\r\n";
    } else if(!threeWayStatus.value) {
        actionCommand = "Action: Redirect\r\n";
        actionCommand += `Channel: ${customerChannel.value?.name}\r\n`;  // Customer channel currently on hold
        actionCommand += "Context: gnudialer_bridge\r\n";      // Context to handle the bridge
        actionCommand += "Exten: s\r\n";                     // Extension to handle bridging
        actionCommand += "Priority: 1\r\n";                  // Priority in the dialplan
        actionCommand += `Setvar: BRIDGE_ID=${bridge.value.id}\r\n`;
    }
    const { data, error } = await useFetch(`/api/asterisk/custom/user-action`, {
        method: 'POST',
        baseURL: config.public.apiBaseUrl,
        headers: {
            Accept: `application/json`,
            Authorization: `Bearer ${authToken}`
        },
        body: {
            server_id: serverData.value?.id,
            action: actionCommand
        }
    })
    if (error.value) {
        console.error('Error during hangup: ', error.value)
        message.error(error.value);
        return null
    } else {
        isChannel2OnHold.value =! isChannel2OnHold.value
    }

}
const handleCBDateSelected = () => {
    if ((!cb_datetime.value || cb_datetime.value == '')) {
        message.error("Please select CB date")
        return
    }
    triggerFormSubmit()
    isCBModalVisible.value = false
}
const formatDateToSQL = (date) => {
    if (!date) return null;

    const padZero = (num) => (num < 10 ? '0' + num : num);

    const year = date.getFullYear();
    const month = padZero(date.getMonth() + 1); // Months are zero-based
    const day = padZero(date.getDate());
    const hours = padZero(date.getHours());
    const minutes = padZero(date.getMinutes());
    const seconds = padZero(date.getSeconds());

    return `${year}-${month}-${day} ${hours}:${minutes}:${seconds}`;
};

const handleSearch = async () => {
    if (!queue.value) {
        message.error('No campaign selected')
        return
    }
    if (searchTerm.value == '') {
        message.error('Please input search data')
        return
    }
    const campaign = queue.value?.campaign?.code
    let phone = '', leadId = '';
    if (searchBy.value == 'phone') {
        phone = searchTerm.value
    } else if (searchBy.value == 'id') {
        leadId = searchTerm.value
    }
    const { data, error } = await useFetch(`/api/leads/search?campaign=${campaign}&lead_id=${leadId}&phone=${phone}`, {
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
        leadSchema.value = data.value?.schema
        leadCampaign.value = campaign
    }
}

const initiateWebsocket = (server) => {
    if (DEBUG) {
        console.log('initiateWebsocket(server)', server?.data.ws.proto)
    }
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
                if (data.value?.includes(`PJSIP/${agent.value?.id}-`)) {
                    onBringePeer(data)
                    customerChannel.value = {name:data.channel.name}
                } 
                break;
        }
        switch (data.dialplan_app) {
            case 'AddQueueMember':
                console.log('AddQueueMember event:', data);
                if (data.channel?.name?.includes(`PJSIP/${agent.value?.id}-`)) {
                    const res = data.dialplan_app_data?.split(",");
                    if (res && res.length > 0) {
                        const queueCode = res[0]
                        queue.value = queues.value.find(v => v?.campaign?.code == queueCode)
                        getAgentStatus()
                    }
                }
                break;
        }
        switch (data.type) {
            case "ChannelDestroyed":
                if (data.channel?.name?.includes(`PJSIP/${agent.value?.id}-`)) {
                    queue.value = null;
                    agentStatus.value = null;
                    bridge.value = null
                    threeWayStatus.value = null
                }
                break;
            case "ChannelEnteredBridge":
                if (data.channel?.name?.includes(`PJSIP/${agent.value?.id}-`)) {
                    if (DEBUG) {
                        console.log('ENTERED_BRIDGE', data.bridge)
                    }
                    if (data.bridge) {
                        customerChannel.value
                        bridge.value = {name:data.bridge.id}
                    }
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

const handleDisposition = async (dispo) => {
    disposition.value = dispo
    if (DEBUG) {
        console.log(dispo)
    }
    if (parseInt(dispo) === 0 && (!cb_datetime.value || cb_datetime.value == '')) {
        isCBModalVisible.value = true
        return
    }
    if (customerChannel.value) {
        hangup()
    }
    triggerFormSubmit()
    gdialDispo(dispo)
    if (!pauseAfterCall.value) {
        setOnWait()
    } else {
        togglePause()
    }
}

const gdialDispo = async (dispo) => {
    const transf = 'TRANSFER'
    let actionCommand = "Action: UserEvent\r\nUserEvent: UserEventDispo\r\n"
    actionCommand += `Header1: Agent: ${agent.value.id}\r\n`
    actionCommand += `Header2: Dispo: ${dispo}\r\n`
    actionCommand += `Header3: Transfer: ${transf}\r\n`
    actionCommand += `Header4: Campaign: ${queue.value?.campaign?.code}\r\n`
    actionCommand += `Header5: Leadid: ${lead.value?.id}\r\n`
    actionCommand += `Header6: Channel: ${customerChannel.value?.name}\r\n`
    const { data, error } = await useFetch(`/api/asterisk/custom/user-action`, {
        method: 'POST',
        baseURL: config.public.apiBaseUrl,
        headers: {
            Accept: `application/json`,
            Authorization: `Bearer ${authToken}`
        },
        body: {
            server_id: serverData.value?.id,
            action: actionCommand
        }
    })
    if (error.value) {
        console.error('Error during hangup: ', error.value)
        message.error(error.value);
        return null
    } else {
        customerChannel.value = null
    }
}

const setOnWait = async () => {
    const { data, error } = await useFetch(`/api/asterisk/custom/user-action`, {
        method: 'POST',
        baseURL: config.public.apiBaseUrl,
        headers: {
            Accept: `application/json`,
            Authorization: `Bearer ${authToken}`
        },
        body: {
            server_id: serverData.value?.id,
            action: "Action: UserEvent\r\nUserEvent: SetOnWait\r\nHeader: Agent: " + agent.value.id
        }
    })
    if (error.value) {
        console.error('Error during hangup: ', error.value)
        message.error(error.value);
        return null
    }
}

const hangup = async () => {
    const { data, error } = await useFetch(`/api/asterisk/call/hangup`, {
        method: 'POST',
        baseURL: config.public.apiBaseUrl,
        headers: {
            Accept: `application/json`,
            Authorization: `Bearer ${authToken}`
        },
        body: {
            server_id: serverData.value?.id,
            channel: customerChannel.value?.name
        }
    })
    if (error.value) {
        console.error('Error during hangup: ', error.value)
        message.error(error.value);
        return null
    }
}

const triggerFormSubmit = () => {
    if (leadFormRef.value) {
        leadFormRef.value.handleSubmit();
    }
};

const handleLeadSave = async (updatedLead) => {
    const { data, error } = await useFetch(`/api/leads`, {
        method: 'PUT',
        baseURL: config.public.apiBaseUrl,
        headers: {
            Accept: `application/json`,
            Authorization: `Bearer ${authToken}`
        },
        body: {
            campaign: leadCampaign.value,
            lead: updatedLead,
            agent: agent.value.id,
            disposition: disposition.value,
            cb_datetime: cb_datetime.value ? formatDateToSQL(cb_datetime.value) : ''
        }
    })
    cb_datetime.value = null
    if (error.value) {
        console.error('Failed to fetch data:', error.value)
        message.error(error.value);
        return null
    } else {
        //console.log('Fetched data:', data.value)
        lead.value = null
        allButtonsDisabled.value = true
    }
}

const onBringePeer = (data) => {
    if (data.channel) {
        agentChannel.value = {name: data.value}
        const str = data.channel.connected.name
        callerId.value = data.channel.caller
        const trimmedStr = str.slice(1, -1);  // Removes the first and last characters

        // Step 2: Split the string by "-"
        const parts = trimmedStr.split('-');

        // Retrieve the values
        const campaign = parts[0];  // "test1"
        const leadId = parts[1]; // "111"
        getLead(campaign, leadId)
        allButtonsDisabled.value = false
        onCall.value = true
    }
}

const getLead = async (campaign, leadId) => {
    if (!campaign || !leadId) {
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
        leadSchema.value = data.value?.schema
        leadCampaign.value = campaign
    }
}

const handleSelectQueue = (ql) => {
    if (DEBUG) {
        console.log(ql)
    }
    queue.value = ql
    if (queue.value) {
        getAgentStatus()
    }
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
        /*
        const resObj = data.value?.data?.QueueMemberPause[0]
        if (DEBUG) {
            console.log('resObj', resObj)
        }
        if (parseInt(resObj?.Status) === 2) {
            agentStatus.value = {
                status: {
                    Status: "2",
                    Paused: resObj?.Paused,
                    InCall: resObj?.Incall,
                }
            }
        }
        if (parseInt(resObj?.Paused) === 0) {
            setOnWait()
            pauseAfterCall.value = false
        } else {
            pauseAfterCall.value = true
        }*/
        agentStatus.value = data.value?.data
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
        if (DEBUG) {
            console.log('Fetched data:', data.value)
        }
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
        if (DEBUG) {
            console.log('Fetched data:', data.value)
        }
        agentStatus.value = data.value?.data
        agentChannel.value = {name: data.value?.data?.agent_channel,id: data.value?.data?.agent_channel_id}
        bridge.value = {id:data.value?.data?.bridge_id,name: data.value?.data?.id}
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
        if (DEBUG) {
            console.log('Fetched data:', data.value)
        }
        return data.value
    }
}

const handle3WayTransfer = async (threeWayId) => {
    console.log(bridgeId.value, threeWayId)
    const serverId = serverData.value.id
    const { data, error, pending, onError } = await useFetch(`/api/asterisk/bridge-3way`, {
        method: 'POST',
        baseURL: config.public.apiBaseUrl,
        headers: {
            Accept: `application/json`,
            Authorization: `Bearer ${authToken}`
        },
        body: {
            server_id: serverId,
            three_way_id: threeWayId,
            bridge: bridgeId.value,
        }
    })
    onError((fetchError) => {
        message.error("Error during 3-way transfer:", fetchError);
        return
    });
    if (!error.value) {
        threeWayStatus.value = data
        console.log('3__Way__Response', data)
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
                initiateWebsocket(d2)
                await getAgentQueues()
            }
        }
    }
})
</script>

<style scoped lang="scss">
.agent-status-wrapper {
    display: grid;
    grid-template-columns: 20px 1fr;
    justify-items: start;
    align-items: center;
    margin-bottom: 10px;
    gap: 10px;

    .agent-status-text {
        font-size: 12px;
        color: #999;
    }
}

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
    padding-bottom: 60px;
    position: relative;

    .debug-panel {
        position: absolute;
        bottom: 0;
        left: 0;
        width: 100%;
        border-top: #777 1px solid;
    }
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

.hold-button {
    background-color: cadetblue;
    border-color: cadetblue;
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

.sticky-form {
    position: relative;
    display: flex;
    justify-content: flex-end;
    width: 100%;
    background-color: white;
    padding: 0 0 16px;
    border-bottom: 1px #ebebeb solid;
    margin-bottom: 20px;

    //border-radius: 8px;
    /*box-shadow: 0 2px 8px rgba(0, 0, 0, 0.15);*/
    .ant-form-item {
        margin-inline-end: 0 !important;
    }
}
</style>
