<template>
  <div>
    <div class="page-header">
      <span class="page-title">消息中心</span>
      <el-button @click="markAllRead" :disabled="unreadCount === 0">全部已读</el-button>
    </div>
    <div class="card">
      <el-tabs v-model="activeTab" @tab-change="loadData">
        <el-tab-pane label="全部" name="" />
        <el-tab-pane label="系统通知" name="system" />
        <el-tab-pane label="社团通知" name="club" />
        <el-tab-pane label="活动通知" name="activity" />
      </el-tabs>
      <div v-loading="loading">
        <el-empty v-if="!loading && list.length === 0" description="暂无消息" />
        <div v-for="msg in list" :key="msg.notif_id" class="msg-item"
          :class="{ unread: !msg.is_read }" @click="readMsg(msg)">
          <div class="msg-icon">
            <el-icon :style="{ color: typeColor(msg.type) }"><component :is="typeIcon(msg.type)" /></el-icon>
          </div>
          <div class="msg-body">
            <div class="msg-title">{{ msg.title }}</div>
            <div class="msg-content">{{ msg.content }}</div>
            <div class="msg-time">{{ msg.created_at }}</div>
          </div>
          <el-badge v-if="!msg.is_read" is-dot class="unread-dot" />
        </div>
      </div>
      <div class="pagination-wrap" v-if="total > 0">
        <el-pagination v-model:current-page="page" :page-size="10" :total="total"
          layout="prev, pager, next" @current-change="loadData" />
      </div>
    </div>
  </div>
</template>

<script setup>
import { ref, onMounted } from 'vue'
import { api } from '@/api'
import { ElMessage } from 'element-plus'

const loading = ref(false)
const list = ref([])
const total = ref(0)
const page = ref(1)
const activeTab = ref('')
const unreadCount = ref(0)

const typeIcon  = t => ({ system:'Setting', club:'OfficeBuilding', activity:'Calendar', announcement:'Bell' }[t] || 'Message')
const typeColor = t => ({ system:'#1677FF', club:'#36CFC9', activity:'#FF7D00', announcement:'#722ED1' }[t] || '#86909C')

async function loadData() {
  loading.value = true
  const res = await api.get('/api/notifications', {
    page: page.value, page_size: 10,
    type: activeTab.value || undefined
  })
  loading.value = false
  if (res.code === 0) {
    list.value = res.data.list || []
    total.value = res.data.total || 0
    unreadCount.value = res.data.unread_count || 0
  }
}

async function readMsg(msg) {
  if (!msg.is_read) {
    await api.post('/api/notifications/read', { notification_id: msg.notif_id })
    msg.is_read = true
    unreadCount.value = Math.max(0, unreadCount.value - 1)
  }
}

async function markAllRead() {
  const res = await api.post('/api/notifications/read-all')
  if (res.code === 0) { ElMessage.success('已全部标记已读'); loadData() }
}

onMounted(loadData)
</script>

<style scoped>
.msg-item {
  display: flex; align-items: flex-start; gap: 14px; padding: 14px 0;
  border-bottom: 1px solid #F2F3F5; cursor: pointer; transition: background .15s;
  border-radius: 6px; padding-left: 8px; margin: 0 -8px;
}
.msg-item:hover { background: #F7F8FA; }
.msg-item.unread { background: #F0F7FF; }
.msg-icon { width: 36px; height: 36px; border-radius: 50%; background: #E6F4FF; display: flex; align-items: center; justify-content: center; flex-shrink: 0; font-size: 16px; margin-top: 2px; }
.msg-body { flex: 1; min-width: 0; }
.msg-title { font-size: 14px; font-weight: 600; margin-bottom: 4px; }
.msg-content { font-size: 13px; color: #86909C; margin-bottom: 4px; overflow: hidden; white-space: nowrap; text-overflow: ellipsis; }
.msg-time { font-size: 12px; color: #C2C7D0; }
.unread-dot { flex-shrink: 0; margin-top: 8px; }
.pagination-wrap { text-align: center; padding-top: 16px; }
</style>
