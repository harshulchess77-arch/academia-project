var defaultUsers = [
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
    { username: "parent1", password: "123", role: "parent", studentId: "STU01" },
    { username: "staff1", password: "123", role: "staff", studentId: "NONE" }
];

const students = [
    {
        studentId: "STU01",
        username: "Stu1",
        firstName: "John",
        lastName: "Smith",
        gradeLevel: 12,
        gpa: 3.80
    }
];

const parents = [
    {
        id: "PAR01",
        username: "Par1",
        firstName: "Jane",
        lastName: "Smith",
        phone: "555-0101",
        studentId: "STU01"
    }
];

const staff = [
    {
        id: "2001",
        username: "Sta1",
        firstName: "John",
        lastName: "Smith",
        position: "Mathematics Teacher",
        department: "Mathematics",
        email: "john.smith@school.edu"
    }
];

const courses = [
    { courseId: "C001", course: "Mathematics" },
    { courseId: "C002", course: "Science" },
    { courseId: "C003", course: "Spanish" }
];

let grades = [
    { studentId: "STU01", courseId: "C001", course: "Mathematics", score: 90 },
    { studentId: "STU01", courseId: "C002", course: "Science", score: 80 },
    { studentId: "STU01", courseId: "C003", course: "Spanish", score: 70 }
];

let attendance = [
    { studentId: "STU01", courseId: "C001", course: "Mathematics", status: "Present" },
    { studentId: "STU01", courseId: "C002", course: "Science", status: "Present" },
    { studentId: "STU01", courseId: "C003", course: "Spanish", status: "Absent" }
];

let announcements = [
    {
        id: "A001",
        courseId: "C001",
        title: "Math Exam",
        message: "Mathematics exam will be next Monday.",
        date: "2026-11-01"
    },
    {
        id: "A002",
        courseId: "C002",
        title: "Science Lab",
        message: "Remember to complete your lab report.",
        date: "2026-11-01"
    }
];

let assignments = [
    {
        id: "AS001",
        courseId: "C001",
        course: "Mathematics",
        assignmentName: "Math Homework",
        dueDate: "2026-11-10"
    },
    {
        id: "AS002",
        courseId: "C002",
        course: "Science",
        assignmentName: "Science Lab Report",
        dueDate: "2026-11-12"
    }
];
