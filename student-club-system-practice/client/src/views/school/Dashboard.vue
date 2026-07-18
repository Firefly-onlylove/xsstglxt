<template>
  <div>
    <div class="page-header">
      <span class="page-title">学校管理员仪表盘</span>
    </div>
    <el-row :gutter="16" style="margin-bottom:20px">
      <el-col :span="6" v-for="c in stats" :key="c.label">
        <div class="stat-card">
          <div class="icon-wrap" :style="{ background: c.bg }">
            <el-icon :style="{ color: c.color, fontSize:'22px' }"><component :is="c.icon" /></el-icon>
          </div>
          <div>
            <div class="label">{{ c.label }}</div>
            <div class="value">{{ c.value ?? '-' }}</div>
          </div>
        </div>
      </el-col>
    </el-row>

    <el-row :gutter="16">
      <el-col :span="12">
        <div class="card" style="margin-bottom:16px">
          <div class="section-title">快捷操作</div>
          <el-row :gutter="12" style="margin-top:12px">
            <el-col :span="8" v-for="btn in shortcuts" :key="btn.label">
              <div class="shortcut-btn" @click="router.push(btn.path)">
                <el-icon><component :is="btn.icon" /></el-icon>
                <span>{{ btn.label }}</span>
              </div>
            </el-col>
          </el-row>
        </div>
      </el-col>
      <el-col :span="12">
        <div class="card">
          <div class="section-title">最近操作日志</div>
          <el-table :data="logs" size="small" style="margin-top:12px">
            <el-table-column prop="username" label="用户" width="100" />
            <el-table-column prop="action" label="操作" />
            <el-table-column prop="created_at" label="时间" width="160" />
          </el-table>
        </div>
      </el-col>
    </el-row>
  </div>
</template>

<script setup>
import { ref, onMounted } from 'vue'
import { useRouter } from 'vue-router'
import { api } from '@/api'

const router = useRouter()
const stats = ref([
  { label: '活跃社团数', value: null, icon: 'OfficeBuilding', color: '#1677FF', bg: '#E6F4FF' },
  { label: '社团成员数', value: null, icon: 'UserFilled',     color: '#36CFC9', bg: '#E6FFFB' },
  { label: '本月活动数', value: null, icon: 'Calendar',       color: '#FF7D00', bg: '#FFF7E6' },
  { label: '待审批',     value: null, icon: 'Warning',        color: '#F53F3F', bg: '#FFF1F0' }
])
const shortcuts = [
  { label: '社团审批', path: '/school/club-approval', icon: 'Checked' },
  { label: '用户管理', path: '/school/user-mgmt',     icon: 'UserFilled' },
  { label: '数据统计', path: '/school/statistics',    icon: 'TrendCharts' },
  { label: '公告管理', path: '/school/announcement',  icon: 'Bell' },
  { label: '系统设置', path: '/school/system-config', icon: 'Setting' },
  { label: '财务监督', path: '/school/finance',       icon: 'Money' }
]
const logs = ref([])

onMounted(async () => {
  const res = await api.get('/api/school/dashboard')
  if (res.code === 0) {
    const d = res.data
    stats.value[0].value = d.active_clubs
    stats.value[1].value = d.total_members
    stats.value[2].value = d.monthly_activities
    stats.value[3].value = d.pending_approvals
    logs.value = d.recent_logs || []
  }
})
</script>

<style scoped>
.section-title { font-size: 15px; font-weight: 600; color: #1D2129; }
.shortcut-btn {
  display: flex; flex-direction: column; align-items: center; gap: 6px;
  padding: 16px 0; border-radius: 8px; cursor: pointer; background: #F7F8FA;
  font-size: 13px; color: #1D2129; transition: all .2s;
}
.shortcut-btn:hover { background: #E6F4FF; color: #1677FF; }
.shortcut-btn .el-icon { font-size: 22px; }
</style>
