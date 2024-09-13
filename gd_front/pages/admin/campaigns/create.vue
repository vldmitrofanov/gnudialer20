<template>
    <a-breadcrumb>
        <a-breadcrumb-item>
            <NuxtLink to="/admin">Admin</NuxtLink>
        </a-breadcrumb-item>
        <a-breadcrumb-item>
            <NuxtLink to="/admin/campaigns">Campaigns</NuxtLink>
        </a-breadcrumb-item>
        <a-breadcrumb-item>Create campaign</a-breadcrumb-item>
    </a-breadcrumb>
    <div class="mc567f">
        <div class="mc567f_1">
            <h2>Create New Campaign</h2>
            <label class="t67">Campaign Name
                <input v-model="newCampaignName" type='text' placeholder="Enter new Campaign name" />
            </label>
            <label class="t67">Campaign Code (only lowercase latin letters an numbers)
                <input v-model="newCampaignCode" type='text' placeholder="Enter new Campaign Code" @input="validateInput"/>
                <p v-if="inputError" style="color: red;margin-bottom: 0;">{{ inputError }}</p>
            </label>
            <a-checkbox v-for="s in servers" :key="s.id" :value="s.id" :checked="serversSelected.includes(s.id)"
                @change="toggleSelection(s.id)">
                {{ s.name }}
            </a-checkbox>
        </div>
        <h2>Design Table Structure</h2>
        <form @submit.prevent="submitForm">
            <!-- Display existing fields -->
            <div v-for="(type, name) in fieldSet" :key="name" class="field-row-p8nre">
                <label>{{ name }}</label>
                <select v-model="fieldSet[name]" :disabled="name === 'phone'">
                    <option value="string">String</option>
                    <option value="text">Text</option>
                    <option value="int">Integer</option>
                    <option value="date-time">Date-Time</option>
                </select>
                <!-- Delete button for fields (except for phone) -->
                <a-button v-if="name !== 'phone'" @click.prevent="removeField(name)">
                    <DeleteOutlined />
                </a-button>
            </div>

            <!-- Add new field -->
            <div class="new-field-l8nfr">
                <input v-model="newFieldName" type='text' placeholder="Enter new field name" />
                <select v-model="newFieldType">
                    <option value="string">String</option>
                    <option value="text">Text</option>
                    <option value="int">Integer</option>
                    <option value="date-time">Date-Time</option>
                </select>
                <a-button @click.prevent="addField">Add Field</a-button>
            </div>

            <!-- Submit button -->
            <a-button type="primary" htmlType="submit">Submit</a-button>
        </form>

        <!-- Modal for confirmation (optional) -->
        <a-modal v-model:open="isModalOpen" title="Form Submitted">
            <p>Form has been successfully submitted.</p>
        </a-modal>
    </div>
</template>
<script setup>
const config = useRuntimeConfig()
const authToken = useCookie('auth_token').value
const isModalOpen = ref(false)
const servers = ref([])
const serversSelected = ref([])
definePageMeta({
    layout: 'admin', // Specify the layout here
})
const fieldSet = ref({
    phone: 'string'
})
const newCampaignName = ref('')
const newCampaignCode = ref('')

const newFieldName = ref('')
const newFieldType = ref('string')

// Function to remove a field (except "phone")
const removeField = (fieldName) => {
    delete fieldSet.value[fieldName]
}
const inputError = ref('')
const validateInput = () => {
  // Allow only lowercase a-z, 0-9, and ensure it starts with a letter
  const regex = /^[a-z][a-z0-9]*$/

  if (!regex.test(newCampaignCode.value)) {
    // Remove invalid characters by replacing anything not matching the regex
    newCampaignCode.value = newCampaignCode.value.replace(/[^a-z0-9]/g, '').replace(/^[^a-z]*/, '')

    // Show error if it does not match the full regex
    inputError.value = 'The input must start with a letter and can only contain lowercase letters and numbers.'
  } else {
    inputError.value = ''
  }
}
const toggleSelection = (id) => {
  if (serversSelected.value.includes(id)) {
    // If the server ID is already selected, remove it from the array
    serversSelected.value = serversSelected.value.filter(serverId => serverId !== id)
  } else {
    // If the server ID is not selected, add it to the array
    serversSelected.value.push(id)
  }
}

// Function to add a new field
const addField = () => {
    if (newFieldName.value && !fieldSet.value[newFieldName.value]) {
        fieldSet.value[newFieldName.value] = newFieldType.value
        newFieldName.value = '' // Clear the input
        newFieldType.value = 'string' // Reset the type selection
    }
}

// Function to submit the form to the server
const submitForm = async () => {
    if(!newCampaignName.value || newCampaignName.value==''){
        message.error('Please give it a name')
        return
    }
    if(!newCampaignCode.value || newCampaignCode.value==''){
        message.error('Campaign code is required')
        return
    }
    if(!serversSelected.value.length==0){
        message.error('Please select at least one server')
        return
    }
    try {
        const { data, error } = await useFetch(`/api/admin/campaigns`, {
            baseURL: config.public.apiBaseUrl,
            method: 'POST',
            headers: {
                'Authorization': `Bearer ${authToken}`,
                'Content-Type': 'application/json',
            },
            body: {
                name: newCampaignName.value,
                code: newCampaignCode.value,
                table: fieldSet.value,
                servers: serversSelected.value
            },
        })

        if (error.value) {
            console.error('Failed to fetch data:', error.value)
            message.error(error.value?.message);
            return null
        } else {
            dataSource.value = data.value
        }
    } catch (error) {
        console.error('Error submitting the form:', error)
    }
}
onMounted(async () => {
    const { data, error } = await useFetch(`/api/admin/servers`, {
        baseURL: config.public.apiBaseUrl,
        method: 'GET',
        headers: {
            'Authorization': `Bearer ${authToken}`,
            'Content-Type': 'application/json',
        },
    })

    if (error.value) {
        console.error('Failed to fetch data:', error.value)
        message.error(error.value?.message);
        return null
    } else {
        servers.value = data.value?.data
    }
})

</script>

<style lang="scss">
.field-row-p8nre {
    margin-bottom: 10px;
    display: grid;
    grid-template-columns: 1fr 1fr 30px;
    row-gap: 8px;
    column-gap: 5px;

    label {
        display: flex;
        align-items: center;
        padding: 6px 10px;
        border-radius: 4px;
        background-color: #f2f7fe;
    }
}

.new-field-l8nfr {
    margin-top: 20px;
    display: grid;
    gap: 5px;
    grid-template-columns: 1fr 1fr 90px;
    margin-bottom: 20px;
}

.mc567f {
    max-width: 400px;
    padding-top: 20px;
    .mc567f_1 {
        display: grid;
        grid-gap: 10px;
        margin-bottom: 20px;
        ;

        label.t67 {
            display: flex;
            flex-direction: column;
        }
    }

    select {
        height: 32px;
    }
}
</style>