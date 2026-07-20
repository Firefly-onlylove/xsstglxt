/**
 * useRole.js — 统一的角色标签/类型映射
 * 学生社团管理系统 SCMS
 *
 * 所有组件中重复的 roleLabel / roleType 映射均从此处导出，
 * 确保角色显示文字和标签颜色全局一致。
 */

/* 用户角色（users.role）→ 中文标签 */
export const USER_ROLE_LABEL = {
  school_admin: '学校管理员',
  college_admin: '学院管理员',
  general_student: '学生',
  club_member: '社团成员',
  club_admin: '社团管理员'
}

/* 用户角色 → Element Plus Tag type */
export const USER_ROLE_TYPE = {
  school_admin: 'danger',
  college_admin: 'warning',
  general_student: '',
  club_member: 'info',
  club_admin: 'success'
}

/* 社团内角色（members.role）→ 中文标签 */
export const CLUB_ROLE_LABEL = {
  president: '社长',
  vice_president: '副社长',
  member: '成员'
}

/* 社团内角色 → Element Plus Tag type */
export const CLUB_ROLE_TYPE = {
  president: 'danger',
  vice_president: 'warning',
  member: ''
}

/* 通用取值辅助：从映射表中取标签，未匹配时返回原值 */
export function roleLabel(map, r) { return map[r] || r }
export function roleType(map, r) { return map[r] || 'info' }
