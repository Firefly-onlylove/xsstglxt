<template>
  <div>
    <div class="page-header">
      <span class="page-title">活动管理</span>
      <el-button type="primary" @click="openCreate">+ 新建活动</el-button>
    </div>
    <FilterBar @search="loadData" @reset="onReset">
      <el-select v-model="filters.status" placeholder="状态" clearable style="width:130px">
        <el-option label="草稿" value="draft" /><el-option label="已发布" value="published" />
        <el-option label="进行中" value="ongoing" /><el-option label="已结束" value="finished" />
        <el-option label="已取消" value="cancelled" />
      </el-select>
    </FilterBar>
    <DataTable :data="tableData" :columns="columns" :total="total" :loading="loading"
      @page-change="onPage">
      <template #status="{ row }">
        <el-tag :type="actStatusType(row.status)" size="small">{{ actStatusLabel(row.status) }}</el-tag>
      </template>
      <template #actions="{ row }">
        <el-button link type="primary" @click="openEdit(row)">编辑</el-button>
        <el-button link type="success" v-if="row.status==='draft'" @click="changeStatus(row,'published')">发布</el-button>
        <el-button link type="warning" v-if="row.status==='published'" @click="changeStatus(row,'ongoing')">开始</el-button>
        <el-button link type="info" v-if="row.status==='ongoing'" @click="openSignin(row)">签到</el-button>
        <el-button link type="primary" v-if="row.status==='ongoing'" @click="changeStatus(row,'finished')">结束</el-button>
        <el-button link type="danger" v-if="['draft','published'].includes(row.status)" @click="openCancel(row)">取消</el-button>
        <el-button link type="danger" v-if="row.status==='draft'" @click="deleteActivity(row)">删除</el-button>
      </template>
    </DataTable>

    <!-- 创建/编辑活动 -->
    <el-dialog v-model="formVisible" :title="editing?.activity_id ? '编辑活动' : '新建活动'" width="640px" destroy-on-close>
      <el-form ref="actFormRef" :model="actForm" :rules="actRules" label-width="110px">
        <el-row :gutter="16">
          <el-col :span="16">
            <el-form-item label="活动名称" prop="title"><el-input v-model="actForm.title" /></el-form-item>
          </el-col>
          <el-col :span="8">
            <el-form-item label="人数上限" prop="max_participants">
              <el-input-number v-model="actForm.max_participants" :min="1" style="width:100%" />
            </el-form-item>
          </el-col>
        </el-row>
        <el-row :gutter="16">
          <el-col :span="12">
            <el-form-item label="开始时间" prop="start_time">
              <el-date-picker v-model="actForm.start_time" type="datetime" value-format="YYYY-MM-DD HH:mm:ss" style="width:100%" />
            </el-form-item>
          </el-col>
          <el-col :span="12">
            <el-form-item label="结束时间" prop="end_time">
              <el-date-picker v-model="actForm.end_time" type="datetime" value-format="YYYY-MM-DD HH:mm:ss" style="width:100%" />
            </el-form-item>
          </el-col>
        </el-row>
        <el-form-item label="报名截止" prop="signup_deadline">
          <el-date-picker v-model="actForm.signup_deadline" type="datetime" value-format="YYYY-MM-DD HH:mm:ss" style="width:100%" />
        </el-form-item>
        <el-form-item label="地点" prop="location"><el-input v-model="actForm.location" /></el-form-item>
        <el-form-item label="活动简介" prop="description">
          <el-input v-model="actForm.description" type="textarea" :rows="3" />
        </el-form-item>
      </el-form>
      <template #footer>
        <el-button @click="formVisible = false">取消</el-button>
        <el-button type="primary" :loading="submitting" @click="saveActivity">保存</el-button>
      </template>
    </el-dialog>

    <!-- 签到管理 -->
    <el-dialog v-model="signinVisible" :title="(current?.title || '') + ' — 签到管理'" width="700px">
      <div style="margin-bottom:12px">
        签到码：<el-tag type="warning" size="large">{{ current?.signin_code }}</el-tag>
        <el-button size="small" style="margin-left:8px" @click="manualSignin">手动签到</el-button>
      </div>
      <el-table :data="signinList" border size="small">
        <el-table-column prop="real_name"  label="姓名"   width="100" />
        <el-table-column prop="student_id" label="学号"   width="120" />
        <el-table-column prop="status"     label="状态"   width="90">
          <template #default="{ row }">
            <el-tag :type="row.status==='signed_in'?'success':'info'" size="small">
              {{ row.status==='signed_in'?'已签到':'未签到' }}
            </el-tag>
          </template>
        </el-table-column>
        <el-table-column prop="signin_time" label="签到时间" />
      </el-table>
    </el-dialog>
  </div>
</template>

<script setup>
import { ref, onMounted } from 'vue'
import { api } from '@/api'
import { ElMessage, ElMessageBox } from 'element-plus'
import FilterBar from '@/components/FilterBar.vue'
import DataTable from '@/components/DataTable.vue'

const loading = ref(false); const submitting = ref(false)
const tableData = ref([]); const total = ref(0); const page = ref(1)
const filters = ref({ status: '' })
const formVisible = ref(false); const signinVisible = ref(false)
const editing = ref(null); const current = ref(null)
const actFormRef = ref(); const signinList = ref([])
const actForm = ref({ title: '', max_participants: 50, start_time: '', end_time: '', signup_deadline: '', location: '', description: '' })
const actRules = {
  title:            [{ required: true }],
  start_time:       [{ required: true, message: '请选择开始时间' }],
  end_time:         [{ required: true, message: '请选择结束时间' }],
  signup_deadline:  [{ required: true, message: '请选择报名截止时间' }],
  location:         [{ required: true }],
  description:      [{ required: true }],
  max_participants: [{ required: true }]
}

const columns = [
  { prop: 'title',       label: '活动名称' },
  { prop: 'start_time',  label: '开始时间',  width: 160 },
  { prop: 'location',    label: '地点',      width: 120 },
  { prop: 'registered_count', label: '报名/上限', width: 90,
    formatter: (row) => `${row.registered_count}/${row.max_participants}` },
  { slot: 'status',      label: '状态',      width: 100 },
  { slot: 'actions',     label: '操作',      width: 260, fixed: 'right' }
]

const actStatusLabel = s => ({ draft:'草稿', published:'已发布', ongoing:'进行中', finished:'已结束', cancelled:'已取消' }[s] || s)
const actStatusType  = s => ({ draft:'info', published:'primary', ongoing:'success', finished:'', cancelled:'danger' }[s] || '')

async function loadData() {
  loading.value = true
  const res = await api.get('/api/club-admin/activities', { page: page.value, page_size: 10, ...filters.value })
  loading.value = false
  if (res.code === 0) { tableData.value = res.data.list || []; total.value = res.data.total || 0 }
}
function onReset() { filters.value = { status: '' }; loadData() }
function onPage(e) { page.value = e.page; loadData() }
function openCreate() { editing.value = {}; actForm.value = { title: '', max_participants: 50, start_time: '', end_time: '', signup_deadline: '', location: '', description: '' }; formVisible.value = true }
function openEdit(row) {
  editing.value = row
  actForm.value = { title: row.title, max_participants: row.max_participants, start_time: row.start_time, end_time: row.end_time, signup_deadline: row.signup_deadline, location: row.location, description: row.description }
  formVisible.value = true
}
async function saveActivity() {
  await actFormRef.value.validate()
  submitting.value = true
  const url = editing.value?.activity_id ? '/api/club-admin/activities/' + editing.value.activity_id : '/api/club-admin/activities'
  const method = editing.value?.activity_id ? 'put' : 'post'
  const res = await api[method](url, actForm.value)
  submitting.value = false
  if (res.code === 0) { ElMessage.success('保存成功'); formVisible.value = false; loadData() }
  else ElMessage.error(res.msg)
}
async function changeStatus(row, status) {
  const labels = { published:'发布', ongoing:'开始', finished:'结束' }
  await ElMessageBox.confirm(`确认${labels[status]}活动 ${row.title}？`, '提示', { type: 'warning' })
  const res = await api.put('/api/club-admin/activities/' + row.activity_id + '/status', { status })
  if (res.code === 0) { ElMessage.success('操作成功'); loadData() }
}
async function openCancel(row) {
  const { value: reason } = await ElMessageBox.prompt('取消原因', '取消活动', { inputValidator: v => v ? true : '请填写原因' })
  const res = await api.put('/api/club-admin/activities/' + row.activity_id + '/status', { status: 'cancelled', reason })
  if (res.code === 0) { ElMessage.success('已取消'); loadData() }
}
async function deleteActivity(row) {
  await ElMessageBox.confirm('确认删除？', '提示', { type: 'warning' })
  const res = await api.delete('/api/club-admin/activities/' + row.activity_id)
  if (res.code === 0) { ElMessage.success('已删除'); loadData() }
}
async function openSignin(row) {
  current.value = row
  const res = await api.get('/api/club-admin/activities/' + row.activity_id + '/registrations')
  if (res.code === 0) signinList.value = res.data || []
  signinVisible.value = true
}
async function manualSignin() {
  const { value: keyword } = await ElMessageBox.prompt('输入学号或姓名', '手动签到')
  const res = await api.post('/api/club-admin/activities/' + current.value.activity_id + '/manual-signin', { keyword })
  if (res.code === 0) { ElMessage.success('签到成功'); openSignin(current.value) }
  else ElMessage.error(res.msg)
}
onMounted(loadData)
</script>
