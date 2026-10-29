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





//sends the username pw role to the backend
async function goPortal()
{
    let username = document.getElementById("username").value;
    let password = document.getElementById("password").value;
    let role = document.getElementById("role").value;

    let response = await fetch("/login", {
        method: "POST",
        headers: {
            "Content-Type": "application/json"
        },
        body: JSON.stringify({
            username: username,
            password: password,
            role: role
        })
    });
    let result = await response.text();

    if(result == "student")
    {
        window.location.href = "../Academia Student/Academia_Student.html";
    }        
    else if(result == "parent")
    {
        window.location.href = "";
    }
    else if(result == "staff")
    {
        window.location.href = "";
    }
}



