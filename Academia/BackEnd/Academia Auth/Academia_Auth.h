/*
Harshul Gupta, Lawrence Mao, Suengho Kim, Dmitrii Buianov
CS50 C Project
Academia
October 27, 2026
*/

/* Academia Auth Header */

#ifndef ACADEMIA_AUTH_H
#define ACADEMIA_AUTH_H

#include <sqlite3.h>
#include <stddef.h>

/* Sign a user in. Returns 1 on success and fills in the token, id, role and
   name. Returns 0 if the username, password and role do not all match. */
int auth_login(sqlite3 *database, const char *username, const char *password,
               char *response, size_t response_size, char *token,
               size_t token_size, int *user_id, char *role, size_t role_size,
               char *name, size_t name_size);

/* Check a session token. Returns 1 if it is valid, 0 if not. */
int auth_check(const char *token, int *user_id, char *role, size_t role_size,
               char *name, size_t name_size);

/* Forget one token. Passing "" forgets every session. */
void auth_logout(const char *token);

#endif
