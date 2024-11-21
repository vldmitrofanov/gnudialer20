<template>
    <a-form layout="vertical" >
        <a-row :gutter="16">
            <a-col v-for="(field, index) in schema" :key="field.name" :span="field.field_type === 'textarea'?16:8"
                :class="{ 'form-item-read-only': field.read_only }">
                <a-form-item :label="field.label" @submit.prevent>
                    <a-input v-if="field.field_type === 'input'" :disabled="parseInt(field.read_only) === 1"
                    v-model:value="localLead[field.name]" :placeholder="field.label" />
                    <a-textarea v-else-if="field.field_type === 'textarea'" :disabled="parseInt(field.read_only) === 1"
                    v-model:value="localLead[field.name]" :placeholder="field.label" />
                </a-form-item>
            </a-col>
        </a-row>
    </a-form>
</template>

<script setup>

// Props
const props = defineProps({
    lead: Object,
    schema: Array
});

// Emit function
const emit = defineEmits(['onSave']);

// Local state for lead object (to avoid direct mutation of props)
const localLead = ref({ ...props.lead });

// Watch for prop changes to update localLead
watch(() => props.lead, (newLead) => {
    localLead.value = { ...newLead };
});

const handleSubmit = () => {
    emit('onSave', localLead.value);
};

defineExpose({
  handleSubmit
});
</script>

<style scoped>
.form-item-read-only {
    opacity: 0.6;
    /* Make read-only fields appear slightly disabled */
}
</style>