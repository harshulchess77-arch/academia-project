// ==========================================
// STUDENT PORTAL FUNCTIONS
// ==========================================
function showPage(pageId) {
    var dash = document.getElementById("dashboard-page");
    var courses = document.getElementById("courses-page");

<<<<<<< Updated upstream
// ---------------------------------------------------------------------------
// ACCOUNTS
//
// The passwords below are stored as SHA-256 hashes, not as plain text. When
// someone signs in, the password they type is hashed and the two hashes are
// compared, so the real password is never written in this file.
//
// You can check a hash by hashing the password yourself and comparing. All
// four accounts use the password: password123
//
// This is a client-side check. It stops someone guessing a password, but
// anyone with the browser tools can still set the session value by hand. A
// real check needs the C server and the user table.
// ---------------------------------------------------------------------------

const ACCOUNTS = [
    { username: 'student1', id: 1, name: 'Alex Rivera',       role: 'student', hash: 'ef92b778bafe771e89245b89ecbc08a44a4e166c06659911881f383d4473e94f' },
    { username: 'student2', id: 2, name: 'Sophia Chen',       role: 'student', hash: 'ef92b778bafe771e89245b89ecbc08a44a4e166c06659911881f383d4473e94f' },
    { username: 'parent1',  id: 3, name: 'Marcus Rivera',     role: 'parent',  hash: 'ef92b778bafe771e89245b89ecbc08a44a4e166c06659911881f383d4473e94f' },
    { username: 'staff1',   id: 4, name: 'Dr. Eleanor Vance', role: 'staff',   hash: 'ef92b778bafe771e89245b89ecbc08a44a4e166c06659911881f383d4473e94f' }
];

// Turns a typed password into its SHA-256 hash, as a lowercase hex string.
async function hashPassword(text) {
    const bytes = new TextEncoder().encode(text);
    const digest = await crypto.subtle.digest('SHA-256', bytes);
    const list = Array.from(new Uint8Array(digest));
    return list.map(b => b.toString(16).padStart(2, '0')).join('');
}

// Looks up an account by the username typed in the form.
function findAccount(username) {
    for (let i = 0; i < ACCOUNTS.length; i++) {
        if (ACCOUNTS[i].username === username) {
            return ACCOUNTS[i];
        }
    }
    return null;
}
=======
    if (!dash || !courses) return;
>>>>>>> Stashed changes

    if (pageId === 'dashboard') {
        dash.style.display = 'block';
        courses.style.display = 'none';
    } else if (pageId === 'courses') {
        dash.style.display = 'none';
        courses.style.display = 'block';
        backToList();
    }
}

function openWorkspace(courseTitle) {
    var listView = document.getElementById("course-list-view");
    var workspaceView = document.getElementById("course-workspace-view");
    var titleEl = document.getElementById("workspace-title");
    
    if (!listView || !workspaceView) return;

    listView.style.display = "none";
    workspaceView.style.display = "block";
    if (titleEl) titleEl.innerText = courseTitle + " - Cmdr. Schenck";

    var playlistDiv = document.getElementById("playlist-links");
    var htmlContent = "";
    var prefix = "post-1"; // Default for HTML

    if (courseTitle === "Complete CSS Tutorial") {
        prefix = "post-2"; // Uses post-2-1 through post-2-6
    } else if (courseTitle === "Complete JavaScript Tutorial") {
        prefix = "post-3"; // Uses post-3-1 through post-3-6
    }

    var lessons = [];
    for (var i = 1; i <= 6; i++) {
        lessons.push({
            title: "Part 0" + i + " Lesson",
            thumb: "../Images/" + prefix + "-" + i + ".png"
        });
    }

    for (var j = 0; j < lessons.length; j++) {
        var les = lessons[j];
        htmlContent += '<div class="lesson-item" onclick="changeVideo(\'' + courseTitle + ' (' + les.title + ')\', \'' + les.thumb + '\', \'../Images/vid-1.mp4\')">';
        htmlContent += '<img src="' + les.thumb + '" onerror="this.style.display=\'none\';">';
        htmlContent += '<span>' + les.title + '</span></div>';
    }

    if (playlistDiv) playlistDiv.innerHTML = htmlContent;

    var vid = document.getElementById("my-video");
    var src = document.getElementById("video-source");
    var vidTitle = document.getElementById("video-title");
    
    if (vid && src) {
        src.src = "";
        vid.poster = lessons[0].thumb;
        vid.load();
    }
    if (vidTitle) vidTitle.innerText = "Please select a lesson from the playlist";
}

<<<<<<< Updated upstream
function clearSession() {
    // Remove the stored session, so the portal pages no longer see a signed-in
    // user and send the visitor back to the login page.
    sessionStorage.removeItem('academia_user');
    localStorage.removeItem('academia_user');
=======
function backToList() {
    var listView = document.getElementById("course-list-view");
    var workspaceView = document.getElementById("course-workspace-view");
    if (listView && workspaceView) {
        listView.style.display = "block";
        workspaceView.style.display = "none";
    }
>>>>>>> Stashed changes
}

function changeVideo(title, posterImg, videoFile) {
    var vidTitle = document.getElementById("video-title");
    var vid = document.getElementById("my-video");
    var src = document.getElementById("video-source");

    if (vidTitle) vidTitle.innerText = title;
    if (vid && src) {
        src.src = videoFile;
        vid.poster = posterImg;
        vid.load();
        vid.play().catch(function() {});
    }
}

var likes = 44;
function likeVideo() {
    likes++;
    var likesEl = document.getElementById("likes");
    if (likesEl) likesEl.innerText = likes;
}

function addComment() {
    var box = document.getElementById("comment-box");
    if (!box) return;
    var txt = box.value;
    if (txt === "") {
        alert("Please write a comment first!");
        return;
    }

<<<<<<< Updated upstream
    if (errEl) errEl.innerHTML = "Checking...";

    // Step 1: is the username known?
    const account = findAccount(u);
    if (account === null) {
        if (errEl) errEl.innerHTML = "Invalid username or password.";
        return;
=======
    var section = document.getElementById("comments-section");
    if (section) {
        section.innerHTML += "<p><b>Alex Rivera:</b> " + txt + "</p>";
>>>>>>> Stashed changes
    }
    box.value = "";
}

<<<<<<< Updated upstream
    // Step 2: does the typed password hash to the stored one?
    const typedHash = await hashPassword(p);
    if (typedHash !== account.hash) {
        if (errEl) errEl.innerHTML = "Invalid username or password.";
        return;
=======
// ==========================================
// STAFF / FACULTY PORTAL FUNCTIONS
// ==========================================
function showStaffTab(tabId) {
    var tabs = ['management', 'directory', 'profile', 'courses', 'lectures'];
    for (var i = 0; i < tabs.length; i++) {
        var el = document.getElementById('staff-' + tabs[i]);
        if (el) el.style.display = 'none';
    }
    var target = document.getElementById('staff-' + tabId);
    if (target) target.style.display = 'block';
}

function loadStaffVideo(title, posterImg) {
    var titleEl = document.getElementById("staff-video-title");
    var vid = document.getElementById("staff-video");

    if (titleEl) titleEl.innerText = title;
    if (vid) {
        vid.poster = posterImg;
        vid.load();
>>>>>>> Stashed changes
    }

    // Step 3: does the Role box agree with this account?
    if (r !== account.role) {
        if (errEl) errEl.innerHTML = "That username is a " + account.role +
            " account. Choose " + account.role + " in the Role box.";
        return;
    }

    // All three checks passed: start the session and open the right portal.
    setCurrentUser({ id: account.id, name: account.name, role: account.role });
    routeToRolePortal(account.role);
}

function saveGrade() {
    var student = document.getElementById("grade-student");
    var course = document.getElementById("grade-course");
    var score = document.getElementById("grade-score");

    if (!score || score.value === "") {
        alert("Please enter a valid grade score!");
        return;
    }
    alert("Successfully recorded " + score.value + "% for " + student.value + " in " + course.value + "!");
    score.value = "";
}

<<<<<<< Updated upstream
// -------------------------------------------------------------------------
// STUDENT PORTAL (Academia_Student.html)
// -------------------------------------------------------------------------
async function loadStudentPortal() {

    // Nobody reaches a portal without signing in first.
    if (!getCurrentUser()) {
        window.location.href = "Academia_Auth.html";
        return;
    }
    const user = getCurrentUser() || { id: 1, name: "Alex Rivera", role: "student" };
    const errorEl = document.getElementById("error");
=======
function saveAttendance() {
    var student = document.getElementById("attendance-student");
    var course = document.getElementById("attendance-course");
    var status = document.getElementById("attendance-status");
>>>>>>> Stashed changes

    if (!student || !course || !status) return;
    alert("Successfully recorded attendance as '" + status.value + "' for " + student.value + " in " + course.value + "!");
}

function postStaffComment() {
    var box = document.getElementById("staff-comment-box");
    if (!box) return;
    var txt = box.value;
    if (txt === "") {
        alert("Please write a message first!");
        return;
    }

    var section = document.getElementById("staff-comments-section");
    if (section) {
        section.innerHTML += "<p><b>Cmdr. Schenck (Instructor):</b> " + txt + "</p>";
    }
    box.value = "";
}

<<<<<<< Updated upstream
// -------------------------------------------------------------------------
// PARENT PORTAL (Academia_Parents.html)
// -------------------------------------------------------------------------
async function loadParentPortal() {

    // Nobody reaches a portal without signing in first.
    if (!getCurrentUser()) {
        window.location.href = "Academia_Auth.html";
        return;
    }
    const user = getCurrentUser() || { id: 3, name: "Marcus Rivera", role: "parent" };
=======
// ==========================================
// PARENT PORTAL FUNCTIONS
// ==========================================
function showParentTab(tabId) {
    var overview = document.getElementById("parent-overview");
    var announcements = document.getElementById("parent-announcements");
>>>>>>> Stashed changes

    if (!overview || !announcements) return;

    if (tabId === 'overview') {
        overview.style.display = 'block';
        announcements.style.display = 'none';
    } else if (tabId === 'announcements') {
        overview.style.display = 'none';
        announcements.style.display = 'block';
    }
<<<<<<< Updated upstream

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

    // parents_states.js keeps the child under "students" (a list).
    // The server sends it under "student" (one object). Accept both, so the
    // parent's own name is never replaced by the child's.
    let s = data.student || {};
    if (!s.firstName && Array.isArray(data.students) && data.students.length > 0) {
        s = data.students[0];
    }

    // The state file keeps the grades, attendance and announcements inside the
    // child; the server sends them at the top level. Read whichever is present.
    const grades = data.grades || s.grades || [];
    const attendance = data.attendance || s.attendance || [];
    const announcements = data.announcements || s.announcements || [];

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
    if (gradeList && grades.length > 0) {
        gradeList.innerHTML = grades.map(g => `
            <tr>
                <td><strong>${escapeHtml(g.course)}</strong></td>
                <td><span class="badge badge-score">${Number(g.score).toFixed(1)}%</span></td>
            </tr>
        `).join("");
    }

    // Attendance Table
    const attList = document.getElementById("attendanceList");
    if (attList && attendance.length > 0) {
        attList.innerHTML = attendance.map(a => {
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
    if (annList && announcements.length > 0) {
        annList.innerHTML = announcements.map(a => `
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

    // Nobody reaches a portal without signing in first.
    if (!getCurrentUser()) {
        window.location.href = "Academia_Auth.html";
        return;
    }
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
    // The state file (staff_states.js) stores students and courses like this:
    //
    //     students: [ { id: 1, name: "Alex Rivera" } ]
    //     courses:  [ { id: 1, name: "AP Computer Science A" } ]
    //
    // The server sends them as "enrollments" and "courses" with different
    // field names. Read both, so the dropdowns fill either way. Reading only
    // one shape is what left the course box empty and the options blank.

    // ---------------- students ----------------
    const studentMap = {};

    // From staff_states.js: students: [ { id, name } ]
    if (Array.isArray(data.students)) {
        data.students.forEach(st => {
            studentMap[st.id] = st.name;
        });
    }

    // From the server: enrollments: [ { studentId, student } ]
    if (Array.isArray(data.enrollments)) {
        data.enrollments.forEach(e => {
            studentMap[e.studentId] = e.student;
        });
    }

    // ---------------- courses ----------------
    const courseMap = {};

    // From staff_states.js: courses: [ { id, name } ]
    // From the server:       courses: [ { courseId, course } ]
    if (Array.isArray(data.courses)) {
        data.courses.forEach(c => {
            if (c.id !== undefined) {
                courseMap[c.id] = c.name;
            } else {
                courseMap[c.courseId] = c.course;
            }
        });
    }

    // Fall back to the courses that appear in the grades and attendance rows,
    // in case the courses list is ever missing.
    ['grades', 'attendance'].forEach(listName => {
        if (Array.isArray(data[listName])) {
            data[listName].forEach(row => {
                if (row.courseId !== undefined && row.course) {
                    courseMap[row.courseId] = row.course;
                }
            });
        }
    });

    // ---------------- build the options ----------------
    let studentOptions = '';
    Object.keys(studentMap).forEach(id => {
        studentOptions += '<option value="' + id + '">' +
            escapeHtml(studentMap[id]) + ' (ID: ' + id + ')</option>';
    });

    let courseOptions = '';
    Object.keys(courseMap).forEach(id => {
        courseOptions += '<option value="' + id + '">' +
            escapeHtml(courseMap[id]) + '</option>';
    });

    // ---------------- fill the three forms ----------------
    // Grade form
    const gradeStudentSelect = document.getElementById("studentName");
    const gradeCourseSelect = document.getElementById("course");
    if (gradeStudentSelect && studentOptions) gradeStudentSelect.innerHTML = studentOptions;
    if (gradeCourseSelect && courseOptions) gradeCourseSelect.innerHTML = courseOptions;

    // Attendance form
    const attStudentSelect = document.getElementById("attendanceName");
    const attCourseSelect = document.getElementById("attendanceCourse");
    if (attStudentSelect && studentOptions) attStudentSelect.innerHTML = studentOptions;
    if (attCourseSelect && courseOptions) attCourseSelect.innerHTML = courseOptions;

    // Announcement form. Keep the school-wide option at the top.
    const annCourseSelect = document.getElementById("announcementCourse");
    if (annCourseSelect && courseOptions) {
        annCourseSelect.innerHTML = '<option value="0">All Courses (School-Wide)</option>' + courseOptions;
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
<<<<<<< HEAD
    if (typeof studentState !== "undefined") loadStudent(studentState);
    else if (typeof parentState !== "undefined") loadParent(parentState);
    else if (typeof staffState !== "undefined") loadStaff(staffState);
    else setText("error", "Portal data not found. Log in again.");

});
=======
    const path = window.location.pathname.toLowerCase();

    // Which page are we on?
    //
    // The parent page and the student page share element ids (welcome,
    // studentName, gradeLevel, studentGpa, gradeList, attendanceList), so an
    // id test alone cannot tell them apart: both have studentGpa. The parent
    // test must come first, or the student loader runs on the parent page and
    // fills the banner with the child's name.
    if (path.includes("academia_parents.html") || document.getElementById("parentUsername")) {
        loadParentPortal();
    } else if (path.includes("academia_student.html") || document.getElementById("studentGpa")) {
        loadStudentPortal();
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
>>>>>>> 57558642637b144e2b83d4799c7f861c409ca802
=======
}
>>>>>>> Stashed changes
