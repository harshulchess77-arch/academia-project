# Architecture Overview

This document provides a detailed guidance to Academia's repository layout. Academia is being developed as a modular school management system, with C modules for application logic and data access, SQLite for structured records, and HTML/CSS/JavaScript files for the user interface. The directory structure shows planned module boundaries, but not every module or integration is provided as completed version as we mentioned to be provided with as little features and later add on if time permits.

## Project Structure

The tree below reflects the current repository. Comments distinguish existing files from intended responsibilities; they do not imply that a feature is finished.

[academia-project]/
├── Academia/
│   ├── BackEnd/                         # C application and data-access code
│   │   ├── Academia Common/
│   │   │   └── Academia_Common.c        # Shared entry point; opens SQLite database
│   │   ├── Academia Database/
│   │   │   ├── Academia.sql             
│   │   │   └── SQLite3/
│   │   │       ├── sqlite3.c             # SQLite implementation source
│   │   │       └── sqlite3.h             # SQLite API declarations
│   │   ├── Academia Parents/
│   │   │   └── Academia_Parents.c       # Parent module scaffold
│   │   ├── Academia School Staff/
│   │   │   └── Academia_School_Staff.c  # Staff module scaffold
│   │   └── Academia Student/
│   │       ├── Academia_Student.c       # Student data/GPA/attendance prototype
│   ├── FrontEnd/                        # HTML/CSS/JS presentation layer
│   │   ├── Academia Authrization/       # Login mockup and styles
│   │   │   ├── Academia_Auth.html
│   │   │   └── Auth_Style.css
│   │   ├── Academia Landing/
│   │   │   └── Academia_Landing.html    # Landing page mockup
│   │   ├── Academia Parents/
│   │   │   ├── Academia_Parents.html    # Parent dashboard mockup
│   │   │   └── Par_Style.css
│   │   ├── Academia School Staff/
│   │   │   ├── Academia_School_Staff.html # Staff dashboard mockup
│   │   │   └── Tea_Style.css
│   │   ├── Academia Student/
│   │   │   ├── Academia_Student.html    # Student dashboard mockup
│   │   │   └── Stu_style.css
│   │   ├── Academia.css                 # Shared styles
│   │   └── Academia.js                  # Shared frontend behavior (currently empty)
│   └── Images/                          # Branding and interface/error images
├── .vscode/
│   └── settings.json                    # VS Code workspace settings
├── readme.md                            # Project overview and setup information
└── ARCHITECTURE.md                      # This architecture and repository guide