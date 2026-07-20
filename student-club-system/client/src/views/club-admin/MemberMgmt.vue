<template>
  <div>
    <div class="page-header">
      <span class="page-title">{{ showMembers ? '成员管理' : '入社审批' }}</span>
    </div>

    <!-- 在籍成员 (from dashboard "社团成员" click) -->
    <div v-if="showMembers" class="card">
      <DataTable :data="members" :columns="memberCols" :total="memberTotal" :loading="loading"
        @page-change="e=>{memberPage=e.page;loadMembers()}">
        <template #role="{ row }">
          <el-tag size="small" :type="roleType(row.role)">{{ roleLabel(row.role) }}</el-tag>
        </template>
        <template #actions="{ row }">
          <el-button link type="warning"
            v-if="row.role === 'member'"
            @click="setManager(row)">设为管理员</el-button>
          <el-button link type="primary"
            v-if="row.role === 'president' || row.role === 'vice_president'"
            @click="demoteToMember(row)">降为成员</el-button>
          <el-button link type="danger"
            v-if="row.role !== 'president' && row.role !== 'vice_president'"
            @click="removeMember(row)">移出</el-button>
        </template>
      </DataTable>
    </div>

    <!-- 入社审批 (from dashboard "待审批入社" click) -->
    <div v-else class="card">
      <DataTable :data="applications" :columns="appCols" :total="appTotal" :loading="loading"
        @page-change="e=>{appPage=e.page;loadApplications()}">
        <template #actions="{ row }">
          <el-button link type="success" @click="approve(row, true)">通过</el-button>
          <el-button link type="danger" @click="openReject(row)">拒绝</el-button>
        </template>
      </DataTable>
    </div>

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
import { ref, computed, onMounted, watch } from 'vue'
import { useRoute, useRouter } from 'vue-router'
import { api } from '@/api'
import { ElMessage, ElMessageBox } from 'element-plus'
import DataTable from '@/components/DataTable.vue'
import { useClub } from '@/composables/useClub'

const route = useRoute()
const showMembers = computed(() => !route.query.tab || route.query.tab !== 'applications')
const loading = ref(false)
const members = ref([]); const memberTotal = ref(0); const memberPage = ref(1)
const applications = ref([]); const appTotal = ref(0); const appPage = ref(1)
const rejectVisible = ref(false); const rejectReason = ref(''); const currentApp = ref(null)
const { clubId } = useClub()

const memberCols = [
  { prop: 'real_name',   label: '姓名',   width: 100 },
  { prop: 'student_no',  label: '学号',   width: 120 },
  { prop: 'college_name',label: '学院' },
  { slot: 'role',        label: '角色',   width: 100 },
  { prop: 'join_date',   label: '加入时间', width: 120 },
  { slot: 'actions',     label: '操作',   width: 160, fixed: 'right' }
]
const appCols = [
  { prop: 'real_name',   label: '申请人',   width: 100 },
  { prop: 'student_no',  label: '学号',     width: 120 },
  { prop: 'college_name',label: '学院' },
  { prop: 'applied_at',  label: '申请时间',  width: 160 },
  { slot: 'actions',     label: '操作',     width: 130, fixed: 'right' }
]

const roleLabel = r => ({ president:'社团管理员', vice_president:'社团管理员', member:'成员' }[r] || r)
const roleType  = r => ({ president:'', vice_president:'', member:'info' }[r] || '')

async function loadMembers() {
  if (!clubId.value) return
  loading.value = true
  const res = await api.get('/api/club/' + clubId.value + '/members', { page: memberPage.value, page_size: 10 })
  loading.value = false
  if (res.code === 0) { members.value = res.data.list || []; memberTotal.value = res.data.total || 0 }
}
async function loadApplications() {
  if (!clubId.value) return
  loading.value = true
  const res = await api.get('/api/club/' + clubId.value + '/join-requests', { page: appPage.value, page_size: 10 })
  loading.value = false
  if (res.code === 0) { applications.value = res.data.list || []; appTotal.value = res.data.total || 0 }
}
async function setManager(row) {
  await ElMessageBox.confirm(`确认将 ${row.real_name} 设为社团管理员？`, '提示', { type: 'warning' })
  const res = await api.post('/api/club/' + clubId.value + '/members/' + row.user_id + '/appoint', { role: 'vice_president' })
  if (res.code === 0) { ElMessage.success('已设为管理员'); loadMembers() }
  else { ElMessage.error(res.msg || '操作失败') }
}
async function demoteToMember(row) {
  await ElMessageBox.confirm(`确认将 ${row.real_name} 降为普通成员？`, '提示', { type: 'warning' })
  const res = await api.post('/api/club/' + clubId.value + '/members/' + row.user_id + '/appoint', { role: 'member' })
  if (res.code === 0) { ElMessage.success('已降为成员'); loadMembers() }
  else { ElMessage.error(res.msg || '操作失败') }
}
async function removeMember(row) {
  await ElMessageBox.confirm(`确认将 ${row.real_name} 移出社团？`, '提示', { type: 'warning' })
  const res = await api.post('/api/club/' + clubId.value + '/members/' + row.user_id + '/remove')
  if (res.code === 0) { ElMessage.success('已移出'); loadMembers() }
}
async function approve(row, pass) {
  if (pass) {
    const res = await api.post('/api/club/' + clubId.value + '/join-requests/' + row.member_id + '/approve')
    if (res.code === 0) { ElMessage.success('已通过'); loadApplications() }
    return
  }
  currentApp.value = row; rejectReason.value = ''; rejectVisible.value = true
}
function openReject(row) { currentApp.value = row; rejectReason.value = ''; rejectVisible.value = true }
async function doReject() {
  if (!rejectReason.value) { ElMessage.warning('请填写拒绝理由'); return }
  const res = await api.post('/api/club/' + clubId.value + '/join-requests/' + currentApp.value.member_id + '/reject',
    { reason: rejectReason.value })
  if (res.code === 0) { ElMessage.success('已拒绝'); rejectVisible.value = false; loadApplications() }
}
const loadAll = () => showMembers.value ? loadMembers() : loadApplications()

onMounted(() => {
  if (clubId.value) loadAll()
  else {
    const stop = watch(clubId, (val) => { if (val) { stop(); loadAll() } })
  }
})
</script>
