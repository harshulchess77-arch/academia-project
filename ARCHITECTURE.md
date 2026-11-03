<!--
Harshul Gupta, Lawrence Mao, Suengho Kim, Dmitrii Buianov
CS50 C Project
Academia
November 1, 2026
-->
# Architecture Overview

Academia is a school management project with C backend modules, SQLite database files, and an HTML/CSS/JavaScript frontend. This document describes the files and folders currently present in the repository; their presence does not imply that every feature or integration is complete.

## Project Structure

The tree below reflects the current working tree, including source files, documentation, workspace settings, database and SQLite utility files, and image assets.

[academia-project]/
├── .vscode/
│   └── settings.json
├── Academia/
│   ├── BackEnd/
│   │   ├── Academia Auth/
│   │   │   ├── Academia_Auth.c
│   │   │   └── Academia_Auth.h
│   │   ├── Academia Common/
│   │   │   ├── Academia_Common.c
│   │   │   └── Academia_Common.h
│   │   ├── Academia Database/
│   │   │   ├── Academia.sql
│   │   │   ├── Database.c
│   │   │   ├── Database.h
│   │   │   ├── academia.db
│   │   │   └── SQLite3 Source Code/
│   │   │       ├── SQLite3 Source Code
│   │   │       ├── sqlite3.c
│   │   │       ├── sqlite3.exe
│   │   │       └── sqlite3.h
│   │   ├── Academia Parents/
│   │   │   ├── Academia_Parents.c
│   │   │   └── Academia_Parents.h
│   │   ├── Academia Server/
│   │   │   └── Server.c
│   │   ├── Academia Staff/
│   │   │   ├── Academia_Staff.c
│   │   │   └── Academia_Staff.h
│   │   └── Academia Student/
│   │       ├── Academia_Student.c
│   │       └── Academia_Student.h
│   ├── FrontEnd/
│   │   ├── Academia Student/
│   │   │   └── Academia_Student.html
│   │   ├── CSS/
│   │   │   └── Academia.css
│   │   ├── HTML/
│   │   │   ├── 403 Error Page.html
│   │   │   ├── 429 Error Page.html
│   │   │   ├── 500 Error Page.html
│   │   │   ├── 504 Error Page.html
│   │   │   ├── Academia_Auth.html
│   │   │   ├── Academia_Landing.html
│   │   │   ├── Academia_Parents.html
│   │   │   ├── Academia_Staff.html
│   │   │   ├── Academia_Student.html
│   │   │   ├── Account Locked Page.html
│   │   │   ├── LinkExpired.html
│   │   │   ├── No Internet Connection.html
│   │   │   ├── Session Expired.html
│   │   │   └── Under Maintainence Page.html
│   │   └── JS/
│   │       └── Academia.js
│   └── Images/
│       ├── 403Error.png
│       ├── 429Error.png
│       ├── 500Error.png
│       ├── 504Error.png
│       ├── AccountLocked.png
│       ├── AuthPage1.png
│       ├── AuthPage2.png
│       ├── AuthPage3.png
│       ├── AuthPage4.png
│       ├── BrandSheet.png
│       ├── Footer.png
│       ├── LandingFeatures1.png
│       ├── LandingFeatures2.png
│       ├── LandingFeatures3.png
│       ├── LandingParents.png
│       ├── LandingStaff.png
│       ├── LandingStudents.png
│       ├── LinkExpired.png
│       ├── NoInternetConnection.png
│       ├── Parent_Dash_Banner.png
│       ├── SessionExpired.png
│       ├── Student_Dash_Banner.png
│       ├── Teacher_Dash_Banner.png
│       ├── UnderMaintenance.png
│       └── UploadFailed.png
├── ARCHITECTURE.md
└── README.md