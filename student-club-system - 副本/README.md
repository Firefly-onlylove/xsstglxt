# Student Club Management System (SCMS)

C HTTP server (Mongoose) + Vue 3 SPA (Element Plus) monorepo.

## Structure

| Directory   | Purpose                                |
|-------------|----------------------------------------|
| `server/`   | C API server (Mongoose + MySQL)        |
| `client/`   | Vue 3 + Vite + Element Plus frontend   |
| `database/` | SQL migration scripts                  |
| `public/`   | Static web root (Vue build output)     |
| `storage/`  | Runtime data (receipts, exports, dumps)|
| `docs/`     | Design documents                       |

## Quick Start

1. **Database** — Run scripts in `database/` sequentially against MySQL.
2. **Config** — Edit `db_config.ini` with your MySQL credentials.
3. **Server** — `cd server && make`
4. **Frontend** — `cd client && npm install && npm run build`
5. **Run** — `./scms.exe` (Windows) or `./scms` (Linux)
6. **Access** — http://127.0.0.1:8000

## Development

- Backend: `cd server && make debug && make run`
- Frontend dev server: `cd client && npm run dev` (proxies API to :8000)
- Clean: `cd server && make clean`
