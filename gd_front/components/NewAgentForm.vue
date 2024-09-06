<template>
    <a-row class="cvb_">
        <a-col :span="24">
            <h3>Copy Agent</h3>
        </a-col>
        <a-col :span="24">
            <a-form-item class="cvb8" label="Select Server">
                <select v-model="copyAgent.server_id" :disabled="serverId">
                    <option :value="s.id" v-for="s in servers">{{ s.label }}</option>
                </select>
            </a-form-item>
            <a-form-item class="cvb8" label="Select an agent to copy from">
                <select v-model="copyAgent.copy_agent_id">
                    <option :value="a.key" v-for="a in agents">{{ a.name }}</option>
                </select>
            </a-form-item>
            <a-form-item class="cvb8" label="First Name">
                <input v-model="copyAgent.first_name" :placeholder="`First Name`" :disabled="formLoading" />
            </a-form-item>
            <a-form-item class="cvb8" label="Last Name">
                <input v-model="copyAgent.last_name" :placeholder="`Last Name`" :disabled="formLoading" />
            </a-form-item>
            <a-form-item class="cvb8" label="Email">
                <input v-model="copyAgent.email" :placeholder="`Email`" :disabled="formLoading" type="email"/>
            </a-form-item>
            <a-form-item class="cvb8" label="Password">
                <input v-model="copyAgent.password" :placeholder="`Password`" :disabled="formLoading"  type="password"/>
            </a-form-item>
        </a-col>
        <a-col :span="24" class="cvb_n">
            <a-button type="primary" ghost @click="handleCopyAgent" :disabled="formLoading" :loading="formLoading">
                <SaveOutlined v-if="!formLoading" /> Save
            </a-button>
        </a-col>
    </a-row>
</template>
<script setup>
const config = useRuntimeConfig()
const authToken = useCookie('auth_token').value
const props = defineProps(
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
            require: false
        }
    }
)

const { agents, servers, serverId } = toRefs(props);

defineEmits([
    'updated'
])
const formLoading = ref(false)
const copyAgent = ref({
    action: 'copy',
    server_id: null,
    copy_agent_id: null,
    first_name: '',
    last_name: '',
    password: '',
    email: ''
})
const handleCopyAgent = async () => {
    if (!copyAgent.value.server_id) {
        message.error('Please select server')
        return
    }
    if (!copyAgent.value.copy_agent_id) {
        console.log(copyAgent.value.copy_agent_id)
        message.error('Please select agent to copy from')
        return
    }
    if (!copyAgent.value.first_name) {
        message.error('First name can\'t be empty')
        return
    }
    if (!copyAgent.value.last_name) {
        message.error('Last name can\'t be empty')
        return
    }
    if (!copyAgent.value.email) {
        message.error('Email can\'t be empty')
        return
    }
    formLoading.value = true
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
        formLoading.value = false
        return null
    } else {
        message.success("Updated")
        $emit("updated")
        formLoading.value = false
        copyAgent.value = {
            server_id: null,
            copy_agent_id: null,
            first_name: '',
            last_name: '',
            email: ''
        }
    }
}
onMounted(() => {
    if(serverId.value) {
        copyAgent.value.server_id = serverId.value
    }
})
</script>
<style lang="scss">
.cvb_ {
    .cvb8 {
        label {
            width: 220px;
        }

        select,
        input {
            min-width: 100%;
        }
    }

    .ant-form-item {
        margin-bottom: 0 !important;
        padding: 8px;

        &:nth-of-type(odd) {
            background-color: #f2f7fe;
        }
    }
}

.cvb_n {
    display: flex;
    justify-content: flex-end;
    padding-top: 10px;
}
</style>