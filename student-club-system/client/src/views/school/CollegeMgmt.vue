<template>
  <div>
    <div class="page-header">
      <span class="page-title">学院 / 专业管理</span>
    </div>
    <el-row :gutter="16">
      <!-- 学院列表 -->
      <el-col :span="10">
        <div class="card" style="height:600px;overflow:auto">
          <div style="display:flex;justify-content:space-between;align-items:center;margin-bottom:12px">
            <b>学院列表</b>
            <el-button type="primary" size="small" @click="openCollegeForm()">+ 新增学院</el-button>
          </div>
          <el-table :data="colleges" size="small" highlight-current-row
            @current-change="selectCollege">
            <el-table-column prop="name" label="学院名称" />
            <el-table-column prop="major_count" label="专业数" width="70" />
            <el-table-column label="操作" width="100">
              <template #default="{ row }">
                <el-button link type="primary" @click.stop="openCollegeForm(row)">编辑</el-button>
              </template>
            </el-table-column>
          </el-table>
        </div>
      </el-col>
      <!-- 专业列表 -->
      <el-col :span="14">
        <div class="card" style="height:600px;overflow:auto">
          <div style="display:flex;justify-content:space-between;align-items:center;margin-bottom:12px">
            <b>专业列表{{ selectedCollege ? ' — ' + selectedCollege.name : '' }}</b>
            <el-button type="primary" size="small" @click="openMajorForm()" :disabled="!selectedCollege">+ 新增专业</el-button>
          </div>
          <el-table :data="majors" size="small">
            <el-table-column prop="name" label="专业名称" />
            <el-table-column prop="class_count" label="班级数" width="70" />
            <el-table-column label="操作" width="120">
              <template #default="{ row }">
                <el-button link type="primary" @click="openMajorForm(row)">编辑</el-button>
                <el-button link type="danger" @click="deleteMajor(row)">删除</el-button>
              </template>
            </el-table-column>
          </el-table>
        </div>
      </el-col>
    </el-row>

    <ModalForm ref="collegeModal" :title="editCollege?.college_id ? '编辑学院' : '新增学院'"
      :fields="collegeFields" @submit="saveCollege" />
    <ModalForm ref="majorModal" :title="editMajor?.major_id ? '编辑专业' : '新增专业'"
      :fields="majorFields" @submit="saveMajor" />
  </div>
</template>

<script setup>
import { ref, onMounted } from 'vue'
import { api } from '@/api'
import { ElMessage, ElMessageBox } from 'element-plus'
import ModalForm from '@/components/ModalForm.vue'

const colleges       = ref([])
const majors         = ref([])
const selectedCollege = ref(null)
const editCollege    = ref(null)
const editMajor      = ref(null)
const collegeModal   = ref()
const majorModal     = ref()

const collegeFields = [{ prop: 'name', label: '学院名称', placeholder: '请输入学院名称' }]
const majorFields   = [{ prop: 'name', label: '专业名称', placeholder: '请输入专业名称' }]

async function loadColleges() {
  const res = await api.get('/api/school/colleges')
  if (res.code === 0) colleges.value = res.data.list || []
}
async function selectCollege(row) {
  selectedCollege.value = row
  if (!row) return
  const res = await api.get('/api/school/majors', { college_id: row.college_id })
  if (res.code === 0) majors.value = res.data.list || []
}

function openCollegeForm(row) {
  editCollege.value = row || {}
  collegeModal.value.open(row ? { name: row.name } : {})
}
function openMajorForm(row) {
  editMajor.value = row || {}
  majorModal.value.open(row ? { name: row.name } : {})
}

async function saveCollege(data) {
  const url = editCollege.value?.college_id
    ? '/api/school/colleges/' + editCollege.value.college_id
    : '/api/school/colleges'
  const method = editCollege.value?.college_id ? 'put' : 'post'
  const res = await api[method](url, data)
  if (res.code === 0) { ElMessage.success('保存成功'); collegeModal.value.close(); loadColleges() }
  else ElMessage.error(res.msg)
}

async function saveMajor(data) {
  data.college_id = selectedCollege.value.college_id
  const url = editMajor.value?.major_id
    ? '/api/school/majors/' + editMajor.value.major_id
    : '/api/school/majors'
  const method = editMajor.value?.major_id ? 'put' : 'post'
  const res = await api[method](url, data)
  if (res.code === 0) {
    ElMessage.success('保存成功'); majorModal.value.close()
    selectCollege(selectedCollege.value)
  } else ElMessage.error(res.msg)
}

async function deleteMajor(row) {
  await ElMessageBox.confirm('确认删除该专业？', '提示', { type: 'warning' })
  const res = await api.delete('/api/school/majors/' + row.major_id)
  if (res.code === 0) { ElMessage.success('已删除'); selectCollege(selectedCollege.value) }
}

onMounted(loadColleges)
</script>
