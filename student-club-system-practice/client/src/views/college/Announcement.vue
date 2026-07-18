<template>
  <div>
    <div class="page-header">
      <span class="page-title">学院公告</span>
      <el-button type="primary" @click="openCreate">+ 发布公告</el-button>
    </div>
    <DataTable :data="tableData" :columns="columns" :total="total" :loading="loading"
      @page-change="onPage">
      <template #actions="{ row }">
        <el-button link type="primary" @click="viewAnnouncement(row)">查看</el-button>
        <el-button link type="danger" @click="deleteAnnouncement(row)">删除</el-button>
      </template>
    </DataTable>
    <el-dialog v-model="createVisible" title="发布公告" width="560px">
      <el-form ref="createForm" :model="form" :rules="rules" label-width="70px">
        <el-form-item label="标题" prop="title"><el-input v-model="form.title" /></el-form-item>
        <el-form-item label="内容" prop="content"><el-input v-model="form.content" type="textarea" :rows="6" /></el-form-item>
      </el-form>
      <template #footer>
        <el-button @click="createVisible = false">取消</el-button>
        <el-button type="primary" :loading="submitting" @click="doCreate">发布</el-button>
      </template>
    </el-dialog>
    <el-dialog v-model="viewVisible" :title="current?.title" width="560px">
      <p style="color:#86909C;font-size:13px;margin-bottom:12px">{{ current?.publisher_name }} · {{ current?.created_at }}</p>
      <div style="white-space:pre-wrap;line-height:1.8">{{ current?.content }}</div>
    </el-dialog>
  </div>
</template>

<script setup>
import { ref, onMounted } from 'vue'
import { api } from '@/api'
import { ElMessage, ElMessageBox } from 'element-plus'
import DataTable from '@/components/DataTable.vue'

const loading = ref(false); const submitting = ref(false)
const tableData = ref([]); const total = ref(0); const page = ref(1)
const createVisible = ref(false); const viewVisible = ref(false)
const current = ref(null); const createForm = ref()
const form = ref({ title: '', content: '' })
const rules = { title: [{ required: true }], content: [{ required: true }] }
const columns = [
  { prop: 'title', label: '标题' },
  { prop: 'publisher_name', label: '发布人', width: 100 },
  { prop: 'created_at', label: '发布时间', width: 160 },
  { slot: 'actions', label: '操作', width: 120, fixed: 'right' }
]
async function loadData() {
  loading.value = true
  const res = await api.get('/api/college/announcements', { page: page.value, page_size: 10 })
  loading.value = false
  if (res.code === 0) { tableData.value = res.data.list || []; total.value = res.data.total || 0 }
}
function onPage(e) { page.value = e.page; loadData() }
function viewAnnouncement(row) { current.value = row; viewVisible.value = true }
function openCreate() { form.value = { title: '', content: '' }; createVisible.value = true }
async function doCreate() {
  await createForm.value.validate()
  submitting.value = true
  const res = await api.post('/api/college/announcements', form.value)
  submitting.value = false
  if (res.code === 0) { ElMessage.success('发布成功'); createVisible.value = false; loadData() }
}
async function deleteAnnouncement(row) {
  await ElMessageBox.confirm('确认删除该公告？', '提示', { type: 'warning' })
  const res = await api.delete('/api/college/announcements/' + row.announce_id)
  if (res.code === 0) { ElMessage.success('已删除'); loadData() }
}
onMounted(loadData)
</script>
