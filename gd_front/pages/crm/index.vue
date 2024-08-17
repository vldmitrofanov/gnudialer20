<template>
    <a-row>
        <!-- Left Pane: Dial Pad and Disposition Buttons -->
        <a-col :span="5" class="left-pane">
            <a-card title="Disposition" class="disposition">
                <a-col :span="24">
                    <span>{{ `${connected?'Connected':'Disconnected'}` }}</span>    
                </a-col>
                <a-row :gutter="{ xs: 8,sm:8,md:8,lg:8}">
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
                        <a-button block class="start-button" :disabled="startButtonDisabled">
                            <template #icon  v-if="running">Pause <pause-circle-filled /></template>
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
                        <a-row gutter="24">
                            <a-col :span="8">
                                <a-form-item label="Lead Name">
                                    <a-input placeholder="Enter lead name" />
                                </a-form-item>
                            </a-col>
                            <a-col :span="8">
                                <a-form-item label="Phone Number">
                                    <a-input placeholder="Enter phone number" />
                                </a-form-item>
                            </a-col>
                            <a-col :span="8">
                                <a-form-item label="Email">
                                    <a-input placeholder="Enter email address" />
                                </a-form-item>
                            </a-col>
                        </a-row>
                        <a-row gutter="24">
                            <a-col :span="8">
                                <a-form-item label="Address">
                                    <a-input placeholder="Enter address" />
                                </a-form-item>
                            </a-col>
                            <a-col :span="8">
                                <a-form-item label="City">
                                    <a-input placeholder="Enter city" />
                                </a-form-item>
                            </a-col>
                            <a-col :span="8">
                                <a-form-item label="Zip Code">
                                    <a-input placeholder="Enter zip code" />
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
                </a-tab-pane>
            </a-tabs>
        </a-col>
    </a-row>
</template>

<script setup>
import { useFetch, useCookie, useRuntimeConfig } from '#app'
const config = useRuntimeConfig()
const authToken = useCookie('auth_token').value
const name = 'DialerCRM'
const serverData = ref(null)
const allButtonsDisabled = ref(true)
const running = ref(false)
const user = ref(null)
const agent = ref(null)
const connected = ref(false)

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
}

const fetchServerData = async(id) => {
    const { data, error } = await useFetch('/api/server/' + id, {
    baseURL: config.public.apiBaseUrl,
    headers: {
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

onMounted(async()=>{
    const chunk = localStorage.getItem('user')
    if(!chunk) {
        //no user, throw some error
        console.log('no user')
    } else {
        user.value = JSON.parse(chunk)
        if(!user.value.agents || user.value.agents.length === 0) {
            // no agents, throw some error
            console.log('no agents')
        } else {
            if(user.value.agents.length>1) {
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
            }
        }
    }
})
</script>

<style scoped>
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
.start-button{
    min-height:  44px;
    margin-top: 18px;
    display: flex;
    justify-content: center;
    align-items: center;
    gap:8px;
    color: #777;
}
</style>
