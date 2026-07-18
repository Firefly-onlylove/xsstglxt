<template>
  <div>
    <div class="page-header"><span class="page-title">换届选举</span>
      <el-button type="primary" @click="openCreate">发起选举</el-button>
    </div>
    <el-row :gutter="16">
      <el-col :span="14">
        <DataTable :data="elections" :columns="elecCols" :total="elecTotal" :loading="loading"
          @page-change="onPage">
          <template #status="{ row }">
            <el-tag :type="elecStatusType(row.status)" size="small">{{ elecStatusLabel(row.status) }}</el-tag>
          </template>
          <template #actions="{ row }">
            <el-button link type="primary" @click="openDetail(row)">详情</el-button>
          </template>
        </DataTable>
      </el-col>
      <el-col :span="10">
        <div class="card" v-if="currentElec">
          <div style="font-size:15px;font-weight:600;margin-bottom:12px">{{ currentElec.title }}</div>
          <el-tag :type="elecStatusType(currentElec.status)">{{ elecStatusLabel(currentElec.status) }}</el-tag>
          <el-divider />

          <!-- 报名阶段：候选人列表 + 我要报名 -->
          <div v-if="currentElec.status === 'signup'">
            <div style="margin-bottom:12px">
              <b>候选人（{{ candidates.length }}人）</b>
            </div>
            <div v-for="c in candidates" :key="c.user_id" class="candidate-item">
              <el-avatar :size="32" :style="{ background:'#1677FF' }">{{ c.real_name[0] }}</el-avatar>
              <div>
                <div style="font-size:14px;font-weight:500">{{ c.real_name }}</div>
                <div style="font-size:12px;color:#86909C">{{ c.declaration || '暂无宣言' }}</div>
              </div>
            </div>
            <el-button v-if="!isCandidate" type="primary" style="width:100%;margin-top:12px"
              @click="applyCandidate">报名参选</el-button>
            <el-tag v-else type="success" style="width:100%;text-align:center">已报名参选</el-tag>
          </div>

          <!-- 投票阶段 -->
          <div v-else-if="currentElec.status === 'voting'">
            <div style="margin-bottom:12px"><b>投票（每人限投一次）</b></div>
            <el-radio-group v-model="selectedCandidate" style="display:flex;flex-direction:column;gap:12px">
              <el-radio v-for="c in candidates" :key="c.user_id" :value="c.user_id" :label="c.user_id">
                {{ c.real_name }} — {{ c.declaration || '暂无宣言' }}
              </el-radio>
            </el-radio-group>
            <el-button v-if="!hasVoted" type="primary" style="width:100%;margin-top:16px"
              :disabled="!selectedCandidate" @click="submitVote">提交投票</el-button>
            <el-tag v-else type="success" style="width:100%;text-align:center;margin-top:12px">已完成投票</el-tag>
          </div>

          <!-- 结果公示 -->
          <div v-else-if="currentElec.status === 'finished'">
            <div style="margin-bottom:12px"><b>投票结果</b></div>
            <div v-for="(c, i) in candidates" :key="c.user_id" class="result-item">
              <div class="rank">{{ i + 1 }}</div>
              <div>
                <div style="font-size:14px;font-weight:500">{{ c.real_name }}</div>
                <el-progress :percentage="votePercent(c.vote_count)" :format="() => c.vote_count + '票'" />
              </div>
            </div>
          </div>
        </div>
        <el-empty v-else description="点击列表中的选举查看详情" />
      </el-col>
    </el-row>

    <!-- 创建选举 -->
    <el-dialog v-model="createVisible" title="发起选举" width="460px">
      <el-form ref="createForm" :model="form" label-width="110px">
        <el-form-item label="选举标题" prop="title" :rules="[{required:true}]">
          <el-input v-model="form.title" />
        </el-form-item>
        <el-form-item label="报名开始" prop="signup_start" :rules="[{required:true}]">
          <el-date-picker v-model="form.signup_start" type="datetime" value-format="YYYY-MM-DD HH:mm:ss" style="width:100%" />
        </el-form-item>
        <el-form-item label="投票开始" prop="vote_start" :rules="[{required:true}]">
          <el-date-picker v-model="form.vote_start" type="datetime" value-format="YYYY-MM-DD HH:mm:ss" style="width:100%" />
        </el-form-item>
        <el-form-item label="投票结束" prop="vote_end" :rules="[{required:true}]">
          <el-date-picker v-model="form.vote_end" type="datetime" value-format="YYYY-MM-DD HH:mm:ss" style="width:100%" />
        </el-form-item>
      </el-form>
      <template #footer>
        <el-button @click="createVisible = false">取消</el-button>
        <el-button type="primary" @click="doCreate">发起</el-button>
      </template>
    </el-dialog>
  </div>
</template>

<script setup>
import { ref, computed, onMounted, watch } from 'vue'
import { api } from '@/api'
import { ElMessage } from 'element-plus'
import DataTable from '@/components/DataTable.vue'
import { useClub } from '@/composables/useClub'

const loading = ref(false)
const elections = ref([]); const elecTotal = ref(0); const elecPage = ref(1)
const currentElec = ref(null); const candidates = ref([])
const isCandidate = ref(false); const hasVoted = ref(false)
const selectedCandidate = ref(null)
const createVisible = ref(false); const createForm = ref()
const form = ref({ title: '', signup_start: '', vote_start: '', vote_end: '' })
const { clubId } = useClub()

const elecCols = [
  { prop: 'title',     label: '选举标题' },
  { slot: 'status',    label: '状态',   width: 90 },
  { prop: 'vote_end',  label: '结束时间', width: 160 },
  { slot: 'actions',   label: '操作',   width: 80, fixed: 'right' }
]
const elecStatusLabel = s => ({ signup:'报名中', voting:'投票中', finished:'已结束', cancelled:'已取消' }[s] || s)
const elecStatusType  = s => ({ signup:'primary', voting:'warning', finished:'success', cancelled:'info' }[s] || '')

const maxVotes = computed(() => Math.max(...candidates.value.map(c => c.vote_count || 0), 1))
const votePercent = (count) => Math.round((count / maxVotes.value) * 100)

async function loadData() {
  if (!clubId.value) return
  loading.value = true
  const res = await api.get('/api/club/' + clubId.value + '/elections', { page: elecPage.value, page_size: 10 })
  loading.value = false
  if (res.code === 0) { elections.value = res.data.list || []; elecTotal.value = res.data.total || 0 }
}
function onPage(e) { elecPage.value = e.page; loadData() }

async function openDetail(row) {
  currentElec.value = row
  const res = await api.get('/api/club/' + clubId.value + '/elections/' + row.election_id + '/candidates')
  if (res.code === 0) {
    candidates.value = res.data.candidates || []
    isCandidate.value = res.data.is_candidate || false
    hasVoted.value = res.data.has_voted || false
  }
}

async function applyCandidate() {
  const res = await api.post('/api/club/' + clubId.value + '/elections/' + currentElec.value.election_id + '/signup')
  if (res.code === 0) { ElMessage.success('报名成功'); isCandidate.value = true }
  else ElMessage.error(res.msg)
}

async function submitVote() {
  const res = await api.post('/api/club/' + clubId.value + '/elections/' + currentElec.value.election_id + '/vote',
    { candidate_id: selectedCandidate.value })
  if (res.code === 0) { ElMessage.success('投票成功'); hasVoted.value = true }
  else ElMessage.error(res.msg)
}

function openCreate() { form.value = { title: '', signup_start: '', vote_start: '', vote_end: '' }; createVisible.value = true }
async function doCreate() {
  await createForm.value.validate()
  const res = await api.post('/api/club/' + clubId.value + '/elections', form.value)
  if (res.code === 0) { ElMessage.success('选举已发起'); createVisible.value = false; loadData() }
  else ElMessage.error(res.msg)
}

onMounted(() => {
  if (clubId.value) loadData()
  else {
    const stop = watch(clubId, (val) => { if (val) { stop(); loadData() } })
  }
})
</script>

<style scoped>
.candidate-item { display: flex; align-items: center; gap: 12px; padding: 8px 0; border-bottom: 1px solid #F2F3F5; }
.result-item { display: flex; align-items: center; gap: 12px; padding: 8px 0; }
.rank { width: 28px; height: 28px; border-radius: 50%; background: #E6F4FF; color: #1677FF; display: flex; align-items: center; justify-content: center; font-weight: 700; flex-shrink: 0; }
</style>
