# Student Club Management System (SCMS) — Practice

C HTTP server (Mongoose) + Vue 3 SPA (Element Plus) monorepo.
**Practice project** — complete college/ and club_admin/ modules replaced with stubs for you to implement.

## Structure

| Directory   | Purpose                                |
|-------------|----------------------------------------|
| `server/`   | C API server (Mongoose + MySQL)        |
| `client/`   | Vue 3 + Vite + Element Plus frontend   |
| `database/` | SQL migration scripts                  |
| `public/`   | Static web root (Vue build output)     |
| `storage/`  | Runtime data (receipts, exports, dumps)|
| `docs/`     | Design documents                       |

## Modules to Implement (13 files)

- `server/src/college/` — 6 files: dashboard, class_mgmt, club_oversight, finance_approval, member_mgmt, announcement
- `server/src/student/club_admin/` — 7 files: club_dashboard, member_mgmt, activity_mgmt, finance, election, election_vote, announcement

## Quick Start

1. **Database** — Run scripts in `database/` sequentially against MySQL.
2. **Config** — Edit `db_config.ini` with your MySQL credentials.
3. **Implement** — Fill in the 13 stub files (marked with `/* TODO */`).
4. **Server** — `cd server && make`
5. **Frontend** — `cd client && npm install && npm run build`
6. **Run** — `./scms.exe` (Windows) or `./scms` (Linux)
7. **Access** — http://127.0.0.1:8000
