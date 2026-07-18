<template>
  <div>
    <div class="page-header"><span class="page-title">社团管理仪表盘</span></div>
    <el-row :gutter="16" style="margin-bottom:20px">
      <el-col :span="6" v-for="c in stats" :key="c.label">
        <div class="stat-card" @click="c.path && router.push(c.path)" :style="{cursor: c.path ? 'pointer' : 'default'}">
          <div class="icon-wrap" :style="{background:c.bg}">
            <el-icon :style="{color:c.color,fontSize:'22px'}"><component :is="c.icon" /></el-icon>
          </div>
          <div><div class="label">{{c.label}}</div><div class="value">{{c.value??'-'}}</div></div>
        </div>
      </el-col>
    </el-row>
    <el-row :gutter="16">
      <el-col :span="12">
        <div class="card" style="margin-bottom:16px">
          <div style="display:flex;justify-content:space-between;margin-bottom:12px">
            <b>基本信息</b>
          </div>
          <el-descriptions :column="2" v-if="clubInfo">
            <el-descriptions-item label="社团名称">{{ clubInfo.club_name }}</el-descriptions-item>
            <el-descriptions-item label="类型">{{ clubInfo.category }}</el-descriptions-item>
            <el-descriptions-item label="等级">{{ clubInfo.level === 'school' ? '校级' : clubInfo.level === 'college' ? '院级' : clubInfo.level }}</el-descriptions-item>
            <el-descriptions-item label="成立时间">{{ clubInfo.created_at }}</el-descriptions-item>
            <el-descriptions-item label="简介" :span="2">{{ clubInfo.description }}</el-descriptions-item>
          </el-descriptions>
        </div>
        <div class="card">
          <b style="display:block;margin-bottom:12px">待处理事项</b>
          <el-row :gutter="12">
            <el-col :span="12">
              <div class="todo-item" @click="router.push('/club-admin/member-mgmt')">
                <span class="todo-count">{{ pendingJoins }}</span>
                <span class="todo-label">待审批入社</span>
              </div>
            </el-col>
            <el-col :span="12">
              <div class="todo-item" @click="router.push('/club-admin/finance')">
                <span class="todo-count">{{ pendingReimb }}</span>
                <span class="todo-label">待审批报销</span>
              </div>
            </el-col>
          </el-row>
        </div>
      </el-col>
      <el-col :span="12">
        <div class="card">
          <b style="display:block;margin-bottom:12px">最近活动</b>
          <div v-for="act in recentActivities" :key="act.activity_id" class="act-item"
            @click="router.push('/club-admin/activity-mgmt?focus=' + act.activity_id)">
            <div style="font-size:14px;font-weight:500">{{ act.title }}</div>
            <div style="font-size:12px;color:#86909C;margin-top:2px">{{ act.start_time }} · {{ act.location }}</div>
            <el-tag size="small" :type="actStatusType(act.status)" style="margin-top:4px">{{ actStatusLabel(act.status) }}</el-tag>
          </div>
          <el-empty v-if="recentActivities.length === 0" description="暂无活动" :image-size="60" />
        </div>
      </el-col>
    </el-row>
  </div>
</template>

<script setup>
import { ref, onMounted, watch } from 'vue'
import { useRouter } from 'vue-router'
import { api } from '@/api'
import { useClub } from '@/composables/useClub'

const router = useRouter()
const { clubId } = useClub()
const stats = ref([
  { label: '社团成员',   value: null, icon: 'UserFilled',     color: '#1677FF', bg: '#E6F4FF', path: '/club-admin/member-mgmt' },
  { label: '活动管理',   value: null, icon: 'Calendar',       color: '#36CFC9', bg: '#E6FFFB', path: '/club-admin/activity-mgmt' },
  { label: '活动总数',   value: null, icon: 'TrendCharts',    color: '#FF7D00', bg: '#FFF7E6' },
  { label: '进行中活动', value: null, icon: 'VideoPlay',      color: '#722ED1', bg: '#F9F0FF' }
])
const clubInfo = ref(null)
const recentActivities = ref([])
const pendingJoins = ref(0)
const pendingReimb = ref(0)

const actStatusLabel = s => ({ draft:'草稿', published:'报名中', ongoing:'进行中', finished:'已结束', cancelled:'已取消' }[s] || s)
const actStatusType  = s => ({ published:'primary', ongoing:'success', finished:'info', cancelled:'danger' }[s] || '')

async function loadDashboard() {
  if (!clubId.value) return
  const res = await api.get('/api/club/' + clubId.value + '/dashboard')
  if (res.code === 0) {
    const d = res.data
    stats.value[0].value = d.member_count
    stats.value[1].value = d.new_members_this_month
    stats.value[2].value = d.activity_count
    stats.value[3].value = d.ongoing_activities
    clubInfo.value = d.club_info?.[0] || null
    recentActivities.value = d.recent_activities || []
    pendingJoins.value = d.pending_joins || 0
    pendingReimb.value = d.pending_reimb || 0
  }
}

onMounted(() => {
  if (clubId.value) loadDashboard()
  else {
    const stop = watch(clubId, (val) => { if (val) { stop(); loadDashboard() } })
  }
})
</script>

<style scoped>
.todo-item { background: #F7F8FA; border-radius: 8px; padding: 16px; text-align: center; cursor: pointer; transition: all .2s; }
.todo-item:hover { background: #E6F4FF; }
.todo-count { display: block; font-size: 28px; font-weight: 700; color: #F53F3F; margin-bottom: 4px; }
.todo-label { font-size: 13px; color: #86909C; }
.act-item { padding: 10px 0; border-bottom: 1px solid #F2F3F5; cursor: pointer; }
.act-item:hover { color: #1677FF; }
</style>
