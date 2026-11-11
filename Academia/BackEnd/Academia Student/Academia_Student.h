/*
Harshul Gupta, Lawrence Mao, Suengho Kim, Dmitrii Buianov
CS50 C Project
Academia
October 27, 2026
*/

// We check for the header file if its defined or not and then we use define to
// use it and show the rest of the code in the file
#ifndef ACADEMIA_STUDENT_H // We use the "ifndef" to call whether the file is there or not there

#define ACADEMIA_STUDENT_H // We use the "define" to call the file to read the code inside after the "ifndef" is successful

// We add required libraries to make sure the code runs properly and for that we have used "sqlite3.h"
#include <sqlite3.h>

// Update the student frontend state file (student_states.js) with database records.
void update_student_frontend_state(int student_user_id);

#endif