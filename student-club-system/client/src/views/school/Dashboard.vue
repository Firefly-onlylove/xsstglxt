<template>
  <div class="dashboard">
    <!-- 头部 -->
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

    <!-- 主体区域：快捷操作 + 学院数据 等高三栏 -->
    <el-row :gutter="20" class="main-row">
      <!-- 快捷入口 -->
      <el-col :span="7">
        <div class="panel shortcut-panel">
          <div class="panel-header"><span>快捷操作</span></div>
          <div class="shortcut-grid">
            <div class="shortcut-item" v-for="s in shortcuts" :key="s.label"
              @click="router.push(s.path)">
              <div class="shortcut-icon" :style="{ background: s.bg }">
                <el-icon :size="24" :style="{ color: s.color }"><component :is="s.icon" /></el-icon>
              </div>
              <span>{{ s.label }}</span>
            </div>
          </div>
          <!-- 操作日志 -->
          <div class="log-section">
            <div class="panel-header" style="margin-top:8px">
              <span>最近操作</span>
              <span class="panel-hint">最近 20 条</span>
            </div>
            <div class="log-scroll">
              <div class="log-item" v-for="(l, i) in logs" :key="i">
                <span class="log-user">{{ l.username }}</span>
                <span class="log-action">{{ l.action }}</span>
                <span class="log-time">{{ l.created_at }}</span>
              </div>
              <el-empty v-if="logs.length === 0" description="暂无操作记录" :image-size="48" />
            </div>
          </div>
        </div>
      </el-col>

      <!-- 各学院数据 -->
      <el-col :span="17">
        <div class="panel college-panel">
          <div class="panel-header">
            <span>各学院社团概况</span>
          </div>
          <el-table :data="collegeData" size="small" stripe style="width:100%" v-loading="collegeLoading"
            :max-height="tableMaxHeight">
            <el-table-column prop="college_name" label="学院" min-width="120" />
            <el-table-column prop="club_count" label="社团数" width="70" align="center" sortable />
            <el-table-column prop="member_count" label="成员数" width="70" align="center" sortable />
            <el-table-column prop="activity_count" label="活动数" width="70" align="center" />
            <el-table-column prop="total_income" label="收入" width="90" align="right">
              <template #default="{ row }">¥{{ (row.total_income || 0).toFixed(0) }}</template>
            </el-table-column>
            <el-table-column prop="total_expense" label="支出" width="90" align="right">
              <template #default="{ row }">¥{{ (row.total_expense || 0).toFixed(0) }}</template>
            </el-table-column>
          </el-table>
        </div>
      </el-col>
    </el-row>
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
const tableMaxHeight = ref(400)

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

function calcHeight() {
  tableMaxHeight.value = Math.max(280, window.innerHeight - 520)
}

onMounted(async () => {
  calcHeight()
  window.addEventListener('resize', calcHeight)

  const res = await api.get('/api/school/dashboard')
  if (res.code === 0) {
    const d = res.data
    statCards.value[0].value = d.active_clubs
    statCards.value[1].value = d.total_members
    statCards.value[2].value = d.monthly_activities
    statCards.value[3].value = d.pending_approvals
    logs.value = d.recent_logs || []
  }

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
  margin-bottom: 16px;
}
.greeting { font-size: 20px; font-weight: 700; margin: 0; color: #1D2129; }
.subtitle { font-size: 13px; color: #86909C; margin: 4px 0 0; }
.header-actions { display: flex; gap: 8px; }

/* 统计卡片 */
.stat-row { margin-bottom: 16px; }
.stat-card {
  border-radius: 10px; overflow: hidden;
  box-shadow: 0 1px 3px rgba(0,0,0,0.06);
  transition: box-shadow .2s, transform .2s;
}
.stat-card:hover { box-shadow: 0 4px 12px rgba(0,0,0,0.1); transform: translateY(-2px); }
.stat-body {
  display: flex; justify-content: space-between; align-items: center;
  padding: 16px 20px 12px; background: #fff;
}
.stat-info { display: flex; flex-direction: column; gap: 2px; }
.stat-label { font-size: 13px; color: #86909C; }
.stat-value { font-size: 28px; font-weight: 700; color: #1D2129; line-height: 1.2; }
.stat-desc { font-size: 12px; color: #C9CDD4; }
.stat-icon-box {
  width: 48px; height: 48px; border-radius: 12px; display: flex;
  align-items: center; justify-content: center;
}
.stat-blue .stat-icon-box { background: #E6F4FF; color: #1677FF; }
.stat-cyan .stat-icon-box { background: #E6FFFB; color: #36CFC9; }
.stat-orange .stat-icon-box { background: #FFF7E6; color: #FF7D00; }
.stat-red .stat-icon-box { background: #FFF1F0; color: #F53F3F; }

.stat-footer { padding: 5px 20px; font-size: 11px; color: rgba(0,0,0,0.45); }
.bg-blue  { background: #F0F6FF; }
.bg-cyan  { background: #F0FCFA; }
.bg-orange { background: #FFFBF5; }
.bg-red   { background: #FFF5F5; }

/* 主体等高布局 */
.main-row { margin-bottom: 0; }

.panel {
  background: #fff; border-radius: 10px; padding: 20px;
  box-shadow: 0 1px 3px rgba(0,0,0,0.06);
}
.panel-header {
  display: flex; justify-content: space-between; align-items: center;
  margin-bottom: 12px; font-size: 15px; font-weight: 600; color: #1D2129;
}
.panel-hint { font-size: 12px; color: #C9CDD4; font-weight: 400; }

/* 左侧面板 — flex 纵向撑满 */
.shortcut-panel {
  display: flex; flex-direction: column;
}
.college-panel {
  height: 100%;
}

/* 快捷入口 — 加大按钮 */
.shortcut-grid {
  display: grid; grid-template-columns: repeat(2, 1fr); gap: 10px;
  margin-bottom: 16px;
}
.shortcut-item {
  display: flex; align-items: center; gap: 12px;
  padding: 20px 16px; border-radius: 10px; cursor: pointer;
  background: #F7F8FA; font-size: 15px; font-weight: 500; color: #1D2129;
  transition: all .2s;
}
.shortcut-item:hover { background: #E6F4FF; color: #1677FF; transform: translateY(-1px); }
.shortcut-icon {
  width: 44px; height: 44px; border-radius: 10px;
  display: flex; align-items: center; justify-content: center;
  flex-shrink: 0;
}

/* 日志区域 — 可滚动小框 */
.log-section {
  flex: 1; display: flex; flex-direction: column; min-height: 0;
  border-top: 1px solid #F2F3F5;
  padding-top: 4px;
}
.log-scroll {
  flex: 1; overflow-y: auto; min-height: 0;
}
.log-scroll::-webkit-scrollbar { width: 4px; }
.log-scroll::-webkit-scrollbar-thumb { background: #E5E6EB; border-radius: 2px; }

.log-item {
  display: flex; align-items: center; gap: 8px;
  padding: 7px 0; border-bottom: 1px solid #FAFAFA;
  font-size: 12px;
}
.log-item:last-child { border-bottom: none; }
.log-user { color: #1D2129; font-weight: 500; min-width: 50px; }
.log-action { color: #4E5969; flex: 1; }
.log-time { color: #C9CDD4; font-size: 11px; white-space: nowrap; }
</style>
