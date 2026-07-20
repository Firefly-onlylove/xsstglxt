<template>
  <div>
    <div class="page-header"><span class="page-title">财务监督</span></div>
    <el-row :gutter="16" style="margin-bottom:20px">
      <el-col :span="6" v-for="c in overview" :key="c.label">
        <div class="stat-card">
          <div class="icon-wrap" :style="{background:c.bg}">
            <el-icon :style="{color:c.color,fontSize:'22px'}"><component :is="c.icon" /></el-icon>
          </div>
          <div><div class="label">{{c.label}}</div><div class="value">{{c.value ?? '-'}}</div></div>
        </div>
      </el-col>
    </el-row>

    <el-tabs v-model="activeTab">
      <el-tab-pane label="报销审批" name="reimbursements">
        <FilterBar @search="loadReimb" @reset="()=>{reimbFilter={};loadReimb()}">
          <el-select v-model="reimbFilter.status" placeholder="状态" clearable style="width:130px">
            <el-option label="待审批" value="pending" />
            <el-option label="已通过" value="approved" />
            <el-option label="已驳回" value="rejected" />
          </el-select>
        </FilterBar>
        <DataTable :data="reimbData" :columns="reimbCols" :total="reimbTotal" :loading="loading"
          @page-change="e=>{reimbPage=e.page;loadReimb()}">
          <template #status="{ row }">
            <el-tag :type="reimbStatusType(row)" size="small">{{ reimbStatusLabel(row) }}</el-tag>
          </template>
          <template #actions="{ row }">
            <el-button link type="primary" @click="openReimbDetail(row)">详情</el-button>
            <template v-if="row.college_reviewed==='approved' && row.status==='pending'">
              <el-button link type="success" @click="approveReimb(row, true)">通过</el-button>
              <el-button link type="danger" @click="approveReimb(row, false)">驳回</el-button>
            </template>
          </template>
        </DataTable>
      </el-tab-pane>

      <el-tab-pane label="学院报销记录" name="collegeReimb">
        <FilterBar @search="loadCollegeReimb" @reset="()=>{loadCollegeReimb()}">
          <el-select v-model="collegeReimbFilter.status" placeholder="学院审批" clearable style="width:130px">
            <el-option label="全部" value="" />
            <el-option label="待学院审" value="pending" />
            <el-option label="学院已批" value="approved" />
            <el-option label="学院驳回" value="rejected" />
          </el-select>
        </FilterBar>
        <DataTable :data="collegeReimbData" :columns="collegeReimbCols" :loading="clgReimbLoading">
          <template #college_reviewed="{ row }">
            <el-tag :type="collegeReviewType(row)" size="small">{{ collegeReviewLabel(row) }}</el-tag>
          </template>
          <template #status="{ row }">
            <el-tag :type="reimbStatusType(row)" size="small">{{ reimbStatusLabel(row) }}</el-tag>
          </template>
        </DataTable>
      </el-tab-pane>

      <el-tab-pane label="各学院财务" name="colleges">
        <el-table :data="collegeFinance" border style="width:100%">
          <el-table-column prop="college_name" label="学院" />
          <el-table-column prop="total_income" label="本学期收入" />
          <el-table-column prop="total_expense" label="总支出" />
          <el-table-column prop="balance" label="余额" />
          <el-table-column prop="club_count" label="社团数" width="80" />
        </el-table>
      </el-tab-pane>
    </el-tabs>

    <!-- 报销详情弹窗 -->
    <el-dialog v-model="reimbVisible" title="报销详情" width="500px">
      <el-descriptions :column="2" border v-if="currentReimb">
        <el-descriptions-item label="社团">{{ currentReimb.club_name }}</el-descriptions-item>
        <el-descriptions-item label="金额">¥{{ currentReimb.amount }}</el-descriptions-item>
        <el-descriptions-item label="用途" :span="2">{{ currentReimb.description }}</el-descriptions-item>
        <el-descriptions-item label="申请人">{{ currentReimb.applicant_name }}</el-descriptions-item>
        <el-descriptions-item label="申请时间">{{ currentReimb.submitted_at }}</el-descriptions-item>
        <el-descriptions-item label="学院审批" v-if="currentReimb.college_reviewed">
          <el-tag size="small" :type="collegeReviewType(currentReimb)">
            {{ collegeReviewLabel(currentReimb) }}
          </el-tag>
        </el-descriptions-item>
        <el-descriptions-item label="学校审批">
          <el-tag size="small" :type="reimbStatusType(currentReimb)">
            {{ reimbStatusLabel(currentReimb) }}
          </el-tag>
        </el-descriptions-item>
      </el-descriptions>
      <div v-if="currentReimb?.receipt_path" style="margin-top:12px">
        <img :src="'/receipts/'+currentReimb.receipt_path" style="max-width:100%;border-radius:4px" />
      </div>
    </el-dialog>
  </div>
</template>

<script setup>
import { ref, onMounted } from 'vue'
import { useRouter } from 'vue-router'
import { api } from '@/api'
import { ElMessage, ElMessageBox } from 'element-plus'
import FilterBar from '@/components/FilterBar.vue'
import DataTable from '@/components/DataTable.vue'

const router = useRouter()

const activeTab = ref('reimbursements')
const loading = ref(false)
const overview = ref([
  { label: '本学期全校社团总经费', value: null, icon: 'TrendCharts', color: '#36CFC9', bg: '#E6FFFB' },
  { label: '全校总支出', value: null, icon: 'Money', color: '#F53F3F', bg: '#FFF1F0' },
  { label: '待审批报销', value: null, icon: 'Warning', color: '#FF7D00', bg: '#FFF7E6' },
  { label: '已通过报销', value: null, icon: 'Checked', color: '#1677FF', bg: '#E6F4FF' }
])
const reimbData = ref([])
const reimbTotal = ref(0)
const reimbPage = ref(1)
const reimbFilter = ref({ status: 'pending' })
const reimbVisible = ref(false)
const currentReimb = ref(null)
const collegeFinance = ref([])
const collegeReimbData = ref([])
const collegeReimbFilter = ref({})
const clgReimbLoading = ref(false)

const collegeReimbCols = [
  { prop: 'club_name',      label: '社团',   width: 120 },
  { prop: 'college_name',   label: '学院',   width: 120 },
  { prop: 'amount',         label: '金额',   width: 90 },
  { prop: 'description',    label: '用途' },
  { prop: 'applicant_name', label: '申请人', width: 90 },
  { prop: 'submitted_at',   label: '时间',   width: 160 },
  { slot: 'college_reviewed', label: '学院审批', width: 100 },
  { slot: 'status',         label: '学校审批', width: 100 }
]

async function loadCollegeReimb() {
  clgReimbLoading.value = true
  const res = await api.get('/api/school/reimbursements/college', collegeReimbFilter.value)
  clgReimbLoading.value = false
  if (res.code === 0) collegeReimbData.value = res.data.list || []
}

const reimbCols = [
  { prop: 'club_name',      label: '社团',   width: 120 },
  { prop: 'amount',         label: '金额',   width: 90 },
  { prop: 'description',    label: '用途' },
  { prop: 'applicant_name', label: '申请人', width: 90 },
  { prop: 'submitted_at',     label: '时间',   width: 120 },
  { slot: 'status',         label: '状态',   width: 110 },
  { slot: 'actions',        label: '操作',   width: 160, fixed: 'right' }
]

/* 学校审批状态映射 */
const reimbStatusLabel = row => {
  const s = typeof row === 'string' ? row : row.status
  if (typeof row === 'object' && row.college_reviewed === 'approved' && s === 'pending') return '待学校终审'
  return { pending:'待学院审', approved:'已通过', rejected:'已驳回' }[s] || s
}
const reimbStatusType = row => {
  const s = typeof row === 'string' ? row : row.status
  if (typeof row === 'object' && row.college_reviewed === 'approved' && s === 'pending') return 'primary'
  return { pending:'warning', approved:'success', rejected:'danger' }[s] || 'info'
}

/* 学院审批状态映射 */
const collegeReviewLabel = row => {
  const cr = row.college_reviewed || 'pending'
  return { pending:'待学院审', approved:'学院已通过', rejected:'学院已驳回' }[cr] || cr
}
const collegeReviewType = row => {
  const cr = row.college_reviewed || 'pending'
  return { pending:'warning', approved:'success', rejected:'danger' }[cr] || 'info'
}

async function loadReimb() {
  loading.value = true
  const res = await api.get('/api/school/reimbursements/pending', { page: reimbPage.value, page_size: 10, ...reimbFilter.value })
  loading.value = false
  if (res.code === 0) { reimbData.value = res.data.list || []; reimbTotal.value = res.data.total || 0 }
}

function openReimbDetail(row) { currentReimb.value = row; reimbVisible.value = true }

async function approveReimb(row, pass) {
  if (!pass) {
    const { value: reason } = await ElMessageBox.prompt('驳回理由', '驳回', { inputValidator: v => v ? true : '请填写理由' })
    const res = await api.post('/api/school/reimbursements/' + row.reimbursement_id + '/reject', { comment: reason })
    if (res.code === 0) { ElMessage.success('已驳回'); loadReimb(); loadOverview() }
    return
  }
  await ElMessageBox.confirm('确认通过此报销申请（通过后将计入社团支出）？', '提示', { type: 'warning' })
  const res = await api.post('/api/school/reimbursements/' + row.reimbursement_id + '/approve', { approved: true })
  if (res.code === 0) { ElMessage.success('已通过（已记支出）'); loadReimb(); loadOverview() }
}

async function loadOverview() {
  const ov = await api.get('/api/school/finance')
  if (ov.code === 0) {
    overview.value[0].value = '¥' + (ov.data.total_income || 0)
    overview.value[1].value = '¥' + (ov.data.total_expense || 0)
    overview.value[2].value = ov.data.pending_reimb
    overview.value[3].value = ov.data.approved_reimb
    collegeFinance.value = ov.data.colleges || []
  }
}

onMounted(async () => {
  loadOverview()
  loadReimb()
  loadCollegeReimb()
})
</script>
