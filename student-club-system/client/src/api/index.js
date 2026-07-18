const BASE = ''

/**
 * 后端有些接口返回 data 是直接数组 [...]，
 * 有些是通过 api_send_result_paged() 返回 {list, total, page, ...}，
 * 还有些是单独对象或 null。
 *
 * 为让前端能统一用 res.data.list / res.data.total 取值，
 * 一旦检测到 data 是数组，就自动包成 {list: 数组, total: 数组长度}。
 */
function normalizeData(data) {
  if (Array.isArray(data)) {
    return { list: data, total: data.length }
  }
  if (data && data.list !== undefined) {
    return data; /* already {list, total, ...} — keep as-is */
  }
  return data; /* object, number, null, etc. — pass through */
}

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
  const json = await res.json()
  if (json.code === 0) {
    json.data = normalizeData(json.data)
  }
  return json
}

export const api = {
  get:    (url, params) => request('GET', url, params),
  post:   (url, data)   => request('POST', url, data),
  put:    (url, data)   => request('PUT', url, data),
  delete: (url, data)   => request('DELETE', url, data),
  upload: (url, formData) => uploadRequest(url, formData)
}

async function uploadRequest(url, formData) {
  const res = await fetch(BASE + url, {
    method: 'POST',
    credentials: 'include',
    body: formData
  })
  if (!res.ok) throw new Error('HTTP ' + res.status)
  const json = await res.json()
  if (json.code === 0) {
    json.data = normalizeData(json.data)
  }
  return json
}

export async function checkAuth() {
  try {
    const res = await api.get('/api/me')
    return res.code === 0 ? res.data : null
  } catch {
    return null
  }
}
