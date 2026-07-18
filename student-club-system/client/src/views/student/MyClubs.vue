<template>
  <div>
    <div class="page-header"><span class="page-title">我的社团</span></div>
    <div v-loading="loading">
      <el-empty v-if="!loading && clubs.length === 0" description="还没有加入任何社团" />
      <el-row :gutter="16">
        <el-col :span="8" v-for="c in clubs" :key="c.club_id" style="margin-bottom:16px">
          <div class="club-card">
            <div style="display:flex;align-items:center;gap:14px;margin-bottom:12px">
              <div style="width:50px;height:50px;background:#E6F4FF;border-radius:10px;display:flex;align-items:center;justify-content:center;flex-shrink:0">
                <el-icon style="font-size:26px;color:#1677FF"><OfficeBuilding /></el-icon>
              </div>
              <div>
                <div style="font-size:16px;font-weight:600">{{ c.club_name }}</div>
                <el-tag size="small" :type="roleType(c.role)">{{ roleLabel(c.role) }}</el-tag>
              </div>
            </div>
            <div style="font-size:12px;color:#86909C;margin-bottom:12px">
              加入时间：{{ c.join_date }} &nbsp;|&nbsp; 成员：{{ c.member_count }}人 &nbsp;|&nbsp; {{ c.college_name }}
            </div>
            <div style="display:flex;gap:8px">
              <el-button size="small" @click="router.push('/student/club-detail/'+c.club_id)">查看详情</el-button>
              <el-button size="small" type="warning"
                v-if="c.role === 'president' || c.role === 'vice_president'"
                @click="router.push('/club-admin/dashboard')">
                管理后台
              </el-button>
            </div>
          </div>
        </el-col>
      </el-row>
      <el-divider v-if="applications.length > 0" content-position="left">申请记录</el-divider>
      <el-table v-if="applications.length > 0" :data="applications" border size="small">
        <el-table-column prop="club_name" label="社团名称" />
        <el-table-column prop="created_at" label="申请时间" width="160" />
        <el-table-column prop="status" label="状态" width="100">
          <template #default="{ row }">
            <el-tag :type="row.status==='pending'?'warning':row.status==='rejected'?'danger':'success'" size="small">
              {{ row.status==='pending'?'审核中':row.status==='rejected'?'已拒绝':'已通过' }}
            </el-tag>
          </template>
        </el-table-column>
        <el-table-column prop="reject_reason" label="拒绝原因" show-overflow-tooltip />
      </el-table>
    </div>
  </div>
</template>

<script setup>
import { ref, onMounted } from 'vue'
import { useRouter } from 'vue-router'
import { api } from '@/api'

const router = useRouter()
const loading = ref(false)
const clubs = ref([])
const applications = ref([])

const roleLabel = r => ({ president:'社长', vice_president:'副社长', member:'普通成员' }[r] || r)
const roleType  = r => ({ president:'danger', vice_president:'warning', member:'' }[r] || '')

onMounted(async () => {
  loading.value = true
  const [clubRes, appRes] = await Promise.all([
    api.get('/api/my/clubs'),
    api.get('/api/my/applications')
  ])
  loading.value = false
  if (clubRes.code === 0) clubs.value = clubRes.data.list || []
  if (appRes.code === 0) applications.value = appRes.data.list || []
})
</script>

<style scoped>
.club-card { background: #fff; border-radius: 8px; padding: 16px; box-shadow: 0 1px 4px rgba(0,0,0,.06); }
</style>
