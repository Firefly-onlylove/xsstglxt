<template>
  <div>
    <div class="page-header"><span class="page-title">成员管理</span></div>
    <el-tabs v-model="activeTab" @tab-change="activeTab === 'members' ? loadMembers() : loadApplications()">
      <el-tab-pane label="在籍成员" name="members">
        <DataTable :data="members" :columns="memberCols" :total="memberTotal" :loading="loading"
          @page-change="e=>{memberPage=e.page;loadMembers()}">
          <template #role="{ row }">
            <el-tag size="small" :type="roleType(row.role)">{{ roleLabel(row.role) }}</el-tag>
          </template>
          <template #actions="{ row }">
            <el-button link type="warning"
              v-if="row.role === 'member'"
              @click="promoteVP(row)">任命副社长</el-button>
            <el-button link type="danger"
              v-if="row.role !== 'president'"
              @click="removeMember(row)">移出</el-button>
          </template>
        </DataTable>
      </el-tab-pane>
      <el-tab-pane label="入社审批" name="applications">
        <DataTable :data="applications" :columns="appCols" :total="appTotal" :loading="loading"
          @page-change="e=>{appPage=e.page;loadApplications()}">
          <template #actions="{ row }">
            <el-button link type="success" @click="approve(row, true)">通过</el-button>
            <el-button link type="danger" @click="openReject(row)">拒绝</el-button>
          </template>
        </DataTable>
      </el-tab-pane>
    </el-tabs>

    <el-dialog v-model="rejectVisible" title="拒绝理由" width="380px">
      <el-input v-model="rejectReason" type="textarea" :rows="3" placeholder="请输入拒绝理由" />
      <template #footer>
        <el-button @click="rejectVisible = false">取消</el-button>
        <el-button type="danger" @click="doReject">确认拒绝</el-button>
      </template>
    </el-dialog>
  </div>
</template>

<script setup>
import { ref, onMounted } from 'vue'
import { api } from '@/api'
import { ElMessage, ElMessageBox } from 'element-plus'
import DataTable from '@/components/DataTable.vue'

const activeTab = ref('members')
const loading = ref(false)
const members = ref([]); const memberTotal = ref(0); const memberPage = ref(1)
const applications = ref([]); const appTotal = ref(0); const appPage = ref(1)
const rejectVisible = ref(false); const rejectReason = ref(''); const currentApp = ref(null)

const memberCols = [
  { prop: 'real_name',  label: '姓名',   width: 100 },
  { prop: 'student_id', label: '学号',   width: 120 },
  { prop: 'class_name', label: '班级' },
  { slot: 'role',       label: '角色',   width: 100 },
  { prop: 'join_date',  label: '加入时间', width: 120 },
  { slot: 'actions',    label: '操作',   width: 160, fixed: 'right' }
]
const appCols = [
  { prop: 'real_name',  label: '申请人',  width: 100 },
  { prop: 'student_id', label: '学号',    width: 120 },
  { prop: 'class_name', label: '班级' },
  { prop: 'created_at', label: '申请时间', width: 160 },
  { slot: 'actions',    label: '操作',    width: 130, fixed: 'right' }
]

const roleLabel = r => ({ president:'社长', vice_president:'副社长', member:'成员' }[r] || r)
const roleType  = r => ({ president:'danger', vice_president:'warning', member:'' }[r] || '')

async function loadMembers() {
  loading.value = true
  const res = await api.get('/api/club-admin/members', { page: memberPage.value, page_size: 10 })
  loading.value = false
  if (res.code === 0) { members.value = res.data.list || []; memberTotal.value = res.data.total || 0 }
}
async function loadApplications() {
  loading.value = true
  const res = await api.get('/api/club-admin/join-requests', { page: appPage.value, page_size: 10 })
  loading.value = false
  if (res.code === 0) { applications.value = res.data.list || []; appTotal.value = res.data.total || 0 }
}
async function promoteVP(row) {
  await ElMessageBox.confirm(`确认任命 ${row.real_name} 为副社长？`, '提示', { type: 'warning' })
  const res = await api.put('/api/club-admin/members/' + row.user_id + '/role', { role: 'vice_president' })
  if (res.code === 0) { ElMessage.success('已任命'); loadMembers() }
}
async function removeMember(row) {
  await ElMessageBox.confirm(`确认将 ${row.real_name} 移出社团？`, '提示', { type: 'warning' })
  const res = await api.delete('/api/club-admin/members/' + row.user_id)
  if (res.code === 0) { ElMessage.success('已移出'); loadMembers() }
}
async function approve(row, pass) {
  if (pass) {
    const res = await api.post('/api/club-admin/join-requests/' + row.request_id + '/approve', { approved: true })
    if (res.code === 0) { ElMessage.success('已通过'); loadApplications() }
    return
  }
  currentApp.value = row; rejectReason.value = ''; rejectVisible.value = true
}
function openReject(row) { currentApp.value = row; rejectReason.value = ''; rejectVisible.value = true }
async function doReject() {
  if (!rejectReason.value) { ElMessage.warning('请填写拒绝理由'); return }
  const res = await api.post('/api/club-admin/join-requests/' + currentApp.value.request_id + '/approve',
    { approved: false, reason: rejectReason.value })
  if (res.code === 0) { ElMessage.success('已拒绝'); rejectVisible.value = false; loadApplications() }
}
onMounted(loadMembers)
</script>
