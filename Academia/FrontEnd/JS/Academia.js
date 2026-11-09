/*
Harshul Gupta, Lawrence Mao, Suengho Kim, Dmitrii Buianov
CS50 C Project
Academia
November 1, 2026
*/
/*Harshul Gupta, Lawrecne Mao, Suengho Kim, Dmitrii Buianov
    APCS50
    3rd Period
    November 3, 2026
*/


const $ = id => document.getElementById(id);
const setText = (id, v) => { const el = $(id); if (el) el.textContent = v; };
const esc = s => String(s ?? "").replace(/[&<>"']/g,
    c => ({ "&": "&amp;", "<": "&lt;", ">": "&gt;", '"': "&quot;", "'": "&#39;" }[c]));
const status = p => (p ? "Present" : "Absent");

// Fills a <tbody> with rows. cols(row) returns the cell values (or HTML if raw).
function fillTable(id, items, cols, emptyText = "No records yet.", raw = false) {
    const el = $(id);
    if (!el) return;
    el.innerHTML = items.length
        ? items.map(r => "<tr>" + cols(r)
            .map(c => `<td>${raw ? c : esc(c)}</td>`).join("") + "</tr>").join("")
        : `<tr><td colspan="9">${emptyText}</td></tr>`;
}

function fillAnnouncements(items, showCourse = false) {
    const el = $("announcementList");
    if (!el) return;
    el.innerHTML = items.length
        ? items.map(a => `<p><strong>${esc(a.title)}</strong>`
            + (showCourse && a.course ? ` (${esc(a.course)})` : "")
            + ` - ${esc(a.date)}<br>${esc(a.message)}</p>`).join("")
        : "<p>No announcements.</p>";
}

function fillOptions(id, items, value, label) {
    const el = $(id);
    if (!el) return;
    el.innerHTML = items.map(i =>
        `<option value="${esc(i[value])}">${esc(i[label])}</option>`).join("");
}

function showError(s) {
    if (!s.success) setText("error", s.error);
}

/* ---------------- Student ---------------- */
function loadStudent(s) {
    showError(s);
    const st = s.student || {};
    const name = `${st.firstName ?? ""} ${st.lastName ?? ""}`.trim();

    setText("welcome", `Welcome, ${name || "Student"}!`);
    setText("studentName", name);
    setText("studentId", st.userId ?? "");
    setText("gradeLevel", st.gradeLevel ?? "");
    setText("studentGpa", st.gpa != null ? st.gpa.toFixed(2) : "");

    fillTable("gradeList", s.grades, g => [g.course, g.score]);
    fillTable("attendanceList", s.attendance, a => [a.course, status(a.present)]);
    fillTable("assignmentList", s.assignments, a => [a.course, a.name, a.due]);
    fillAnnouncements(s.announcements);
}

/* ---------------- Parent ---------------- */
function loadParent(s) {
    showError(s);
    const p = s.parent || {};
    const kid = s.students[0]; // shows the first linked student

    setText("welcome", `Welcome, ${p.firstName ?? "Parent"}!`);
    setText("parentUsername", p.username ?? "");
    setText("parentPhone", p.phone ?? "");

    if (kid) {
        setText("studentName", `${kid.firstName} ${kid.lastName}`);
        setText("gradeLevel", kid.gradeLevel);
        setText("studentGpa", kid.gpa.toFixed(2));
    } else {
        ["studentName", "gradeLevel", "studentGpa"].forEach(i => setText(i, "No student linked"));
    }

    const mine = list => list.filter(r => kid && r.studentId === kid.studentId);
    fillTable("gradeList", mine(s.grades), g => [g.course, g.score]);
    fillTable("attendanceList", mine(s.attendance), a => [a.course, status(a.present)]);
    fillAnnouncements(s.announcements);
}

/* ---------------- Staff ---------------- */
// A browser page cannot write to SQLite, so new entries are kept in
// localStorage as "pending" until the C backend applies them.
function loadStaff(s) {
    const KEY = "academiaStaffPending";
    const pending = JSON.parse(localStorage.getItem(KEY)
        || '{"grades":[],"attendance":[],"announcements":[]}');
    const save = () => localStorage.setItem(KEY, JSON.stringify(pending));
    const label = id => $(id).selectedOptions[0]?.text ?? "";

    const t = s.staff || {};
    setText("welcome", `Welcome, ${t.firstName ?? "Staff"}!`);
    setText("staffName", `${t.firstName ?? ""} ${t.lastName ?? ""}`.trim());
    setText("staffId", t.userId ?? "");
    setText("staffPhone", t.phone ?? "");

    // Dropdowns: unique students + this staff member's courses.
    const students = [...new Map(s.enrollments.map(e => [e.studentId, e])).values()];
    ["studentName", "attendanceName"].forEach(id => fillOptions(id, students, "studentId", "student"));
    ["course", "attendanceCourse", "announcementCourse"]
        .forEach(id => fillOptions(id, s.courses, "courseId", "course"));

    const del = (type, i) =>
        `<button class="btn" data-type="${type}" data-i="${i}">Remove</button>`;

    function render() {
        const withAction = (saved, pend, type) => [
            ...saved.map(r => ({ r, a: "Saved" })),
            ...pend.map((r, i) => ({ r, a: del(type, i) }))
        ];

        const g = withAction(s.grades, pending.grades, "grades");
        fillTable("gradeList", g, x => [esc(x.r.student), esc(x.r.course), x.r.score, x.a],
            "No grades yet.", true);

        const a = withAction(s.attendance, pending.attendance, "attendance");
        fillTable("attendanceList", a,
            x => [esc(x.r.student), esc(x.r.course), status(x.r.present), x.a],
            "No attendance yet.", true);

        fillAnnouncements([...pending.announcements, ...s.announcements], true);
    }

    $("gradeForm").onsubmit = e => {
        e.preventDefault();
        pending.grades.push({
            studentId: +$("studentName").value, student: label("studentName"),
            courseId: +$("course").value, course: label("course"),
            score: parseFloat($("grade").value)
        });
        save(); render(); e.target.reset();
    };

    $("attendanceForm").onsubmit = e => {
        e.preventDefault();
        pending.attendance.push({
            studentId: +$("attendanceName").value, student: label("attendanceName"),
            courseId: +$("attendanceCourse").value, course: label("attendanceCourse"),
            present: +$("attendanceStatus").value
        });
        save(); render();
    };

    $("announcementForm").onsubmit = e => {
        e.preventDefault();
        pending.announcements.unshift({
            courseId: +$("announcementCourse").value, course: label("announcementCourse"),
            title: $("announcementTitle").value, message: $("announcementMessage").value,
            date: new Date().toISOString().slice(0, 10)
        });
        save(); render(); e.target.reset();
    };

    document.addEventListener("click", e => {
        const b = e.target.closest("button[data-type]");
        if (!b) return;
        pending[b.dataset.type].splice(+b.dataset.i, 1);
        save(); render();
    });

    render();
}

/* ---------------- Start ---------------- */
document.addEventListener("DOMContentLoaded", () => {
    if (typeof studentState !== "undefined") loadStudent(studentState);
    else if (typeof parentState !== "undefined") loadParent(parentState);
    else if (typeof staffState !== "undefined") loadStaff(staffState);
    else setText("error", "Portal data not found. Log in again.");

});
