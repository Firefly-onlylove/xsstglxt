<template>
  <div>
    <div class="page-header">
      <span class="page-title">社团审批</span>
    </div>
    <el-tabs v-model="activeTab" @tab-change="loadData">
      <el-tab-pane label="待审批" name="pending" />
      <el-tab-pane label="已审批记录" name="history" />
    </el-tabs>

    <FilterBar @search="loadData" @reset="onReset">
      <el-input v-model="filters.keyword" placeholder="社团名称" clearable style="width:200px" />
      <el-select v-model="filters.club_type" placeholder="类型" clearable style="width:130px">
        <el-option v-for="t in clubTypes" :key="t" :label="t" :value="t" />
      </el-select>
    </FilterBar>

    <DataTable :data="tableData" :columns="columns" :total="total" :loading="loading"
      @page-change="onPage">
      <template #status="{ row }">
        <el-tag :type="statusType(row.status)" size="small">{{ statusLabel(row.status) }}</el-tag>
      </template>
      <template #actions="{ row }">
        <el-button link type="primary" @click="openDetail(row)">详情</el-button>
        <template v-if="row.status === 'pending'">
          <el-button link type="success" @click="approve(row, true)">通过</el-button>
          <el-button link type="danger" @click="openReject(row)">驳回</el-button>
        </template>
      </template>
    </DataTable>

    <!-- 详情弹窗 -->
    <el-dialog v-model="detailVisible" title="社团申请详情" width="560px">
      <el-descriptions :column="2" border v-if="current">
        <el-descriptions-item label="社团名称">{{ current.name }}</el-descriptions-item>
        <el-descriptions-item label="类型">{{ current.club_type }}</el-descriptions-item>
        <el-descriptions-item label="申请人">{{ current.applicant_name }}</el-descriptions-item>
        <el-descriptions-item label="所属学院">{{ current.college_name }}</el-descriptions-item>
        <el-descriptions-item label="指导老师">{{ current.advisor }}</el-descriptions-item>
        <el-descriptions-item label="申请时间">{{ current.created_at }}</el-descriptions-item>
        <el-descriptions-item label="简介" :span="2">{{ current.description }}</el-descriptions-item>
        <el-descriptions-item label="创建理由" :span="2">{{ current.reason }}</el-descriptions-item>
      </el-descriptions>
      <template #footer v-if="current?.status === 'pending'">
        <el-button @click="detailVisible = false">关闭</el-button>
        <el-button type="success" @click="approve(current, true)">通过</el-button>
        <el-button type="danger" @click="openReject(current)">驳回</el-button>
      </template>
    </el-dialog>

    <!-- 驳回弹窗 -->
    <el-dialog v-model="rejectVisible" title="驳回申请" width="420px">
      <el-form ref="rejectForm" :model="rejectData" :rules="{ reason: [{ required: true }] }">
        <el-form-item label="驳回理由" prop="reason">
          <el-input v-model="rejectData.reason" type="textarea" :rows="3" />
        </el-form-item>
      </el-form>
      <template #footer>
        <el-button @click="rejectVisible = false">取消</el-button>
        <el-button type="danger" :loading="submitting" @click="doReject">确认驳回</el-button>
      </template>
    </el-dialog>
  </div>
</template>

<script setup>
import { ref, onMounted } from 'vue'
import { api } from '@/api'
import { ElMessage, ElMessageBox } from 'element-plus'
import FilterBar from '@/components/FilterBar.vue'
import DataTable from '@/components/DataTable.vue'

const clubTypes = ['学术', '文艺', '体育', '实践', '其他']
const activeTab = ref('pending')
const loading   = ref(false)
const submitting = ref(false)
const tableData = ref([])
const total     = ref(0)
const page      = ref(1)
const filters   = ref({ keyword: '', club_type: '' })
const current   = ref(null)
const detailVisible = ref(false)
const rejectVisible = ref(false)
const rejectForm = ref()
const rejectData = ref({ reason: '' })

const columns = [
  { prop: 'name',           label: '社团名称' },
  { prop: 'club_type',      label: '类型',   width: 80 },
  { prop: 'applicant_name', label: '申请人', width: 100 },
  { prop: 'college_name',   label: '学院',   width: 120 },
  { prop: 'created_at',     label: '申请时间', width: 160 },
  { slot: 'status',         label: '状态',   width: 90 },
  { slot: 'actions',        label: '操作',   width: 180, fixed: 'right' }
]

const statusLabel = s => ({ pending: '待审批', approved: '已通过', rejected: '已驳回' }[s] || s)
const statusType  = s => ({ pending: 'warning', approved: 'success', rejected: 'danger' }[s] || '')

async function loadData() {
  loading.value = true
  const res = await api.get('/api/school/club-approvals', {
    status: activeTab.value === 'pending' ? 'pending' : 'all',
    page: page.value, page_size: 10, ...filters.value
  })
  loading.value = false
  if (res.code === 0) { tableData.value = res.data.list || []; total.value = res.data.total || 0 }
}

function onReset() { filters.value = { keyword: '', club_type: '' }; loadData() }
function onPage(e) { page.value = e.page; loadData() }
function openDetail(row) { current.value = row; detailVisible.value = true }

function openReject(row) {
  current.value = row
  rejectData.value = { reason: '', club_id: row.club_id }
  detailVisible.value = false
  rejectVisible.value = true
}

async function approve(row, pass) {
  await ElMessageBox.confirm(`确认${pass ? '通过' : '驳回'}该社团申请？`, '提示', { type: 'warning' })
  const res = await api.post('/api/school/club-approvals/' + row.club_id + '/approve', { approved: pass })
  if (res.code === 0) { ElMessage.success('操作成功'); detailVisible.value = false; loadData() }
  else ElMessage.error(res.msg)
}

async function doReject() {
  await rejectForm.value.validate()
  submitting.value = true
  const res = await api.post('/api/school/club-approvals/' + current.value.club_id + '/reject',
    { reason: rejectData.value.reason })
  submitting.value = false
  if (res.code === 0) { ElMessage.success('已驳回'); rejectVisible.value = false; loadData() }
  else ElMessage.error(res.msg)
}

onMounted(loadData)
</script>
