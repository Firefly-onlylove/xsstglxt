import { ref, computed } from 'vue'
import { useRoute } from 'vue-router'
import { api } from '@/api'

const clubId = ref(null)
const clubPromise = ref(null)

export function useClub() {
  const route = useRoute()

  // Priority 1: query param from navigation
  const qClubId = computed(() => {
    const q = route.query.clubId
    return q ? parseInt(q) : null
  })

  if (!clubPromise.value) {
    // Priority 2: fetch from API
    clubPromise.value = api.get('/api/my/clubs').then(res => {
      if (res.code === 0) {
        const clubs = res.data.list || []
        const admin = clubs.find(c => c.role === 'president' || c.role === 'vice_president')
        if (admin) clubId.value = admin.club_id
      }
    }).catch(() => {})

    // Also check query param immediately
    if (qClubId.value) {
      clubId.value = qClubId.value
    }
  }

  // Watch for query param changes
  if (qClubId.value) {
    clubId.value = qClubId.value
  }

  return { clubId, clubPromise }
}
