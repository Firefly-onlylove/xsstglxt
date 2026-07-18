<template>
  <el-dialog v-model="visible" :title="title" :width="width" destroy-on-close @closed="$emit('closed')">
    <el-form ref="formRef" :model="formData" :rules="rules" label-width="100px">
      <template v-for="field in fields" :key="field.prop">
        <el-form-item :label="field.label" :prop="field.prop" v-if="!field.hidden">
          <!-- select -->
          <el-select v-if="field.type === 'select'" v-model="formData[field.prop]"
            :placeholder="field.placeholder" clearable style="width:100%">
            <el-option v-for="opt in field.options" :key="opt.value" :label="opt.label" :value="opt.value" />
          </el-select>
          <!-- textarea -->
          <el-input v-else-if="field.type === 'textarea'" v-model="formData[field.prop]"
            type="textarea" :rows="3" :placeholder="field.placeholder" />
          <!-- datetime -->
          <el-date-picker v-else-if="field.type === 'datetime'" v-model="formData[field.prop]"
            type="datetime" value-format="YYYY-MM-DD HH:mm:ss" :placeholder="field.placeholder" style="width:100%" />
          <!-- number -->
          <el-input-number v-else-if="field.type === 'number'" v-model="formData[field.prop]"
            :min="field.min ?? 0" :max="field.max" :placeholder="field.placeholder" style="width:100%" />
          <!-- password -->
          <el-input v-else-if="field.type === 'password'" v-model="formData[field.prop]"
            type="password" show-password :placeholder="field.placeholder" />
          <!-- default input -->
          <el-input v-else v-model="formData[field.prop]" :placeholder="field.placeholder" />
        </el-form-item>
      </template>
      <slot name="extra" :form="formData" />
    </el-form>
    <template #footer>
      <el-button @click="visible = false">取消</el-button>
      <el-button type="primary" :loading="loading" @click="submit">确认</el-button>
    </template>
  </el-dialog>
</template>

<script setup>
import { ref, watch } from 'vue'

const props = defineProps({
  title:    { type: String,  default: '操作' },
  fields:   { type: Array,   default: () => [] },
  rules:    { type: Object,  default: () => ({}) },
  width:    { type: String,  default: '500px' },
  loading:  { type: Boolean, default: false },
  initData: { type: Object,  default: () => ({}) }
})

const emit = defineEmits(['submit', 'closed'])
const visible = ref(false)
const formRef  = ref()
const formData = ref({})

watch(() => props.initData, (v) => { formData.value = { ...v } }, { deep: true })

function open(data) {
  formData.value = data ? { ...data } : { ...props.initData }
  visible.value = true
}

async function submit() {
  await formRef.value.validate()
  emit('submit', { ...formData.value })
}

function close() { visible.value = false }

defineExpose({ open, close })
</script>
