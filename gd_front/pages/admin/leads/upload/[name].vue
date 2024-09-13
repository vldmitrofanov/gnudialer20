<template>
    <div style="max-width: 600px;" v-if="!stats">
        <div v-if="!uploadedFileName">
            <h2>Upload File for Campaign: {{ campaignName }}</h2>

            <input type="file" @change="onFileChange" accept=".csv, .xlsx" />

            <a-button @click="uploadFile" :disabled="!file">
                Upload
            </a-button>

            <div v-if="fieldNamesOrig.length && tableFieldNamesOrig.length">
                <h3>Map Fields</h3>
                <p>Uploaded File Fields: {{ fieldNamesOrig }}</p>
                <p>Table Fields: {{ tableFieldNamesOrig }}</p>
                <!-- Implement your field mapping logic here -->
            </div>
        </div>
        <div v-else>
            <div class="field-mapper-k7y5">
                <div class="lo9567">
                    <h3>
                        <FileExcelOutlined /> In File
                    </h3>
                    <h3>
                        <DatabaseOutlined /> In Table
                    </h3>
                </div>
                <!-- Matched Fields Displayed Above -->
                <div class="matched-fields-it7">
                    <div v-for="(mapping, index) in matchedFields" :key="index" class="matched-field-it7-item">
                        <div class="hp5b">
                            <span class="l" :class="{ 'custom': mapping.fieldType === 'custom' }">{{
                                mapping.uploadedField }}</span>
                            <span class="c">➔</span>
                            <span class="r">{{ mapping.tableField }}</span>
                            <a-button @click="removeMatch(index)">
                                <DeleteOutlined />
                            </a-button>
                        </div>
                    </div>
                </div>
                <div class="dny6h-4">
                    <!-- Left Pane: Uploaded Fields Dropdown -->
                    <div class="field-pane">
                        <h3>Select Field in File</h3>
                        <select v-model="selectedUploadedField">
                            <option disabled value="">Select Uploaded Field</option>
                            <option v-for="field in uploadedFields" :key="field" :value="field">{{ field }}</option>
                            <option value="custom">+ Add Custom Field</option>
                        </select>
                        <input type="text" v-if="selectedUploadedField === 'custom'" v-model="customUploadedField"
                            placeholder="Enter custom value" />
                    </div>

                    <!-- Right Pane: Table Fields Dropdown -->
                    <div class="field-pane">
                        <h3>Select Field in Target Table</h3>
                        <select v-model="selectedTableField">
                            <option disabled value="">Select Table Field</option>
                            <option v-for="field in tableFields" :key="field" :value="field">{{ field }}</option>
                        </select>
                    </div>
                </div>
                <div style="display:flex;justify-content: flex-end;">
                    <a-button v-if="selectedUploadedField === 'custom'" @click="matchFields">Add custom</a-button>
                </div>
            </div>
            <div style="display:flex;justify-content: center"><a-button type="primary" @click="submitFields">Submit
                    <UploadOutlined />
                </a-button></div>

        </div>
    </div>
    <div v-else style="max-width: 300px;">
        <div>Imported: {{ stats.imported }}</div>
        <div>Skipped: {{ stats.skipped }}</div>
        <div>Total: {{ stats.total }}</div>
    </div>
</template>

<script setup>
import hiddenFields from '~/lib/leads/hiddenFields';
const config = useRuntimeConfig()
const authToken = useCookie('auth_token').value

definePageMeta({
    layout: 'admin', // Specify the layout here
})
const route = useRoute();
const router = useRouter()
const campaignName = route.params.name;

const file = ref(null);
const fieldNamesOrig = ref([]);
const tableFieldNamesOrig = ref([]);
const uploadedFileName = ref(null)
// Matched fields array
const matchedFields = ref([]);

// State for selected fields and custom values
const selectedUploadedField = ref('');
const selectedTableField = ref('');
const customUploadedField = ref('');
const customTableField = ref('');

const uploadedFields = ref([])
const tableFields = ref([])
const stats = ref(null)

// Handle file selection
const onFileChange = (event) => {
    const selectedFile = event.target.files[0];
    if (selectedFile) {
        file.value = selectedFile;
    }
};


// Function to upload the file using useFetch
const uploadFile = async () => {
    if (!file.value) return;

    const formData = new FormData();
    formData.append('file', file.value);
    formData.append('campaign_name', campaignName);

    const { data, error } = await useFetch(`/api/admin/leads/upload`, {
        baseURL: config.public.apiBaseUrl,
        method: 'POST',
        body: formData,
        headers: {
            ContentType: 'multipart/form-data',
            Accept: `application/json`,
            Authorization: `Bearer ${authToken}`
        },
    });

    if (error.value) {
        console.error('Error uploading file:', error.value);
        return;
    }

    if (data.value) {
        fieldNamesOrig.value = data.value.uploadedFields.filter(field => !hiddenFields.includes(field));
        tableFieldNamesOrig.value = data.value.tableFields.filter(field => !hiddenFields.includes(field));
        uploadedFileName.value = data.value.fileName

        uploadedFields.value = [...fieldNamesOrig.value]
        tableFields.value = [...tableFieldNamesOrig.value]
    }
};

const canMatch = ref(false);
watch([selectedUploadedField, selectedTableField], () => {
    canMatch.value = selectedUploadedField.value !== '' && selectedTableField.value !== '';
    if (selectedUploadedField.value !== '' && selectedTableField.value !== '') {
        matchFields()
    }
});

const matchFields = () => {
    const uploaded = selectedUploadedField.value === 'custom' ? customUploadedField.value : selectedUploadedField.value;
    const table = selectedTableField.value === 'custom' ? customTableField.value : selectedTableField.value;
    const fieldType = selectedUploadedField.value === 'custom' ? 'custom' : 'field'

    if (uploaded && table) {
        matchedFields.value.push({
            uploadedField: uploaded,
            tableField: table,
            fieldType: fieldType
        });

        // Remove the matched fields from the lists
        uploadedFields.value = uploadedFields.value.filter(field => field !== uploaded);
        tableFields.value = tableFields.value.filter(field => field !== table);

        // Reset selections
        selectedUploadedField.value = '';
        selectedTableField.value = '';
        customUploadedField.value = '';
        customTableField.value = '';
    }
};

const removeMatch = (index) => {
    const match = matchedFields.value.splice(index, 1)[0];
    if (match.uploadedField !== 'custom') uploadedFields.value.push(match.uploadedField);
    if (match.tableField !== 'custom') tableFields.value.push(match.tableField);
};
const submitFields = async () => {
    console.log(matchedFields.value)
    const isPhonePresent = matchedFields.value.some(
        (field) => field.tableField === 'phone'
    );

    if (!isPhonePresent) {
        message.error('Phone field is not present in dataset');
        return
    }
    const {response,error} = await useFetch(`/api/admin/leads/import-uploaded`, {
        baseURL: config.public.apiBaseUrl,
        method: 'POST',
        headers: {
            'Authorization': `Bearer ${authToken}`, 
        },
        body: {
            mapping: matchedFields.value,
            file_name: uploadedFileName.value,
            campaign_name: campaignName 
        }
    });
    if (error.value) {
        console.error('Failed to fetch data:', error.value)
        message.error(error.value?.message);
        return null
    } else {
        message.success(response.value?.message)
        stats.value = response.value?.stats
    }
}
</script>
<style lang="scss">
.field-mapper-k7y5 {
    max-width: 600px;
}

.hp5b {
    display: grid;
    grid-template-columns: 1fr 30px 1fr 40px;
    margin-bottom: 10px;

    .l,
    .r {
        background-color: #f2f7fe;
        padding: 8px 10px;
        border-radius: 4px;
        border: 1px solid #f2f7fe;
    }

    .l.custom {
        background-color: white;
        border: 1px solid #efefef;
    }

    button {
        margin-left: 5px;
        display: flex;
        justify-content: center;
        align-items: center;
    }

    .c {
        display: flex;
        justify-content: center;
        align-items: center;
    }
}

.matched-fields-it7 {
    margin-bottom: 20px;
}

.lo9567 {
    display: grid;
    grid-template-columns: calc(50% - 35px) calc(50%);
    column-gap: 28px;
}

.dny6h-4 {
    display: grid;
    grid-template-columns: calc(50% - 35px) calc(50%);
    column-gap: 30px;
    margin-bottom: 30px;

    select,
    input[type=text] {
        height: 32px;
        width: 100%;
    }

    select {
        margin-bottom: 10px;
    }
}
</style>