<template>
  <div>
    <div class="page-header"><span class="page-title">个人中心</span></div>
    <el-row :gutter="20">
      <el-col :span="8">
        <div class="card" style="text-align:center;padding:32px">
          <el-avatar :size="80" :style="{ background:'#1677FF', fontSize:'32px' }">
            {{ (user?.real_name || user?.username || '?')[0] }}
          </el-avatar>
          <div style="font-size:20px;font-weight:700;margin:16px 0 4px">{{ user?.real_name }}</div>
          <el-tag>{{ roleLabel(user?.role) }}</el-tag>
          <div style="margin-top:16px;font-size:13px;color:#86909C">
            <div>{{ user?.college_name }} · {{ user?.class_name }}</div>
          </div>
        </div>
      </el-col>
      <el-col :span="16">
        <div class="card" style="margin-bottom:16px">
          <div style="display:flex;justify-content:space-between;margin-bottom:16px">
            <b>基本信息</b>
            <el-button size="small" @click="editing = !editing">{{ editing ? '取消' : '编辑' }}</el-button>
          </div>
          <el-form :model="form" label-width="90px">
            <el-row :gutter="16">
              <el-col :span="12">
                <el-form-item label="用户名"><el-input v-model="form.username" disabled /></el-form-item>
              </el-col>
              <el-col :span="12">
                <el-form-item label="姓名"><el-input v-model="form.real_name" :disabled="!editing" /></el-form-item>
              </el-col>
              <el-col :span="12">
                <el-form-item label="学号"><el-input v-model="form.student_id" :disabled="!editing" /></el-form-item>
              </el-col>
              <el-col :span="12">
                <el-form-item label="手机号"><el-input v-model="form.phone" :disabled="!editing" /></el-form-item>
              </el-col>
              <el-col :span="12">
                <el-form-item label="邮箱"><el-input v-model="form.email" :disabled="!editing" /></el-form-item>
              </el-col>
              <el-col :span="12">
                <el-form-item label="学院"><el-input v-model="form.college_name" disabled /></el-form-item>
              </el-col>
            </el-row>
            <el-form-item v-if="editing">
              <el-button type="primary" :loading="saving" @click="saveProfile">保存</el-button>
            </el-form-item>
          </el-form>
        </div>
        <div class="card">
          <div style="display:flex;justify-content:space-between;margin-bottom:16px">
            <b>修改密码</b>
          </div>
          <el-form :model="pwdForm" label-width="90px" :rules="pwdRules" ref="pwdFormRef">
            <el-form-item label="当前密码" prop="old_password">
              <el-input v-model="pwdForm.old_password" type="password" show-password />
            </el-form-item>
            <el-form-item label="新密码" prop="new_password">
              <el-input v-model="pwdForm.new_password" type="password" show-password />
            </el-form-item>
            <el-form-item label="确认密码" prop="confirm_password">
              <el-input v-model="pwdForm.confirm_password" type="password" show-password />
            </el-form-item>
            <el-form-item>
              <el-button type="primary" @click="changePwd">修改密码</el-button>
            </el-form-item>
          </el-form>
        </div>
      </el-col>
    </el-row>
  </div>
</template>

<script setup>
import { ref, onMounted } from 'vue'
import { api } from '@/api'
import { ElMessage } from 'element-plus'

const user = ref(null)
const editing = ref(false)
const saving = ref(false)
const form = ref({ username: '', real_name: '', student_id: '', phone: '', email: '', college_name: '' })
const pwdFormRef = ref()
const pwdForm = ref({ old_password: '', new_password: '', confirm_password: '' })
const pwdRules = {
  old_password: [{ required: true }],
  new_password: [{ required: true }, { min: 6 }],
  confirm_password: [{ required: true, validator: (r, v, cb) =>
    v !== pwdForm.value.new_password ? cb('两次密码不一致') : cb() }]
}
const roleLabel = r => ({ school_admin:'学校管理员', college_admin:'学院管理员', student:'学生', club_admin:'社团管理员' }[r] || r)

async function saveProfile() {
  saving.value = true
  const res = await api.put('/api/student/profile', { real_name: form.value.real_name, phone: form.value.phone, email: form.value.email, student_id: form.value.student_id })
  saving.value = false
  if (res.code === 0) { ElMessage.success('保存成功'); editing.value = false }
  else ElMessage.error(res.msg)
}

async function changePwd() {
  await pwdFormRef.value.validate()
  const res = await api.put('/api/student/password', { old_password: pwdForm.value.old_password, new_password: pwdForm.value.new_password })
  if (res.code === 0) { ElMessage.success('密码已修改'); pwdForm.value = { old_password:'', new_password:'', confirm_password:'' } }
  else ElMessage.error(res.msg)
}

onMounted(async () => {
  const res = await api.get('/api/me')
  if (res.code === 0) {
    user.value = res.data
    Object.assign(form.value, res.data)
  }
})
</script>
