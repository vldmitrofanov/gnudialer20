<template>
    <a-row>
        <a-col>
            <h3>Copy Agent</h3>
        </a-col>
        <a-col>
            <a-form-item class="ert5" label="Select Server">
                <select v-model="copyAgent.server_id">
                    <option :value="s.id" v-for="s in servers">{{ s.name }}</option>
                </select>
            </a-form-item>
            <a-form-item class="ert5" label="Select an agent to copy from">
                <select v-model="copyAgent.copy_agent_id">
                    <option :value="a.id" v-for="a in agents">{{ a.name }}</option>
                </select>
            </a-form-item>
            <a-form-item class="ert5" label="First Name">
                <input v-model="copyAgent.first_name" :placeholder="`First Name`" style="width: 160px;"
                    :disabled="formLoading" />
            </a-form-item>
            <a-form-item class="ert5" label="Last Name">
                <input v-model="copyAgent.last_name" :placeholder="`Last Name`" style="width: 160px;"
                    :disabled="formLoading" />
            </a-form-item>
            <a-form-item class="ert5" label="Email">
                <input v-model="copyAgent.email" :placeholder="`Email`" style="width: 160px;" :disabled="formLoading" />
            </a-form-item>
            <a-button type="primary" ghost @click="handleCopyAgent()" :disabled="formLoading" :loading="formLoading">
                <SaveOutlined v-if="!formLoading" /> Save
            </a-button>
        </a-col>
    </a-row>
</template>
<script setup>
const {agents, servers, serverId } = defineProps(
    {
        agents: {
            require: true,
            type: Array
        },
        servers: {
            require: true,
            type: Array
        },
        serverId: {
            require:false
        }
    }
)
defineEmits([
    'updated'
])
const formLoading = ref(false)
const copyAgent = ref({
    server_id: null,
    copy_agent_id: null,
    first_name: '',
    last_name: '',
    email:''
})
const handleCopyAgent = async() => {
    const url = `/api/admin/agents`;
    const { data, error } = await useFetch(url, {
        method: 'POST',
        baseURL: config.public.apiBaseUrl,
        headers: {
            Accept: `application/json`,
            Authorization: `Bearer ${authToken}`
        },
        body: {
            ...copyAgent.value
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
        $emit("updated")
    }
}
</script>