var defaultUsers = [
    { username: "student1", password: "123", role: "student", studentId: "STU01" },
    { username: "parent1", password: "123", role: "parent", studentId: "STU01" },
    { username: "staff1", password: "123", role: "staff", studentId: "NONE" }
];

var defaultGrades = [
    { studentId: "STU01", course: "Computer Science", score: 95 },
    { studentId: "STU01", course: "Calculus", score: 88 },
    { studentId: "STU01", course: "Physics", score: 92 }
];

var defaultAttendance = [
    { studentId: "STU01", date: "2026-10-21", status: "Present" },
    { studentId: "STU01", date: "2026-10-22", status: "Present" },
    { studentId: "STU01", date: "2026-10-23", status: "Absent" }
];

var defaultAnnouncements = [
    { date: "2026-10-25", message: "Welcome to Academia! Quarter 1 grades are posted." },
    { date: "2026-10-26", message: "Parent-Teacher conferences are next week." }
];

if (!localStorage.getItem("users")) {
    localStorage.setItem("users", JSON.stringify(defaultUsers));
}
if (!localStorage.getItem("grades")) {
    localStorage.setItem("grades", JSON.stringify(defaultGrades));
}
if (!localStorage.getItem("attendance")) {
    localStorage.setItem("attendance", JSON.stringify(defaultAttendance));
}
if (!localStorage.getItem("announcements")) {
    localStorage.setItem("announcements", JSON.stringify(defaultAnnouncements));
}



/*
Parent Portal Functions
This section only runs on Academia_Parents.html.
*/

document.addEventListener("DOMContentLoaded", function () {
    var parentPage =
        document.getElementById("student-information");

    /*
    Stop if the current page is not the Parent Portal.
    */
    if (parentPage === null) {
        return;
    }

    /*
    parentState is created by parent_state.js.
    */
    if (typeof parentState === "undefined") {
        parentShowLoadingError(
            "parent_state.js could not be loaded. " +
            "Run parent.c first and confirm that parent_state.js " +
            "is in the Academia Parents folder."
        );

        return;
    }

    parentDisplayParentInformation();
    parentDisplayAnnouncements();

    if (
        !Array.isArray(parentState.students) ||
        parentState.students.length === 0
    ) {
        parentShowNoStudent();
        return;
    }

    parentCreateStudentSelector(
        parentState.students
    );

    parentDisplayStudent(
        parentState.students[0]
    );
});


function parentDisplayParentInformation() {
    var parentNameElement =
        document.getElementById("parent-name");

    var firstName =
        parentState.first_name || "";

    var lastName =
        parentState.last_name || "";

    var fullName =
        (firstName + " " + lastName).trim();

    if (fullName === "") {
        fullName =
            parentState.username || "Parent";
    }

    parentNameElement.textContent =
        "Welcome, " + fullName;
}


function parentCreateStudentSelector(students) {
    if (students.length <= 1) {
        return;
    }

    var selectorBox =
        document.getElementById(
            "student-selector-box"
        );

    var selector =
        document.getElementById(
            "student-selector"
        );

    selectorBox.style.display = "block";
    selector.innerHTML = "";

    students.forEach(function (student, index) {
        var option =
            document.createElement("option");

        var firstName =
            student.first_name || "";

        var lastName =
            student.last_name || "";

        var fullName =
            (firstName + " " + lastName).trim();

        if (fullName === "") {
            fullName =
                student.username || "Student";
        }

        option.value = index;

        option.textContent =
            fullName +
            " (Student ID: " +
            student.student_id +
            ")";

        selector.appendChild(option);
    });

    selector.addEventListener(
        "change",
        function () {
            var selectedIndex =
                Number(selector.value);

            parentDisplayStudent(
                students[selectedIndex]
            );
        }
    );
}


function parentDisplayStudent(student) {
    document.getElementById(
        "student-id"
    ).textContent =
        student.student_id;

    var firstName =
        student.first_name || "";

    var lastName =
        student.last_name || "";

    var fullName =
        (firstName + " " + lastName).trim();

    if (fullName === "") {
        fullName =
            student.username || "Unknown Student";
    }

    document.getElementById(
        "student-name"
    ).textContent =
        fullName;

    document.getElementById(
        "student-username"
    ).textContent =
        student.username || "Unknown";

    document.getElementById(
        "grade-level"
    ).textContent =
        student.grade_level;

    document.getElementById(
        "gpa-box"
    ).textContent =
        parentFormatNumber(
            student.stored_gpa,
            2
        );

    document.getElementById(
        "average-box"
    ).textContent =
        parentFormatNumber(
            student.average_grade,
            2
        ) + "%";

    document.getElementById(
        "att-box"
    ).textContent =
        parentFormatNumber(
            student.attendance_rate,
            1
        ) + "%";

    parentDisplayGrades(
        student.grades
    );

    parentDisplayAttendance(
        student.attendance
    );
}


function parentDisplayGrades(grades) {
    var gradeTable =
        document.getElementById(
            "student-grades"
        );

    gradeTable.innerHTML = "";

    if (
        !Array.isArray(grades) ||
        grades.length === 0
    ) {
        parentAddEmptyTableMessage(
            gradeTable,
            "No grades available."
        );

        return;
    }

    grades.forEach(function (gradeRecord) {
        var row =
            document.createElement("tr");

        var courseCell =
            document.createElement("td");

        var gradeCell =
            document.createElement("td");

        courseCell.textContent =
            gradeRecord.course ||
            "Unknown Course";

        gradeCell.textContent =
            parentFormatNumber(
                gradeRecord.grade,
                0
            ) + "%";

        var gradeNumber =
            Number(gradeRecord.grade);

        if (gradeNumber >= 90) {
            gradeCell.style.color = "green";
        }
        else if (gradeNumber < 70) {
            gradeCell.style.color = "red";
        }
        else {
            gradeCell.style.color = "";
        }

        row.appendChild(courseCell);
        row.appendChild(gradeCell);

        gradeTable.appendChild(row);
    });
}


function parentDisplayAttendance(attendance) {
    var attendanceTable =
        document.getElementById(
            "student-attendance"
        );

    attendanceTable.innerHTML = "";

    if (
        !Array.isArray(attendance) ||
        attendance.length === 0
    ) {
        parentAddEmptyTableMessage(
            attendanceTable,
            "No attendance records available."
        );

        return;
    }

    attendance.forEach(function (record) {
        var row =
            document.createElement("tr");

        var dateCell =
            document.createElement("td");

        var statusCell =
            document.createElement("td");

        dateCell.textContent =
            record.date || "Unknown Date";

        statusCell.textContent =
            record.status || "Unknown";

        var normalizedStatus =
            String(
                record.status || ""
            ).toLowerCase();

        if (normalizedStatus === "present") {
            statusCell.style.color = "green";
        }
        else if (normalizedStatus === "absent") {
            statusCell.style.color = "red";
        }
        else if (normalizedStatus === "late") {
            statusCell.style.color = "orange";
        }
        else {
            statusCell.style.color = "";
        }

        row.appendChild(dateCell);
        row.appendChild(statusCell);

        attendanceTable.appendChild(row);
    });
}


function parentDisplayAnnouncements() {
    var announcementArea =
        document.getElementById(
            "student-announcements"
        );

    announcementArea.innerHTML = "";

    if (
        !Array.isArray(
            parentState.announcements
        ) ||
        parentState.announcements.length === 0
    ) {
        var emptyMessage =
            document.createElement("p");

        emptyMessage.textContent =
            "No school announcements available.";

        announcementArea.appendChild(
            emptyMessage
        );

        return;
    }

    parentState.announcements.forEach(
        function (announcement) {
            var announcementItem =
                document.createElement("div");

            var title =
                document.createElement("h4");

            var message =
                document.createElement("p");

            title.textContent =
                announcement.title ||
                "School Announcement";

            message.textContent =
                announcement.message || "";

            announcementItem.appendChild(
                title
            );

            announcementItem.appendChild(
                message
            );

            announcementArea.appendChild(
                announcementItem
            );
        }
    );
}


function parentAddEmptyTableMessage(
    tableBody,
    message
) {
    var row =
        document.createElement("tr");

    var cell =
        document.createElement("td");

    cell.colSpan = 2;
    cell.textContent = message;

    row.appendChild(cell);
    tableBody.appendChild(row);
}


function parentShowNoStudent() {
    document.getElementById(
        "student-information"
    ).style.display = "none";

    document.getElementById(
        "grades-box"
    ).style.display = "none";

    document.getElementById(
        "attendance-box"
    ).style.display = "none";

    document.getElementById(
        "no-student-box"
    ).style.display = "block";
}


function parentShowLoadingError(message) {
    document.getElementById(
        "student-information"
    ).style.display = "none";

    document.getElementById(
        "grades-box"
    ).style.display = "none";

    document.getElementById(
        "attendance-box"
    ).style.display = "none";

    var errorBox =
        document.getElementById(
            "error-box"
        );

    var errorMessage =
        document.getElementById(
            "error-message"
        );

    errorBox.style.display = "block";
    errorMessage.textContent = message;

    document.getElementById(
        "student-announcements"
    ).textContent =
        "Announcements could not be loaded.";

    console.error(message);
}


function parentFormatNumber(
    value,
    decimalPlaces
) {
    var number = Number(value);

    if (!Number.isFinite(number)) {
        number = 0;
    }

    return number.toFixed(
        decimalPlaces
    );
}