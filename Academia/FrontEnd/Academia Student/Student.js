
document.addEventListener("DOMContentLoaded", function () {
    const user = JSON.parse(localStorage.getItem("currentUser"));

    if (!user || user.role !== "student") {
        window.location.href =
            "../Academia Authorization/Academia_Auth.html";
        return;
    }

    const data = getAcademiaData();
    const student = data.student;

    document.querySelector(".blue-box h2").textContent =
        `Welcome ${student.firstName} ${student.lastName}!`;

    const info = document.querySelectorAll(".box")[0];

    info.innerHTML = `
        <h2>Student Information</h2>
        <p><strong>Name:</strong> ${student.firstName} ${student.lastName}</p>
        <p><strong>Student ID:</strong> ${student.id}</p>
        <p><strong>Grade Level:</strong> ${student.gradeLevel}</p>
        <p><strong>GPA:</strong> ${student.gpa.toFixed(2)}</p>
    `;

    const gradeTable = document.getElementById("gradesTable");
    gradeTable.innerHTML = "";

    data.grades
        .filter(grade => grade.studentId === student.id)
        .forEach(grade => {
            const row = document.createElement("tr");

            const courseCell = document.createElement("td");
            courseCell.textContent = grade.course;

            const scoreCell = document.createElement("td");
            scoreCell.textContent = grade.score;

            row.append(courseCell, scoreCell);
            gradeTable.appendChild(row);
        });

    // Attendance
    const attendanceBox = document.createElement("div");
    attendanceBox.className = "box";
    attendanceBox.innerHTML = `
        <h2>Attendance</h2>
        <table>
            <thead>
                <tr><th>Course</th><th>Status</th></tr>
            </thead>
            <tbody id="studentAttendance"></tbody>
        </table>
    `;

    document.body.insertBefore(
        attendanceBox,
        document.querySelector(".footer")
    );

    const attendanceTable =
        document.getElementById("studentAttendance");

    data.attendance
        .filter(record => record.studentId === student.id)
        .forEach(record => {
            const row = document.createElement("tr");
            row.innerHTML = `
                <td>${record.course}</td>
                <td>${record.status}</td>
            `;
            attendanceTable.appendChild(row);
        });

    // Announcements
    const announcementBox = document.createElement("div");
    announcementBox.className = "box";
    announcementBox.innerHTML = "<h2>Announcements</h2>";

    data.announcements.forEach(item => {
        const announcement = document.createElement("div");

        const title = document.createElement("h3");
        title.textContent = item.title;

        const date = document.createElement("p");
        date.textContent = item.date;

        const message = document.createElement("p");
        message.textContent = item.message;

        announcement.append(title, date, message);
        announcementBox.appendChild(announcement);
    });

    document.body.insertBefore(
        announcementBox,
        document.querySelector(".footer")
    );
});
