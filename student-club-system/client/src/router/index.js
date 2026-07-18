import { createRouter, createWebHashHistory } from 'vue-router'
import { api } from '@/api'

const routes = [
  { path: '/', redirect: '/login' },
  { path: '/login', component: () => import('@/views/Login.vue'), meta: { public: true } },
  {
    path: '/school',
    component: () => import('@/layouts/MainLayout.vue'),
    meta: { role: 'school_admin' },
    children: [
      { path: '', redirect: '/school/dashboard' },
      { path: 'dashboard',    component: () => import('@/views/school/Dashboard.vue') },
      { path: 'club-approval', component: () => import('@/views/school/ClubApproval.vue') },
      { path: 'club-mgmt',    component: () => import('@/views/school/ClubMgmt.vue') },
      { path: 'college-mgmt', component: () => import('@/views/school/CollegeMgmt.vue') },
      { path: 'user-mgmt',    component: () => import('@/views/school/UserMgmt.vue') },
      { path: 'finance',      component: () => import('@/views/school/Finance.vue') },
      { path: 'statistics',   component: () => import('@/views/school/Statistics.vue') },
      { path: 'announcement', component: () => import('@/views/school/Announcement.vue') },
      { path: 'system-config', component: () => import('@/views/school/SystemConfig.vue') }
    ]
  },
  {
    path: '/college',
    component: () => import('@/layouts/MainLayout.vue'),
    meta: { role: 'college_admin' },
    children: [
      { path: '', redirect: '/college/dashboard' },
      { path: 'dashboard',       component: () => import('@/views/college/Dashboard.vue') },
      { path: 'class-mgmt',      component: () => import('@/views/college/ClassMgmt.vue') },
      { path: 'club-oversight',  component: () => import('@/views/college/ClubOversight.vue') },
      { path: 'finance-approval', component: () => import('@/views/college/FinanceApproval.vue') },
      { path: 'student-mgmt',    component: () => import('@/views/college/StudentMgmt.vue') },
      { path: 'announcement',    component: () => import('@/views/college/Announcement.vue') }
    ]
  },
  {
    path: '/student',
    component: () => import('@/layouts/MainLayout.vue'),
    meta: { role: 'student' },
    children: [
      { path: '', redirect: '/student/club-square' },
      { path: 'club-square',     component: () => import('@/views/student/ClubSquare.vue') },
      { path: 'club-detail/:id', component: () => import('@/views/student/ClubDetail.vue') },
      { path: 'activity-center', component: () => import('@/views/student/ActivityCenter.vue') },
      { path: 'activity/:id',    component: () => import('@/views/student/ActivityDetail.vue') },
      { path: 'my-clubs',        component: () => import('@/views/student/MyClubs.vue') },
      { path: 'profile',         component: () => import('@/views/student/Profile.vue') },
      { path: 'messages',        component: () => import('@/views/student/MessageCenter.vue') },
      { path: 'create-club',     component: () => import('@/views/student/CreateClub.vue') }
    ]
  },
  {
    path: '/club-admin',
    component: () => import('@/layouts/MainLayout.vue'),
    meta: { role: 'club_admin' },
    children: [
      { path: '', redirect: '/club-admin/dashboard' },
      { path: 'dashboard',     component: () => import('@/views/club-admin/Dashboard.vue') },
      { path: 'member-mgmt',   component: () => import('@/views/club-admin/MemberMgmt.vue') },
      { path: 'activity-mgmt', component: () => import('@/views/club-admin/ActivityMgmt.vue') },
      { path: 'finance',       component: () => import('@/views/club-admin/Finance.vue') },
      { path: 'election',      component: () => import('@/views/club-admin/Election.vue') }
    ]
  },
  { path: '/:pathMatch(.*)*', component: () => import('@/views/NotFound.vue') }
]

const router = createRouter({
  history: createWebHashHistory(),
  routes
})

const ROLE_HOME = {
  school_admin: '/school/dashboard',
  college_admin: '/college/dashboard',
  general_student: '/student/club-square',
  club_member: '/student/club-square',
  club_admin: '/student/club-square'
}

// 路由区域(meta.role) → 允许进入的后端角色
const AREA_ROLES = {
  school_admin: ['school_admin'],
  college_admin: ['college_admin'],
  student: ['general_student', 'club_member', 'club_admin'],
  club_admin: ['club_admin']
}

router.beforeEach(async (to) => {
  if (to.meta.public) return true
  try {
    const res = await api.get('/api/me')
    if (res.code !== 0) return '/login'
    const role = res.data.role
    if (to.meta.role) {
      const allowed = AREA_ROLES[to.meta.role] || [to.meta.role]
      if (!allowed.includes(role)) return ROLE_HOME[role] || '/login'
    }
    return true
  } catch {
    return '/login'
  }
})

export { ROLE_HOME }
export default router
