<template>
  <div class="dashboard">
    <!-- 头部问候 -->
    <div class="welcome-bar">
      <div>
        <h2 class="greeting">社团管理系统</h2>
        <p class="subtitle">全校社团运营数据概览</p>
      </div>
      <div class="header-actions">
        <el-button type="primary" @click="router.push('/school/club-approval')" v-if="pendingCount > 0">
          待审批 ({{ pendingCount }})
        </el-button>
      </div>
    </div>

    <!-- 统计卡片 -->
    <el-row :gutter="20" class="stat-row">
      <el-col :span="6" v-for="(item, index) in statCards" :key="index">
        <div class="stat-card" :class="'stat-' + item.theme">
          <div class="stat-body">
            <div class="stat-info">
              <span class="stat-label">{{ item.label }}</span>
              <span class="stat-value">{{ item.value ?? '—' }}</span>
              <span class="stat-desc">{{ item.desc }}</span>
            </div>
            <div class="stat-icon-box">
              <el-icon :size="28"><component :is="item.icon" /></el-icon>
            </div>
          </div>
          <div class="stat-footer" :class="'bg-' + item.theme">
            <span>{{ item.footer }}</span>
          </div>
        </div>
      </el-col>
    </el-row>

    <!-- 中部双栏 -->
    <el-row :gutter="20" class="mid-row">
      <!-- 快捷入口 -->
      <el-col :span="8">
        <div class="panel">
          <div class="panel-header"><span>快捷操作</span></div>
          <div class="shortcut-grid">
            <div class="shortcut-item" v-for="s in shortcuts" :key="s.label"
              @click="router.push(s.path)">
              <div class="shortcut-icon" :style="{ background: s.bg }">
                <el-icon :size="20" :style="{ color: s.color }"><component :is="s.icon" /></el-icon>
              </div>
              <span>{{ s.label }}</span>
            </div>
          </div>
        </div>
      </el-col>

      <!-- 各学院数据 -->
      <el-col :span="16">
        <div class="panel">
          <div class="panel-header">
            <span>各学院社团概况</span>
            <el-button link type="primary" @click="router.push('/school/college-mgmt')">学院管理 →</el-button>
          </div>
          <el-table :data="collegeData" size="small" stripe style="width:100%" v-loading="collegeLoading">
            <el-table-column prop="college_name" label="学院" min-width="140" />
            <el-table-column prop="club_count" label="社团数" width="80" align="center" sortable />
            <el-table-column prop="member_count" label="成员数" width="80" align="center" sortable />
            <el-table-column prop="activity_count" label="活动数" width="80" align="center" />
            <el-table-column prop="total_income" label="收入" width="100" align="right">
              <template #default="{ row }">¥{{ (row.total_income || 0).toFixed(0) }}</template>
            </el-table-column>
            <el-table-column prop="total_expense" label="支出" width="100" align="right">
              <template #default="{ row }">¥{{ (row.total_expense || 0).toFixed(0) }}</template>
            </el-table-column>
          </el-table>
        </div>
      </el-col>
    </el-row>

    <!-- 底部操作日志 -->
    <div class="panel">
      <div class="panel-header">
        <span>最近操作记录</span>
        <span class="panel-hint">最近 20 条</span>
      </div>
      <el-table :data="logs" size="small" style="width:100%">
        <el-table-column prop="username" label="操作人" width="120" />
        <el-table-column prop="action" label="操作类型" width="180" />
        <el-table-column prop="target_type" label="对象" width="120" />
        <el-table-column prop="detail" label="详情" min-width="200" show-overflow-tooltip />
        <el-table-column prop="created_at" label="时间" width="170" />
      </el-table>
    </div>
  </div>
</template>

<script setup>
import { ref, onMounted, computed } from 'vue'
import { useRouter } from 'vue-router'
import { api } from '@/api'

const router = useRouter()
const logs = ref([])
const collegeData = ref([])
const collegeLoading = ref(false)

const statCards = ref([
  { label: '活跃社团', value: null, desc: '个', footer: '全校已批准社团', icon: 'OfficeBuilding', theme: 'blue' },
  { label: '社团成员', value: null, desc: '人', footer: '在籍成员总数', icon: 'UserFilled', theme: 'cyan' },
  { label: '本月活动', value: null, desc: '场', footer: '进行中的活动', icon: 'Calendar', theme: 'orange' },
  { label: '待审批', value: null, desc: '项', footer: '社团创建/报销待处理', icon: 'WarningFilled', theme: 'red' }
])

const pendingCount = computed(() => statCards.value[3].value || 0)

const shortcuts = [
  { label: '社团审批', path: '/school/club-approval', icon: 'Checked', color: '#1677FF', bg: '#E6F4FF' },
  { label: '用户管理', path: '/school/user-mgmt', icon: 'UserFilled', color: '#36CFC9', bg: '#E6FFFB' },
  { label: '数据统计', path: '/school/statistics', icon: 'TrendCharts', color: '#FF7D00', bg: '#FFF7E6' },
  { label: '财务监督', path: '/school/finance', icon: 'Money', color: '#F53F3F', bg: '#FFF1F0' },
  { label: '公告管理', path: '/school/announcement', icon: 'Bell', color: '#722ED1', bg: '#F9F0FF' },
  { label: '系统设置', path: '/school/system-config', icon: 'Setting', color: '#7C7C7C', bg: '#F5F5F5' }
]

onMounted(async () => {
  // 仪表盘数据
  const res = await api.get('/api/school/dashboard')
  if (res.code === 0) {
    const d = res.data
    statCards.value[0].value = d.active_clubs
    statCards.value[1].value = d.total_members
    statCards.value[2].value = d.monthly_activities
    statCards.value[3].value = d.pending_approvals
    logs.value = d.recent_logs || []
  }

  // 学院统计数据
  collegeLoading.value = true
  const statsRes = await api.get('/api/school/stats')
  if (statsRes.code === 0) {
    collegeData.value = statsRes.data.college_stats || []
  }
  collegeLoading.value = false
})
</script>

<style scoped>
.dashboard { max-width: 1280px; margin: 0 auto; }

/* 欢迎栏 */
.welcome-bar {
  display: flex; justify-content: space-between; align-items: flex-start;
  margin-bottom: 20px;
}
.greeting { font-size: 20px; font-weight: 700; margin: 0; color: #1D2129; }
.subtitle { font-size: 13px; color: #86909C; margin: 4px 0 0; }
.header-actions { display: flex; gap: 8px; }

/* 统计卡片 */
.stat-row { margin-bottom: 20px; }
.stat-card {
  border-radius: 10px; overflow: hidden;
  box-shadow: 0 1px 3px rgba(0,0,0,0.06);
  transition: box-shadow .2s, transform .2s;
}
.stat-card:hover { box-shadow: 0 4px 12px rgba(0,0,0,0.1); transform: translateY(-2px); }
.stat-body {
  display: flex; justify-content: space-between; align-items: center;
  padding: 20px 20px 16px; background: #fff;
}
.stat-info { display: flex; flex-direction: column; gap: 4px; }
.stat-label { font-size: 13px; color: #86909C; }
.stat-value { font-size: 30px; font-weight: 700; color: #1D2129; line-height: 1.2; }
.stat-desc { font-size: 12px; color: #C9CDD4; }
.stat-icon-box {
  width: 52px; height: 52px; border-radius: 12px; display: flex;
  align-items: center; justify-content: center;
}
.stat-blue .stat-icon-box { background: #E6F4FF; color: #1677FF; }
.stat-cyan .stat-icon-box { background: #E6FFFB; color: #36CFC9; }
.stat-orange .stat-icon-box { background: #FFF7E6; color: #FF7D00; }
.stat-red .stat-icon-box { background: #FFF1F0; color: #F53F3F; }

.stat-footer {
  padding: 6px 20px; font-size: 11px; color: rgba(0,0,0,0.45);
}
.bg-blue  { background: #F0F6FF; }
.bg-cyan  { background: #F0FCFA; }
.bg-orange { background: #FFFBF5; }
.bg-red   { background: #FFF5F5; }

/* 面板 */
.mid-row { margin-bottom: 20px; }
.panel {
  background: #fff; border-radius: 10px; padding: 20px;
  box-shadow: 0 1px 3px rgba(0,0,0,0.06);
  margin-bottom: 20px; height: 100%;
}
.panel-header {
  display: flex; justify-content: space-between; align-items: center;
  margin-bottom: 16px; font-size: 15px; font-weight: 600; color: #1D2129;
}
.panel-hint { font-size: 12px; color: #C9CDD4; font-weight: 400; }

/* 快捷入口 */
.shortcut-grid {
  display: grid; grid-template-columns: repeat(2, 1fr); gap: 12px;
}
.shortcut-item {
  display: flex; align-items: center; gap: 10px;
  padding: 14px; border-radius: 8px; cursor: pointer;
  background: #F7F8FA; font-size: 13px; color: #4E5969;
  transition: all .2s;
}
.shortcut-item:hover { background: #E6F4FF; color: #1677FF; }
.shortcut-icon {
  width: 36px; height: 36px; border-radius: 8px;
  display: flex; align-items: center; justify-content: center;
}
</style>
