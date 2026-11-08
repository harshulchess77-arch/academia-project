/*
   ==========================================================================
   ACADEMIA - Frontend Connector & State Controller (Academia.js)
   Connects HTML pages with the C SQLite3 Server Backend
   Harshul Gupta, Lawrence Mao, Suengho Kim, Dmitrii Buianov
   CS50 C Project - Academia
   ==========================================================================
*/

// Server base URL (relative for same-origin, or localhost:3000)
const API_BASE = window.location.origin.includes('localhost') || window.location.origin.includes('127.0.0.1')
    ? ''
    : 'http://localhost:3000';

// Current session helper
function getCurrentUser() {
    try {
        const stored = sessionStorage.getItem('academia_user') || localStorage.getItem('academia_user');
        if (stored) {
            return JSON.parse(stored);
        }
    } catch (e) {
        console.warn('Error reading stored session:', e);
    }
    return null;
}

function setCurrentUser(user) {
    sessionStorage.setItem('academia_user', JSON.stringify(user));
    localStorage.setItem('academia_user', JSON.stringify(user));
}

function clearSession() {
    sessionStorage.removeItem('academia_user');
    localStorage.removeItem('academia_user');
    fetch(`${API_BASE}/api/logout`, { method: 'POST' }).catch(() => {});
}

// -------------------------------------------------------------------------
// AUTHENTICATION (Academia_Auth.html)
// -------------------------------------------------------------------------
async function goPortal() {
    const userEl = document.getElementById("username");
    const passEl = document.getElementById("password");
    const roleEl = document.getElementById("role");
    const errEl = document.getElementById("err");

    if (!userEl || !passEl) return;

    const u = userEl.value.trim();
    const p = passEl.value.trim();
    const r = roleEl ? roleEl.value : 'student';

    if (u === "" || p === "") {
        if (errEl) errEl.innerHTML = "Please enter both username and password.";
        return;
    }

    if (errEl) errEl.innerHTML = "Signing in...";

    try {
        const response = await fetch(`${API_BASE}/api/login`, {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify({ username: u, password: p })
        });

        if (response.ok) {
            const data = await response.json();
            if (data.success && data.user) {
                setCurrentUser(data.user);
                routeToRolePortal(data.user.role);
                return;
            } else {
                if (errEl) errEl.innerHTML = data.message || "Invalid credentials.";
                return;
            }
        }
    } catch (err) {
        console.warn("Backend server not responding, using offline fallback:", err);
    }

    // Offline / Demo Fallback
    const demoAccounts = {
        'student1': { id: 1, name: 'Alex Rivera', role: 'student' },
        'student2': { id: 2, name: 'Sophia Chen', role: 'student' },
        'parent1': { id: 3, name: 'Marcus Rivera', role: 'parent' },
        'staff1': { id: 4, name: 'Dr. Eleanor Vance', role: 'staff' }
    };

    if (demoAccounts[u] && p === 'password123') {
        const user = demoAccounts[u];
        setCurrentUser(user);
        routeToRolePortal(user.role);
    } else {
        // Fallback for simple testing with role selector
        const fallbackUser = { id: r === 'student' ? 1 : (r === 'parent' ? 3 : 4), name: u, role: r };
        setCurrentUser(fallbackUser);
        routeToRolePortal(r);
    }
}

function routeToRolePortal(role) {
    if (role === 'student') {
        window.location.href = "Academia_Student.html";
    } else if (role === 'parent') {
        window.location.href = "Academia_Parents.html";
    } else if (role === 'staff') {
        window.location.href = "Academia_Staff.html";
    } else {
        window.location.href = "Academia_Landing.html";
    }
}

// -------------------------------------------------------------------------
// STUDENT PORTAL (Academia_Student.html)
// -------------------------------------------------------------------------
async function loadStudentPortal() {
    const user = getCurrentUser() || { id: 1, name: "Alex Rivera", role: "student" };
    const errorEl = document.getElementById("error");

    try {
        const res = await fetch(`${API_BASE}/api/student-data?id=${user.id}`);
        if (res.ok) {
            const data = await res.json();
            if (data.success) {
                renderStudentData(data);
                return;
            }
        }
    } catch (e) {
        console.warn("Could not fetch student data from C server, using local state:", e);
    }

    // Check if studentState is defined globally (from student_states.js)
    if (typeof studentState !== 'undefined' && studentState.student) {
        renderStudentData(studentState);
    } else {
        // Safe default mock data
        renderStudentData({
            student: { userId: user.id || 1, firstName: "Alex", lastName: "Rivera", gradeLevel: 11, gpa: 3.85 },
            grades: [
                { course: "AP Computer Science A", score: 96.5 },
                { course: "Advanced Mathematics", score: 92.0 },
                { course: "Physics with Calculus", score: 94.5 },
                { course: "World History & Civics", score: 89.0 }
            ],
            attendance: [
                { course: "AP Computer Science A", status: "Present" },
                { course: "Advanced Mathematics", status: "Present" },
                { course: "Physics with Calculus", status: "Present" },
                { course: "World History & Civics", status: "Absent" }
            ],
            assignments: [
                { course: "AP Computer Science A", assignmentName: "C Binary Search Tree Lab", dueDate: "2026-11-15" },
                { course: "Advanced Mathematics", assignmentName: "Integral Calculus Problem Set 5", dueDate: "2026-11-18" },
                { course: "Physics with Calculus", assignmentName: "Kinematics & Optics Lab Report", dueDate: "2026-11-20" }
            ],
            announcements: [
                { course: "AP Computer Science A", title: "CS Project Milestone Submission", message: "Please submit your C server and database integration milestones by Friday 5:00 PM.", date: "2026-11-06" },
                { course: "General", title: "Academia Unified Digital Ecosystem Live", message: "Welcome students, parents, and staff. Education Today. A Brighter Tomorrow.", date: "2026-11-01" }
            ]
        });
    }
}

function renderStudentData(data) {
    const s = data.student || {};
    const welcomeEl = document.getElementById("welcome");
    const nameEl = document.getElementById("studentName");
    const idEl = document.getElementById("studentId");
    const gradeLevelEl = document.getElementById("gradeLevel");
    const gpaEl = document.getElementById("studentGpa");

    const fullName = `${s.firstName || 'Student'} ${s.lastName || ''}`.trim();
    if (welcomeEl) welcomeEl.innerText = `Welcome, ${fullName}!`;
    if (nameEl) nameEl.innerText = fullName;
    if (idEl) idEl.innerText = s.userId || "1";
    if (gradeLevelEl) gradeLevelEl.innerText = `Grade ${s.gradeLevel || 11}`;
    if (gpaEl) gpaEl.innerText = `${typeof s.gpa === 'number' ? s.gpa.toFixed(2) : (s.gpa || '3.85')} / 4.00`;

    // Grades Table
    const gradeList = document.getElementById("gradeList");
    if (gradeList && data.grades) {
        gradeList.innerHTML = data.grades.map(g => `
            <tr>
                <td><strong>${escapeHtml(g.course)}</strong></td>
                <td><span class="badge badge-score">${Number(g.score).toFixed(1)}%</span></td>
            </tr>
        `).join("");
    }

    // Attendance Table
    const attList = document.getElementById("attendanceList");
    if (attList && data.attendance) {
        attList.innerHTML = data.attendance.map(a => {
            const isPresent = a.status === 'Present' || a.present === 1 || a.present === true;
            return `
                <tr>
                    <td><strong>${escapeHtml(a.course)}</strong></td>
                    <td><span class="badge ${isPresent ? 'badge-present' : 'badge-absent'}">${isPresent ? 'Present' : 'Absent'}</span></td>
                </tr>
            `;
        }).join("");
    }

    // Assignments Table
    const assList = document.getElementById("assignmentList");
    if (assList && data.assignments) {
        assList.innerHTML = data.assignments.map(a => `
            <tr>
                <td><strong>${escapeHtml(a.course)}</strong></td>
                <td>${escapeHtml(a.assignmentName || a.name)}</td>
                <td><span class="badge" style="background:#F1F5F9; color:#475569;">${escapeHtml(a.dueDate || a.due)}</span></td>
            </tr>
        `).join("");
    }

    // Announcements List
    const annList = document.getElementById("announcementList");
    if (annList && data.announcements) {
        annList.innerHTML = data.announcements.map(a => `
            <div class="announcement-card">
                <div class="card-header">
                    <span class="card-title">${escapeHtml(a.title)}</span>
                    <span class="card-date">${escapeHtml(a.date)}</span>
                </div>
                <div class="card-course">${escapeHtml(a.course || 'School-Wide')}</div>
                <div class="card-body">${escapeHtml(a.message)}</div>
            </div>
        `).join("");
    }
}

// -------------------------------------------------------------------------
// PARENT PORTAL (Academia_Parents.html)
// -------------------------------------------------------------------------
async function loadParentPortal() {
    const user = getCurrentUser() || { id: 3, name: "Marcus Rivera", role: "parent" };

    try {
        const res = await fetch(`${API_BASE}/api/parent-data?id=${user.id}`);
        if (res.ok) {
            const data = await res.json();
            if (data.success) {
                renderParentData(data);
                return;
            }
        }
    } catch (e) {
        console.warn("Could not fetch parent data from C server, using local state:", e);
    }

    if (typeof parentState !== 'undefined' && parentState.parent) {
        renderParentData(parentState);
    } else {
        renderParentData({
            parent: { userId: user.id || 3, username: "parent1", firstName: "Marcus", lastName: "Rivera", phone: "(555) 234-5678" },
            student: { studentId: 1, firstName: "Alex", lastName: "Rivera", gradeLevel: 11, gpa: 3.85 },
            grades: [
                { course: "AP Computer Science A", score: 96.5 },
                { course: "Advanced Mathematics", score: 92.0 },
                { course: "Physics with Calculus", score: 94.5 },
                { course: "World History & Civics", score: 89.0 }
            ],
            attendance: [
                { course: "AP Computer Science A", status: "Present" },
                { course: "Advanced Mathematics", status: "Present" },
                { course: "Physics with Calculus", status: "Present" },
                { course: "World History & Civics", status: "Absent" }
            ],
            announcements: [
                { course: "School-Wide", title: "Academia Unified Digital Ecosystem Live", message: "Welcome parents and guardians to the school community network.", date: "2026-11-01" }
            ]
        });
    }
}

function renderParentData(data) {
    const p = data.parent || {};
    const s = data.student || {};

    const welcomeEl = document.getElementById("welcome");
    const userEl = document.getElementById("parentUsername");
    const phoneEl = document.getElementById("parentPhone");
    const stuNameEl = document.getElementById("studentName");
    const stuGradeEl = document.getElementById("gradeLevel");
    const stuGpaEl = document.getElementById("studentGpa");

    const parentFullName = `${p.firstName || 'Parent'} ${p.lastName || ''}`.trim();
    if (welcomeEl) welcomeEl.innerText = `Welcome, ${parentFullName}!`;
    if (userEl) userEl.innerText = p.username || "parent1";
    if (phoneEl) phoneEl.innerText = p.phone || "(555) 234-5678";

    const childFullName = `${s.firstName || 'Alex'} ${s.lastName || 'Rivera'}`.trim();
    if (stuNameEl) stuNameEl.innerText = childFullName;
    if (stuGradeEl) stuGradeEl.innerText = `Grade ${s.gradeLevel || 11}`;
    if (stuGpaEl) stuGpaEl.innerText = `${typeof s.gpa === 'number' ? s.gpa.toFixed(2) : (s.gpa || '3.85')} / 4.00`;

    // Grades Table
    const gradeList = document.getElementById("gradeList");
    if (gradeList && data.grades) {
        gradeList.innerHTML = data.grades.map(g => `
            <tr>
                <td><strong>${escapeHtml(g.course)}</strong></td>
                <td><span class="badge badge-score">${Number(g.score).toFixed(1)}%</span></td>
            </tr>
        `).join("");
    }

    // Attendance Table
    const attList = document.getElementById("attendanceList");
    if (attList && data.attendance) {
        attList.innerHTML = data.attendance.map(a => {
            const isPresent = a.status === 'Present' || a.present === 1 || a.present === true;
            return `
                <tr>
                    <td><strong>${escapeHtml(a.course)}</strong></td>
                    <td><span class="badge ${isPresent ? 'badge-present' : 'badge-absent'}">${isPresent ? 'Present' : 'Absent'}</span></td>
                </tr>
            `;
        }).join("");
    }

    // Announcements List
    const annList = document.getElementById("announcementList");
    if (annList && data.announcements) {
        annList.innerHTML = data.announcements.map(a => `
            <div class="announcement-card">
                <div class="card-header">
                    <span class="card-title">${escapeHtml(a.title)}</span>
                    <span class="card-date">${escapeHtml(a.date)}</span>
                </div>
                <div class="card-course">${escapeHtml(a.course || 'Notice')}</div>
                <div class="card-body">${escapeHtml(a.message)}</div>
            </div>
        `).join("");
    }
}

// -------------------------------------------------------------------------
// STAFF PORTAL (Academia_Staff.html)
// -------------------------------------------------------------------------
let currentStaffData = null;

async function loadStaffPortal() {
    const user = getCurrentUser() || { id: 4, name: "Dr. Eleanor Vance", role: "staff" };

    try {
        const res = await fetch(`${API_BASE}/api/staff-data?id=${user.id}`);
        if (res.ok) {
            const data = await res.json();
            if (data.success) {
                currentStaffData = data;
                renderStaffData(data);
                attachStaffFormHandlers();
                return;
            }
        }
    } catch (e) {
        console.warn("Could not fetch staff data from C server, using local state:", e);
    }

    if (typeof staffState !== 'undefined' && staffState.staff) {
        currentStaffData = staffState;
        renderStaffData(staffState);
    } else {
        const defaultStaff = {
            staff: { userId: 4, username: "staff1", firstName: "Dr. Eleanor", lastName: "Vance", phone: "(555) 987-6543" },
            courses: [
                { courseId: 1, course: "AP Computer Science A" },
                { courseId: 2, course: "Advanced Mathematics" },
                { courseId: 3, course: "Physics with Calculus" }
            ],
            enrollments: [
                { studentId: 1, student: "Alex Rivera", courseId: 1, course: "AP Computer Science A" },
                { studentId: 2, student: "Sophia Chen", courseId: 1, course: "AP Computer Science A" },
                { studentId: 1, student: "Alex Rivera", courseId: 2, course: "Advanced Mathematics" }
            ],
            grades: [
                { gradeId: 1, student: "Alex Rivera", course: "AP Computer Science A", score: 96.5 },
                { gradeId: 2, student: "Sophia Chen", course: "AP Computer Science A", score: 98.0 }
            ],
            attendance: [
                { attendanceId: 1, student: "Alex Rivera", course: "AP Computer Science A", present: 1 },
                { attendanceId: 2, student: "Sophia Chen", course: "AP Computer Science A", present: 1 }
            ],
            announcements: [
                { announcementId: 1, course: "AP Computer Science A", title: "CS Project Milestone Submission", message: "Please submit your C server and database integration milestones by Friday.", date: "2026-11-06" }
            ]
        };
        currentStaffData = defaultStaff;
        renderStaffData(defaultStaff);
    }
    attachStaffFormHandlers();
}

function renderStaffData(data) {
    const s = data.staff || {};
    const welcomeEl = document.getElementById("welcome");
    const nameEl = document.getElementById("staffName");
    const idEl = document.getElementById("staffId");
    const phoneEl = document.getElementById("staffPhone");

    const staffFullName = `${s.firstName || 'Staff'} ${s.lastName || ''}`.trim();
    if (welcomeEl) welcomeEl.innerText = `Welcome, ${staffFullName}!`;
    if (nameEl) nameEl.innerText = staffFullName;
    if (idEl) idEl.innerText = s.userId || "4";
    if (phoneEl) phoneEl.innerText = s.phone || "(555) 987-6543";

    // Populate dropdowns
    populateStaffDropdowns(data);

    // Render Grade Records
    const gradeList = document.getElementById("gradeList");
    if (gradeList && data.grades) {
        gradeList.innerHTML = data.grades.map(g => `
            <tr>
                <td><strong>${escapeHtml(g.student)}</strong></td>
                <td>${escapeHtml(g.course)}</td>
                <td><span class="badge badge-score">${Number(g.score).toFixed(1)}%</span></td>
                <td><button class="btn" style="padding: 0.4rem 1rem; font-size: 1.2rem;" onclick="prefillGrade('${escapeHtml(g.student)}', '${escapeHtml(g.course)}', ${g.score})">Edit</button></td>
            </tr>
        `).join("");
    }

    // Render Attendance Records
    const attList = document.getElementById("attendanceList");
    if (attList && data.attendance) {
        attList.innerHTML = data.attendance.map(a => {
            const isPresent = a.present === 1 || a.present === true;
            return `
                <tr>
                    <td><strong>${escapeHtml(a.student)}</strong></td>
                    <td>${escapeHtml(a.course)}</td>
                    <td><span class="badge ${isPresent ? 'badge-present' : 'badge-absent'}">${isPresent ? 'Present' : 'Absent'}</span></td>
                    <td><span style="color:#64748B; font-size:1.2rem;">Recorded</span></td>
                </tr>
            `;
        }).join("");
    }

    // Render Announcements
    const annList = document.getElementById("announcementList");
    if (annList && data.announcements) {
        annList.innerHTML = data.announcements.map(a => `
            <div class="announcement-card">
                <div class="card-header">
                    <span class="card-title">${escapeHtml(a.title)}</span>
                    <span class="card-date">${escapeHtml(a.date)}</span>
                </div>
                <div class="card-course">${escapeHtml(a.course || 'General')}</div>
                <div class="card-body">${escapeHtml(a.message)}</div>
            </div>
        `).join("");
    }
}

function populateStaffDropdowns(data) {
    // Unique students from enrollments
    const enrollments = data.enrollments || [];
    const courses = data.courses || [];

    const studentMap = {};
    enrollments.forEach(e => {
        studentMap[e.studentId] = e.student;
    });

    const studentOptions = Object.keys(studentMap).map(id => 
        `<option value="${id}">${escapeHtml(studentMap[id])} (ID: ${id})</option>`
    ).join("");

    const courseOptions = courses.map(c => 
        `<option value="${c.courseId}">${escapeHtml(c.course)}</option>`
    ).join("");

    // Grade form dropdowns (student select in HTML might have id studentName or gradeStudent)
    const gradeStudentSelect = document.querySelector("#gradeForm select#studentName, #gradeForm select");
    const gradeCourseSelect = document.querySelector("#gradeForm select#course");
    if (gradeStudentSelect && studentOptions) gradeStudentSelect.innerHTML = studentOptions;
    if (gradeCourseSelect && courseOptions) gradeCourseSelect.innerHTML = courseOptions;

    // Attendance form dropdowns
    const attStudentSelect = document.getElementById("attendanceName");
    const attCourseSelect = document.getElementById("attendanceCourse");
    if (attStudentSelect && studentOptions) attStudentSelect.innerHTML = studentOptions;
    if (attCourseSelect && courseOptions) attCourseSelect.innerHTML = courseOptions;

    // Announcement form dropdowns
    const annCourseSelect = document.getElementById("announcementCourse");
    if (annCourseSelect) {
        annCourseSelect.innerHTML = `<option value="0">All Courses (School-Wide)</option>` + courseOptions;
    }
}

function attachStaffFormHandlers() {
    // Grade Form Submission
    const gradeForm = document.getElementById("gradeForm");
    if (gradeForm && !gradeForm.dataset.bound) {
        gradeForm.dataset.bound = "true";
        gradeForm.onsubmit = async (e) => {
            e.preventDefault();
            const studentSelect = gradeForm.querySelector("select");
            const courseSelect = document.getElementById("course");
            const gradeInput = document.getElementById("grade");

            const studentId = studentSelect ? studentSelect.value : 1;
            const courseId = courseSelect ? courseSelect.value : 1;
            const score = gradeInput ? parseFloat(gradeInput.value) : 95.0;

            try {
                const res = await fetch(`${API_BASE}/api/staff/grade`, {
                    method: 'POST',
                    headers: { 'Content-Type': 'application/json' },
                    body: JSON.stringify({ studentId: parseInt(studentId), courseId: parseInt(courseId), grade: score })
                });
                if (res.ok) {
                    alert("Grade saved successfully!");
                    loadStaffPortal();
                    return;
                }
            } catch (err) {
                console.warn("Could not save to C server, updating locally:", err);
            }

            // Local fallback update
            if (currentStaffData && currentStaffData.grades) {
                const stuName = studentSelect.options[studentSelect.selectedIndex].text.split('(')[0].trim();
                const courseName = courseSelect.options[courseSelect.selectedIndex].text;
                currentStaffData.grades.unshift({
                    gradeId: Date.now(),
                    student: stuName,
                    course: courseName,
                    score: score
                });
                renderStaffData(currentStaffData);
                alert("Grade saved locally!");
            }
        };
    }

    // Attendance Form Submission
    const attForm = document.getElementById("attendanceForm");
    if (attForm && !attForm.dataset.bound) {
        attForm.dataset.bound = "true";
        attForm.onsubmit = async (e) => {
            e.preventDefault();
            const studentSelect = document.getElementById("attendanceName");
            const courseSelect = document.getElementById("attendanceCourse");
            const statusSelect = document.getElementById("attendanceStatus");

            const studentId = studentSelect ? studentSelect.value : 1;
            const courseId = courseSelect ? courseSelect.value : 1;
            const present = statusSelect ? parseInt(statusSelect.value) : 1;

            try {
                const res = await fetch(`${API_BASE}/api/staff/attendance`, {
                    method: 'POST',
                    headers: { 'Content-Type': 'application/json' },
                    body: JSON.stringify({ studentId: parseInt(studentId), courseId: parseInt(courseId), present: present })
                });
                if (res.ok) {
                    alert("Attendance recorded successfully!");
                    loadStaffPortal();
                    return;
                }
            } catch (err) {
                console.warn("Could not record attendance on C server, updating locally:", err);
            }

            if (currentStaffData && currentStaffData.attendance) {
                const stuName = studentSelect.options[studentSelect.selectedIndex].text.split('(')[0].trim();
                const courseName = courseSelect.options[courseSelect.selectedIndex].text;
                currentStaffData.attendance.unshift({
                    attendanceId: Date.now(),
                    student: stuName,
                    course: courseName,
                    present: present
                });
                renderStaffData(currentStaffData);
                alert("Attendance recorded locally!");
            }
        };
    }

    // Announcement Form Submission
    const annForm = document.getElementById("announcementForm");
    if (annForm && !annForm.dataset.bound) {
        annForm.dataset.bound = "true";
        annForm.onsubmit = async (e) => {
            e.preventDefault();
            const courseSelect = document.getElementById("announcementCourse");
            const titleInput = document.getElementById("announcementTitle");
            const msgInput = document.getElementById("announcementMessage");

            const courseId = courseSelect ? parseInt(courseSelect.value) : 0;
            const title = titleInput ? titleInput.value.trim() : "";
            const message = msgInput ? msgInput.value.trim() : "";

            if (!title || !message) return;

            try {
                const res = await fetch(`${API_BASE}/api/staff/announcement`, {
                    method: 'POST',
                    headers: { 'Content-Type': 'application/json' },
                    body: JSON.stringify({ courseId: courseId, title: title, message: message })
                });
                if (res.ok) {
                    alert("Announcement published successfully!");
                    if (titleInput) titleInput.value = "";
                    if (msgInput) msgInput.value = "";
                    loadStaffPortal();
                    return;
                }
            } catch (err) {
                console.warn("Could not publish announcement to C server, updating locally:", err);
            }

            if (currentStaffData && currentStaffData.announcements) {
                const courseName = courseSelect.options[courseSelect.selectedIndex].text;
                currentStaffData.announcements.unshift({
                    announcementId: Date.now(),
                    course: courseName,
                    title: title,
                    message: message,
                    date: new Date().toISOString().split('T')[0]
                });
                if (titleInput) titleInput.value = "";
                if (msgInput) msgInput.value = "";
                renderStaffData(currentStaffData);
                alert("Announcement published locally!");
            }
        };
    }
}

function prefillGrade(student, course, score) {
    const gradeInput = document.getElementById("grade");
    if (gradeInput) {
        gradeInput.value = score;
        gradeInput.focus();
    }
}

// Helper: Escape HTML strings to prevent XSS
function escapeHtml(text) {
    if (text === null || text === undefined) return '';
    return String(text)
        .replace(/&/g, "&amp;")
        .replace(/</g, "&lt;")
        .replace(/>/g, "&gt;")
        .replace(/"/g, "&quot;")
        .replace(/'/g, "&#039;");
}

// -------------------------------------------------------------------------
// AUTO-INITIALIZE ON PAGE LOAD
// -------------------------------------------------------------------------
document.addEventListener("DOMContentLoaded", () => {
    const path = window.location.pathname.toLowerCase();

    // Check which page we are currently on
    if (path.includes("academia_student.html") || document.getElementById("studentGpa")) {
        loadStudentPortal();
    } else if (path.includes("academia_parents.html") || document.getElementById("parentUsername")) {
        loadParentPortal();
    } else if (path.includes("academia_staff.html") || document.getElementById("gradeForm")) {
        loadStaffPortal();
    }

    // Bind logout link if present
    document.querySelectorAll('a[href*="Academia_Auth.html"], a[href*="logout"]').forEach(el => {
        if (el.innerText.toLowerCase().includes("log out") || el.innerText.toLowerCase().includes("logout")) {
            el.addEventListener("click", () => {
                clearSession();
            });
        }
    });
});
