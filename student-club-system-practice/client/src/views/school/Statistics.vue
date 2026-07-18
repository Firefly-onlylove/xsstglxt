<template>
  <div>
    <div class="page-header"><span class="page-title">数据统计</span>
      <el-button @click="exportData"><el-icon><Download /></el-icon>导出报表</el-button>
    </div>
    <el-row :gutter="16" style="margin-bottom:20px">
      <el-col :span="6" v-for="c in summary" :key="c.label">
        <div class="stat-card">
          <div class="icon-wrap" :style="{background:c.bg}">
            <el-icon :style="{color:c.color,fontSize:'22px'}"><component :is="c.icon" /></el-icon>
          </div>
          <div><div class="label">{{c.label}}</div><div class="value">{{c.value ?? '-'}}</div></div>
        </div>
      </el-col>
    </el-row>
    <el-row :gutter="16">
      <el-col :span="12">
        <div class="card" style="margin-bottom:16px">
          <div class="section-title" style="margin-bottom:12px">各学院社团数量</div>
          <el-table :data="collegeStats" size="small" border>
            <el-table-column prop="college_name" label="学院" />
            <el-table-column prop="club_count" label="社团数" width="90" />
            <el-table-column prop="member_count" label="成员数" width="90" />
            <el-table-column prop="activity_count" label="活动数" width="90" />
          </el-table>
        </div>
      </el-col>
      <el-col :span="12">
        <div class="card" style="margin-bottom:16px">
          <div class="section-title" style="margin-bottom:12px">社团成员规模排名 TOP 10</div>
          <el-table :data="topClubs" size="small" border>
            <el-table-column type="index" width="50" />
            <el-table-column prop="name" label="社团名称" />
            <el-table-column prop="college_name" label="学院" width="120" />
            <el-table-column prop="member_count" label="成员数" width="80" />
          </el-table>
        </div>
      </el-col>
    </el-row>
    <el-row :gutter="16">
      <el-col :span="12">
        <div class="card">
          <div class="section-title" style="margin-bottom:12px">社团类型分布</div>
          <el-table :data="typeStats" size="small" border>
            <el-table-column prop="club_type" label="类型" />
            <el-table-column prop="count" label="数量" width="80" />
            <el-table-column prop="percentage" label="占比" width="80" />
          </el-table>
        </div>
      </el-col>
      <el-col :span="12">
        <div class="card">
          <div class="section-title" style="margin-bottom:12px">近12月活动趋势</div>
          <el-table :data="monthlyActivity" size="small" border>
            <el-table-column prop="month" label="月份" />
            <el-table-column prop="activity_count" label="活动数" width="90" />
            <el-table-column prop="participant_count" label="参与人次" width="100" />
          </el-table>
        </div>
      </el-col>
    </el-row>
  </div>
</template>

<script setup>
import { ref, onMounted } from 'vue'
import { api } from '@/api'
import { ElMessage } from 'element-plus'

const summary       = ref([
  { label: '社团总数',   value: null, icon: 'OfficeBuilding', color: '#1677FF', bg: '#E6F4FF' },
  { label: '成员总数',   value: null, icon: 'UserFilled',     color: '#36CFC9', bg: '#E6FFFB' },
  { label: '活动总数',   value: null, icon: 'Calendar',       color: '#FF7D00', bg: '#FFF7E6' },
  { label: '本月新增社团', value: null, icon: 'Plus',         color: '#722ED1', bg: '#F9F0FF' }
])
const collegeStats   = ref([])
const topClubs       = ref([])
const typeStats      = ref([])
const monthlyActivity = ref([])

async function exportData() {
  const res = await api.get('/api/school/statistics/export')
  if (res.code === 0) ElMessage.success('报表已导出')
}

onMounted(async () => {
  const res = await api.get('/api/school/statistics')
  if (res.code === 0) {
    const d = res.data
    summary.value[0].value = d.total_clubs
    summary.value[1].value = d.total_members
    summary.value[2].value = d.total_activities
    summary.value[3].value = d.new_clubs_this_month
    collegeStats.value   = d.college_stats || []
    topClubs.value       = d.top_clubs || []
    typeStats.value      = d.type_distribution || []
    monthlyActivity.value = d.monthly_activities || []
  }
})
</script>
