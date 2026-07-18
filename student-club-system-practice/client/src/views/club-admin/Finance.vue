<template>
  <div>
    <div class="page-header">
      <span class="page-title">财务管理</span>
    </div>
    <el-row :gutter="16" style="margin-bottom:20px">
      <el-col :span="6" v-for="c in summary" :key="c.label">
        <div class="stat-card">
          <div class="icon-wrap" :style="{background:c.bg}">
            <el-icon :style="{color:c.color,fontSize:'22px'}"><component :is="c.icon" /></el-icon>
          </div>
          <div><div class="label">{{c.label}}</div><div class="value">{{c.value??'-'}}</div></div>
        </div>
      </el-col>
    </el-row>
    <el-tabs v-model="activeTab">
      <el-tab-pane label="收支记录" name="records">
        <div style="text-align:right;margin-bottom:12px">
          <el-button type="primary" @click="openAddRecord">+ 新增收支</el-button>
        </div>
        <FilterBar @search="loadRecords" @reset="()=>{recFilter={};loadRecords()}">
          <el-select v-model="recFilter.type" placeholder="类型" clearable style="width:110px">
            <el-option label="收入" value="income" />
            <el-option label="支出" value="expense" />
          </el-select>
        </FilterBar>
        <DataTable :data="records" :columns="recCols" :total="recTotal" :loading="loading"
          @page-change="e=>{recPage=e.page;loadRecords()}">
          <template #type="{ row }">
            <el-tag :type="row.type==='income'?'success':'danger'" size="small">{{ row.type==='income'?'收入':'支出' }}</el-tag>
          </template>
          <template #actions="{ row }">
            <el-button link type="danger" @click="deleteRecord(row)">删除</el-button>
          </template>
        </DataTable>
      </el-tab-pane>

      <el-tab-pane label="报销申请" name="reimbursements">
        <div style="text-align:right;margin-bottom:12px">
          <el-button type="primary" @click="reimbVisible = true">+ 申请报销</el-button>
        </div>
        <DataTable :data="reimbs" :columns="reimbCols" :total="reimbTotal" :loading="loading"
          @page-change="e=>{reimbPage=e.page;loadReimbs()}">
          <template #status="{ row }">
            <el-tag :type="reimbStatusType(row.status)" size="small">{{ reimbStatusLabel(row.status) }}</el-tag>
          </template>
        </DataTable>
      </el-tab-pane>
    </el-tabs>

    <!-- 新增收支 -->
    <el-dialog v-model="addRecordVisible" title="新增收支" width="420px">
      <el-form ref="recFormRef" :model="recForm" label-width="80px">
        <el-form-item label="类型" prop="type" :rules="[{required:true}]">
          <el-radio-group v-model="recForm.type">
            <el-radio value="income">收入</el-radio>
            <el-radio value="expense">支出</el-radio>
          </el-radio-group>
        </el-form-item>
        <el-form-item label="金额" prop="amount" :rules="[{required:true}]">
          <el-input-number v-model="recForm.amount" :min="0.01" :precision="2" style="width:100%" />
        </el-form-item>
        <el-form-item label="说明" prop="description" :rules="[{required:true}]">
          <el-input v-model="recForm.description" />
        </el-form-item>
        <el-form-item label="日期" prop="date">
          <el-date-picker v-model="recForm.date" type="date" value-format="YYYY-MM-DD" style="width:100%" />
        </el-form-item>
      </el-form>
      <template #footer>
        <el-button @click="addRecordVisible = false">取消</el-button>
        <el-button type="primary" @click="saveRecord">确认</el-button>
      </template>
    </el-dialog>

    <!-- 报销申请 -->
    <el-dialog v-model="reimbVisible" title="申请报销" width="420px">
      <el-form ref="reimbFormRef" :model="reimbForm" label-width="80px">
        <el-form-item label="金额" prop="amount" :rules="[{required:true}]">
          <el-input-number v-model="reimbForm.amount" :min="0.01" :precision="2" style="width:100%" />
        </el-form-item>
        <el-form-item label="用途" prop="purpose" :rules="[{required:true}]">
          <el-input v-model="reimbForm.purpose" type="textarea" :rows="2" />
        </el-form-item>
        <el-form-item label="发票" prop="receipt" :rules="[{required:true,message:'请上传发票'}]">
          <el-upload action="/api/club-admin/upload-receipt" :on-success="onUploadSuccess" :limit="1" accept="image/*">
            <el-button type="primary" plain>上传发票图片</el-button>
          </el-upload>
          <span v-if="reimbForm.receipt_path" style="font-size:12px;color:#36CFC9">已上传</span>
        </el-form-item>
      </el-form>
      <template #footer>
        <el-button @click="reimbVisible = false">取消</el-button>
        <el-button type="primary" @click="submitReimb">提交申请</el-button>
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

const activeTab = ref('records')
const loading = ref(false)
const summary = ref([
  { label: '总收入', value: null, icon: 'TrendCharts', color: '#36CFC9', bg: '#E6FFFB' },
  { label: '总支出', value: null, icon: 'Money',       color: '#F53F3F', bg: '#FFF1F0' },
  { label: '账户余额', value: null, icon: 'Wallet',    color: '#1677FF', bg: '#E6F4FF' },
  { label: '待审批报销', value: null, icon: 'Warning', color: '#FF7D00', bg: '#FFF7E6' }
])
const records = ref([]); const recTotal = ref(0); const recPage = ref(1); const recFilter = ref({})
const reimbs  = ref([]); const reimbTotal = ref(0); const reimbPage = ref(1)
const addRecordVisible = ref(false); const reimbVisible = ref(false)
const recFormRef = ref(); const reimbFormRef = ref()
const recForm   = ref({ type: 'income', amount: 0, description: '', date: '' })
const reimbForm = ref({ amount: 0, purpose: '', receipt_path: '' })

const recCols = [
  { prop: 'date',        label: '日期',   width: 120 },
  { slot: 'type',        label: '类型',   width: 80 },
  { prop: 'amount',      label: '金额',   width: 100 },
  { prop: 'description', label: '说明' },
  { prop: 'operator_name', label: '经手人', width: 100 },
  { slot: 'actions',     label: '操作',   width: 80, fixed: 'right' }
]
const reimbCols = [
  { prop: 'amount',      label: '金额',   width: 100 },
  { prop: 'purpose',     label: '用途' },
  { prop: 'created_at',  label: '申请时间', width: 160 },
  { slot: 'status',      label: '状态',   width: 120 }
]
const reimbStatusLabel = s => ({ college_pending:'院级待审', school_pending:'校级待审', approved:'已通过', rejected:'已驳回' }[s] || s)
const reimbStatusType  = s => ({ college_pending:'warning', school_pending:'primary', approved:'success', rejected:'danger' }[s] || '')

async function loadRecords() {
  loading.value = true
  const res = await api.get('/api/club-admin/finance', { page: recPage.value, page_size: 10, ...recFilter.value })
  loading.value = false
  if (res.code === 0) { records.value = res.data.list || []; recTotal.value = res.data.total || 0 }
}
async function loadReimbs() {
  loading.value = true
  const res = await api.get('/api/club-admin/reimbursements', { page: reimbPage.value, page_size: 10 })
  loading.value = false
  if (res.code === 0) { reimbs.value = res.data.list || []; reimbTotal.value = res.data.total || 0 }
}
function openAddRecord() { recForm.value = { type: 'income', amount: 0, description: '', date: '' }; addRecordVisible.value = true }
async function saveRecord() {
  await recFormRef.value.validate()
  const res = await api.post('/api/club-admin/finance', recForm.value)
  if (res.code === 0) { ElMessage.success('已添加'); addRecordVisible.value = false; loadRecords() }
}
async function deleteRecord(row) {
  await ElMessageBox.confirm('确认删除该记录？', '提示', { type: 'warning' })
  const res = await api.delete('/api/club-admin/finance/' + row.record_id)
  if (res.code === 0) { ElMessage.success('已删除'); loadRecords() }
}
function onUploadSuccess(res) { if (res.code === 0) reimbForm.value.receipt_path = res.data.path }
async function submitReimb() {
  await reimbFormRef.value.validate()
  if (!reimbForm.value.receipt_path) { ElMessage.warning('请先上传发票'); return }
  const res = await api.post('/api/club-admin/reimbursements', reimbForm.value)
  if (res.code === 0) { ElMessage.success('申请已提交'); reimbVisible.value = false; loadReimbs() }
}
onMounted(async () => {
  const res = await api.get('/api/club-admin/finance-overview')
  if (res.code === 0) {
    summary.value[0].value = '¥' + res.data.total_income
    summary.value[1].value = '¥' + res.data.total_expense
    summary.value[2].value = '¥' + res.data.balance
    summary.value[3].value = res.data.pending_reimb
  }
  loadRecords()
})
</script>
