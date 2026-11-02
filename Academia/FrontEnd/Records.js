
const defaultUsers = [
    {
        username: "student1",
        password: "123",
        role: "student",
        studentId: "STU01",
        firstName: "John",
        lastName: "Smith",
        gradeLevel: 12,
        gpa: 3.80
    },
    {
        username: "parent1",
        password: "123",
        role: "parent",
        studentId: "STU01",
        firstName: "Jane",
        lastName: "Smith"
        phone: "123-4567"
    },
    {
        username: "staff1",
        password: "123",
        role: "staff",
        studentId: "NONE",
        firstName: "John",
        lastName: "Smith"
    }
];

const defaultGrades = [
    { studentId: "STU01", course: "Computer Science", score: 95 },
    { studentId: "STU01", course: "Calculus", score: 88 },
    { studentId: "STU01", course: "Physics", score: 92 }
];

const defaultAttendance = [
    { studentId: "STU01", date: "2026-10-21", status: "Present" },
    { studentId: "STU01", date: "2026-10-22", status: "Present" },
    { studentId: "STU01", date: "2026-10-23", status: "Absent" }
];

const defaultAnnouncements = [
    {
        date: "2026-10-25",
        message: "Welcome to Academia! Quarter 1 grades are posted."
    },
    {
        date: "2026-10-26",
        message: "Parent-Teacher conferences are next week."
    }
];

const defaultAssignments = [
    {
        studentId: "STU01",
        course: "Computer Science",
        assignmentName: "Programming Project",
        dueDate: "2026-11-10"
    },
    {
        studentId: "STU01",
        course: "Calculus",
        assignmentName: "Derivative Homework",
        dueDate: "2026-11-12"
    }
];

// Initialize records if they do not exist
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

if (!localStorage.getItem("assignments")) {
    localStorage.setItem("assignments", JSON.stringify(defaultAssignments));
}
