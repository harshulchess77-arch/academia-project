// Academia Staff State
const staffState = {
    staff: {
        userId: 4,
        firstName: "Dr. Eleanor",
        lastName: "Vance",
        department: "Computer Science",
        phone: "(555) 987-6543"
    },
    students: [
        { id: 1, name: "Alex Rivera", gradeLevel: 11 },
        { id: 2, name: "Sophia Chen", gradeLevel: 12 }
    ],
    courses: [
        { id: 1, name: "AP Computer Science A" },
        { id: 2, name: "Advanced Mathematics" },
        { id: 3, name: "Physics with Calculus" }
    ],
    grades: [
        { studentId: 1, student: "Alex Rivera", courseId: 1, course: "AP Computer Science A", score: 96.5 },
        { studentId: 2, student: "Sophia Chen", courseId: 1, course: "AP Computer Science A", score: 91.0 },
        { studentId: 1, student: "Alex Rivera", courseId: 2, course: "Advanced Mathematics", score: 92.0 },
        { studentId: 2, student: "Sophia Chen", courseId: 2, course: "Advanced Mathematics", score: 98.0 },
        { studentId: 1, student: "Alex Rivera", courseId: 3, course: "Physics with Calculus", score: 94.5 }
    ],
    attendance: [
        { attendanceId: 1, studentId: 1, student: "Alex Rivera", courseId: 1, course: "AP Computer Science A", present: 1, status: "Present" },
        { attendanceId: 2, studentId: 2, student: "Sophia Chen", courseId: 1, course: "AP Computer Science A", present: 1, status: "Present" },
        { attendanceId: 3, studentId: 1, student: "Alex Rivera", courseId: 2, course: "Advanced Mathematics", present: 1, status: "Present" },
        { attendanceId: 4, studentId: 1, student: "Alex Rivera", courseId: 3, course: "Physics with Calculus", present: 0, status: "Absent" }
    ],
    announcements: [
        { announcementId: 1, courseId: 1, course: "AP Computer Science A", title: "CS Project Milestone Submission", message: "Please submit your C server and database integration milestones by Friday.", date: "2026-11-06" },
        { announcementId: 2, courseId: 0, course: "All Courses (School-Wide)", title: "End of Quarter Evaluations", message: "Please ensure all assignment submissions and quiz re-takes are finalized.", date: "2026-11-04" }
    ]
};
