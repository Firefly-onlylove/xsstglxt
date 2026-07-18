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
            <el-form-item label="每学期社团上限（个）">
              <el-input-number v-model="config.max_clubs_per_student" :min="1" :max="20" />
            </el-form-item>
            <el-form-item label="社团成员上限（人）">
              <el-input-number v-model="config.max_members_per_club" :min="5" :max="500" />
            </el-form-item>
            <el-form-item label="报名截止提前（分钟）">
              <el-input-number v-model="config.signup_close_before_min" :min="0" :max="1440" />
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
            <el-table-column prop="filename" label="文件名" show-overflow-tooltip />
            <el-table-column prop="size" label="大小" width="80" />
            <el-table-column prop="created_at" label="时间" width="120" />
            <el-table-column label="操作" width="60">
              <template #default="{ row }">
                <el-button link type="primary" @click="downloadBackup(row)">下载</el-button>
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
import { api } from '@/api'
import { ElMessage } from 'element-plus'

const saving  = ref(false)
const backing = ref(false)
const config  = ref({ system_name: '', max_clubs_per_student: 3, max_members_per_club: 100, signup_close_before_min: 30 })
const backups = ref([])

async function saveConfig() {
  saving.value = true
  const res = await api.post('/api/school/config', config.value)
  saving.value = false
  if (res.code === 0) ElMessage.success('配置已保存')
  else ElMessage.error(res.msg)
}

async function doBackup() {
  backing.value = true
  const res = await api.post('/api/school/backup')
  backing.value = false
  if (res.code === 0) { ElMessage.success('备份成功'); loadBackups() }
  else ElMessage.error(res.msg)
}

function downloadBackup(row) { window.open('/api/school/backup/download/' + row.backup_id) }

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
