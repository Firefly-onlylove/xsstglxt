<template>
  <div class="login-page">
    <div class="left-panel">
      <el-icon class="hero-icon"><School /></el-icon>
      <h1>学生社团管理系统</h1>
      <p class="subtitle">校园活动一站式管理平台</p>
    </div>
    <div class="right-panel">
      <div class="login-card">
        <div v-if="mode === 'login'">
          <h2 class="card-title">欢迎登录</h2>
          <el-form ref="loginForm" :model="loginData" :rules="loginRules" @submit.prevent="doLogin">
            <el-form-item prop="username">
              <el-input v-model="loginData.username" placeholder="用户名" size="large"
                prefix-icon="User" clearable />
            </el-form-item>
            <el-form-item prop="password">
              <el-input v-model="loginData.password" type="password" placeholder="密码" size="large"
                prefix-icon="Lock" show-password @keyup.enter="doLogin" />
            </el-form-item>
            <div v-if="loginError" class="error-msg">{{ loginError }}</div>
            <el-button type="primary" size="large" :loading="loading" style="width:100%;margin-top:8px"
              @click="doLogin">{{ loading ? '登录中...' : '登录' }}</el-button>
            <div class="hint">初始管理员 admin / admin123</div>
            <div class="switch-link">没有账号？<el-link type="primary" @click="mode='register'">立即注册</el-link></div>
          </el-form>
        </div>

        <div v-else>
          <h2 class="card-title">学生注册</h2>
          <el-form ref="regForm" :model="regData" :rules="regRules" label-position="top">
            <el-row :gutter="12">
              <el-col :span="12">
                <el-form-item label="用户名" prop="username">
                  <el-input v-model="regData.username" placeholder="4-20位字母数字" />
                </el-form-item>
              </el-col>
              <el-col :span="12">
                <el-form-item label="姓名" prop="real_name">
                  <el-input v-model="regData.real_name" />
                </el-form-item>
              </el-col>
            </el-row>
            <el-row :gutter="12">
              <el-col :span="12">
                <el-form-item label="密码" prop="password">
                  <el-input v-model="regData.password" type="password" show-password />
                </el-form-item>
              </el-col>
              <el-col :span="12">
                <el-form-item label="确认密码" prop="confirm_password">
                  <el-input v-model="regData.confirm_password" type="password" show-password />
                </el-form-item>
              </el-col>
            </el-row>
            <el-form-item label="学号" prop="student_id">
              <el-input v-model="regData.student_id" />
            </el-form-item>
            <el-row :gutter="12">
              <el-col :span="8">
                <el-form-item label="学院" prop="college_id">
                  <el-select v-model="regData.college_id" placeholder="选择学院" style="width:100%"
                    @change="loadMajors">
                    <el-option v-for="c in colleges" :key="c.college_id" :label="c.name" :value="c.college_id" />
                  </el-select>
                </el-form-item>
              </el-col>
              <el-col :span="8">
                <el-form-item label="专业" prop="major_id">
                  <el-select v-model="regData.major_id" placeholder="选择专业" style="width:100%"
                    @change="loadClasses">
                    <el-option v-for="m in majors" :key="m.major_id" :label="m.name" :value="m.major_id" />
                  </el-select>
                </el-form-item>
              </el-col>
              <el-col :span="8">
                <el-form-item label="班级" prop="class_id">
                  <el-select v-model="regData.class_id" placeholder="选择班级" style="width:100%">
                    <el-option v-for="cls in classes" :key="cls.class_id" :label="cls.name" :value="cls.class_id" />
                  </el-select>
                </el-form-item>
              </el-col>
            </el-row>
            <el-form-item label="手机号" prop="phone">
              <el-input v-model="regData.phone" placeholder="选填" />
            </el-form-item>
            <div v-if="regError" class="error-msg">{{ regError }}</div>
            <el-button type="primary" size="large" :loading="loading" style="width:100%;margin-top:4px"
              @click="doRegister">注册</el-button>
            <div class="switch-link">已有账号？<el-link type="primary" @click="mode='login'">返回登录</el-link></div>
          </el-form>
        </div>
      </div>
    </div>
  </div>
</template>

<script setup>
import { ref, onMounted } from 'vue'
import { useRouter } from 'vue-router'
import { api } from '@/api'
import { ElMessage } from 'element-plus'

const ROLE_HOME = {
  school_admin: '/school/dashboard',
  college_admin: '/college/dashboard',
  general_student: '/student/club-square',
  club_member: '/student/club-square',
  club_admin: '/student/club-square'
}

const router = useRouter()
const mode      = ref('login')
const loading   = ref(false)
const loginError = ref('')
const regError   = ref('')
const loginForm = ref()
const regForm   = ref()
const colleges  = ref([])
const majors    = ref([])
const classes   = ref([])

const loginData = ref({ username: '', password: '' })
const regData   = ref({
  username: '', password: '', confirm_password: '',
  real_name: '', student_id: '', college_id: null,
  major_id: null, class_id: null, phone: ''
})

const loginRules = {
  username: [{ required: true, message: '请输入用户名' }],
  password: [{ required: true, message: '请输入密码' }]
}
const regRules = {
  username:         [{ required: true }, { min: 4, max: 20, message: '4-20位' }],
  password:         [{ required: true }, { min: 6, message: '至少6位' }],
  confirm_password: [{ required: true, validator: (r, v, cb) =>
      v !== regData.value.password ? cb('两次密码不一致') : cb() }],
  real_name:  [{ required: true, message: '请输入姓名' }],
  student_id: [{ required: true, message: '请输入学号' }],
  college_id: [{ required: true, message: '请选择学院' }],
  major_id:   [{ required: true, message: '请选择专业' }],
  class_id:   [{ required: true, message: '请选择班级' }]
}

async function doLogin() {
  await loginForm.value.validate().catch(() => {})
  loading.value = true
  loginError.value = ''
  try {
    const res = await api.post('/api/login', loginData.value)
    if (res.code !== 0) { loginError.value = res.msg || '登录失败'; return }
    router.replace(ROLE_HOME[res.data.role] || '/student/club-square')
  } catch { loginError.value = '网络错误，请重试' }
  finally { loading.value = false }
}

async function doRegister() {
  await regForm.value.validate().catch(() => {})
  loading.value = true
  regError.value = ''
  try {
    const res = await api.post('/api/student/register', regData.value)
    if (res.code !== 0) { regError.value = res.msg || '注册失败'; return }
    ElMessage.success('注册成功，请登录')
    mode.value = 'login'
    loginData.value.username = regData.value.username
  } catch { regError.value = '网络错误，请重试' }
  finally { loading.value = false }
}

async function loadMajors() {
  regData.value.major_id = null
  regData.value.class_id = null
  majors.value = []
  classes.value = []
  if (!regData.value.college_id) return
  const res = await api.get('/api/school/majors', { college_id: regData.value.college_id })
  if (res.code === 0) majors.value = res.data || []
}

async function loadClasses() {
  regData.value.class_id = null
  classes.value = []
  if (!regData.value.major_id) return
  const res = await api.get('/api/school/classes', { major_id: regData.value.major_id })
  if (res.code === 0) classes.value = res.data || []
}

onMounted(async () => {
  const res = await api.get('/api/school/colleges')
  if (res.code === 0) colleges.value = res.data || []
})
</script>

<style scoped>
.login-page { display: flex; min-height: 100vh; }
.left-panel {
  flex: 1; background: linear-gradient(135deg, #1677FF 0%, #0958d9 100%);
  display: flex; flex-direction: column; align-items: center; justify-content: center;
  color: #fff; padding: 48px;
}
.hero-icon { font-size: 72px; margin-bottom: 20px; }
.left-panel h1 { font-size: 32px; font-weight: 700; margin: 0 0 12px; }
.subtitle { font-size: 16px; opacity: .8; margin: 0; }
.right-panel {
  width: 480px; display: flex; align-items: center; justify-content: center;
  background: #fff; padding: 40px;
}
.login-card { width: 100%; }
.card-title { font-size: 22px; font-weight: 700; margin: 0 0 28px; color: #1D2129; }
.error-msg { color: #F53F3F; font-size: 13px; margin-bottom: 8px; }
.hint { font-size: 12px; color: #86909C; text-align: center; margin-top: 12px; }
.switch-link { text-align: center; margin-top: 16px; font-size: 13px; color: #86909C; }
</style>
