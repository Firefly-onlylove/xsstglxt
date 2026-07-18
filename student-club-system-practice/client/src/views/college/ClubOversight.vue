<template>
  <div>
    <div class="page-header"><span class="page-title">本院社团监督</span></div>
    <FilterBar @search="loadData" @reset="onReset">
      <el-input v-model="filters.keyword" placeholder="社团名称" clearable style="width:200px" />
    </FilterBar>
    <DataTable :data="tableData" :columns="columns" :total="total" :loading="loading"
      @page-change="onPage">
      <template #actions="{ row }">
        <el-button link type="primary" @click="openDetail(row)">详情</el-button>
        <el-button link type="success" @click="openMembers(row)">成员</el-button>
      </template>
    </DataTable>

    <!-- 详情弹窗 -->
    <el-dialog v-model="detailVisible" :title="current?.name" width="560px">
      <el-descriptions :column="2" border v-if="current">
        <el-descriptions-item label="类型">{{ current.club_type }}</el-descriptions-item>
        <el-descriptions-item label="等级">{{ current.level || '未评级' }}</el-descriptions-item>
        <el-descriptions-item label="社长">{{ current.president_name }}</el-descriptions-item>
        <el-descriptions-item label="成员数">{{ current.member_count }}</el-descriptions-item>
        <el-descriptions-item label="活动数">{{ current.activity_count }}</el-descriptions-item>
        <el-descriptions-item label="成立时间">{{ current.created_at }}</el-descriptions-item>
        <el-descriptions-item label="简介" :span="2">{{ current.description }}</el-descriptions-item>
      </el-descriptions>
    </el-dialog>

    <!-- 成员弹窗 -->
    <el-dialog v-model="membersVisible" :title="current?.name + ' — 成员列表'" width="600px">
      <el-table :data="members" size="small" border>
        <el-table-column prop="real_name" label="姓名" width="100" />
        <el-table-column prop="student_id" label="学号" width="120" />
        <el-table-column prop="class_name" label="班级" />
        <el-table-column prop="role" label="角色" width="100">
          <template #default="{ row }">
            <el-tag size="small" :type="row.role==='president'?'danger':row.role==='vice_president'?'warning':''">
              {{ roleLabel(row.role) }}
            </el-tag>
          </template>
        </el-table-column>
        <el-table-column prop="join_date" label="加入时间" width="120" />
      </el-table>
    </el-dialog>
  </div>
</template>

<script setup>
import { ref, onMounted } from 'vue'
import { api } from '@/api'
import FilterBar from '@/components/FilterBar.vue'
import DataTable from '@/components/DataTable.vue'

const loading = ref(false)
const tableData = ref([])
const total = ref(0)
const page = ref(1)
const filters = ref({ keyword: '' })
const current = ref(null)
const detailVisible = ref(false)
const membersVisible = ref(false)
const members = ref([])

const columns = [
  { prop: 'name',           label: '社团名称' },
  { prop: 'club_type',      label: '类型',   width: 80 },
  { prop: 'president_name', label: '社长',   width: 100 },
  { prop: 'member_count',   label: '成员数', width: 80 },
  { prop: 'activity_count', label: '活动数', width: 80 },
  { prop: 'created_at',     label: '成立时间', width: 120 },
  { slot: 'actions',        label: '操作',   width: 140, fixed: 'right' }
]

const roleLabel = r => ({ president: '社长', vice_president: '副社长', member: '成员' }[r] || r)

async function loadData() {
  loading.value = true
  const res = await api.get('/api/college/clubs', { page: page.value, page_size: 10, ...filters.value })
  loading.value = false
  if (res.code === 0) { tableData.value = res.data.list || []; total.value = res.data.total || 0 }
}
function onReset() { filters.value = { keyword: '' }; loadData() }
function onPage(e) { page.value = e.page; loadData() }
function openDetail(row) { current.value = row; detailVisible.value = true }
async function openMembers(row) {
  current.value = row
  const res = await api.get('/api/college/clubs/' + row.club_id + '/members')
  if (res.code === 0) members.value = res.data || []
  membersVisible.value = true
}
onMounted(loadData)
</script>
