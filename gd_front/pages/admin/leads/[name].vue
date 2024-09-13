<template>
    <div>
        <a-breadcrumb>
            <a-breadcrumb-item>
                <NuxtLink to="/admin">Admin</NuxtLink>
            </a-breadcrumb-item>
            <a-breadcrumb-item>
                <NuxtLink to="/admin/leads">Leads</NuxtLink>
            </a-breadcrumb-item>
            <a-breadcrumb-item>Leads in "{{ campaignName }}"</a-breadcrumb-item>
        </a-breadcrumb>
        <div class="yhn76">
            <div class="yhn76__custom-dd" ref="dropdownContainer">
                <!-- Button to toggle dropdown -->
                <a-button @click="toggleDropdown">
                    Show/Hide fields
                    <DownOutlined />
                </a-button>

                <!-- Dropdown -->
                <div v-if="fieldSelectVisible" class="dropdown">
                    <div v-for="field in fieldNamesAll" :key="field" class="dropdown-item">
                        <label>
                            <input type="checkbox" :value="field" @change="toggleSelection(field)"
                                :checked="fieldNames.includes(field)" />
                            {{ field }}</label>
                    </div>
                </div>
            </div>
            <div class="yhn76__search">
                <select v-model="requestProps.filterBy">
                    <option v-for="nf in fieldNamesAll" :key="`${nf}g4`">{{ nf }}</option>
                </select>
                <select v-model="requestProps.filterCondition">
                    <option value="=">=</option>
                    <option value=">">&gt;</option>
                    <option value="<">&lt;</option>
                    <option value=">=">&gt;=</option>
                    <option value="<=">&lt;=</option>
                    <option value="like_%t%">LIKE %&lt;Query String&gt;%</option>
                    <option value="like_t%">LIKE &lt;Query String&gt;%</option>
                    <option value="like_%t">LIKE %&lt;Query String&gt;</option>
                    <option value="!=">!=</option>
                </select>
                <input type="text" v-model="requestProps.filter" />
                <a-button type="dashed" :icon="h(SearchOutlined)" @click="fetchLeads">Search</a-button>
            </div>
        </div>
        <div class="yhn76">
            <div class="yhn76_1">
                <a-button size="30" :loading="leadsDownloading" @click="exportLeads">
                    <template #icon>
                        <DownloadOutlined />
                    </template>
                    Export
                </a-button>
                <NuxtLink :to="`/admin/leads/upload/${campaignName}`">
                    <a-button size="30">
                        <template #icon>
                            <UploadOutlined />
                        </template>
                        Import
                    </a-button>
                </NuxtLink>
                <a-button size="30">
                    <template #icon>
                        <PlusCircleOutlined />
                    </template>
                    Add
                </a-button>
            </div>
            <div class="yhn76__search has-sql"><input type="text" v-model="requestProps.sqlFilter" />
                <a-button type="dashed" :icon="h(SearchOutlined)" @click="fetchLeads">SQL Query</a-button>
            </div>
        </div>
        <div class="yhn76" v-if="leadDownloadLink">
            <div>Download CSV file: <a href="#" @click.stop.prevent="downloadFile" target="_blank">{{ leadDownloadLink }}</a></div>
        </div>
        <div class="ant-table-wrapper">
            <div class="ant-table">
                <div class="ant-table-container">
                    <div class="ant-table-content">
                        <table style="table-layout: auto;" class="lead-table">
                            <thead>
                                <tr>
                                    <th v-for="nf in fieldNames" class="ant-table-cell mft6__th">
                                        <span @click="toggleSort(nf)" class="sortable-header">{{ nf }}
                                            <UpOutlined class="sort-icon sort-icon-up"
                                                :class="{ active: isActiveSort(nf, 'asc') }"
                                                @click.stop="toggleSort(nf, 'asc')" />
                                            <DownOutlined class="sort-icon sort-icon-down"
                                                :class="{ active: isActiveSort(nf, 'desc') }"
                                                @click.stop="toggleSort(nf, 'desc')" />
                                        </span>
                                    </th>
                                </tr>
                            </thead>
                            <tbody>
                                <tr v-for="record in dataSource.data">
                                    <td v-for="nf in fieldNames">{{ record[nf] }}</td>
                                </tr>
                            </tbody>
                        </table>
                    </div>
                </div>
            </div>
            <a-pagination :current="pagination.current" :pageSize="pagination.per_page" :total="pagination.total" @showSizeChange="handlePageSizeChange"
                @change="fetchLeads" />
        </div>
    </div>
</template>
<script setup>
import { SearchOutlined } from '@ant-design/icons-vue';
import { useRoute } from '#app';
definePageMeta({
    layout: 'admin',
    middleware: 'auth-admin',
})
const config = useRuntimeConfig()
const authToken = useCookie('auth_token').value
const route = useRoute();
const campaignName = route.params.name; // Access the dynamic route parameter
const fieldsToRemove = ['subdispo', 'tzl', 'tzh', 'wdayl', 'wdayh', 'satl', 'sath', 'sunl', 'sunh', 'holl', 'holh'];
const fieldNames = ref([])
const fieldNamesAll = ref([])
const fieldSelectVisible = ref(false)
const dropdownContainer = ref(null);
const dataSource = ref({
    data: []
})
const requestProps = ref({
    perPage: 30,
    orderBy: 'id',
    order: 'desc',
    filterBy: 'id',
    filter: '',
    filterCondition: '=',
    sqlFilter: '',
    export: 0
})
const leadsDownloading = ref(false)
const leadDownloadLink = ref(null)
const pagination = computed(() => dataSource.value)
const isActiveSort = (field, direction) => {
    return requestProps.value.orderBy === field && requestProps.value.order === direction;
};
const exportLeads = async () => {
    leadsDownloading.value = true
    requestProps.value.export = 1
    await fetchLeads()
    leadsDownloading.value = false
    requestProps.value.export = 0
}

const handlePageSizeChange = (currentPage, newPageSize) => {
  requestProps.value.perPage = newPageSize; // Update perPage in requestProps
  fetchLeads(); // Fetch leads with new page size
};

const toggleSort = (field, direction) => {
    if (requestProps.value.orderBy === field) {
        // If already sorting by this field, toggle between asc/desc
        requestProps.value.order = requestProps.value.order === 'asc' ? 'desc' : 'asc';
    } else {
        requestProps.value.orderBy = field;
        requestProps.value.order = 'asc';
    }
    fetchLeads(1)
};
const fetchLeads = async (page = 1) => {
    leadDownloadLink.value = null
    const params = new URLSearchParams({
        campaign: campaignName,
        page: page,
        ...requestProps.value,
    });
    const url = `/api/admin/leads?${params.toString()}`;
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
        if (requestProps.value.export > 0) {
            console.log(data.value)
            leadDownloadLink.value = data.value.download_url
            message.success(data.value.message)
        } else {
            dataSource.value = data.value.leads
            fieldNamesAll.value = Object.keys(data.value?.leads?.data[0]);
            fieldNames.value = fieldNamesAll.value.filter(field => !fieldsToRemove.includes(field));
        }
        return
    }
}

const downloadFile = async () => {
    if(!leadDownloadLink.value) {
        message.error('No download lead found')
        return
    }
    const fileName = leadDownloadLink.value.split('/').pop();
    const response = await fetch(leadDownloadLink.value, {
        method: 'GET',
        headers: {
            'Authorization': `Bearer ${authToken}`, // Attach the Bearer token
        },
    });

    if (response.status === 200) {
    const blob = await response.blob();
    const url = window.URL.createObjectURL(blob);
    const link = document.createElement('a');
    link.href = url;
    link.setAttribute('download', fileName); // Set file name
    document.body.appendChild(link);
    link.click();
    document.body.removeChild(link);
  } else {
    message.error('Error downloading file:', response.statusText);
  }
};

const toggleDropdown = () => {
    fieldSelectVisible.value = !fieldSelectVisible.value;
    if (fieldSelectVisible.value) {
        document.addEventListener('click', handleOutsideClick);
    } else {
        document.removeEventListener('click', handleOutsideClick);
    }
};

// Toggle selection of a field
const toggleSelection = (field) => {
    if (fieldNames.value.includes(field)) {
        fieldNames.value = fieldNames.value.filter(f => f !== field);
    } else {
        fieldNames.value.push(field);
    }
};

// Close dropdown on outside click
const handleOutsideClick = (event) => {
    const dropdown = dropdownContainer.value.querySelector('.dropdown');
    if (dropdown && !dropdown.contains(event.target)) {
        //fieldSelectVisible.value = false;
    }
};
onMounted(async () => {
    await fetchLeads(1)
})
onBeforeUnmount(() => {
    document.removeEventListener('click', handleOutsideClick);
});
</script>

<style lang="scss">
.mft6__th {
    position: relative;
    color: rgba(0, 0, 0, 0.88);
    font-weight: 600;
    text-align: start;
    background: #fafafa;
    border-bottom: 1px solid #f0f0f0;
    transition: background 0.2s ease;
    padding: 16px 16px;
    overflow-wrap: break-word;

    .sortable-header {
        cursor: pointer;
        width: 100%;
        display: block;
        text-align: center;
        padding-right: 20px;
        position: relative;
    }

    .sort-icon {
        position: absolute;
        font-size: 12px;
        color: #bfbfbf;
        /* Grayed out when not active */
        transition: color 0.3s;
        right: 0;
        top: 30%;
    }

    .sort-icon-up {
        transform: translateY(-51%);
    }

    .sort-icon.active {
        color: #1890ff;
        /* Highlighted when active */
    }
}

.lead-table {

    td,
    th {
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
}

.yhn76 {
    display: flex;
    justify-content: space-between;
    align-items: center;
    padding: 0;
    gap: 40px;

    &:first-of-type {
        padding-top: 20px !important;
    }

    &:not(:first-of-type) {
        padding-top: 0 !important;
    }

    &:not(:last-of-type) {
        padding-bottom: 6px !important;
    }

    &:last-of-type {
        margin-bottom: 20px !important;
    }

    .has-sql {
        flex: 1;

        input[type=text] {
            width: 100%;
        }
    }

    .yhn76_1 {
        display: flex;
        gap: 3px;
        align-items: center;

    }

    .yhn76__search {
        display: flex;
        gap: 3px;
        align-items: center;

        input,
        select {
            height: 30px;
        }
    }
}

.yhn76__custom-dd {
    position: relative;

    .dropdown {
        position: absolute;
        background-color: white;
        border: 1px solid #ccc;
        border-radius: 4px;
        padding: 10px;
        min-width: 600px;
        display: grid;
        grid-template-columns: repeat(3, 1fr);
        top: 34px;
        z-index: 10;
        left: 0;
    }

    .dropdown-item {
        margin: 5px 0;
    }

    .dropdown-item input {
        margin-right: 8px;
    }
}
</style>