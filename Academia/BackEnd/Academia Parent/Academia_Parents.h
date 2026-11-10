/*
Harshul Gupta, Lawrence Mao, Suengho Kim, Dmitrii Buianov
CS50 C Project
Academia
October 27, 2026
*/

// We check for the header file if its defined or not and then we use define to
// use it and show the rest of the code in the file
#ifndef ACADEMIA_PARENTS_H // We use the "ifndef" to call whether the file is there
                           // or not there
#define ACADEMIA_PARENTS_H // We use the "define" to call the file to read the code
                           // inside after the "ifndef" is successful

// We add required libraries to make sure the code runs properly and for that we
// have used "sqlite3.h"
#include <sqlite3.h>

// Update the parent frontend state file (parents_states.js) with family records.
void update_parent_frontend_state(int parent_user_id);

#endif