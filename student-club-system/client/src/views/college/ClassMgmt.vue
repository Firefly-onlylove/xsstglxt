<template>
  <div>
    <div class="page-header"><span class="page-title">班级管理</span></div>
    <FilterBar @search="loadData" @reset="onReset">
      <el-select v-model="filters.major_id" placeholder="按专业筛选" clearable style="width:180px">
        <el-option v-for="m in majors" :key="m.major_id" :label="m.major_name" :value="m.major_id" />
      </el-select>
      <el-input v-model="filters.keyword" placeholder="班级名称" clearable style="width:160px" />
      <template #actions-extra>
        <el-button type="primary" @click="openCreate">+ 新增班级</el-button>
      </template>
    </FilterBar>
    <div style="text-align:right;margin-bottom:12px">
      <el-button type="primary" @click="openCreate">+ 新增班级</el-button>
    </div>
    <DataTable :data="tableData" :columns="columns" :total="total" :loading="loading"
      @page-change="onPage">
      <template #actions="{ row }">
        <el-button link type="primary" @click="openEdit(row)">编辑</el-button>
        <el-button link type="danger" @click="deleteClass(row)">删除</el-button>
      </template>
    </DataTable>

    <ModalForm ref="formModal" :title="editing?.class_id ? '编辑班级' : '新增班级'"
      :fields="fields" @submit="save" />
  </div>
</template>

<script setup>
import { ref, onMounted } from 'vue'
import { api } from '@/api'
import { ElMessage, ElMessageBox } from 'element-plus'
import FilterBar from '@/components/FilterBar.vue'
import DataTable from '@/components/DataTable.vue'
import ModalForm from '@/components/ModalForm.vue'

const loading = ref(false)
const tableData = ref([])
const total = ref(0)
const page = ref(1)
const majors = ref([])
const filters = ref({ major_id: null, keyword: '' })
const editing = ref(null)
const formModal = ref()

const columns = [
  { prop: 'class_name',  label: '班级名称' },
  { prop: 'major_name',  label: '所属专业', width: 160 },
  { prop: 'student_count', label: '学生数', width: 80 },
  { slot: 'actions',     label: '操作',    width: 120, fixed: 'right' }
]

const fields = computed(() => [
  { prop: 'class_name', label: '班级名称', placeholder: '如 软件工程2301班' },
  { prop: 'major_id',   label: '所属专业', type: 'select',
    options: majors.value.map(m => ({ label: m.major_name, value: m.major_id })) }
])

async function loadData() {
  loading.value = true
  const res = await api.get('/api/college/classes', { page: page.value, page_size: 10, ...filters.value })
  loading.value = false
  if (res.code === 0) { tableData.value = res.data.list || []; total.value = res.data.total || 0 }
}
function onReset() { filters.value = { major_id: null, keyword: '' }; loadData() }
function onPage(e) { page.value = e.page; loadData() }

function openCreate() { editing.value = {}; formModal.value.open({}) }
function openEdit(row) { editing.value = row; formModal.value.open({ class_name: row.class_name, major_id: row.major_id }) }

async function save(data) {
  const url = editing.value?.class_id ? '/api/college/classes/' + editing.value.class_id : '/api/college/classes'
  const method = editing.value?.class_id ? 'put' : 'post'
  const res = await api[method](url, data)
  if (res.code === 0) { ElMessage.success('保存成功'); formModal.value.close(); loadData() }
  else ElMessage.error(res.msg)
}

async function deleteClass(row) {
  await ElMessageBox.confirm('确认删除班级 ' + row.class_name + '？', '提示', { type: 'warning' })
  const res = await api.delete('/api/college/classes/' + row.class_id)
  if (res.code === 0) { ElMessage.success('已删除'); loadData() }
}

import { computed } from 'vue'
onMounted(async () => {
  const me = await api.get('/api/me')
  const cid = me.code === 0 ? me.data.college_id : null
  if (cid) {
    const res = await api.get('/api/majors', { college_id: cid })
    if (res.code === 0) majors.value = res.data.list || []
  }
  loadData()
})
</script>
