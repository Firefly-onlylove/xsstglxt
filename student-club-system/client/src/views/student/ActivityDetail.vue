<template>
  <div v-loading="loading">
    <el-breadcrumb style="margin-bottom:16px">
      <el-breadcrumb-item @click="router.push('/student/activity-center')" style="cursor:pointer">活动中心</el-breadcrumb-item>
      <el-breadcrumb-item>{{ activity?.title }}</el-breadcrumb-item>
    </el-breadcrumb>
    <el-row :gutter="20" v-if="activity">
      <el-col :span="16">
        <div class="card" style="margin-bottom:16px">
          <div style="display:flex;justify-content:space-between;align-items:flex-start;margin-bottom:12px">
            <div>
              <el-tag :type="actStatusType(activity.status)" style="margin-right:8px">{{ actStatusLabel(activity.status) }}</el-tag>
              <span style="font-size:22px;font-weight:700">{{ activity.title }}</span>
            </div>
          </div>
          <el-descriptions :column="2" border>
            <el-descriptions-item label="所属社团">{{ activity.club_name }}</el-descriptions-item>
            <el-descriptions-item label="地点">{{ activity.location }}</el-descriptions-item>
            <el-descriptions-item label="开始时间">{{ activity.start_time }}</el-descriptions-item>
            <el-descriptions-item label="结束时间">{{ activity.end_time }}</el-descriptions-item>
            <el-descriptions-item label="报名截止">{{ activity.signup_deadline }}</el-descriptions-item>
            <el-descriptions-item label="人数上限">{{ activity.max_participants }}人</el-descriptions-item>
            <el-descriptions-item label="简介" :span="2">{{ activity.description }}</el-descriptions-item>
            <el-descriptions-item label="详细内容" :span="2" v-if="activity.content">{{ activity.content }}</el-descriptions-item>
          </el-descriptions>
          <div v-if="activity.summary" style="margin-top:16px;padding:16px;background:#F7F8FA;border-radius:8px">
            <div style="font-weight:600;margin-bottom:8px">活动总结</div>
            <div>{{ activity.summary }}</div>
          </div>
        </div>
      </el-col>
      <el-col :span="8">
        <!-- 我的状态 -->
        <div class="card" style="margin-bottom:16px">
          <div style="font-size:15px;font-weight:600;margin-bottom:16px">我的状态</div>
          <div v-if="!myStatus || myStatus === 'not_registered'">
            <el-button type="primary" style="width:100%" :disabled="activity.status !== 'published'"
              @click="doRegister">
              {{ activity.status === 'published' ? '立即报名' : '报名未开放' }}
            </el-button>
          </div>
          <div v-else-if="myStatus === 'registered'">
            <el-tag type="success" style="width:100%;text-align:center;height:40px;line-height:40px;font-size:14px">已报名</el-tag>
            <div v-if="activity.status === 'ongoing'" style="margin-top:12px">
              <el-input v-model="signinCode" placeholder="输入签到码" style="margin-bottom:8px" />
              <el-button type="primary" style="width:100%" :loading="signing" @click="doSignin">签到</el-button>
            </div>
            <el-button style="width:100%;margin-top:8px" type="danger" plain
              v-if="activity.status === 'published'" @click="cancelRegister">取消报名</el-button>
          </div>
          <div v-else-if="myStatus === 'signed_in'">
            <el-result icon="success" title="签到成功" sub-title="您已成功签到" style="padding:20px 0" />
          </div>
          <div v-else-if="myStatus === 'cancelled'">
            <el-tag type="info" style="width:100%;text-align:center">已取消报名</el-tag>
          </div>
        </div>

        <!-- 报名列表 -->
        <div class="card">
          <div style="font-size:15px;font-weight:600;margin-bottom:12px">
            已报名（{{ registrations.length }}/{{ activity.max_participants }}）
          </div>
          <div v-for="r in registrations.slice(0,6)" :key="r.user_id" style="display:flex;align-items:center;gap:8px;margin-bottom:8px">
            <el-avatar :size="28" :style="{ background: '#1677FF', fontSize:'12px' }">{{ r.real_name[0] }}</el-avatar>
            <span style="font-size:13px">{{ r.real_name }}</span>
            <el-tag v-if="r.status === 'signed_in'" type="success" size="small">已签到</el-tag>
          </div>
          <div v-if="registrations.length > 6" style="font-size:12px;color:#86909C;text-align:center;margin-top:4px">
            共 {{ registrations.length }} 人报名
          </div>
        </div>
      </el-col>
    </el-row>
  </div>
</template>

<script setup>
import { ref, onMounted } from 'vue'
import { useRoute, useRouter } from 'vue-router'
import { api } from '@/api'
import { ElMessage, ElMessageBox } from 'element-plus'

const route = useRoute()
const router = useRouter()
const loading = ref(false)
const signing = ref(false)
const activity = ref(null)
const myStatus = ref('not_registered')
const registrations = ref([])
const signinCode = ref('')

const actStatusLabel = s => ({ draft:'草稿', published:'报名中', ongoing:'进行中', finished:'已结束', cancelled:'已取消' }[s] || s)
const actStatusType  = s => ({ published:'primary', ongoing:'success', finished:'info', cancelled:'danger' }[s] || '')

async function doRegister() {
  const res = await api.post('/api/activities/' + route.params.id + '/register')
  if (res.code === 0) { ElMessage.success('报名成功'); myStatus.value = 'registered'; loadData() }
  else ElMessage.error(res.msg)
}

async function cancelRegister() {
  await ElMessageBox.confirm('确认取消报名？', '提示', { type: 'warning' })
  const res = await api.post('/api/activities/' + route.params.id + '/cancel')
  if (res.code === 0) { ElMessage.success('已取消'); myStatus.value = 'cancelled' }
}

async function doSignin() {
  if (!signinCode.value) { ElMessage.warning('请输入签到码'); return }
  signing.value = true
  const res = await api.post('/api/activities/' + route.params.id + '/signin', { code: signinCode.value })
  signing.value = false
  if (res.code === 0) { ElMessage.success('签到成功！'); myStatus.value = 'signed_in' }
  else ElMessage.error(res.msg || '签到码错误')
}

async function loadData() {
  loading.value = true
  const id = route.params.id
  const [actRes, regRes] = await Promise.all([
    api.get('/api/activities/' + id),
    api.get('/api/activities/' + id + '/registrations')
  ])
  loading.value = false
  if (actRes.code === 0) { activity.value = actRes.data; myStatus.value = actRes.data.my_status || 'not_registered' }
  if (regRes.code === 0) registrations.value = regRes.data.list || []
}

onMounted(loadData)
</script>
