<!--
Harshul Gupta, Lawrence Mao, Suengho Kim, Dmitrii Buianov
CS50 C Project
Academia
November 1, 2026
-->
# Academia

**Transforming education through a unified digital ecosystem**

Academia is a complete school management system designed to bring grades, attendance, announcements, and student information together in one place. It gives students, parents, and school staff one unified system to use, improving communication, organization, and efficiency across the school community.

Built as a CS50 Final Project featuring a **C backend with SQLite3**, a **local HTTP server**, and a **modern HTML/CSS/JavaScript frontend** styled to the official **Academia Brand Guidelines** (`Images/BrandSheet.png`).

---

## Quick Start (Run the Project)

### Option 1: 1-Click Launch (Recommended)
Double-click or run:
```bat
run_server.bat
```
This automatically initializes SQLite3, starts the local C server on port `3000`, and opens `http://localhost:3000` in your default browser.

### Option 2: Command Line
```powershell
.\academia_server.exe
```
Then navigate to `http://localhost:3000/` in Chrome, Edge, or Firefox.

---

## Demo Login Credentials

All demo accounts use password: `password123`

| Role | Username | Password | Features / View |
| :--- | :--- | :--- | :--- |
| **Student** | `student1` | `password123` | Alex Rivera — View live course grades, 4.0 GPA, daily attendance history, assignments, and school news |
| **Parent** | `parent1` | `password123` | Marcus Rivera — Monitor child's grades, GPA, attendance logs, and school notices |
| **Faculty / Staff** | `staff1` | `password123` | Dr. Eleanor Vance — Enter student grades (auto-recalculates GPA), record daily attendance, and publish course announcements |

---

## Brand Guidelines (`Images/BrandSheet.png`)

- **Primary**: Deep Navy Blue (`#0B3D91`)
- **Secondary**: Vibrant Royal Blue (`#2563EB`)
- **Accent**: Light Sky Blue (`#60A5FA`)
- **Highlight**: Warm Golden Yellow (`#FACC15`)
- **Neutral**: Soft Slate Gray (`#E5E7EB` / `#F8FAFC`)
- **Typography**: `Poppins`, sans-serif
- **Motto**: *Education Today. A Brighter Tomorrow. | Learn. Grow. Achieve. Together.*

---

## Technical Architecture

- **Backend**: Pure C (`Academia_Server.c`, `Academia_Auth.c`, `Academia_Database.c`, `Academia_Student.c`, `Academia_Staff.c`, `Academia_Parents.c`) using Windows `winsock2` and native Windows `winsqlite3`.
- **Database**: SQLite3 (`academia.db` initialized via `Academia.sql`) storing users, parents, students, staff, courses, enrollments, grades, attendance, assignments, and announcements.
- **Frontend**: Clean HTML5, CSS3 (`FrontEnd/CSS/Academia.css`), and JavaScript (`FrontEnd/JS/Academia.js`).
- **Connection**: REST API endpoints (`/api/login`, `/api/student-data`, `/api/parent-data`, `/api/staff-data`, `/api/staff/grade`, `/api/staff/attendance`, `/api/staff/announcement`) backed by real-time SQLite read/write operations and automatic JS state exports (`student_states.js`, `staff_states.js`, `parents_states.js`).

---

## Contributions

- **Harshul Gupta**
- **Lawrence Mao**
- **Dmitrii Buianov**
- **Suengho Kim**
