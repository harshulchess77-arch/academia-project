
// staff.js

const API = "/api";

// Get staff ID from the URL or localStorage
const params = new URLSearchParams(window.location.search);
const STAFF_ID = params.get("staff_id") || localStorage.getItem("staff_id");

// Check staff ID
if (!STAFF_ID) {
    alert("Staff ID not found. Please log in again.");
}

// Send requests to the server
async function apiRequest(endpoint, method = "GET", data = null) {
    const options = {
        method,
        headers: {
            "Content-Type": "application/json"
        }
    };

    if (data) {
        options.body = JSON.stringify(data);
    }

    const response = await fetch(API + endpoint, options);

    if (!response.ok) {
        throw new Error("Request failed: " + response.status);
    }

    return await response.json();
}

// Display data in an HTML element
function displayList(elementId, items, formatter) {
    const element = document.getElementById(elementId);

    if (!element) return;

    element.innerHTML = "";

    if (!items || items.length === 0) {
        element.textContent = "No data available.";
        return;
    }

    items.forEach(item => {
        const div = document.createElement("div");
        div.innerHTML = formatter(item);
        element.appendChild(div);
    });
}

// Load staff dashboard
async function loadStaffDashboard() {
    try {
        const data = await apiRequest(
            `/staff/dashboard?staff_id=${encodeURIComponent(STAFF_ID)}`
        );

        // Staff profile
        const nameElement = document.getElementById("staffName");

        if (nameElement && data.staff) {
            nameElement.textContent =
                `${data.staff.first_name || ""} ${data.staff.last_name || ""}`;
        }

        // Courses
        displayList("coursesList", data.courses, course => `
            <p>${course.course_name}</p>
        `);

        // Students
        displayList("studentsList", data.students, student => `
            <p>
                ${student.first_name || ""} ${student.last_name || ""}
                — Grade ${student.grade_level || "N/A"}
            </p>
        `);

        // Assignments
        displayList("assignmentsList", data.assignments, assignment => `
            <p>
                ${assignment.assignment_name}
                — Due: ${assignment.due_date}
            </p>
        `);

        // Grades
        displayList("gradesList", data.grades, grade => `
            <p>
                Student ID: ${grade.student_id}
                — Course ID: ${grade.course_id}
                — Score: ${grade.score}
            </p>
        `);

        // Attendance
        displayList("attendanceList", data.attendance, record => `
            <p>
                Student ID: ${record.student_id}
                — ${record.present ? "Present" : "Absent"}
            </p>
        `);

        // Announcements
        displayList("announcementsList", data.announcements, announcement => `
            <p>
                <strong>${announcement.title}</strong><br>
                ${announcement.message}
            </p>
        `);

    } catch (error) {
        console.error("Dashboard error:", error);
        alert("Could not load the staff dashboard.");
    }
}

// Submit a grade
document.getElementById("gradeForm")?.addEventListener("submit", async event => {
    event.preventDefault();

    try {
        await apiRequest("/grades", "POST", {
            staff_id: Number(STAFF_ID),
            student_id: Number(document.getElementById("studentId").value),
            course_id: Number(document.getElementById("courseId").value),
            score: Number(document.getElementById("score").value)
        });

        alert("Grade added successfully.");
        event.target.reset();
        loadStaffDashboard();

    } catch (error) {
        console.error(error);
        alert("Failed to add grade.");
    }
});

// Submit attendance
document.getElementById("attendanceForm")?.addEventListener("submit", async event => {
    event.preventDefault();

    try {
        await apiRequest("/attendance", "POST", {
            staff_id: Number(STAFF_ID),
            student_id: Number(document.getElementById("attendanceStudentId").value),
            course_id: Number(document.getElementById("attendanceCourseId").value),
            present: Number(document.getElementById("attendanceStatus").value)
        });

        alert("Attendance recorded.");
        event.target.reset();
        loadStaffDashboard();

    } catch (error) {
        console.error(error);
        alert("Failed to record attendance.");
    }
});

// Submit announcement
document.getElementById("announcementForm")?.addEventListener("submit", async event => {
    event.preventDefault();

    try {
        await apiRequest("/announcements", "POST", {
            staff_id: Number(STAFF_ID),
            course_id: Number(document.getElementById("announcementCourseId").value),
            title: document.getElementById("announcementTitle").value,
            message: document.getElementById("announcementMessage").value
        });

        alert("Announcement published.");
        event.target.reset();
        loadStaffDashboard();

    } catch (error) {
        console.error(error);
        alert("Failed to publish announcement.");
    }
});

// Submit assignment
document.getElementById("assignmentForm")?.addEventListener("submit", async event => {
    event.preventDefault();

    try {
        await apiRequest("/assignments", "POST", {
            staff_id: Number(STAFF_ID),
            course_id: Number(document.getElementById("assignmentCourseId").value),
            assignment_name: document.getElementById("assignmentName").value,
            due_date: document.getElementById("assignmentDueDate").value
        });

        alert("Assignment created.");
        event.target.reset();
        loadStaffDashboard();

    } catch (error) {
        console.error(error);
        alert("Failed to create assignment.");
    }
});

// Load dashboard when page opens
document.addEventListener("DOMContentLoaded", loadStaffDashboard);
