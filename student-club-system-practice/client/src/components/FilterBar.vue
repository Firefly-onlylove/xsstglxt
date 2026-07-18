<template>
  <div class="filter-bar">
    <slot />
    <div class="filter-actions">
      <el-button type="primary" @click="$emit('search')">
        <el-icon><Search /></el-icon>查询
      </el-button>
      <el-button @click="$emit('reset')">重置</el-button>
    </div>
  </div>
</template>

<script setup>
defineEmits(['search', 'reset'])
</script>

<style scoped>
.filter-bar {
  background: #fff;
  border-radius: 8px;
  padding: 16px 20px;
  margin-bottom: 16px;
  display: flex;
  flex-wrap: wrap;
  gap: 12px;
  align-items: center;
  box-shadow: 0 1px 4px rgba(0,0,0,.06);
}
.filter-actions { margin-left: auto; display: flex; gap: 8px; }
</style>
