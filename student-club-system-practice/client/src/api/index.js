const BASE = ''

async function request(method, url, data) {
  const opts = {
    method,
    headers: {},
    credentials: 'include'
  }
  if (data && method !== 'GET') {
    opts.headers['Content-Type'] = 'application/json'
    opts.body = JSON.stringify(data)
  }
  const query = data && method === 'GET'
    ? '?' + new URLSearchParams(data).toString()
    : ''
  const res = await fetch(BASE + url + query, opts)
  if (!res.ok) throw new Error(`HTTP ${res.status}`)
  return res.json()
}

export const api = {
  get:    (url, params) => request('GET', url, params),
  post:   (url, data)   => request('POST', url, data),
  put:    (url, data)   => request('PUT', url, data),
  delete: (url, data)   => request('DELETE', url, data)
}

export async function checkAuth() {
  try {
    const res = await api.get('/api/me')
    return res.code === 0 ? res.data : null
  } catch {
    return null
  }
}
