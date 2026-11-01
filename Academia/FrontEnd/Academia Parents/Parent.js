/*
Lawrence Mao
CS50 C Project
Parent Dashboard Logic
*/

window.onload = function ()
{
    if (typeof parentState === "undefined")
    {
        document.getElementById("error").innerHTML =
            "Parent data could not be loaded.";

        return;
    }

    if (parentState.success === false)
    {
        document.getElementById("error").innerHTML =
            parentState.error;

        return;
    }

    loadParentInfo();

    if (parentState.students.length > 0)
    {
        const student = parentState.students[0];

        loadStudentInfo(student);

        showGrades(student.studentId);
        showAttendance(student.studentId);
    }
    else
    {
        showNoStudentState();
    }

    showAnnouncements();
};

function loadParentInfo()
{
    document.getElementById("welcome").innerHTML =
        "Welcome, " +
        parentState.parent.firstName +
        " " +
        parentState.parent.lastName;

    document.getElementById("parentUsername").innerHTML =
        parentState.parent.username;

    document.getElementById("parentPhone").innerHTML =
        parentState.parent.phone;
}

function loadStudentInfo(student)
{
    document.getElementById("studentName").innerHTML =
        student.firstName +
        " " +
        student.lastName;

    document.getElementById("gradeLevel").innerHTML =
        student.gradeLevel;

    document.getElementById("studentGpa").innerHTML =
        student.gpa.toFixed(2);
}

function showNoStudentState()
{
    document.getElementById("studentName").innerHTML =
        "No connected student";

    document.getElementById("gradeLevel").innerHTML =
        "Not available";

    document.getElementById("studentGpa").innerHTML =
        "Not available";

    document.getElementById("gradeList").innerHTML =
        "<tr><td colspan='2'>No grades available.</td></tr>";

    document.getElementById("attendanceList").innerHTML =
        "<tr><td colspan='2'>No attendance available.</td></tr>";
}

function showGrades(studentId)
{
    const gradeList =
        document.getElementById("gradeList");

    gradeList.innerHTML = "";

    let found = false;

    parentState.grades.forEach(grade =>
    {
        if (grade.studentId === studentId)
        {
            found = true;

            gradeList.innerHTML +=
                `<tr>
                    <td>${grade.course}</td>
                    <td>${grade.score}</td>
                </tr>`;
        }
    });

    if (!found)
    {
        gradeList.innerHTML =
            "<tr><td colspan='2'>No grades available.</td></tr>";
    }
}

function showAttendance(studentId)
{
    const attendanceList =
        document.getElementById("attendanceList");

    attendanceList.innerHTML = "";

    let found = false;

    parentState.attendance.forEach(record =>
    {
        if (record.studentId === studentId)
        {
            found = true;

            attendanceList.innerHTML +=
                `<tr>
                    <td>${record.course}</td>
                    <td>${record.status}</td>
                </tr>`;
        }
    });

    if (!found)
    {
        attendanceList.innerHTML =
            "<tr><td colspan='2'>No attendance available.</td></tr>";
    }
}

function showAnnouncements()
{
    const announcementList =
        document.getElementById("announcementList");

    announcementList.innerHTML = "";

    if (parentState.announcements.length === 0)
    {
        announcementList.innerHTML =
            "<p>No announcements available.</p>";

        return;
    }

    parentState.announcements.forEach(announcement =>
    {
        announcementList.innerHTML +=
            `
            <div class="announcement">
                <h3>${announcement.title}</h3>

                <p>
                    <strong>Date:</strong>
                    ${announcement.date}
                </p>

                <p>
                    ${announcement.message}
                </p>

                <hr>
            </div>
            `;
    });
}