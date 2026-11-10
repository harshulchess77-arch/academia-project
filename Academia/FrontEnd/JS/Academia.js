// STUDENT PORTAL FUNCTIONS
function showPage(pageId) {
    var dash = document.getElementById("dashboard-page");
    var courses = document.getElementById("courses-page");

    if (!dash || !courses) return;

    if (pageId === 'dashboard') {
        dash.style.display = 'block';
        courses.style.display = 'none';
    } else if (pageId === 'courses') {
        dash.style.display = 'none';
        courses.style.display = 'block';
        backToList();
    }
}

function openWorkspace(courseTitle) {
    var listView = document.getElementById("course-list-view");
    var workspaceView = document.getElementById("course-workspace-view");
    var titleEl = document.getElementById("workspace-title");
    
    if (!listView || !workspaceView) return;

    listView.style.display = "none";
    workspaceView.style.display = "block";
    if (titleEl) titleEl.innerText = courseTitle + " - Cmdr. Schenck";

    var playlistDiv = document.getElementById("playlist-links");
    var htmlContent = "";
    var prefix = "post-1";

    if (courseTitle === "Complete CSS Tutorial") {
        prefix = "post-2";
    } else if (courseTitle === "Complete JavaScript Tutorial") {
        prefix = "post-3";
    }

    var lessons = [];
    for (var i = 1; i <= 6; i++) {
        lessons.push({
            title: "Part 0" + i + " Lesson",
            thumb: "../Images/" + prefix + "-" + i + ".png"
        });
    }

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
    
    if (vid && src) {
        src.src = "";
        vid.poster = lessons[0].thumb;
        vid.load();
    }
    if (vidTitle) vidTitle.innerText = "Please select a lesson from the playlist";
}

function backToList() {
    var listView = document.getElementById("course-list-view");
    var workspaceView = document.getElementById("course-workspace-view");
    if (listView && workspaceView) {
        listView.style.display = "block";
        workspaceView.style.display = "none";
    }
}

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

var likes = 44;
function likeVideo() {
    likes++;
    var likesEl = document.getElementById("likes");
    if (likesEl) likesEl.innerText = likes;
}

function addComment() {
    var box = document.getElementById("comment-box");
    if (!box) return;
    var txt = box.value;
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
function showStaffTab(tabId) {
    var tabs = ['management', 'directory', 'profile', 'courses', 'lectures'];
    for (var i = 0; i < tabs.length; i++) {
        var el = document.getElementById('staff-' + tabs[i]);
        if (el) el.style.display = 'none';
    }
    var target = document.getElementById('staff-' + tabId);
    if (target) target.style.display = 'block';
}

function loadStaffVideo(title, posterImg) {
    var titleEl = document.getElementById("staff-video-title");
    var vid = document.getElementById("staff-video");

    if (titleEl) titleEl.innerText = title;
    if (vid) {
        vid.poster = posterImg;
        vid.load();
    }
}

function saveGrade() {
    var student = document.getElementById("grade-student");
    var course = document.getElementById("grade-course");
    var score = document.getElementById("grade-score");

    if (!score || score.value === "") {
        alert("Please enter a valid grade score!");
        return;
    }
    alert("Successfully recorded " + score.value + "% for " + student.value + " in " + course.value + "!");
    score.value = "";
}

function saveAttendance() {
    var student = document.getElementById("attendance-student");
    var course = document.getElementById("attendance-course");
    var status = document.getElementById("attendance-status");

    if (!student || !course || !status) return;
    alert("Successfully recorded attendance as '" + status.value + "' for " + student.value + " in " + course.value + "!");
}

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