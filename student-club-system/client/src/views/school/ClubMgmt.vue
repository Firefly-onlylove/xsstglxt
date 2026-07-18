<template>
  <div>
    <div class="page-header">
      <span class="page-title">全校社团管理</span>
    </div>
    <FilterBar @search="loadData" @reset="onReset">
      <el-input v-model="filters.keyword" placeholder="社团名称" clearable style="width:200px" />
      <el-select v-model="filters.college_id" placeholder="所属学院" clearable style="width:150px">
        <el-option v-for="c in colleges" :key="c.college_id" :label="c.name" :value="c.college_id" />
      </el-select>
      <el-select v-model="filters.status" placeholder="状态" clearable style="width:110px">
        <el-option label="正常" value="active" />
        <el-option label="已解散" value="dissolved" />
      </el-select>
    </FilterBar>
    <DataTable :data="tableData" :columns="columns" :total="total" :loading="loading"
      @page-change="onPage">
      <template #status="{ row }">
        <el-tag :type="row.status === 'active' ? 'success' : 'info'" size="small">
          {{ row.status === 'active' ? '正常' : '已解散' }}
        </el-tag>
      </template>
      <template #actions="{ row }">
        <el-button link type="primary" @click="openDetail(row)">详情</el-button>
        <el-button link type="warning" @click="openSetLevel(row)" v-if="row.status==='active'">设置等级</el-button>
        <el-button link type="danger" @click="dissolve(row)" v-if="row.status==='active'">解散</el-button>
      </template>
    </DataTable>

    <!-- 详情弹窗 -->
    <el-dialog v-model="detailVisible" :title="current?.name" width="600px">
      <el-descriptions :column="2" border v-if="current">
        <el-descriptions-item label="类型">{{ current.club_type }}</el-descriptions-item>
        <el-descriptions-item label="等级">{{ current.level || '未评级' }}</el-descriptions-item>
        <el-descriptions-item label="学院">{{ current.college_name }}</el-descriptions-item>
        <el-descriptions-item label="社长">{{ current.president_name }}</el-descriptions-item>
        <el-descriptions-item label="成员数">{{ current.member_count }}</el-descriptions-item>
        <el-descriptions-item label="活动数">{{ current.activity_count }}</el-descriptions-item>
        <el-descriptions-item label="成立时间">{{ current.created_at }}</el-descriptions-item>
        <el-descriptions-item label="简介" :span="2">{{ current.description }}</el-descriptions-item>
      </el-descriptions>
    </el-dialog>

    <!-- 设置等级 -->
    <el-dialog v-model="levelVisible" title="设置社团等级" width="360px">
      <el-form>
        <el-form-item label="等级">
          <el-select v-model="levelVal" style="width:100%">
            <el-option label="一星" :value="1" /><el-option label="二星" :value="2" />
            <el-option label="三星" :value="3" /><el-option label="四星" :value="4" />
            <el-option label="五星" :value="5" />
          </el-select>
        </el-form-item>
      </el-form>
      <template #footer>
        <el-button @click="levelVisible = false">取消</el-button>
        <el-button type="primary" @click="doSetLevel">确认</el-button>
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
const levelVal      = ref(1)

const columns = [
  { prop: 'name',           label: '社团名称' },
  { prop: 'college_name',   label: '学院',   width: 120 },
  { prop: 'president_name', label: '社长',   width: 100 },
  { prop: 'member_count',   label: '成员数', width: 80 },
  { prop: 'level',          label: '等级',   width: 80 },
  { slot: 'status',         label: '状态',   width: 90 },
  { prop: 'created_at',     label: '创建时间', width: 120 },
  { slot: 'actions',        label: '操作',   width: 200, fixed: 'right' }
]

async function loadData() {
  loading.value = true
  const res = await api.get('/api/school/clubs', { page: page.value, page_size: 10, ...filters.value })
  loading.value = false
  if (res.code === 0) { tableData.value = res.data.list || []; total.value = res.data.total || 0 }
}
function onReset() { filters.value = { keyword: '', college_id: null, status: '' }; loadData() }
function onPage(e) { page.value = e.page; loadData() }
function openDetail(row) { current.value = row; detailVisible.value = true }
function openSetLevel(row) { current.value = row; levelVal.value = row.level || 1; levelVisible.value = true }

async function doSetLevel() {
  const res = await api.post('/api/school/clubs/' + current.value.club_id + '/level', { level: levelVal.value })
  if (res.code === 0) { ElMessage.success('设置成功'); levelVisible.value = false; loadData() }
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
