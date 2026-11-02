
window.onload = function () {

    const parent = parents.find(
        p => p.username === (sessionStorage.getItem("username") || "Par1")
    );

    if (!parent) {
        document.getElementById("error").textContent =
            "Parent data could not be loaded.";
        return;
    }

    document.getElementById("welcome").textContent =
        "Welcome, " + parent.firstName + " " + parent.lastName;

    document.getElementById("parentUsername").textContent =
        parent.username;

    document.getElementById("parentPhone").textContent =
        parent.phone;

    const student = students.find(
        s => s.studentId === parent.studentId
    );

    if (!student) {
        document.getElementById("studentName").textContent =
            "No connected student";

        return;
    }

    document.getElementById("studentName").textContent =
        student.firstName + " " + student.lastName;

    document.getElementById("gradeLevel").textContent =
        student.gradeLevel;

    document.getElementById("studentGpa").textContent =
        student.gpa;

    showGrades(student.studentId);
    showAttendance(student.studentId);
    showAnnouncements();
};

function showGrades(studentId) {

    const list = document.getElementById("gradeList");
    list.innerHTML = "";

    const records = grades.filter(g => g.studentId == studentId);

    records.forEach(g => {
        list.innerHTML += `
            <tr>
                <td>${g.course}</td>
                <td>${g.score}</td>
            </tr>
        `;
    });

    if (records.length == 0) {
        list.innerHTML = "<tr><td colspan='2'>No grades available.</td></tr>";
    }
}

function showAttendance(studentId) {

    const list = document.getElementById("attendanceList");
    list.innerHTML = "";

    const records = attendance.filter(a => a.studentId == studentId);

    records.forEach(a => {
        list.innerHTML += `
            <tr>
                <td>${a.course}</td>
                <td>${a.status}</td>
            </tr>
        `;
    });

    if (records.length == 0) {
        list.innerHTML = "<tr><td colspan='2'>No attendance available.</td></tr>";
    }
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
