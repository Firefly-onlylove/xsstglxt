import { ref, onMounted } from 'vue'
import { api } from '@/api'

const clubId = ref(null)

export function useClub() {
  if (!clubId.value) {
    api.get('/api/my/clubs').then(res => {
      if (res.code === 0) {
        const clubs = res.data.list || []
        const admin = clubs.find(c => c.my_role === 'president' || c.my_role === 'vice_president')
        if (admin) clubId.value = admin.club_id
      }
    })
  }
  return { clubId }
}
