<template>
  <div>
    <div class="page-header"><span class="page-title">本院学生管理</span></div>
    <FilterBar @search="loadData" @reset="onReset">
      <el-input v-model="filters.keyword" placeholder="姓名/学号" clearable style="width:180px" />
      <el-select v-model="filters.class_id" placeholder="按班级筛选" clearable style="width:180px">
        <el-option v-for="c in classes" :key="c.class_id" :label="c.class_name" :value="c.class_id" />
      </el-select>
      <el-select v-model="filters.status" placeholder="状态" clearable style="width:110px">
        <el-option label="正常" value="active" />
        <el-option label="已限制" value="restricted" />
      </el-select>
    </FilterBar>
    <DataTable :data="tableData" :columns="columns" :total="total" :loading="loading"
      @page-change="onPage">
      <template #role="{ row }">
        <el-tag size="small" :type="roleType(row.role)">{{ roleLabel(row.role) }}</el-tag>
      </template>
      <template #status="{ row }">
        <el-tag size="small" :type="(row.restrictions && row.restrictions !== '' && row.restrictions !== 'NULL') ? 'danger' : 'success'">
          {{ (row.restrictions && row.restrictions !== '' && row.restrictions !== 'NULL') ? '已限制' : '正常' }}
        </el-tag>
      </template>
      <template #actions="{ row }">
        <el-button link type="danger" @click="openRestrict(row)" v-if="!row.restrictions || row.restrictions === '' || row.restrictions === 'NULL'">限制</el-button>
        <el-button link type="success" @click="removeRestrict(row)" v-else>解除限制</el-button>
      </template>
    </DataTable>

    <!-- 限制弹窗 -->
    <el-dialog v-model="restrictVisible" title="限制学生" width="420px">
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
  </div>
</template>

<script setup>
import { ref, onMounted } from 'vue'
import { api } from '@/api'
import { ElMessage, ElMessageBox } from 'element-plus'
import FilterBar from '@/components/FilterBar.vue'
import DataTable from '@/components/DataTable.vue'

const loading = ref(false)
const tableData = ref([])
const total = ref(0)
const page = ref(1)
const classes = ref([])
const filters = ref({ keyword: '', class_id: null, status: '' })
const current = ref(null)
const restrictVisible = ref(false)
const restrictForm = ref()
const restrictData = ref({ restriction_type: '', reason: '' })

import { USER_ROLE_LABEL, USER_ROLE_TYPE, roleLabel as _rl, roleType as _rt } from '@/composables/useRole'
const roleLabel = r => _rl(USER_ROLE_LABEL, r)
const roleType  = r => _rt(USER_ROLE_TYPE, r)

const columns = [
  { prop: 'student_no',  label: '学号',     width: 120 },
  { prop: 'real_name',   label: '姓名',     width: 100 },
  { slot: 'role',        label: '角色',     width: 90 },
  { prop: 'username',    label: '用户名',   width: 120 },
  { prop: 'phone',       label: '手机号',   width: 120 },
  { prop: 'class_name',  label: '班级',     width: 160 },
  { prop: 'last_login',  label: '最后登录', width: 160 },
  { slot: 'status',      label: '状态',     width: 90 },
  { slot: 'actions',     label: '操作',     width: 140, fixed: 'right' }
]

async function loadData() {
  loading.value = true
  const res = await api.get('/api/college/students', { page: page.value, page_size: 10, ...filters.value })
  loading.value = false
  if (res.code === 0) { tableData.value = res.data.list || []; total.value = res.data.total || 0 }
}
function onReset() { filters.value = { keyword: '', class_id: null, status: '' }; loadData() }
function onPage(e) { page.value = e.page; loadData() }
function openRestrict(row) { current.value = row; restrictData.value = { restriction_type: '', reason: '' }; restrictVisible.value = true }
async function doRestrict() {
  await restrictForm.value.validate()
  const res = await api.post('/api/college/students/' + current.value.user_id + '/restrict', restrictData.value)
  if (res.code === 0) { ElMessage.success('已限制'); restrictVisible.value = false; loadData() }
}
async function removeRestrict(row) {
  await ElMessageBox.confirm('确认解除该学生的限制？', '提示', { type: 'warning' })
  const res = await api.post('/api/college/students/' + row.user_id + '/lift-restriction')
  if (res.code === 0) { ElMessage.success('已解除'); loadData() }
}
onMounted(async () => {
  const res = await api.get('/api/college/classes')
  if (res.code === 0) classes.value = res.data.list || []
  loadData()
})
</script>
