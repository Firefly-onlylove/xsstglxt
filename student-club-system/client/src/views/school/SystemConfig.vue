<template>
  <div>
    <div class="page-header"><span class="page-title">系统设置</span></div>
    <el-row :gutter="16">
      <el-col :span="14">
        <div class="card" style="margin-bottom:16px">
          <div class="section-title" style="margin-bottom:16px">基本配置</div>
          <el-form ref="configForm" :model="config" label-width="160px">
            <el-form-item label="系统名称">
              <el-input v-model="config.system_name" />
            </el-form-item>
            <el-form-item label="校级社团人数上限">
              <el-input-number v-model="config.max_members_school" :min="5" :max="500" />
            </el-form-item>
            <el-form-item label="院级社团人数上限">
              <el-input-number v-model="config.max_members_college" :min="5" :max="500" />
            </el-form-item>
            <el-form-item label="社团成员上限（全局）">
              <el-input-number v-model="config.max_members_per_club" :min="5" :max="500" />
            </el-form-item>
            <el-form-item>
              <el-button type="primary" :loading="saving" @click="saveConfig">保存配置</el-button>
            </el-form-item>
          </el-form>
        </div>
      </el-col>
      <el-col :span="10">
        <div class="card">
          <div class="section-title" style="margin-bottom:16px">数据库备份</div>
          <el-button type="primary" :loading="backing" @click="doBackup" style="width:100%;margin-bottom:16px">
            <el-icon><Download /></el-icon>立即备份
          </el-button>
          <el-table :data="backups" size="small" border>
            <el-table-column prop="file_path" label="文件名" show-overflow-tooltip />
            <el-table-column label="大小" width="80">
              <template #default="{ row }">
                {{ row.file_size ? (row.file_size / 1024).toFixed(1) + ' KB' : '-' }}
              </template>
            </el-table-column>
            <el-table-column prop="operator" label="操作人" width="80" />
            <el-table-column prop="status" label="状态" width="70">
              <template #default="{ row }">
                <el-tag :type="row.status === 'success' ? 'success' : 'danger'" size="small">
                  {{ row.status === 'success' ? '成功' : '失败' }}
                </el-tag>
              </template>
            </el-table-column>
            <el-table-column prop="created_at" label="时间" width="160" />
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

const saving  = ref(false)
const backing = ref(false)
const config  = ref({ system_name: '', max_members_school: 60, max_members_college: 120, max_members_per_club: 100 })
const backups = ref([])

async function saveConfig() {
  saving.value = true
  const res = await api.post('/api/school/config', config.value)
  saving.value = false
  if (res.code === 0) {
    ElMessage.success('配置已保存')
    // 更新系统名称：写入 localStorage 并发送自定义事件通知 MainLayout
    if (config.value.system_name) {
      localStorage.setItem('scms_system_name', config.value.system_name)
      window.dispatchEvent(new CustomEvent('scms:title-updated', { detail: config.value.system_name }))
    }
  } else {
    ElMessage.error(res.msg)
  }
}

async function doBackup() {
  backing.value = true
  const res = await api.post('/api/school/backup')
  backing.value = false
  if (res.code === 0) { ElMessage.success('备份成功'); loadBackups() }
  else ElMessage.error(res.msg)
}

async function loadBackups() {
  const res = await api.get('/api/school/backups')
  if (res.code === 0) backups.value = res.data.list || []
}

onMounted(async () => {
  const res = await api.get('/api/school/config')
  if (res.code === 0) config.value = res.data
  loadBackups()
})
</script>
