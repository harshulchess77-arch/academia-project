# Architecture Overview
This document serves as a critical, living template designed to equip agents with a rapid and comprehensive understanding of the codebase's architecture, enabling efficient navigation and effective contribution from day one. Update this document as the codebase evolves.

## Project Structure
This section provides a high-level overview of the project's directory and file structure, categorised by architectural layer or major functional area. It is essential for quickly navigating the codebase, locating relevant files, and understanding the overall organization and separation of concerns.

[academia-project]/
├── Academia/             # Main application directory
│   ├── BackEnd/          # C-based backend modules
│   │   ├── Academia Common/       # Shared utilities and common functions
│   │   │   └── Academia_common.c
│   │   ├── Academia Database/     # Database schema and initialization
│   │   │   └── Academia.sql
│   │   ├── Academia School Staff/ # Staff-specific business logic
│   │   │   └── Academia_School_Staff.c
│   │   ├── Academia Student/      # Student-specific business logic
│   │   │   └── Academia_Student.c
│   │   └── Academia Parents/     # Parent-specific business logic
│   │       └── Academia_Parents.c
│   └── FrontEnd/         # HTML/CSS/JS frontend interface
│       ├── Academia Authrization/  # Login and authentication UI
│       │   └── Academia_Auth.html
│       ├── Academia Landing/      # Landing page
│       │   └── Academia_Landing.html
│       ├── Academia Parents/      # Parent portal UI
│       │   └── Academia_Parents.html
│       ├── Academia School Staff/ # Staff portal UI
│       │   └── Academia_School_Staff.html
│       ├── Academia Student/      # Student portal UI
│       │   └── Academia_Student.html
│       ├── Academia.css           # Global styles
│       └── Academia.js            # Client-side JavaScript
├── Images/               # Project images and documentation assets
│   └── BrandSheet.png    # Academia branding information, logo, and banner
├── README.md             # Project overview about Academia
└── ARCHITECTURE.md       # This document shows user about how the project is laid out about Academia
