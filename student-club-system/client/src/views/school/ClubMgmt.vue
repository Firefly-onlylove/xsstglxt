<template>
  <div>
    <div class="page-header">
      <span class="page-title">全校社团管理</span>
    </div>
    <FilterBar @search="loadData" @reset="onReset">
      <el-input v-model="filters.keyword" placeholder="社团名称" clearable style="width:200px" />
      <el-select v-model="filters.college_id" placeholder="所属学院" clearable style="width:150px">
        <el-option v-for="c in colleges" :key="c.college_id" :label="c.college_name" :value="c.college_id" />
      </el-select>
      <el-select v-model="filters.status" placeholder="状态" clearable style="width:110px">
        <el-option label="正常" value="approved" />
        <el-option label="已解散" value="dissolved" />
      </el-select>
    </FilterBar>
    <DataTable :data="tableData" :columns="columns" :total="total" :loading="loading"
      @page-change="onPage">
      <template #level="{ row }">
        <el-tag size="small" :type="row.level === 'school' ? 'danger' : 'warning'">
          {{ row.level === 'school' ? '校级' : row.level === 'college' ? '院级' : row.level || '-' }}
        </el-tag>
      </template>
      <template #status="{ row }">
        <el-tag :type="row.status === 'approved' ? 'success' : 'info'" size="small">
          {{ row.status === 'approved' ? '正常' : row.status === 'dissolved' ? '已解散' : row.status }}
        </el-tag>
      </template>
      <template #actions="{ row }">
        <el-button link type="primary" @click="openDetail(row)">详情</el-button>
        <el-button link type="warning" @click="openSetLevel(row)" v-if="row.status==='approved'">设置等级</el-button>
        <el-button link type="primary" @click="openRename(row)" v-if="row.status==='approved'">重命名</el-button>
        <el-button link type="danger" @click="dissolve(row)" v-if="row.status==='approved'">解散</el-button>
      </template>
    </DataTable>

    <!-- 详情弹窗 -->
    <el-dialog v-model="detailVisible" :title="current?.club_name" width="600px">
      <el-descriptions :column="2" border v-if="current">
        <el-descriptions-item label="类型">{{ current.category }}</el-descriptions-item>
        <el-descriptions-item label="级别">{{ current.level === 'school' ? '校级' : current.level === 'college' ? '院级' : '未评级' }}</el-descriptions-item>
        <el-descriptions-item label="学院">{{ current.college_name }}</el-descriptions-item>
        <el-descriptions-item label="社长">{{ current.president }}</el-descriptions-item>
        <el-descriptions-item label="成员数">{{ current.member_count }}</el-descriptions-item>
        <el-descriptions-item label="加入权限">{{ current.join_permission === 'college_only' ? '仅本学院' : '全校开放' }}</el-descriptions-item>
        <el-descriptions-item label="指导老师">{{ current.advisor || '-' }}</el-descriptions-item>
        <el-descriptions-item label="状态">{{ current.status === 'approved' ? '正常' : current.status === 'dissolved' ? '已解散' : current.status }}</el-descriptions-item>
        <el-descriptions-item label="简介" :span="2">{{ current.description }}</el-descriptions-item>
      </el-descriptions>
    </el-dialog>

    <!-- 设置等级 -->
    <el-dialog v-model="levelVisible" title="设置社团级别" width="420px">
      <el-form>
        <el-form-item label="级别">
          <el-select v-model="levelForm.level" style="width:100%">
            <el-option label="校级" value="school" />
            <el-option label="院级" value="college" />
          </el-select>
        </el-form-item>
        <el-form-item label="所属学院" v-if="levelForm.level === 'college'">
          <el-select v-model="levelForm.college_id" placeholder="请选择学院" style="width:100%">
            <el-option v-for="c in colleges" :key="c.college_id" :label="c.college_name" :value="c.college_id" />
          </el-select>
        </el-form-item>
      </el-form>
      <template #footer>
        <el-button @click="levelVisible = false">取消</el-button>
        <el-button type="primary" @click="doSetLevel">确认</el-button>
      </template>
    </el-dialog>

    <!-- 重命名 -->
    <el-dialog v-model="renameVisible" title="重命名社团" width="360px">
      <el-form>
        <el-form-item label="新名称">
          <el-input v-model="renameName" placeholder="请输入新的社团名称" />
        </el-form-item>
      </el-form>
      <template #footer>
        <el-button @click="renameVisible = false">取消</el-button>
        <el-button type="primary" @click="doRename">确认</el-button>
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

const loading      = ref(false)
const tableData    = ref([])
const total        = ref(0)
const page         = ref(1)
const colleges     = ref([])
const filters      = ref({ keyword: '', college_id: null, status: '' })
const current      = ref(null)
const detailVisible = ref(false)
const levelVisible  = ref(false)
const levelForm     = ref({ level: 'school', college_id: null })
const renameVisible = ref(false)
const renameName    = ref('')

const columns = [
  { prop: 'club_name',     label: '社团名称' },
  { prop: 'college_name',  label: '学院',   width: 120 },
  { prop: 'president',     label: '社长',   width: 100 },
  { prop: 'member_count',  label: '成员数', width: 80 },
  { slot: 'level',         label: '等级',   width: 80 },
  { slot: 'status',        label: '状态',   width: 90 },
  { prop: 'created_at',    label: '创建时间', width: 120 },
  { slot: 'actions',       label: '操作',   width: 240, fixed: 'right' }
]

async function loadData() {
  loading.value = true
  const res = await api.get('/api/school/clubs', { page: page.value, page_size: 10, ...filters.value })
  loading.value = false
  if (res.code === 0) { tableData.value = res.data.list || []; total.value = res.data.total || 0 }
}
function onReset() { filters.value = { keyword: '', college_id: null, status: '' }; loadData() }
function onPage(e) { page.value = e.page; loadData() }
async function openDetail(row) {
  loading.value = true
  const res = await api.get('/api/school/clubs/' + row.club_id)
  loading.value = false
  if (res.code === 0) {
    current.value = { ...row, ...(Array.isArray(res.data) ? res.data[0] : res.data) }
  } else {
    current.value = row
  }
  detailVisible.value = true
}
function openSetLevel(row) {
  current.value = row
  levelForm.value = { level: row.level || 'school', college_id: null }
  levelVisible.value = true
}
function openRename(row) { current.value = row; renameName.value = row.club_name; renameVisible.value = true }

async function doSetLevel() {
  const body = { level: levelForm.value.level }
  if (levelForm.value.level === 'college') {
    if (!levelForm.value.college_id) { ElMessage.warning('院级社团必须指定所属学院'); return }
    body.college_id = levelForm.value.college_id
  }
  const res = await api.post('/api/school/clubs/' + current.value.club_id + '/level', body)
  if (res.code === 0) { ElMessage.success('设置成功'); levelVisible.value = false; loadData() }
  else ElMessage.error(res.msg)
}

async function doRename() {
  if (!renameName.value.trim()) { ElMessage.warning('请输入新名称'); return }
  const res = await api.post('/api/school/clubs/' + current.value.club_id + '/rename', { name: renameName.value.trim() })
  if (res.code === 0) { ElMessage.success('重命名成功'); renameVisible.value = false; loadData() }
  else ElMessage.error(res.msg)
}

async function dissolve(row) {
  const { value: reason } = await ElMessageBox.prompt('请输入解散理由', '解散社团', {
    confirmButtonText: '确认解散', cancelButtonText: '取消',
    inputValidator: v => v ? true : '请输入理由'
  })
  const res = await api.post('/api/school/clubs/' + row.club_id + '/dissolve', { reason })
  if (res.code === 0) { ElMessage.success('已解散'); loadData() }
  else ElMessage.error(res.msg)
}

onMounted(async () => {
  const res = await api.get('/api/school/colleges')
  if (res.code === 0) colleges.value = res.data.list || []
  loadData()
})
</script>
