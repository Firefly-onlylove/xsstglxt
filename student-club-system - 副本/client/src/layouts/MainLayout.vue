<template>
  <el-container class="main-layout">
    <el-header class="top-nav">
      <div class="nav-left">
        <el-icon class="logo-icon"><School /></el-icon>
        <span class="system-title">{{ systemName }}</span>
      </div>
      <div class="nav-menu">
        <el-menu mode="horizontal" :default-active="activeMenu" router :ellipsis="false"
          background-color="transparent" text-color="#1D2129" active-text-color="#1677FF">
          <el-menu-item v-for="item in menuItems" :key="item.path" :index="item.path">
            <el-icon><component :is="item.icon" /></el-icon>{{ item.label }}
          </el-menu-item>
        </el-menu>
      </div>
      <div class="nav-right">
        <el-icon class="icon-btn" @click="goMessages"><Bell /></el-icon>
        <el-dropdown @command="handleUserCmd">
          <span class="user-info">
            <el-avatar :size="30" :style="{ background: '#1677FF' }">{{ userInitial }}</el-avatar>
            <span class="username">{{ user?.real_name || user?.username }}</span>
            <el-icon><ArrowDown /></el-icon>
          </span>
          <template #dropdown>
            <el-dropdown-menu>
              <el-dropdown-item command="logout" divided>退出登录</el-dropdown-item>
            </el-dropdown-menu>
          </template>
        </el-dropdown>
      </div>
    </el-header>
    <el-main class="main-content">
      <div class="page-container">
        <router-view />
      </div>
    </el-main>
  </el-container>
</template>

<script setup>
import { ref, computed, onMounted } from 'vue'
import { useRoute, useRouter } from 'vue-router'
import { api } from '@/api'
import { ElMessage } from 'element-plus'

const route = useRoute()
const router = useRouter()
const user = ref(null)
const systemName = ref(localStorage.getItem('scms_system_name') || 'SCMS · 学生社团管理系统')

const activeMenu = computed(() => route.path)

const MENUS = {
  school_admin: [
    { path: '/school/dashboard',    label: '首页',     icon: 'HomeFilled' },
    { path: '/school/club-approval', label: '社团审批', icon: 'Checked' },
    { path: '/school/club-mgmt',    label: '社团管理', icon: 'OfficeBuilding' },
    { path: '/school/user-mgmt',    label: '用户管理', icon: 'UserFilled' },
    { path: '/school/finance',      label: '财务监督', icon: 'Money' },
    { path: '/school/statistics',   label: '数据统计', icon: 'TrendCharts' },
    { path: '/school/announcement', label: '公告',     icon: 'Bell' },
    { path: '/school/system-config', label: '系统设置', icon: 'Setting' }
  ],
  college_admin: [
    { path: '/college/dashboard',       label: '首页',     icon: 'HomeFilled' },
    { path: '/college/club-oversight',  label: '本院社团', icon: 'OfficeBuilding' },
    { path: '/college/class-mgmt',      label: '班级管理', icon: 'Grid' },
    { path: '/college/finance-approval', label: '报销审批', icon: 'Money' },
    { path: '/college/student-mgmt',    label: '学生管理', icon: 'UserFilled' },
    { path: '/college/announcement',    label: '公告',     icon: 'Bell' }
  ],
  student: [
    { path: '/student/club-square',     label: '社团广场', icon: 'Compass' },
    { path: '/student/activity-center', label: '活动中心', icon: 'Calendar' },
    { path: '/student/my-clubs',        label: '我的社团', icon: 'UserFilled' },
    { path: '/student/messages',        label: '消息',     icon: 'Message' }
  ],
  club_admin: [
    { path: '/student/club-square',     label: '社团广场', icon: 'Compass' },
    { path: '/student/activity-center', label: '活动中心', icon: 'Calendar' },
    { path: '/student/my-clubs',        label: '我的社团', icon: 'UserFilled' },
    { path: '/club-admin/dashboard',    label: '管理后台', icon: 'Setting' },
    { path: '/student/messages',        label: '消息',     icon: 'Message' }
  ]
}

const ROLE_MENU_KEY = {
  general_student: 'student',
  club_member: 'student',
  club_admin: 'club_admin',
  school_admin: 'school_admin',
  college_admin: 'college_admin'
}

const menuItems = computed(() => MENUS[ROLE_MENU_KEY[user.value?.role]] || [])
const userInitial = computed(() => (user.value?.real_name || user.value?.username || '?')[0])

async function loadUser() {
  const res = await api.get('/api/me')
  if (res.code === 0) user.value = res.data
}

function goMessages() {
  const role = user.value?.role
  if (role === 'school_admin') router.push('/school/announcement')
  else if (role === 'college_admin') router.push('/college/announcement')
  else router.push('/student/messages')
}

async function handleUserCmd(cmd) {
  if (cmd === 'logout') {
    await api.post('/api/logout')
    router.push('/login')
  }
}

onMounted(() => {
  loadUser()
  // 监听系统名称变更事件（SystemConfig 保存时触发）
  window.addEventListener('scms:title-updated', (e) => {
    systemName.value = e.detail
  })
})
</script>

<style scoped>
.main-layout { min-height: 100vh; }
.top-nav {
  height: 60px;
  background: #fff;
  border-bottom: 1px solid #E5E6EB;
  display: flex;
  align-items: center;
  padding: 0 24px;
  position: sticky;
  top: 0;
  z-index: 100;
  gap: 20px;
}
.nav-left { display: flex; align-items: center; gap: 8px; white-space: nowrap; }
.logo-icon { font-size: 22px; color: #1677FF; }
.system-title { font-size: 16px; font-weight: 700; color: #1D2129; }
.nav-menu { flex: 1; overflow: hidden; }
.nav-menu .el-menu { border-bottom: none; }
.nav-right { display: flex; align-items: center; gap: 16px; white-space: nowrap; }
.icon-btn { font-size: 20px; cursor: pointer; color: #86909C; }
.icon-btn:hover { color: #1677FF; }
.user-info { display: flex; align-items: center; gap: 6px; cursor: pointer; }
.username { font-size: 14px; color: #1D2129; }
.main-content { padding: 0; background: #F7F8FA; }
</style>
