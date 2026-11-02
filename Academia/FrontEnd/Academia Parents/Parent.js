
window.onload = function () {

    // Get logged-in username
    const username = sessionStorage.getItem("username") || "parent1";

    // Get users from localStorage
    const users = JSON.parse(localStorage.getItem("users")) || [];

    const parent = users.find(
        p => p.username === username && p.role === "parent"
    );

    if (!parent) {
        document.getElementById("error").textContent =
            "Parent data could not be loaded.";
        return;
    }

    // Display parent information
    document.getElementById("welcome").textContent =
        "Welcome, " + parent.firstName + " " + parent.lastName;

    document.getElementById("parentUsername").textContent =
        parent.username;

    document.getElementById("parentPhone").textContent =
        parent.phone || "Not available";

    // Find connected student
    const student = users.find(
        s => s.studentId === parent.studentId && s.role === "student"
    );

    if (!student) {
        document.getElementById("studentName").textContent =
            "No connected student";
        return;
    }

    // Display student information
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

    const records = JSON.parse(localStorage.getItem("grades")) || [];

    list.innerHTML = "";

    const studentGrades = records.filter(
        g => g.studentId === studentId
    );

    studentGrades.forEach(g => {
        list.innerHTML += `
            <tr>
                <td>${g.course}</td>
                <td>${g.score}</td>
            </tr>
        `;
    });

    if (studentGrades.length === 0) {
        list.innerHTML =
            "<tr><td colspan='2'>No grades available.</td></tr>";
    }
}


function showAttendance(studentId) {

    const list = document.getElementById("attendanceList");

    const records = JSON.parse(localStorage.getItem("attendance")) || [];

    list.innerHTML = "";

    const studentAttendance = records.filter(
        a => a.studentId === studentId
    );

    studentAttendance.forEach(a => {
        list.innerHTML += `
            <tr>
                <td>${a.date}</td>
                <td>${a.status}</td>
            </tr>
        `;
    });

    if (studentAttendance.length === 0) {
        list.innerHTML =
            "<tr><td colspan='2'>No attendance available.</td></tr>";
    }
}


function showAnnouncements() {

    const list = document.getElementById("announcementList");

    const records =
        JSON.parse(localStorage.getItem("announcements")) || [];

    list.innerHTML = "";

    records.forEach(a => {
        list.innerHTML += `
            <h3>${a.date}</h3>
            <p>${a.message}</p>
            <hr>
        `;
    });

    if (records.length === 0) {
        list.innerHTML = "No announcements available.";
    }
}
