// Academia Parents State
const parentState = {
    parent: {
        userId: 3,
        firstName: "Marcus",
        lastName: "Rivera",
        username: "parent1",
        email: "m.rivera@example.com"
    },
    students: [
        {
            studentId: 1,
            firstName: "Alex",
            lastName: "Rivera",
            gradeLevel: 11,
            gpa: 3.85,
            grades: [
                { course: "AP Computer Science A", score: 96.5 },
                { course: "Advanced Mathematics", score: 92.0 },
                { course: "Physics with Calculus", score: 94.5 },
                { course: "World History & Civics", score: 89.0 }
            ],
            attendance: [
                { course: "AP Computer Science A", present: 1, status: "Present" },
                { course: "Advanced Mathematics", present: 1, status: "Present" },
                { course: "Physics with Calculus", present: 1, status: "Present" },
                { course: "World History & Civics", present: 0, status: "Absent" }
            ],
            announcements: [
                { course: "School-Wide", title: "Academia Unified Digital Ecosystem Live", message: "Welcome parents and guardians to the school community network.", date: "2026-11-01" },
                { course: "AP Computer Science A", title: "Upcoming CS Capstone Exhibition", message: "Student projects will be showcased in the science hall next week.", date: "2026-11-05" }
            ]
        }
    ]
};
