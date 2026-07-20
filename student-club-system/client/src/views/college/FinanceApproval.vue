<template>
  <div>
    <div class="page-header"><span class="page-title">学院报销审批</span></div>
    <el-tabs v-model="activeTab" @tab-change="loadData">
      <el-tab-pane label="待审批" name="pending" />
      <el-tab-pane label="已审批记录" name="history" />
    </el-tabs>
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
    <el-dialog v-model="detailVisible" title="报销详情" width="500px">
      <el-descriptions :column="2" border v-if="current">
        <el-descriptions-item label="社团">{{ current.club_name }}</el-descriptions-item>
        <el-descriptions-item label="金额">¥{{ current.amount }}</el-descriptions-item>
        <el-descriptions-item label="用途" :span="2">{{ current.description }}</el-descriptions-item>
        <el-descriptions-item label="申请人">{{ current.applicant_name }}</el-descriptions-item>
        <el-descriptions-item label="申请时间">{{ current.submitted_at }}</el-descriptions-item>
      </el-descriptions>
      <div v-if="current?.receipt_path" style="margin-top:12px">
        <img :src="'/receipts/'+current.receipt_path" style="max-width:100%;border-radius:4px" />
      </div>
      <div style="margin-top:12px;color:#86909C;font-size:13px">
        本院剩余可审批额度：¥{{ availableLimit }}
      </div>
      <template #footer v-if="current?.status==='pending'">
        <el-button @click="detailVisible = false">关闭</el-button>
        <el-button type="success" @click="approve(current, true)">通过</el-button>
        <el-button type="danger" @click="openReject(current)">驳回</el-button>
      </template>
    </el-dialog>

    <!-- 驳回 -->
    <el-dialog v-model="rejectVisible" title="驳回理由" width="400px">
      <el-input v-model="rejectReason" type="textarea" :rows="3" placeholder="请输入驳回理由" />
      <template #footer>
        <el-button @click="rejectVisible = false">取消</el-button>
        <el-button type="danger" :loading="submitting" @click="doReject">确认</el-button>
      </template>
    </el-dialog>
  </div>
</template>

<script setup>
import { ref, onMounted } from 'vue'
import { api } from '@/api'
import { ElMessage, ElMessageBox } from 'element-plus'
import DataTable from '@/components/DataTable.vue'

const activeTab = ref('pending')
const loading = ref(false)
const submitting = ref(false)
const tableData = ref([])
const total = ref(0)
const page = ref(1)
const current = ref(null)
const detailVisible = ref(false)
const rejectVisible = ref(false)
const rejectReason = ref('')
const availableLimit = ref(0)

const columns = [
  { prop: 'club_name',      label: '社团',   width: 120 },
  { prop: 'amount',         label: '金额',   width: 90 },
  { prop: 'description',    label: '用途' },
  { prop: 'applicant_name', label: '申请人', width: 90 },
  { prop: 'submitted_at',   label: '时间',   width: 120 },
  { slot: 'status',         label: '状态',   width: 100 },
  { slot: 'actions',        label: '操作',   width: 160, fixed: 'right' }
]

const statusLabel = s => ({ pending:'待审批', approved:'已通过', rejected:'已驳回' }[s] || s)
const statusType  = s => ({ pending:'warning', approved:'success', rejected:'danger' }[s])

async function loadData() {
  loading.value = true
  const res = await api.get('/api/college/reimbursements/pending', {
    status: activeTab.value === 'pending' ? 'pending' : 'history',
    page: page.value, page_size: 10
  })
  loading.value = false
  if (res.code === 0) { tableData.value = res.data.list || []; total.value = res.data.total || 0 }
}
function onPage(e) { page.value = e.page; loadData() }
async function openDetail(row) {
  current.value = row
  const res = await api.get('/api/college/limits')
  if (res.code === 0) availableLimit.value = res.data.available
  detailVisible.value = true
}
function openReject(row) { current.value = row; rejectReason.value = ''; detailVisible.value = false; rejectVisible.value = true }
async function approve(row, pass) {
  if (pass) {
    await ElMessageBox.confirm('确认通过该报销申请？', '提示', { type: 'warning' })
    const res = await api.post('/api/college/reimbursements/' + row.reimbursement_id + '/approve', { approved: true })
    if (res.code === 0) { ElMessage.success('已通过'); detailVisible.value = false; loadData() }
    return
  }
  openReject(row)
}
async function doReject() {
  if (!rejectReason.value) { ElMessage.warning('请填写驳回理由'); return }
  submitting.value = true
  const res = await api.post('/api/college/reimbursements/' + current.value.reimbursement_id + '/reject',
    { reason: rejectReason.value })
  submitting.value = false
  if (res.code === 0) { ElMessage.success('已驳回'); rejectVisible.value = false; loadData() }
}
onMounted(loadData)
</script>
