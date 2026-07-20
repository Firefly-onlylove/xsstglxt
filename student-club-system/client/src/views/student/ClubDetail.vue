<template>
  <div>
    <el-breadcrumb style="margin-bottom:16px">
      <el-breadcrumb-item @click="router.push('/student/club-square')" style="cursor:pointer">社团广场</el-breadcrumb-item>
      <el-breadcrumb-item>{{ club?.club_name }}</el-breadcrumb-item>
    </el-breadcrumb>
    <div v-loading="loading">
      <el-row :gutter="20" v-if="club">
        <el-col :span="16">
          <div class="card" style="margin-bottom:16px">
            <div style="display:flex;gap:20px;align-items:flex-start">
              <div style="width:80px;height:80px;background:#E6F4FF;border-radius:12px;display:flex;align-items:center;justify-content:center;flex-shrink:0">
                <el-icon style="font-size:40px;color:#1677FF"><OfficeBuilding /></el-icon>
              </div>
              <div style="flex:1">
                <div style="font-size:22px;font-weight:700;margin-bottom:8px">{{ club.club_name }}</div>
                <div style="margin-bottom:8px">
                  <el-tag style="margin-right:8px">{{ club.category }}</el-tag>
                  <el-tag type="info" style="margin-right:8px">{{ club.college_name }}</el-tag>
                  <el-tag type="warning" v-if="club.level === 'school'">校级</el-tag>
                  <el-tag type="success" v-if="club.level === 'college'">院级</el-tag>
                </div>
                <div style="color:#86909C;font-size:14px;margin-bottom:12px">{{ club.description }}</div>
                <div style="font-size:13px;color:#86909C">
                  指导老师：{{ club.advisor || '暂无' }} &nbsp;|&nbsp; 社长：{{ club.president_name }} &nbsp;|&nbsp; 成员：{{ club.member_count }}人
                </div>
              </div>
            </div>
            <div style="margin-top:20px;display:flex;gap:12px">
              <el-button v-if="joinStatus === 'not_joined'" type="primary" @click="showJoinDialog = true">申请加入</el-button>
              <el-button v-if="joinStatus === 'pending'" disabled>审核中</el-button>
              <el-button v-if="joinStatus === 'joined' && !isPresident" type="danger" plain @click="leaveClub">退出社团</el-button>
              <el-button v-if="isPresident" type="warning" @click="router.push('/club-admin/dashboard')">管理后台</el-button>
            </div>
          </div>

          <el-tabs v-model="activeTab">
            <el-tab-pane label="近期活动" name="activities">
              <div v-for="act in activities" :key="act.activity_id" class="activity-item"
                @click="router.push('/student/activity/' + act.activity_id)">
                <div class="act-title">{{ act.title }}</div>
                <div class="act-meta">{{ act.start_time }} · {{ act.location }}</div>
                <el-tag size="small" :type="actStatusType(act.status)">{{ actStatusLabel(act.status) }}</el-tag>
              </div>
              <el-empty v-if="activities.length === 0" description="暂无活动" />
            </el-tab-pane>
          </el-tabs>
        </el-col>
        <el-col :span="8">
          <div class="card">
            <div style="font-size:15px;font-weight:600;margin-bottom:12px">成员列表</div>
            <div v-for="m in members" :key="m.member_id" class="member-item">
              <el-avatar :size="30" :style="{ background: roleColor(m.role) }">{{ m.real_name[0] }}</el-avatar>
              <div>
                <div style="font-size:14px">{{ m.real_name }}</div>
                <div style="font-size:12px;color:#86909C">{{ roleLabel(m.role) }} · {{ m.join_date }}</div>
              </div>
            </div>
            <el-empty v-if="members.length === 0" description="暂无成员" />
          </div>
        </el-col>
      </el-row>
    </div>

    <!-- 申请加入弹窗 -->
    <el-dialog v-model="showJoinDialog" title="请上传社团申请表" width="440px" :close-on-click-modal="false">
      <div style="text-align:center;padding:20px 0">
        <el-upload
          ref="uploadRef"
          :auto-upload="false"
          :limit="1"
          :on-change="handleFileChange"
          :on-remove="handleFileRemove"
          :before-upload="beforeUpload"
          accept=".png,.jpg,.jpeg,.pdf,.doc,.docx"
          drag
        >
          <el-icon class="el-icon--upload"><UploadFilled /></el-icon>
          <div class="el-upload__text">点击或将文件拖拽到此处上传</div>
          <template #tip>
            <div class="el-upload__tip">支持 PNG/JPG/PDF/DOC/DOCX 格式</div>
          </template>
        </el-upload>
      </div>
      <template #footer>
        <el-button @click="showJoinDialog = false">取消</el-button>
        <el-button type="primary" :loading="joining" @click="doJoinWithFile">提交申请</el-button>
      </template>
    </el-dialog>
  </div>
</template>

<script setup>
import { ref, onMounted, computed } from 'vue'
import { useRoute, useRouter } from 'vue-router'
import { api } from '@/api'
import { ElMessage, ElMessageBox } from 'element-plus'

const route = useRoute()
const router = useRouter()
const loading = ref(false)
const joining = ref(false)
const club = ref(null)
const members = ref([])
const activities = ref([])
const activeTab = ref('activities')
const joinStatus = ref('not_joined')
const showJoinDialog = ref(false)
const uploadRef = ref(null)
const selectedFile = ref(null)

const isPresident = computed(() => joinStatus.value === 'joined' && club.value?.my_role === 'president')

import { CLUB_ROLE_LABEL, roleLabel as _rl } from '@/composables/useRole'
const roleLabel = r => _rl(CLUB_ROLE_LABEL, r)
const roleColor = r => ({ president: '#F53F3F', vice_president: '#FF7D00', member: '#1677FF' }[r] || '#1677FF')
const actStatusLabel = s => ({ published:'报名中', ongoing:'进行中', finished:'已结束', cancelled:'已取消' }[s] || s)
const actStatusType  = s => ({ published:'primary', ongoing:'success', finished:'info', cancelled:'danger' }[s] || '')

function handleFileChange(file) {
  selectedFile.value = file.raw
}

function handleFileRemove() {
  selectedFile.value = null
}

function beforeUpload(file) {
  const validTypes = ['image/png', 'image/jpeg', 'image/jpg', 'application/pdf',
    'application/msword', 'application/vnd.openxmlformats-officedocument.wordprocessingml.document']
  if (!validTypes.includes(file.type)) {
    ElMessage.error('仅支持 PNG/JPG/PDF/DOC/DOCX 格式')
    return false
  }
  return true
}

async function doJoinWithFile() {
  joining.value = true
  const id = route.params.id
  const formData = new FormData()
  if (selectedFile.value) {
    formData.append('attachment', selectedFile.value)
  }
  const res = await api.upload('/api/clubs/' + id + '/join', formData)
  joining.value = false
  if (res.code === 0) {
    ElMessage.success('申请已提交，等待社长审批')
    joinStatus.value = 'pending'
    showJoinDialog.value = false
    selectedFile.value = null
  } else {
    ElMessage.error(res.message)
  }
}

async function leaveClub() {
  await ElMessageBox.confirm('确认退出该社团？', '提示', { type: 'warning' })
  const res = await api.post('/api/clubs/' + route.params.id + '/leave')
  if (res.code === 0) { ElMessage.success('已退出'); joinStatus.value = 'not_joined' }
}

onMounted(async () => {
  loading.value = true
  const id = route.params.id
  const [clubRes, membersRes, actRes] = await Promise.all([
    api.get('/api/clubs/' + id),
    api.get('/api/clubs/' + id + '/members'),
    api.get('/api/clubs/' + id + '/activities')
  ])
  loading.value = false
  if (clubRes.code === 0) { club.value = clubRes.data.list?.[0] || null; joinStatus.value = clubRes.data.list?.[0]?.join_status || 'not_joined' }
  if (membersRes.code === 0) members.value = membersRes.data.list || []
  if (actRes.code === 0) activities.value = actRes.data.list || []
})
</script>

<style scoped>
.activity-item { padding: 12px 0; border-bottom: 1px solid #F2F3F5; cursor: pointer; }
.activity-item:hover { color: #1677FF; }
.act-title { font-size: 14px; font-weight: 500; margin-bottom: 4px; }
.act-meta { font-size: 12px; color: #86909C; margin-bottom: 4px; }
.member-item { display: flex; align-items: center; gap: 10px; padding: 8px 0; border-bottom: 1px solid #F2F3F5; }
</style>
