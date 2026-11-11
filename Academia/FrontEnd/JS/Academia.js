/*
Harshul Gupta, Lawrence Mao, Suengho Kim, Dmitrii Buianov
CS50 C Project
Academia JavaScript Logic
November 1, 2026
*/

// AUTHENTICATION & LOGIN LOGIC
// We define the goPortal function to handle user sign-in verification and backend redirection
function goPortal() {
    /* We grab references to the input fields and error container from the DOM */
    var usernameField = document.getElementById("username");
    var passwordField = document.getElementById("password");
    var roleField = document.getElementById("role");
    var errEl = document.getElementById("err");
    /* We check if any essential element is missing before proceeding */
    if (!usernameField || !passwordField || !roleField) return;
    /* We extract and trim the text values typed by the user */
    var username = usernameField.value.trim();
    var password = passwordField.value.trim();
    var role = roleField.value;
    /* We validate that neither username nor password fields are left blank */
    if (username === "" || password === "") {
        if (errEl) errEl.innerText = "Please enter both username and password.";
        return;
    }
    /* We use the fetch API to send a POST request to our C backend server endpoint '/api/login' */
    fetch('/api/login', {
        method: 'POST',
        headers: {
            'Content-Type': 'application/json'
        },
        body: JSON.stringify({
            username: username,
            password: password,
            role: role
        })
    })
    .then(response => response.json())
    .then(data => {
        /* We check if the server returned a successful login flag */
        if (data.success) {
            /* We conditionally redirect the user to their respective portal page based on their role */
            if (role === 'student') {
                window.location.href = 'Academia_Student.html';
            } else if (role === 'parent') {
                window.location.href = 'Academia_Parents.html';
            } else if (role === 'staff') {
                window.location.href = 'Academia_Staff.html';
            }
        } else {
            /* If login fails, we display the error message provided by the server */
            if (errEl) errEl.innerText = data.message || "Invalid login credentials.";
        }
    })
    .catch(error => {
        console.error('Backend connection notice:', error);
        /* We provide a fallback redirection for offline testing or static preview environments */
        if (role === 'student') {
            window.location.href = 'Academia_Student.html';
        } else if (role === 'parent') {
            window.location.href = 'Academia_Parents.html';
        } else if (role === 'staff') {
            window.location.href = 'Academia_Staff.html';
        }
    });
}

// STUDENT PORTAL FUNCTIONS
// We define showPage to toggle between different views inside the student portal
function showPage(pageId) {
    var dash = document.getElementById("dashboard-page");
    var courses = document.getElementById("courses-page");
    if (!dash || !courses) return;
    /* We display the dashboard view and hide the courses view when selected */
    if (pageId === 'dashboard') {
        dash.style.display = 'block';
        courses.style.display = 'none';
    /* We display the courses view, hide the dashboard, and reset the list view */
    } else if (pageId === 'courses') {
        dash.style.display = 'none';
        courses.style.display = 'block';
        backToList();
    }
}
// We define openWorkspace to load specific course lecture modules and video players
function openWorkspace(courseTitle) {
    var listView = document.getElementById("course-list-view");
    var workspaceView = document.getElementById("course-workspace-view");
    var titleEl = document.getElementById("workspace-title");
    
    if (!listView || !workspaceView) return;
    /* We hide the course list card layout and reveal the interactive workspace */
    listView.style.display = "none";
    workspaceView.style.display = "block";
    if (titleEl) titleEl.innerText = courseTitle + " - Cmdr. Schenck";
    var playlistDiv = document.getElementById("playlist-links");
    var htmlContent = "";
    var prefix = "post-1";
    /* We assign custom thumbnail image prefixes depending on which course was selected */
    if (courseTitle === "Complete CSS Tutorial") {
        prefix = "post-2";
    } else if (courseTitle === "Complete JavaScript Tutorial") {
        prefix = "post-3";
    }
    /* We generate an array containing 6 lessons for the course playlist */
    var lessons = [];
    for (var i = 1; i <= 6; i++) {
        lessons.push({
            title: "Part 0" + i + " Lesson",
            thumb: "../Images/" + prefix + "-" + i + ".png"
        });
    }
    /* We loop through the lessons to build the clickable playlist HTML elements */
    for (var j = 0; j < lessons.length; j++) {
        var les = lessons[j];
        htmlContent += '<div class="lesson-item" onclick="changeVideo(\'' + courseTitle + ' (' + les.title + ')\', \'' + les.thumb + '\', \'../Images/vid-1.mp4\')">';
        htmlContent += '<img src="' + les.thumb + '" onerror="this.style.display=\'none\';">';
        htmlContent += '<span>' + les.title + '</span></div>';
    }
    if (playlistDiv) playlistDiv.innerHTML = htmlContent;
    var vid = document.getElementById("my-video");
    var src = document.getElementById("video-source");
    var vidTitle = document.getElementById("video-title");
    
    /* We reset the video player state with the first lesson's poster image */
    if (vid && src) {
        src.src = "";
        vid.poster = lessons[0].thumb;
        vid.load();
    }
    if (vidTitle) vidTitle.innerText = "Please select a lesson from the playlist";
}
// We define backToList to return the user from the video workspace back to the main course catalog
function backToList() {
    var listView = document.getElementById("course-list-view");
    var workspaceView = document.getElementById("course-workspace-view");
    if (listView && workspaceView) {
        listView.style.display = "block";
        workspaceView.style.display = "none";
    }
}
// We define changeVideo to dynamically switch video sources and posters when a user clicks a playlist item
function changeVideo(title, posterImg, videoFile) {
    var vidTitle = document.getElementById("video-title");
    var vid = document.getElementById("my-video");
    var src = document.getElementById("video-source");
    if (vidTitle) vidTitle.innerText = title;
    if (vid && src) {
        src.src = videoFile;
        vid.poster = posterImg;
        vid.load();
        vid.play().catch(function() {});
    }
}
/* We initialize a counter variable to track video likes */
var likes = 44;
// We define likeVideo to increment and update the like counter on screen
function likeVideo() {
    likes++;
    var likesEl = document.getElementById("likes");
    if (likesEl) likesEl.innerText = likes;
}
// We define addComment to let students post questions and feedback to the course discussion board
function addComment() {
    var box = document.getElementById("comment-box");
    if (!box) return;
    var txt = box.value;
    /* We validate that the comment box is not empty before submitting */
    if (txt === "") {
        alert("Please write a comment first!");
        return;
    }
    var section = document.getElementById("comments-section");
    if (section) {
        section.innerHTML += "<p><b>Alex Rivera:</b> " + txt + "</p>";
    }
    box.value = "";
}

// STAFF PORTAL FUNCTIONS
// We define an array of grades to show on the page. The first few are sample
// records so the roster is not empty when the page opens, and any grade saved
// from the form is added to this same array
var savedGrades = [
    { student: "Alex Rivera",  course: "Complete HTML Tutorial",       score: "97.0" },
    { student: "Alex Rivera",  course: "Complete CSS Tutorial",        score: "96.5" },
    { student: "Alex Rivera",  course: "Complete JavaScript Tutorial", score: "95.0" },
    { student: "Sophia Chen",  course: "Complete HTML Tutorial",       score: "98.0" },
    { student: "Sophia Chen",  course: "Complete CSS Tutorial",        score: "94.0" },
    { student: "Sophia Chen",  course: "Complete JavaScript Tutorial", score: "92.5" }
];
// We define an array of attendance records to show on the page. These first few
// are sample records, and any attendance submitted is added to this same array
var savedAttendance = [
    { student: "Alex Rivera",  course: "Complete HTML Tutorial",       status: "Present" },
    { student: "Alex Rivera",  course: "Complete CSS Tutorial",        status: "Present" },
    { student: "Alex Rivera",  course: "Complete JavaScript Tutorial", status: "Absent"  },
    { student: "Sophia Chen",  course: "Complete HTML Tutorial",       status: "Present" },
    { student: "Sophia Chen",  course: "Complete CSS Tutorial",        status: "Present" }
];

// We define showStaffTab to switch between different management tabs in the staff portal
function showStaffTab(tabId) {
    var tabs = ['management', 'directory', 'profile', 'courses', 'lectures'];
    for (var i = 0; i < tabs.length; i++) {
        var el = document.getElementById('staff-' + tabs[i]);
        if (el) el.style.display = 'none';
    }
    var target = document.getElementById('staff-' + tabId);
    if (target) target.style.display = 'block';
    /* We redraw the tables whenever the class management tab is opened */
    if (tabId === 'management') loadStaffRecords();
}
// We define loadStaffVideo to let teachers preview curriculum videos inside the faculty dashboard
function loadStaffVideo(title, posterImg) {
    var titleEl = document.getElementById("staff-video-title");
    var vid = document.getElementById("staff-video");

    if (titleEl) titleEl.innerText = title;
    if (vid) {
        vid.poster = posterImg;
        vid.load();
    }
}
// We define loadStaffRecords to draw the two tables from the arrays above
function loadStaffRecords() {
    /* We build the grade rows from the grades saved so far */
    var grades = document.getElementById("grade-rows");
    if (grades) {
        var rows = "";
        for (var i = 0; i < savedGrades.length; i++) {
            rows += "<tr><td>" + savedGrades[i].student + "</td><td>" + savedGrades[i].course +
                    "</td><td>" + savedGrades[i].score + "%</td></tr>";
        }
        if (rows === "") rows = "<tr><td colspan='3'>No grades recorded yet.</td></tr>";
        grades.innerHTML = rows;
    }

    /* We build the attendance rows the same way */
    var attendance = document.getElementById("attendance-rows");
    if (attendance) {
        var arows = "";
        for (var j = 0; j < savedAttendance.length; j++) {
            arows += "<tr><td>" + savedAttendance[j].student + "</td><td>" + savedAttendance[j].course +
                     "</td><td>" + savedAttendance[j].status + "</td></tr>";
        }
        if (arows === "") arows = "<tr><td colspan='3'>No attendance recorded yet.</td></tr>";
        attendance.innerHTML = arows;
    }
}
// We define saveGrade to add the entered grade to the roster table on the page
function saveGrade() {
    var student = document.getElementById("grade-student");
    var course = document.getElementById("grade-course");
    var score = document.getElementById("grade-score");
    var known;

    if (!student || !course || !score) return;
    /* We validate that a score has been typed before saving */
    if (score.value === "") {
        alert("Please enter a valid grade score!");
        return;
    }

    known = false;

    /* We replace the score if this student already has a grade for this course */
    for (var i = 0; i < savedGrades.length; i++) {
        if (savedGrades[i].student === student.value && savedGrades[i].course === course.value) {
            savedGrades[i].score = score.value;
            known = true;
        }
    }

    if (!known) {
        savedGrades.push({
            student: student.value,
            course: course.value,
            score: score.value
        });
    }

    alert("Successfully recorded " + score.value + "% for " + student.value + " in " + course.value + "!");
    score.value = "";
    /* We redraw the table so the new grade appears straight away */
    loadStaffRecords();
}
// We define saveAttendance to add the chosen status to the attendance table on the page
function saveAttendance() {
    var student = document.getElementById("attendance-student");
    var course = document.getElementById("attendance-course");
    var status = document.getElementById("attendance-status");

    if (!student || !course || !status) return;

    savedAttendance.push({
        student: student.value,
        course: course.value,
        status: status.value
    });

    alert("Successfully recorded attendance as '" + status.value + "' for " + student.value + " in " + course.value + "!");
    /* We redraw the table so the new record appears straight away */
    loadStaffRecords();
}
// We define postStaffComment to let instructors publish announcements or feedback responses
function postStaffComment() {
    var box = document.getElementById("staff-comment-box");
    if (!box) return;
    var txt = box.value;
    if (txt === "") {
        alert("Please write a message first!");
        return;
    }
    var section = document.getElementById("staff-comments-section");
    if (section) {
        section.innerHTML += "<p><b>Cmdr. Schenck (Instructor):</b> " + txt + "</p>";
    }
    box.value = "";
}

// PARENT PORTAL FUNCTIONS
// We define showParentTab to toggle between viewing child grade summaries and school announcements
function showParentTab(tabId) {
    var overview = document.getElementById("parent-overview");
    var announcements = document.getElementById("parent-announcements");
    if (!overview || !announcements) return;
    if (tabId === 'overview') {
        overview.style.display = 'block';
        announcements.style.display = 'none';
    } else if (tabId === 'announcements') {
        overview.style.display = 'none';
        announcements.style.display = 'block';
    }
}

// We draw the staff tables as soon as the page opens
window.addEventListener('load', loadStaffRecords);
