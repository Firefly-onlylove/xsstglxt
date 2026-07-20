<template>
  <div>
    <div class="page-header">
      <span class="page-title">用户管理</span>
    </div>
    <FilterBar @search="loadData" @reset="onReset">
      <el-input v-model="filters.keyword" placeholder="用户名/姓名" clearable style="width:200px" />
      <el-select v-model="filters.role" placeholder="角色" clearable style="width:130px">
        <el-option label="学校管理员" value="school_admin" />
        <el-option label="学院管理员" value="college_admin" />
        <el-option label="社长" value="club_admin" />
        <el-option label="社团成员" value="club_member" />
        <el-option label="学生" value="general_student" />
      </el-select>
      <el-select v-model="filters.college_id" placeholder="学院" clearable style="width:150px">
        <el-option v-for="c in colleges" :key="c.college_id" :label="c.college_name" :value="c.college_id" />
      </el-select>
      <el-select v-model="filters.status" placeholder="状态" clearable style="width:110px">
        <el-option label="正常" value="1" />
        <el-option label="已禁用" value="0" />
        <el-option label="已限制" value="restricted" />
      </el-select>
    </FilterBar>
    <DataTable :data="tableData" :columns="columns" :total="total" :loading="loading"
      @page-change="onPage">
      <template #role="{ row }">
        <el-tag size="small" :type="roleType(row.role)">{{ roleLabel(row.role) }}</el-tag>
      </template>
      <template #status="{ row }">
        <el-tag size="small" :type="row.status == 1 ? 'success' : row.status == 0 ? 'danger' : 'warning'">
          {{ row.status == 1 ? '正常' : row.status == 0 ? '已禁用' : '已限制' }}
        </el-tag>
      </template>
      <template #actions="{ row }">
        <el-button link type="primary" @click="openDetail(row)">详情</el-button>
        <el-button link type="primary" @click="openEdit(row)">编辑</el-button>
        <el-button link :type="row.status == 0 ? 'success' : 'warning'"
          @click="toggleStatus(row)">{{ row.status == 0 ? '启用' : '禁用' }}</el-button>
        <el-button link type="info" @click="resetPwd(row)">重置密码</el-button>
        <el-button v-if="row.status === 2" link type="success" @click="openLiftRestrict(row)">解除限制</el-button>
        <el-button v-else link type="danger" @click="openRestrict(row)">限制</el-button>
      </template>
    </DataTable>

    <!-- 创建学院管理员 -->
    <ModalForm ref="createModal" title="创建学院管理员"
      :fields="createFields" @submit="doCreateAdmin" />

    <!-- 编辑管理员 -->
    <el-dialog v-model="editVisible" title="编辑管理员" width="460px">
      <el-form ref="editForm" :model="editData" label-width="110px">
        <el-form-item label="姓名">
          <el-input v-model="editData.real_name" placeholder="真实姓名" />
        </el-form-item>
        <el-form-item label="手机号">
          <el-input v-model="editData.phone" placeholder="手机号" />
        </el-form-item>
        <el-form-item label="角色" prop="role" :rules="[{required:true}]">
          <el-radio-group v-model="editData.role">
            <el-radio value="college_admin">学院管理员</el-radio>
            <el-radio value="club_admin">社长/副社长</el-radio>
            <el-radio value="club_member">社团成员</el-radio>
            <el-radio value="general_student">学生</el-radio>
          </el-radio-group>
        </el-form-item>
        <el-form-item label="所属学院" v-if="editData.role === 'college_admin'">
          <el-select v-model="editData.college_id" placeholder="请选择学院" style="width:100%">
            <el-option v-for="c in colleges" :key="c.college_id" :label="c.college_name" :value="c.college_id" />
          </el-select>
        </el-form-item>
        <el-form-item label="所属社团" v-if="editData.role === 'club_admin' || editData.role === 'club_member'">
          <el-select v-model="editData.club_id" placeholder="请选择社团" style="width:100%" @focus="loadClubMembers">
            <el-option v-for="c in allClubs" :key="c.club_id" :label="c.club_name" :value="c.club_id" />
          </el-select>
        </el-form-item>
      </el-form>
      <template #footer>
        <el-button @click="editVisible = false">取消</el-button>
        <el-button type="primary" @click="doEdit">确认保存</el-button>
      </template>
    </el-dialog>

    <!-- 用户详情 -->
    <el-dialog v-model="detailVisible" :title="current?.real_name" width="560px">
      <el-descriptions :column="2" border v-if="current">
        <el-descriptions-item label="用户名">{{ current.username }}</el-descriptions-item>
        <el-descriptions-item label="角色">{{ roleLabel(current.role) }}</el-descriptions-item>
        <el-descriptions-item label="姓名">{{ current.real_name }}</el-descriptions-item>
        <el-descriptions-item label="学院">{{ current.college_name }}</el-descriptions-item>
        <el-descriptions-item label="学号/工号">{{ current.student_id }}</el-descriptions-item>
        <el-descriptions-item label="手机">{{ current.phone }}</el-descriptions-item>
        <el-descriptions-item label="最后登录">{{ current.last_login }}</el-descriptions-item>
        <el-descriptions-item label="状态">{{ current.status }}</el-descriptions-item>
      </el-descriptions>
    </el-dialog>

    <!-- 限制用户 -->
    <el-dialog v-model="restrictVisible" title="限制用户" width="420px">
      <el-form ref="restrictForm" :model="restrictData" label-width="110px">
        <el-form-item label="限制类型" prop="restriction_type" :rules="[{required:true}]">
          <el-select v-model="restrictData.restriction_type" style="width:100%">
            <el-option label="禁止加入社团" value="ban_join" />
            <el-option label="禁止参加活动" value="ban_activity" />
          </el-select>
        </el-form-item>
        <el-form-item label="限制原因" prop="reason" :rules="[{required:true}]">
          <el-input v-model="restrictData.reason" type="textarea" :rows="2" />
        </el-form-item>
      </el-form>
      <template #footer>
        <el-button @click="restrictVisible = false">取消</el-button>
        <el-button type="danger" @click="doRestrict">确认限制</el-button>
      </template>
    </el-dialog>

    <!-- 解除限制 -->
    <el-dialog v-model="liftVisible" title="解除限制" width="420px">
      <p style="margin-bottom:12px">用户：<b>{{ current?.real_name }}</b></p>
      <el-table :data="activeRestrictions" max-height="240" style="width:100%">
        <el-table-column prop="restriction_type" label="限制类型" width="120">
          <template #default="{ row }">
            {{ row.restriction_type === 'ban_join' ? '禁止加入社团' : '禁止参加活动' }}
          </template>
        </el-table-column>
        <el-table-column prop="reason" label="原因" />
        <el-table-column label="操作" width="80">
          <template #default="{ row }">
            <el-button link type="danger" @click="doLiftRestrict(row.restriction_id)">解除</el-button>
          </template>
        </el-table-column>
      </el-table>
      <el-empty v-if="activeRestrictions.length === 0" description="该用户当前没有生效中的限制" />
    </el-dialog>
  </div>
</template>

<script setup>
import { ref, onMounted } from 'vue'
import { api } from '@/api'
import { ElMessage, ElMessageBox } from 'element-plus'
import FilterBar from '@/components/FilterBar.vue'
import DataTable from '@/components/DataTable.vue'
import ModalForm from '@/components/ModalForm.vue'

const loading = ref(false)
const tableData = ref([])
const total = ref(0)
const page = ref(1)
const colleges = ref([])
const filters = ref({ keyword: '', role: '', college_id: null, status: '' })
const current = ref(null)
const detailVisible = ref(false)
const restrictVisible = ref(false)
const restrictForm = ref()
const restrictData = ref({ restriction_type: '', reason: '' })
const liftVisible = ref(false)
const activeRestrictions = ref([])
const createModal = ref()
const editVisible = ref(false)
const editForm = ref()
const editData = ref({ role: 'college_admin', college_id: null, club_id: null })
const clubMembers = ref([])
const allClubs = ref([])

const columns = [
  { prop: 'username',    label: '用户名',  width: 120 },
  { prop: 'real_name',  label: '姓名',    width: 100 },
  { slot: 'role',        label: '角色',    width: 110 },
  { prop: 'college_name', label: '学院',   width: 120 },
  { prop: 'phone',       label: '手机',    width: 120 },
  { slot: 'status',      label: '状态',    width: 90 },
  { prop: 'last_login',  label: '最后登录', width: 160 },
  { slot: 'actions',     label: '操作',    width: 220, fixed: 'right' }
]

const createFields = [
  { prop: 'username',   label: '用户名', placeholder: '登录账号' },
  { prop: 'password',   label: '密码',   type: 'password' },
  { prop: 'real_name',  label: '姓名' },
  { prop: 'college_id', label: '学院',   type: 'select',
    options: [] /* 在 onMounted 后填充 */ },
  { prop: 'phone',      label: '手机号', placeholder: '选填' }
]

const roleLabel = r => ({ school_admin: '学校管理员', college_admin: '学院管理员', general_student: '学生', club_member: '社团成员', club_admin: '社长' }[r] || r)
const roleType  = r => ({ school_admin: 'danger', college_admin: 'warning', general_student: '', club_member: 'info', club_admin: 'success' }[r] || '')

async function loadData() {
  loading.value = true
  const res = await api.get('/api/school/users', { page: page.value, page_size: 10, ...filters.value })
  loading.value = false
  if (res.code === 0) { tableData.value = res.data.list || []; total.value = res.data.total || 0 }
}
function onReset() { filters.value = { keyword: '', role: '', college_id: null, status: '' }; loadData() }
function onPage(e) { page.value = e.page; loadData() }
function openDetail(row) { current.value = row; detailVisible.value = true }

async function toggleStatus(row) {
  const action = row.status == 0 ? '启用' : '禁用'
  await ElMessageBox.confirm(`确认${action}用户 ${row.real_name}？`, '提示', { type: 'warning' })
  const res = await api.post('/api/school/users/' + row.user_id + '/toggle')
  if (res.code === 0) {
    // 乐观更新：立即切换本地状态，确保UI即时响应
    row.status = row.status == 0 ? 1 : 0
    ElMessage.success(action + '成功')
    loadData()
  } else { ElMessage.error(res.msg || '操作失败') }
}

async function resetPwd(row) {
  await ElMessageBox.confirm(`确认重置 ${row.real_name} 的密码为 scms123456 ？`, '提示', { type: 'warning' })
  const res = await api.post('/api/school/users/' + row.user_id + '/reset-password')
  if (res.code === 0) ElMessage.success('密码已重置为 scms123456')
  else ElMessage.error(res.msg)
}

function openRestrict(row) { current.value = row; restrictData.value = { restriction_type: '', reason: '' }; restrictVisible.value = true }

async function doRestrict() {
  await restrictForm.value.validate()
  const res = await api.post('/api/school/users/' + current.value.user_id + '/restrict', restrictData.value)
  if (res.code === 0) { ElMessage.success('已限制'); restrictVisible.value = false; loadData() }
  else ElMessage.error(res.msg || '限制失败')
}

async function openLiftRestrict(row) {
  current.value = row
  const res = await api.get('/api/school/restrictions', { user_id: row.user_id })
  activeRestrictions.value = res.code === 0 ? (res.data.list || []) : []
  liftVisible.value = true
}

async function doLiftRestrict(restriction_id) {
  await ElMessageBox.confirm('确认解除该限制？', '提示', { type: 'warning' })
  const res = await api.post('/api/school/users/' + current.value.user_id + '/lift-restriction', { restriction_id })
  if (res.code === 0) {
    ElMessage.success('已解除限制')
    // 从列表中移除已解除的
    activeRestrictions.value = activeRestrictions.value.filter(r => r.restriction_id !== restriction_id)
    if (activeRestrictions.value.length === 0) liftVisible.value = false
    loadData()
  } else {
    ElMessage.error(res.msg || '解除失败')
  }
}

function openCreateAdmin() {
  createFields[3].options = colleges.value.map(c => ({ label: c.college_name, value: c.college_id }))
  createModal.value.open({})
}

function openEdit(row) {
  let role = 'general_student'
  if (row.role === 'college_admin') role = 'college_admin'
  else if (row.role === 'club_admin') role = 'club_admin'
  else if (row.role === 'club_member') role = 'club_member'
  editData.value = {
    role,
    college_id: row.college_id || null,
    club_id: null,
    real_name: row.real_name || '',
    phone: row.phone || ''
  }
  current.value = row
  editVisible.value = true
}

async function loadClubMembers() {
  const res = await api.get('/api/school/clubs')
  if (res.code === 0) allClubs.value = res.data.list || []
}

async function doEdit() {
  const row = current.value
  const res = await api.post('/api/school/users/' + row.user_id + '/set-role', editData.value)
  if (res.code === 0) { ElMessage.success('修改成功'); editVisible.value = false; loadData() }
  else ElMessage.error(res.msg)
}

async function doCreateAdmin(data) {
  const res = await api.post('/api/school/users/college-admin', data)
  if (res.code === 0) { ElMessage.success('创建成功'); createModal.value.close(); loadData() }
  else ElMessage.error(res.msg)
}

onMounted(async () => {
  const res = await api.get('/api/colleges')
  if (res.code === 0) colleges.value = res.data.list || []
  loadData()
})
</script>
