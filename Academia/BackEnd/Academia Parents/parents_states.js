const parentState = {
  parent: {
    userId: 2,
    username: "Par1",
    firstName: "C",
    lastName: "D",
    phone: "770-555-0100"
  },
  students: [
    { studentId: 1, username: "Stu1", firstName: "A", lastName: "B", gradeLevel: 11, gpa: 3.50 },
  ],
  grades: [
    { studentId: 1, course: "Math", score: 90.00 },
    { studentId: 1, course: "Science", score: 80.00 },
    { studentId: 1, course: "Spanish", score: 70.00 },
  ],
  attendance: [
    { studentId: 1, course: "Math", status: "Present" },
    { studentId: 1, course: "Science", status: "Present" },
    { studentId: 1, course: "Spanish", status: "Absent" },
  ],
  announcements: [
    { title: "Science Project", message: "The Science project is due next week.", date: "2026-11-09" },
    { title: "Math Test", message: "The Math test is scheduled for Friday.", date: "2026-11-06" },
  ],
  success: true,
  error: ""
};
