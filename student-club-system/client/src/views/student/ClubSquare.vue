<template>
  <div>
    <div class="page-header"><span class="page-title">社团广场</span>
      <el-button type="primary" @click="router.push('/student/create-club')">申请创建社团</el-button>
    </div>
    <FilterBar @search="loadData" @reset="onReset">
      <el-input v-model="filters.keyword" placeholder="搜索社团名称" clearable style="width:220px">
        <template #prefix><el-icon><Search /></el-icon></template>
      </el-input>
      <el-select v-model="filters.category" placeholder="类型" clearable style="width:120px">
        <el-option v-for="t in clubTypes" :key="t" :label="t" :value="t" />
      </el-select>
      <el-select v-model="filters.college_id" placeholder="学院" clearable style="width:160px">
        <el-option v-for="c in colleges" :key="c.college_id" :label="c.college_name" :value="c.college_id" />
      </el-select>
    </FilterBar>

    <div v-loading="loading">
      <el-empty v-if="!loading && tableData.length === 0" description="暂无社团" />
      <el-row :gutter="16">
        <el-col :span="6" v-for="club in tableData" :key="club.club_id" style="margin-bottom:16px">
          <div class="club-card" @click="router.push('/student/club-detail/' + club.club_id)">
            <div class="club-logo">
              <el-icon style="font-size:36px;color:#1677FF"><OfficeBuilding /></el-icon>
            </div>
            <div class="club-name">{{ club.club_name }}</div>
            <div class="club-tags">
              <el-tag size="small" style="margin-right:4px">{{ club.category }}</el-tag>
              <el-tag size="small" type="info">{{ club.college_name || '校级社团' }}</el-tag>
              <el-tag size="small" :type="club.level === 'school' ? '' : 'success'" style="margin-left:4px">{{ club.level === 'school' ? '校级' : '院级' }}</el-tag>
            </div>
            <div class="club-desc">{{ club.description || '暂无简介' }}</div>
            <div style="margin-bottom:8px">
              <el-button v-if="!club.join_status || club.join_status === 'not_joined'"
                type="primary" size="small" @click.stop="joinClub(club)">申请加入</el-button>
            </div>
            <div class="club-footer">
              <span><el-icon><UserFilled /></el-icon> {{ club.member_count }}</span>
              <span><el-icon><Calendar /></el-icon> {{ club.activity_count }}</span>
              <el-tag size="small" :type="joinStatusType(club.join_status)">
                {{ joinStatusLabel(club.join_status) }}
              </el-tag>
            </div>
          </div>
        </el-col>
      </el-row>
      <div class="pagination-wrap" v-if="total > 0">
        <el-pagination v-model:current-page="page" :page-size="12" :total="total" layout="prev, pager, next" @current-change="loadData" />
      </div>
    </div>
  </div>
</template>

<script setup>
import { ref, onMounted } from 'vue'
import { useRouter } from 'vue-router'
import { api } from '@/api'
import { ElMessage } from 'element-plus'
import FilterBar from '@/components/FilterBar.vue'

const router = useRouter()
const loading = ref(false)
const tableData = ref([])
const total = ref(0)
const page = ref(1)
const colleges = ref([])
const filters = ref({ keyword: '', category: '', college_id: null })
const clubTypes = ['学术', '文艺', '体育', '公益', '实践', '其他']

const joinStatusLabel = s => ({ not_joined: '可加入', pending: '审核中', approved: '已加入', joined: '已加入' }[s] || '可加入')
const joinStatusType  = s => ({ not_joined: 'primary', pending: 'warning', approved: 'success', joined: 'success' }[s] || 'primary')

async function loadData() {
  loading.value = true
  const res = await api.get('/api/clubs', { page: page.value, page_size: 12, ...filters.value })
  loading.value = false
  if (res.code === 0) { tableData.value = res.data.list || []; total.value = res.total || res.data.total || 0 }
}
function onReset() { filters.value = { keyword: '', category: '', college_id: null }; loadData() }

async function joinClub(club) {
  const res = await api.post('/api/clubs/' + club.club_id + '/join')
  if (res.code === 0) {
    ElMessage.success('申请已提交')
    club.join_status = 'pending'
  } else {
    ElMessage.error(res.msg)
  }
}

onMounted(async () => {
  const res = await api.get('/api/colleges')
  if (res.code === 0) colleges.value = res.data.list || []
  loadData()
})
</script>

<style scoped>
.club-card {
  background: #fff; border-radius: 8px; padding: 20px; cursor: pointer;
  box-shadow: 0 1px 4px rgba(0,0,0,.06); transition: all .2s; height: 100%;
}
.club-card:hover { transform: translateY(-2px); box-shadow: 0 4px 16px rgba(22,119,255,.15); }
.club-logo { text-align: center; margin-bottom: 12px; height: 48px; display: flex; align-items: center; justify-content: center; }
.club-name { font-size: 16px; font-weight: 600; text-align: center; margin-bottom: 8px; }
.club-tags { text-align: center; margin-bottom: 8px; }
.club-desc { font-size: 13px; color: #86909C; margin-bottom: 12px; display: -webkit-box; -webkit-line-clamp: 2; -webkit-box-orient: vertical; overflow: hidden; }
.club-footer { display: flex; align-items: center; justify-content: space-between; font-size: 13px; color: #86909C; }
.club-footer .el-icon { margin-right: 2px; vertical-align: middle; }
.pagination-wrap { text-align: center; margin-top: 24px; }
</style>
