<template>
  <div class="data-table-wrap card">
    <el-table :data="data" v-loading="loading" stripe border style="width:100%"
      @sort-change="$emit('sort-change', $event)">
      <el-table-column v-for="col in columns" :key="col.prop || col.type"
        :type="col.type" :prop="col.prop" :label="col.label" :width="col.width"
        :min-width="col.minWidth" :sortable="col.sortable ? 'custom' : false"
        :align="col.align || 'left'" :fixed="col.fixed">
        <template #default="{ row }" v-if="col.slot">
          <slot :name="col.slot" :row="row" />
        </template>
      </el-table-column>
    </el-table>
    <div class="pagination-wrap" v-if="total > 0">
      <el-pagination
        v-model:current-page="currentPage"
        v-model:page-size="currentSize"
        :total="total"
        :page-sizes="[10, 20, 50]"
        layout="total, sizes, prev, pager, next, jumper"
        @size-change="$emit('page-change', { page: 1, size: $event })"
        @current-change="$emit('page-change', { page: $event, size: currentSize })"
      />
    </div>
  </div>
</template>

<script setup>
import { ref } from 'vue'

const props = defineProps({
  data:    { type: Array,   default: () => [] },
  columns: { type: Array,   default: () => [] },
  total:   { type: Number,  default: 0 },
  loading: { type: Boolean, default: false },
  page:    { type: Number,  default: 1 },
  size:    { type: Number,  default: 10 }
})

defineEmits(['page-change', 'sort-change'])

const currentPage = ref(props.page)
const currentSize = ref(props.size)
</script>

<style scoped>
.data-table-wrap { padding: 0; overflow: hidden; }
.pagination-wrap { padding: 16px 20px; display: flex; justify-content: flex-end; }
</style>
