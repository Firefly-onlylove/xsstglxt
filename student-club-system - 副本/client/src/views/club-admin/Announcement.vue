<template>
  <div>
    <div class="page-header">
      <span class="page-title">社团公告</span>
      <el-button type="primary" @click="openCreate">+ 发布公告</el-button>
    </div>
    <DataTable :data="tableData" :columns="columns" :total="total" :loading="loading"
      @page-change="onPage">
      <template #actions="{ row }">
        <el-button link type="primary" @click="viewDetail(row)">查看</el-button>
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
      <div style="white-space:pre-wrap;line-height:1.8">{{ current?.content }}</div>
    </el-dialog>
  </div>
</template>

<script setup>
import { ref, onMounted, computed } from 'vue'
import { api } from '@/api'
import { ElMessage } from 'element-plus'
import DataTable from '@/components/DataTable.vue'
import { useClub } from '@/composables/useClub'

const { clubId } = useClub()
const loading = ref(false); const submitting = ref(false)
const tableData = ref([]); const total = ref(0); const page = ref(1)
const createVisible = ref(false); const viewVisible = ref(false)
const current = ref(null); const createForm = ref()
const form = ref({ title: '', content: '' })
const rules = { title: [{ required: true, message: '请填写标题' }], content: [{ required: true, message: '请填写内容' }] }
const columns = [
  { prop: 'title', label: '标题' },
  { prop: 'created_at', label: '发布时间', width: 160 },
  { slot: 'actions', label: '操作', width: 100, fixed: 'right' }
]

async function loadData() {
  if (!clubId.value) return
  loading.value = true
  const res = await api.get('/api/notifications', { type: 'club', page: page.value })
  loading.value = false
  if (res.code === 0) { tableData.value = res.data.list || res.data || []; total.value = res.data.total || 0 }
}
function onPage(e) { page.value = e.page; loadData() }
function viewDetail(row) { current.value = row; viewVisible.value = true }
function openCreate() { form.value = { title: '', content: '' }; createVisible.value = true }
async function doCreate() {
  await createForm.value.validate()
  if (!clubId.value) return
  submitting.value = true
  const res = await api.post('/api/club/' + clubId.value + '/announcements', form.value)
  submitting.value = false
  if (res.code === 0) { ElMessage.success(res.message || '发布成功'); createVisible.value = false; loadData() }
}
onMounted(loadData)
</script>
