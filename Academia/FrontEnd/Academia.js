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
    { date: "2026-10-25", message: "Welcome to Academia! Quarter 1 grades are posted." }
];

if (!localStorage.getItem("users")) {
    localStorage.setItem("users", JSON.stringify(defaultUsers));
    localStorage.setItem("grades", JSON.stringify(defaultGrades));
    localStorage.setItem("attendance", JSON.stringify(defaultAttendance));
    localStorage.setItem("announcements", JSON.stringify(defaultAnnouncements));
}