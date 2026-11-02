
window.onload = function () {

    // Get the logged-in username
    const username = sessionStorage.getItem("username") || "student1";

    // Get student records from localStorage
    const users = JSON.parse(localStorage.getItem("users")) || [];
    const student = users.find(
        s => s.username === username && s.role === "student"
    );

    if (!student) {
        document.getElementById("error").textContent =
            "Student data could not be loaded.";
        return;
    }

    // Display student information
    document.getElementById("welcome").textContent =
        "Welcome, " + student.firstName + " " + student.lastName + "!";

    document.getElementById("studentName").textContent =
        student.firstName + " " + student.lastName;

    document.getElementById("studentId").textContent = student.studentId;
    document.getElementById("gradeLevel").textContent = student.gradeLevel;
    document.getElementById("studentGpa").textContent = student.gpa;

    // Display grades
    showGrades(student.studentId);

    // Display attendance
    showAttendance(student.studentId);

    // Display assignments
    showAssignments();

    // Display announcements
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


function showAssignments() {

    const list = document.getElementById("assignmentList");

    const records = JSON.parse(
        localStorage.getItem("assignments")
    ) || [];

    list.innerHTML = "";

    records.forEach(a => {
        list.innerHTML += `
            <tr>
                <td>${a.course}</td>
                <td>${a.assignmentName}</td>
                <td>${a.dueDate}</td>
            </tr>
        `;
    });
}

function showAnnouncements() {

    const list = document.getElementById("announcementList");
    const records = JSON.parse(localStorage.getItem("announcements")) || [];

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
