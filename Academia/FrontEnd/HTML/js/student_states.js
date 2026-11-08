// Academia Student State
const studentState = {
    student: {
        userId: 1,
        firstName: "Alex",
        lastName: "Rivera",
        gradeLevel: 11,
        gpa: 3.85
    },
    grades: [
        { course: "AP Computer Science A", score: 96.5 },
        { course: "Advanced Mathematics", score: 92.0 },
        { course: "Physics with Calculus", score: 94.5 },
        { course: "World History & Civics", score: 89.0 }
    ],
    attendance: [
        { course: "AP Computer Science A", status: "Present", present: 1 },
        { course: "Advanced Mathematics", status: "Present", present: 1 },
        { course: "Physics with Calculus", status: "Present", present: 1 },
        { course: "World History & Civics", status: "Absent", present: 0 }
    ],
    assignments: [
        { course: "AP Computer Science A", assignmentName: "C Binary Search Tree Lab", dueDate: "2026-11-15" },
        { course: "Advanced Mathematics", assignmentName: "Integral Calculus Problem Set 5", dueDate: "2026-11-18" },
        { course: "Physics with Calculus", assignmentName: "Kinematics & Optics Lab Report", dueDate: "2026-11-20" }
    ],
    announcements: [
        { course: "AP Computer Science A", title: "CS Project Milestone Submission", message: "Please submit your C server and database integration milestones by Friday 5:00 PM.", date: "2026-11-06" },
        { course: "General", title: "Academia Unified Digital Ecosystem Live", message: "Welcome students, parents, and staff. Education Today. A Brighter Tomorrow.", date: "2026-11-01" }
    ]
};
