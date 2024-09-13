<template>
    <a-menu mode="inline" :default-selected-keys="['1']" :default-open-keys="['sub1']"
        style="height: 100%; border-right: 0;padding-top: 20px;" :inline-collapsed="collapsed">
        <!-- Servers Menu -->
        <a-sub-menu key="sub1" :title="collapsed ? '' : 'Servers'" :icon="h(DatabaseOutlined)">
            <a-menu-item key="1">
                <NuxtLink to="/admin/servers">List</NuxtLink>
            </a-menu-item>
            <a-menu-item key="2">Server Status</a-menu-item>
        </a-sub-menu>

        <!-- Campaigns Menu -->
        <a-sub-menu key="sub2" :title="collapsed ? '' : 'Campaigns'" :icon="h(AppstoreOutlined)">
            <a-menu-item key="3">
                <NuxtLink to="/admin/campaigns">Campaign List</NuxtLink>
            </a-menu-item>
            <a-menu-item key="13">
                <NuxtLink to="/admin/campaigns/create">CreateCampaign</NuxtLink>
            </a-menu-item>
        </a-sub-menu>

        <!-- Agents Menu -->
        <a-sub-menu key="sub3" :title="collapsed ? '' : 'Agents'" :icon="h(TeamOutlined)">
            <a-menu-item key="5">
                <NuxtLink to="/admin/agents">Agent List</NuxtLink>
            </a-menu-item>
            <a-menu-item key="6">Agent Stats</a-menu-item>
        </a-sub-menu>

        <!-- Leads Menu -->
        <a-sub-menu key="sub4" :title="collapsed ? '' : 'Leads'" :icon="h(ProfileOutlined)">
            <a-menu-item key="7">
                <NuxtLink to="/admin/leads">Lead List</NuxtLink>
            </a-menu-item>
            <a-menu-item key="8">
                <NuxtLink to="/admin/leads/import">Lead Import</NuxtLink>
            </a-menu-item>
        </a-sub-menu>

        <!-- Reports Menu -->
        <a-sub-menu key="sub5" :title="collapsed ? '' : 'Reports'" :icon="h(PieChartOutlined)">
            <a-menu-item key="9">
                <NuxtLink to="/admin/reports/sales">Sales Reports</NuxtLink>
            </a-menu-item>
            <a-menu-item key="10">
                <NuxtLink to="/admin/reports/activity">Activity Reports</NuxtLink>
            </a-menu-item>
            <a-menu-item key="10">
                <NuxtLink to="/admin/reports/campaign">Campaign Reports</NuxtLink>
            </a-menu-item>
        </a-sub-menu>
    </a-menu>
</template>
<script setup>
import { DatabaseOutlined, AppstoreOutlined, SettingOutlined, TeamOutlined, ProfileOutlined, PieChartOutlined } from '@ant-design/icons-vue';
const { collapsed } = defineProps({
    collapsed: {
        required: true
    }
})
</script>