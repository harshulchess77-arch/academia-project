/*
Dmitri Buiamov
CS50 C Project
Database Setup
10.27.26
*/

#include <stdio.h>
#include <stdlib.h>
#include "SQLite3/sqlite3.h"

int main(void)
{
    sqlite3 *db = NULL;
    char *error_message = NULL;

    const char *database_path =
        "Academia/BackEnd/Academia Database/academia.db";

    const char *sql_path =
        "Academia/BackEnd/Academia Database/Academia.sql";

    /*
    Open or create academia.db.
    */
    if (sqlite3_open(database_path, &db) != SQLITE_OK)
    {
        fprintf(
            stderr,
            "[Error] Could not open database: %s\n",
            sqlite3_errmsg(db)
        );

        if (db != NULL)
        {
            sqlite3_close(db);
        }

        return 1;
    }

    printf("[Database] Connected to academia.db.\n");

    /*
    Open Academia.sql.
    */
    FILE *sql_file = fopen(sql_path, "rb");

    if (sql_file == NULL)
    {
        fprintf(
            stderr,
            "[Error] Could not open Academia.sql.\n"
        );

        sqlite3_close(db);
        return 1;
    }

    /*
    Find the size of Academia.sql.
    */
    if (fseek(sql_file, 0, SEEK_END) != 0)
    {
        fprintf(
            stderr,
            "[Error] Could not read Academia.sql.\n"
        );

        fclose(sql_file);
        sqlite3_close(db);
        return 1;
    }

    long file_size = ftell(sql_file);

    if (file_size < 0)
    {
        fprintf(
            stderr,
            "[Error] Could not determine SQL file size.\n"
        );

        fclose(sql_file);
        sqlite3_close(db);
        return 1;
    }

    rewind(sql_file);

    /*
    Allocate memory for the SQL text.
    */
    char *sql_text =
        malloc((size_t) file_size + 1);

    if (sql_text == NULL)
    {
        fprintf(
            stderr,
            "[Error] Could not allocate memory.\n"
        );

        fclose(sql_file);
        sqlite3_close(db);
        return 1;
    }

    size_t bytes_read = fread(
        sql_text,
        1,
        (size_t) file_size,
        sql_file
    );

    sql_text[bytes_read] = '\0';

    fclose(sql_file);

    /*
    Execute all commands from Academia.sql.
    */
    int result = sqlite3_exec(
        db,
        sql_text,
        NULL,
        NULL,
        &error_message
    );

    free(sql_text);

    if (result != SQLITE_OK)
    {
        fprintf(
            stderr,
            "[Error] SQL setup failed: %s\n",
            error_message != NULL
                ? error_message
                : "Unknown SQL error"
        );

        if (error_message != NULL)
        {
            sqlite3_free(error_message);
        }

        sqlite3_close(db);
        return 1;
    }

    printf(
        "[Database] Tables were created successfully.\n"
    );

    sqlite3_close(db);

    printf("[Database] Connection closed.\n");

    return 0;
}