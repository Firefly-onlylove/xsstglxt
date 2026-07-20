<template>
  <div>
    <div class="page-header"><span class="page-title">本院社团</span></div>
    <FilterBar :filters="filters" @search="loadData" placeholder="搜索社团名称" />
    <DataTable :data="tableData" :columns="columns" :total="total" :loading="loading"
      @page-change="e=>{page=e.page;loadData()}">
      <template #role="{ row }">
        <el-tag size="small" :type="row.role==='president'?'danger':row.role==='vice_president'?'warning':''">
          {{ roleLabel(row.role) }}
        </el-tag>
      </template>
      <template #created_at="{ row }">{{ row.created_at?.substring?.(0, 10) || row.created_at }}</template>
      <template #actions="{ row }">
        <el-button link type="primary" @click="viewMembers(row)" v-if="row.member_count>0">成员</el-button>
        <el-button link @click="openDetail(row)">详情</el-button>
      </template>
    </DataTable>

    <!-- 详情弹窗 -->
    <el-dialog v-model="detailVisible" :title="current?.club_name || '社团详情'" width="480px">
      <el-descriptions v-if="current" :column="1" border size="small">
        <el-descriptions-item label="社长">{{ current.president_name }}</el-descriptions-item>
        <el-descriptions-item label="成员数">{{ current.member_count }}</el-descriptions-item>
        <el-descriptions-item label="活动数">{{ current.activity_count }}</el-descriptions-item>
        <el-descriptions-item label="成立时间">{{ (current.created_at||'').substring(0,10) }}</el-descriptions-item>
      </el-descriptions>
    </el-dialog>

    <!-- 成员弹窗 -->
    <el-dialog v-model="membersVisible" :title="current?.club_name + ' — 成员'" width="600px">
      <el-table :data="members" size="small" border>
        <el-table-column prop="real_name" label="姓名" width="100" />
        <el-table-column prop="student_no" label="学号" width="120" />
        <el-table-column label="角色" width="90">
          <template #default="{ row }">{{ roleLabel(row.role) }}</template>
        </el-table-column>
        <el-table-column prop="join_date" label="加入时间" />
      </el-table>
    </el-dialog>
  </div>
</template>

<script setup>
import { ref } from 'vue'
import { ElMessageBox } from 'element-plus'
import { api } from '@/api'
import FilterBar from '@/components/FilterBar.vue'
import DataTable from '@/components/DataTable.vue'
import { CLUB_ROLE_LABEL, roleLabel as _rl } from '@/composables/useRole'

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
  { prop: 'club_name',      label: '社团名称' },
  { prop: 'category',       label: '类型',   width: 80 },
  { prop: 'president_name', label: '社长',   width: 100 },
  { prop: 'member_count',   label: '成员数', width: 80 },
  { prop: 'activity_count', label: '活动数', width: 80 },
  { prop: 'created_at',     label: '成立时间', width: 120 },
  { slot: 'actions',        label: '操作',   width: 140, fixed: 'right' }
]

const roleLabel = r => _rl(CLUB_ROLE_LABEL, r)

async function loadData() {
  loading.value = true
  const res = await api.get('/api/college/clubs', { page: page.value, keyword: filters.value.keyword })
  loading.value = false
  if (res.code === 0) { tableData.value = res.data.list || []; total.value = res.data.total || 0 }
}

function openDetail(row) {
  current.value = row
  detailVisible.value = true
}

async function viewMembers(row) {
  current.value = row
  const res = await api.get('/api/college/clubs/' + row.club_id + '/members')
  if (res.code === 0) members.value = res.data.list || []
  membersVisible.value = true
}
</script>
