<template>
  <el-row :gutter="16" class="stat-cards">
    <el-col v-for="card in cards" :key="card.label" :span="span">
      <div class="stat-card">
        <div class="icon-wrap" :style="{ background: card.bg || '#E6F4FF' }">
          <el-icon :style="{ color: card.color || '#1677FF', fontSize: '22px' }">
            <component :is="card.icon" />
          </el-icon>
        </div>
        <div>
          <div class="label">{{ card.label }}</div>
          <div class="value" :style="{ color: card.color || '#1D2129' }">{{ card.value ?? '-' }}</div>
          <div v-if="card.sub" class="sub">{{ card.sub }}</div>
        </div>
      </div>
    </el-col>
  </el-row>
</template>

<script setup>
const props = defineProps({
  cards: { type: Array, default: () => [] },
  cols:  { type: Number, default: 4 }
})
const span = 24 / (props.cols || 4)
</script>

<script>
export default { name: 'StatCards' }
</script>

<style scoped>
.stat-cards { margin-bottom: 20px; }
.stat-card { background: #fff; border-radius: 8px; padding: 20px; box-shadow: 0 1px 4px rgba(0,0,0,.06);
  display: flex; align-items: center; gap: 14px; }
.icon-wrap { width: 48px; height: 48px; border-radius: 12px; display: flex; align-items: center; justify-content: center; flex-shrink: 0; }
.label { font-size: 13px; color: #86909C; margin-bottom: 4px; }
.value { font-size: 26px; font-weight: 700; line-height: 1; color: #1D2129; }
.sub { font-size: 12px; color: #86909C; margin-top: 4px; }
</style>
