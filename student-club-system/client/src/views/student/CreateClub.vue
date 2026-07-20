<template>
  <div>
    <div class="page-header"><span class="page-title">申请创建社团</span></div>
    <div class="card" style="max-width:680px;margin:0 auto">
      <el-form ref="formRef" :model="form" :rules="rules" label-width="100px">
        <el-form-item label="社团名称" prop="name">
          <el-input v-model="form.name" placeholder="请输入社团名称（5-30字）" />
        </el-form-item>
        <el-form-item label="社团类型" prop="club_type">
          <el-select v-model="form.club_type" style="width:100%">
            <el-option v-for="t in clubTypes" :key="t" :label="t" :value="t" />
          </el-select>
        </el-form-item>
        <el-form-item label="所属学院" prop="college_id">
          <el-select v-model="form.college_id" style="width:100%">
            <el-option v-for="c in colleges" :key="c.college_id" :label="c.college_name" :value="c.college_id" />
          </el-select>
        </el-form-item>
        <el-form-item label="社团简介" prop="description">
          <el-input v-model="form.description" type="textarea" :rows="4" placeholder="请介绍社团的主要活动、特色等" />
        </el-form-item>
        <el-form-item label="创建理由" prop="reason">
          <el-input v-model="form.reason" type="textarea" :rows="3" placeholder="为什么要创建这个社团？" />
        </el-form-item>
        <el-form-item label="指导老师">
          <el-input v-model="form.advisor" placeholder="选填" />
        </el-form-item>
        <el-form-item>
          <el-button type="primary" :loading="submitting" @click="doSubmit" style="width:200px">提交申请</el-button>
          <el-button @click="router.back()">取消</el-button>
        </el-form-item>
      </el-form>

      <!-- 我的申请记录 -->
      <el-divider>我的申请记录</el-divider>
      <el-table :data="myApplications" border size="small">
        <el-table-column prop="club_name" label="社团名称" />
        <el-table-column prop="category" label="类型" width="80" />
        <el-table-column prop="created_at" label="申请时间" width="160" />
        <el-table-column prop="status" label="状态" width="90">
          <template #default="{ row }">
            <el-tag size="small" :type="row.status==='pending'?'warning':row.status==='approved'?'success':'danger'">
              {{ row.status==='pending'?'待审批':row.status==='approved'?'已通过':'已驳回' }}
            </el-tag>
          </template>
        </el-table-column>
        <el-table-column prop="reject_reason" label="驳回理由" show-overflow-tooltip />
      </el-table>
    </div>
  </div>
</template>

<script setup>
import { ref, onMounted } from 'vue'
import { useRouter } from 'vue-router'
import { api } from '@/api'
import { ElMessage } from 'element-plus'

const router = useRouter()
const submitting = ref(false)
const formRef = ref()
const colleges = ref([])
const myApplications = ref([])
const clubTypes = ['学术', '文艺', '体育', '实践', '其他']

const form = ref({ name: '', club_type: '学术', college_id: null, description: '', reason: '', advisor: '' })
const rules = {
  name:        [{ required: true }, { min: 2, max: 30 }],
  club_type:   [{ required: true }],
  college_id:  [{ required: true, message: '请选择所属学院' }],
  description: [{ required: true }, { min: 20, message: '简介至少20字' }],
  reason:      [{ required: true }]
}

async function doSubmit() {
  await formRef.value.validate()
  submitting.value = true
  const payload = {
    club_name: form.value.name,
    category: form.value.club_type,
    description: form.value.description,
    advisor: form.value.advisor,
    college_id: form.value.college_id
  }
  const res = await api.post('/api/clubs/apply', payload)
  submitting.value = false
  if (res.code === 0) {
    ElMessage.success('申请已提交，等待学校管理员审批')
    form.value = { name: '', club_type: '学术', college_id: null, description: '', reason: '', advisor: '' }
    loadMyApplications()
  } else ElMessage.error(res.msg)
}

async function loadMyApplications() {
  const res = await api.get('/api/my/applications')
  if (res.code === 0) myApplications.value = res.data.list || []
}

onMounted(async () => {
  const res = await api.get('/api/colleges')
  if (res.code === 0) colleges.value = res.data.list || []
  loadMyApplications()
})
</script>
