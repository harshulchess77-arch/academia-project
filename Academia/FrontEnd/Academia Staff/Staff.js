
window.onload = function () {

    const currentStaff = staff.find(
        s => s.username === (sessionStorage.getItem("username") || "Sta1")
    );

    if (!currentStaff) {
        alert("Staff data could not be loaded.");
        return;
    }

    document.getElementById("welcome").textContent =
        "Welcome, " + currentStaff.firstName + " " + currentStaff.lastName + "!";

    document.getElementById("staffName").textContent =
        currentStaff.firstName + " " + currentStaff.lastName;

    document.getElementById("staffId").textContent = currentStaff.id;
    document.getElementById("staffPosition").textContent = currentStaff.position;
    document.getElementById("staffDepartment").textContent = currentStaff.department;
    document.getElementById("staffEmail").textContent = currentStaff.email;

    loadStudents();
    loadCourses();

    showGrades();
    showAttendance();
    showAnnouncements();
};

function loadStudents() {

    const studentOptions = students.map(s => `
        <option value="${s.studentId}">
            ${s.firstName} ${s.lastName}
        </option>
    `).join("");

    document.getElementById("studentName").innerHTML = studentOptions;
    document.getElementById("attendanceName").innerHTML = studentOptions;
}

function loadCourses() {

    const courseOptions = courses.map(c => `
        <option value="${c.courseId}">
            ${c.course}
        </option>
    `).join("");

    document.getElementById("course").innerHTML = courseOptions;
    document.getElementById("attendanceCourse").innerHTML = courseOptions;
    document.getElementById("announcementCourse").innerHTML = courseOptions;
}

function showGrades() {

    const list = document.getElementById("gradeList");
    list.innerHTML = "";

    grades.forEach(g => {

        const student = students.find(s => s.studentId == g.studentId);

        list.innerHTML += `
            <tr>
                <td>${student.firstName} ${student.lastName}</td>
                <td>${g.course}</td>
                <td>${g.score}</td>
            </tr>
        `;
    });
}

function showAttendance() {

    const list = document.getElementById("attendanceList");
    list.innerHTML = "";

    attendance.forEach(a => {

        const student = students.find(s => s.studentId == a.studentId);

        list.innerHTML += `
            <tr>
                <td>${student.firstName} ${student.lastName}</td>
                <td>${a.course}</td>
                <td>${a.status}</td>
            </tr>
        `;
    });
}

function showAnnouncements() {

    const list = document.getElementById("announcementList");
    list.innerHTML = "";

    announcements.forEach(a => {

        list.innerHTML += `
            <h3>${a.title}</h3>
            <p>${a.date}</p>
            <p>${a.message}</p>
            <hr>
        `;
    });
}

// Add or update a grade
document.getElementById("gradeForm").addEventListener("submit", function(event) {

    event.preventDefault();

    const studentId = document.getElementById("studentName").value;
    const courseId = document.getElementById("course").value;
    const score = Number(document.getElementById("grade").value);

    const course = courses.find(c => c.courseId == courseId);

    const existingGrade = grades.find(
        g => g.studentId == studentId && g.courseId == courseId
    );

    if (existingGrade) {
        existingGrade.score = score;
    } else {
        grades.push({
            studentId: studentId,
            courseId: courseId,
            course: course.course,
            score: score
        });
    }

    showGrades();

    this.reset();
});

// Record attendance
document.getElementById("attendanceForm").addEventListener("submit", function(event) {

    event.preventDefault();

    const studentId = document.getElementById("attendanceName").value;
    const courseId = document.getElementById("attendanceCourse").value;
    const status = document.getElementById("attendanceStatus").value;

    const course = courses.find(c => c.courseId == courseId);

    const existingRecord = attendance.find(
        a => a.studentId == studentId && a.courseId == courseId
    );

    if (existingRecord) {
        existingRecord.status = status;
    } else {
        attendance.push({
            studentId: studentId,
            courseId: courseId,
            course: course.course,
            status: status
        });
    }

    showAttendance();

    this.reset();
});

// Post announcement
document.getElementById("announcementForm").addEventListener("submit", function(event) {

    event.preventDefault();

    const courseId = document.getElementById("announcementCourse").value;
    const title = document.getElementById("announcementTitle").value;
    const message = document.getElementById("announcementMessage").value;

    announcements.unshift({
        id: "A" + Date.now(),
        courseId: courseId,
        title: title,
        message: message,
        date: new Date().toISOString().slice(0, 10)
    });

    showAnnouncements();

    this.reset();
});
