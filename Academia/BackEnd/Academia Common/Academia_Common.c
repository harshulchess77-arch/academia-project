/*
Lawrence Mao
Seungho Kim
Harshul Gupta
Dmitrii Buianov

CS50 C Project
Common Backend Functions
10.22.26
*/

#include <stdio.h>

#include "../Academia Database/SQLite3 Source Code/sqlite3.h"
#include "Academia_Common.h"


/*
The program is expected to run from the project root folder,
which directly contains the Academia folder.
*/
#define DATABASE_PATH \
    "Academia/BackEnd/Academia Database/academia.db"


/*
Opens the Academia database and returns its connection.

The caller is responsible for closing the returned connection
with sqlite3_close when it is no longer needed.

Returns NULL when the database cannot be opened.
*/
sqlite3 *open_academia_database(void)
{
    sqlite3 *db = NULL;

    if (sqlite3_open(DATABASE_PATH, &db) != SQLITE_OK)
    {
        printf(
            "[Database Error] Cannot open database: %s\n",
            sqlite3_errmsg(db)
        );

        sqlite3_close(db);

        return NULL;
    }

    return db;
}


/*
Finalizes a prepared SQLite statement and resets its pointer.

Resetting the pointer prevents the program from accidentally
attempting to reuse a statement after it has been finalized.
*/
void finalize_statement(sqlite3_stmt **stmt)
{
    if (stmt != NULL && *stmt != NULL)
    {
        sqlite3_finalize(*stmt);
        *stmt = NULL;
    }
}


/*
Writes SQLite text safely inside a JavaScript string.

The quotation marks surrounding the JavaScript string must be
written by the calling function.
*/
void write_js_text(
    FILE *out,
    const unsigned char *text
)
{
    int i;

    if (out == NULL || text == NULL)
    {
        return;
    }

    for (i = 0; text[i] != '\0'; i++)
    {
        switch (text[i])
        {
            case '\\':
            case '"':
                fputc('\\', out);
                fputc(text[i], out);
                break;

            case '\n':
                fputs("\\n", out);
                break;

            case '\r':
                fputs("\\r", out);
                break;

            case '\t':
                fputs("\\t", out);
                break;

            default:
                fputc(text[i], out);
        }
    }
}