<template>
  <div>
    <div class="page-header">
      <span class="page-title">社团审批</span>
    </div>
    <el-tabs v-model="activeTab" @tab-change="onTabChange">
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
      <template #category="{ row }">
        <span>{{ categoryLabel(row.category) }}</span>
      </template>
      <template #college_name="{ row }">
        <span>{{ row.level === 'school' ? '学校' : row.college_name }}</span>
      </template>
      <template #status="{ row }">
        <el-tag :type="statusType(row.status)" size="small">{{ statusLabel(row.status) }}</el-tag>
      </template>
      <template #actions="{ row }">
        <el-button link type="primary" @click="openDetail(row)">详情</el-button>
        <template v-if="activeTab === 'pending'">
          <el-button link type="success" @click="quickApproveSchool(row)">通过(校级)</el-button>
          <el-button link type="primary" @click="showCollegeFor(row)">通过(院级)</el-button>
          <el-button link type="danger" @click="openReject(row)">驳回</el-button>
        </template>
      </template>
    </DataTable>

    <!-- 详情弹窗 -->
    <el-dialog v-model="detailVisible" title="社团申请详情" width="560px">
      <el-descriptions :column="2" border v-if="current">
        <el-descriptions-item label="社团名称">{{ current.club_name }}</el-descriptions-item>
        <el-descriptions-item label="类型">{{ categoryLabel(current.category) }}</el-descriptions-item>
        <el-descriptions-item label="申请人">{{ current.creator }}</el-descriptions-item>
        <el-descriptions-item label="附属">{{ current.college_name }}</el-descriptions-item>
        <el-descriptions-item label="级别">{{ current.level === 'school' ? '校级' : current.level === 'college' ? '院级' : '-' }}</el-descriptions-item>
        <el-descriptions-item label="申请时间">{{ current.created_at }}</el-descriptions-item>
        <el-descriptions-item label="简介" :span="2">{{ current.description }}</el-descriptions-item>
        <el-descriptions-item label="驳回理由" :span="2" v-if="current.status === 'rejected'">{{ current.reject_reason }}</el-descriptions-item>
      </el-descriptions>
      <template #footer>
        <el-button @click="detailVisible = false">关闭</el-button>
        <template v-if="current?.status === 'pending'">
          <el-button type="success" @click="approveFromDetail">通过（校级）</el-button>
          <el-button type="primary" @click="showCollegeApprove">通过（院级）</el-button>
          <el-button type="danger" @click="openReject(current)">驳回</el-button>
        </template>
      </template>
    </el-dialog>

    <!-- 院级审批：选择学院 -->
    <el-dialog v-model="collegeVisible" title="院级社团 — 选择所属学院" width="420px">
      <el-form :model="collegeForm">
        <el-form-item label="所属学院">
          <el-select v-model="collegeForm.college_id" placeholder="请选择学院" style="width:100%">
            <el-option v-for="c in collegeList" :key="c.college_id" :label="c.college_name" :value="c.college_id" />
          </el-select>
        </el-form-item>
      </el-form>
      <template #footer>
        <el-button @click="collegeVisible = false">取消</el-button>
        <el-button type="success" @click="approve(current, false)">确认通过</el-button>
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

const clubTypes = ['文艺', '学术', '体育', '实践', '其他']
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
  { prop: 'club_name',   label: '社团名称' },
  { slot: 'category',    label: '类型',   width: 80 },
  { prop: 'creator',     label: '申请人', width: 100 },
  { slot: 'college_name', label: '附属',   width: 120 },
  { prop: 'created_at',  label: '申请时间', width: 160 },
  { slot: 'status',      label: '状态',   width: 90 },
  { slot: 'actions',     label: '操作',   width: 240, fixed: 'right' }
]

const collegeList = ref([])
const collegeVisible = ref(false)
const collegeForm = ref({ college_id: null })

const statusLabel = s => ({ pending: '待审批', approved: '已通过', rejected: '已驳回' }[s] || s)
const statusType  = s => ({ pending: 'warning', approved: 'success', rejected: 'danger' }[s] || '')
const categoryLabel = c => ({
  art: '文艺', academic: '学术', sports: '体育', practice: '实践', other: '其他',
  '文艺': '文艺', '学术': '学术', '体育': '体育', '实践': '实践', '其他': '其他'
}[c] || c)

function onTabChange() { page.value = 1; loadData() }

async function loadData() {
  loading.value = true
  const endpoint = activeTab.value === 'pending'
    ? '/api/school/clubs/pending'
    : '/api/school/clubs/history'
  const res = await api.get(endpoint, {
    page: page.value, page_size: 10, ...filters.value
  })
  loading.value = false
  if (res.code === 0) {
    tableData.value = res.data.list || []
    total.value = res.data.total || 0
  }
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

async function quickApproveSchool(row) {
  await ElMessageBox.confirm('确认通过该社团申请（校级）？', '提示', { type: 'warning' })
  const res = await api.post('/api/school/clubs/' + row.club_id + '/approve', { level: 'school' })
  if (res.code === 0) { ElMessage.success('已通过'); loadData() }
  else ElMessage.error(res.msg)
}

function showCollegeFor(row) {
  current.value = row
  collegeForm.value.college_id = null
  collegeVisible.value = true
}

async function approveFromDetail() {
  detailVisible.value = false
  await ElMessageBox.confirm('确认通过该社团申请（校级）？', '提示', { type: 'warning' })
  const res = await api.post('/api/school/clubs/' + current.value.club_id + '/approve', { level: 'school' })
  if (res.code === 0) { ElMessage.success('已通过'); loadData() }
  else ElMessage.error(res.msg)
}

function showCollegeApprove() {
  detailVisible.value = false
  collegeForm.value.college_id = null
  collegeVisible.value = true
}

async function approve(row, isSchool) {
  const level = isSchool ? 'school' : 'college'
  const label = isSchool ? '校级' : '院级'
  await ElMessageBox.confirm(`确认通过该社团申请（${label}）？`, '提示', { type: 'warning' })
  const body = isSchool
    ? { level: 'school' }
    : { level: 'college', college_id: collegeForm.value.college_id }
  const res = await api.post('/api/school/clubs/' + current.value.club_id + '/approve', body)
  if (res.code === 0) { ElMessage.success('已通过'); collegeVisible.value = false; detailVisible.value = false; loadData() }
  else ElMessage.error(res.msg)
}

async function doReject() {
  await rejectForm.value.validate()
  submitting.value = true
  const res = await api.post('/api/school/clubs/' + current.value.club_id + '/reject',
    { reason: rejectData.value.reason })
  submitting.value = false
  if (res.code === 0) { ElMessage.success('已驳回'); rejectVisible.value = false; loadData() }
  else ElMessage.error(res.msg)
}

onMounted(async () => {
  const res = await api.get('/api/school/colleges')
  if (res.code === 0) collegeList.value = res.data.list || res.data || []
  loadData()
})
</script>
