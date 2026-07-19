<template>
  <div>
    <div class="page-header"><span class="page-title">学院管理仪表盘</span></div>
    <el-row :gutter="16" style="margin-bottom:20px">
      <el-col :span="6" v-for="c in stats" :key="c.label">
        <div class="stat-card">
          <div class="icon-wrap" :style="{background:c.bg}">
            <el-icon :style="{color:c.color,fontSize:'22px'}"><component :is="c.icon" /></el-icon>
          </div>
          <div><div class="label">{{c.label}}</div><div class="value">{{c.value??'-'}}</div></div>
        </div>
      </el-col>
    </el-row>
    <el-row :gutter="16">
      <el-col :span="14">
        <div class="card">
          <div class="section-title" style="margin-bottom:12px">本院社团列表</div>
          <el-table :data="clubs" size="small" border>
            <el-table-column prop="club_name" label="社团名称" />
            <el-table-column prop="president_name" label="社长" width="90" />
            <el-table-column prop="member_count" label="成员数" width="80" />
            <el-table-column label="操作" width="80">
              <template #default="{ row }">
                <el-button link type="primary" @click="router.push('/college/club-oversight')">查看</el-button>
              </template>
            </el-table-column>
          </el-table>
        </div>
      </el-col>
      <el-col :span="10">
        <div class="card">
          <div class="section-title" style="margin-bottom:12px">待审批报销</div>
          <el-table :data="pendingReimb" size="small" border>
            <el-table-column prop="club_name" label="社团" />
            <el-table-column prop="amount" label="金额" width="80" />
            <el-table-column label="操作" width="80">
              <template #default>
                <el-button link type="primary" @click="router.push('/college/finance-approval')">审批</el-button>
              </template>
            </el-table-column>
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
  { label: '本院社团数', value: null, icon: 'OfficeBuilding', color: '#1677FF', bg: '#E6F4FF' },
  { label: '本院学生数', value: null, icon: 'UserFilled',     color: '#36CFC9', bg: '#E6FFFB' },
  { label: '活动总数',   value: null, icon: 'Calendar',       color: '#FF7D00', bg: '#FFF7E6' },
  { label: '待审批报销', value: null, icon: 'Warning',        color: '#F53F3F', bg: '#FFF1F0' }
])
const clubs = ref([])
const pendingReimb = ref([])
onMounted(async () => {
  const res = await api.get('/api/college/dashboard')
  if (res.code === 0) {
    const d = res.data
    stats.value[0].value = d.club_count
    stats.value[1].value = d.student_count
    stats.value[2].value = d.activity_count
    stats.value[3].value = d.pending_reimb
  }
  // 额外加载社团列表
  const cr = await api.get('/api/college/clubs')
  if (cr.code === 0) clubs.value = cr.data.list || cr.data || []
  // 额外加载待审批报销
  const pr = await api.get('/api/college/reimbursements/pending')
  if (pr.code === 0) pendingReimb.value = pr.data.list || pr.data || []
})
</script>
