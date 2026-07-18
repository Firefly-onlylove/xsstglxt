<template>
  <div>
    <div class="page-header"><span class="page-title">活动中心</span></div>
    <FilterBar @search="loadData" @reset="onReset">
      <el-select v-model="filters.status" placeholder="状态" clearable style="width:130px">
        <el-option label="报名中" value="published" />
        <el-option label="进行中" value="ongoing" />
        <el-option label="已结束" value="finished" />
      </el-select>
      <el-select v-model="filters.club_id" placeholder="所属社团" clearable style="width:180px">
        <el-option v-for="c in myClubs" :key="c.club_id" :label="c.club_name" :value="c.club_id" />
      </el-select>
    </FilterBar>
    <div v-loading="loading">
      <el-empty v-if="!loading && tableData.length === 0" description="暂无活动" />
      <el-row :gutter="16">
        <el-col :span="8" v-for="act in tableData" :key="act.activity_id" style="margin-bottom:16px">
          <div class="act-card" @click="router.push('/student/activity/'+act.activity_id)">
            <div class="act-header">
              <el-tag size="small" :type="actStatusType(act.status)">{{ actStatusLabel(act.status) }}</el-tag>
              <el-tag size="small" type="info" v-if="act.my_status">{{ myStatusLabel(act.my_status) }}</el-tag>
            </div>
            <div class="act-title">{{ act.title }}</div>
            <div class="act-club">{{ act.club_name }}</div>
            <div class="act-info">
              <el-icon><Calendar /></el-icon> {{ act.start_time }}
            </div>
            <div class="act-info">
              <el-icon><Location /></el-icon> {{ act.location }}
            </div>
            <div class="act-footer">
              <span>报名 {{ act.current_count }}/{{ act.max_participants }}</span>
            </div>
          </div>
        </el-col>
      </el-row>
      <div class="pagination-wrap" v-if="total > 0">
        <el-pagination v-model:current-page="page" :page-size="9" :total="total" layout="prev, pager, next" @current-change="loadData" />
      </div>
    </div>
  </div>
</template>

<script setup>
import { ref, onMounted } from 'vue'
import { useRouter } from 'vue-router'
import { api } from '@/api'
import FilterBar from '@/components/FilterBar.vue'

const router = useRouter()
const loading = ref(false)
const tableData = ref([])
const total = ref(0)
const page = ref(1)
const myClubs = ref([])
const filters = ref({ status: '', club_id: null })

const actStatusLabel = s => ({ published:'报名中', ongoing:'进行中', finished:'已结束', cancelled:'已取消' }[s] || s)
const actStatusType  = s => ({ published:'primary', ongoing:'success', finished:'info', cancelled:'danger' }[s] || '')
const myStatusLabel  = s => ({ registered:'已报名', signed_in:'已签到', cancelled:'已取消' }[s] || s)

async function loadData() {
  loading.value = true
  const res = await api.get('/api/activities', { page: page.value, page_size: 9, ...filters.value })
  loading.value = false
  if (res.code === 0) { tableData.value = res.data.list || []; total.value = res.data.total || 0 }
}
function onReset() { filters.value = { status: '', club_id: null }; loadData() }

onMounted(async () => {
  const res = await api.get('/api/my/clubs')
  if (res.code === 0) myClubs.value = res.data.list || []
  loadData()
})
</script>

<style scoped>
.act-card {
  background: #fff; border-radius: 8px; padding: 16px; cursor: pointer;
  box-shadow: 0 1px 4px rgba(0,0,0,.06); transition: all .2s;
}
.act-card:hover { transform: translateY(-2px); box-shadow: 0 4px 16px rgba(22,119,255,.12); }
.act-header { display: flex; gap: 6px; margin-bottom: 8px; }
.act-title { font-size: 15px; font-weight: 600; margin-bottom: 4px; }
.act-club { font-size: 13px; color: #1677FF; margin-bottom: 8px; }
.act-info { font-size: 12px; color: #86909C; display: flex; align-items: center; gap: 4px; margin-bottom: 4px; }
.act-footer { margin-top: 8px; font-size: 12px; color: #86909C; }
.pagination-wrap { text-align: center; margin-top: 24px; }
</style>
