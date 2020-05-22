#include <ncursesw/curses.h>
#include <sqlite3.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#define MidWinH 15
#define MidWinW 55
#define SmallWinH 12
#define SmallWinW 25

int wread_input(WINDOW *win, char input[]);
int wread_change_string(WINDOW *win, char input[]);
int draw_welcome_window(WINDOW *welwin);
int draw_opendb_window(WINDOW *opendb_window, char *);
int draw_select_work_mode(WINDOW *select_work_mode_win, int highlight_item);
int bookedit_print_sheet_noisbn(char **queryResult, int nRow, int nColumn, int MainWinH, int MainWinW);
int bookedit_print_sheet_noisbn_askdel(char **queryResult, int nRow, int nColumn, int MainWinH, int MainWinW);
int keepreturn_print_sheet_noisbn_askkeep(char **queryResult, int nRow, int nColumn, int MainWinH, int MainWinW);
int keepreturn_print_sheet_noisbn_askkr(char **studentQueryResult, char **bookQueryResult, int nRow, int nColumn, int MainWinH, int MainWinW);
int bookedit_add_book(int MainWinH, int MainWinW, sqlite3 *db);
int bookedit_find_book(int MainWinH, int MainWinW, sqlite3 *db);
int bookedit_delete_book(int MainWinH, int MainWinW, sqlite3 *db);
int bookedit_edit_book(int MainWinH, int MainWinW, sqlite3 *db);
int workwin_book_edit(int MainWinH, int MainWinW, sqlite3 *db);
int quote_process(char *input, int len, char *target);
int keepreturn_keep_book(int MainWinH, int MainWinW, sqlite3 *db, char student_ID[]);
int keepreturn_return_book(int MainWinH, int MainWinW, sqlite3 *db, char student_ID[]);
int workwin_keep_return(int MainWinH, int MainWinW, sqlite3 *db);

int wread_input(WINDOW *win, char input[])
{
    //This Function differs from wread_input() with return -1 when read a ESC key
    keypad(win, 1);
    noecho();
    cbreak();
    int len = 0;
    int ch;
    while (1)
    {
        ch = wgetch(win);
        if (ch == 27)
        {
            return -1;
        }

        if (ch == '\n')
        {
            *input = 0;
            return len;
        }
        // if (ch == '\177')
        if (ch == '\010')
        {
            if (len > 0)
            {
                int x, y;
                getyx(win, y, x);
                mvwaddch(win, y, x - 1, ' ');
                wmove(win, y, x - 1);
                len--;
                input--;
            }
        }
        else
        {
            *(input++) = ch;
            len++;
            waddch(win, ch);
            wrefresh(win);
        }
    }
}

int wread_change_string(WINDOW *win, char input[])
{
    keypad(win, 1);
    noecho();
    cbreak();
    int len = strlen(input);
    int ch;
    int x, y;
    getyx(win, y, x);
    input += len;
    wmove(win, y, x + len);
    while (1)
    {
        ch = wgetch(win);
        if (ch == 27)
        {
            return -1;
        }

        if (ch == '\n')
        {
            *input = 0;
            return len;
        }
        // if (ch == '\177')
        if (ch == '\010')
        {
            if (len > 0)
            {
                getyx(win, y, x);
                mvwaddch(win, y, x - 1, ' ');
                wmove(win, y, x - 1);
                len--;
                input--;
            }
        }
        else
        {
            *(input++) = ch;
            len++;
            waddch(win, ch);
            wrefresh(win);
        }
    }
}

int draw_welcome_window(WINDOW *welcome_window)
{
    box(welcome_window, 0, 0);
    char printSentence[200];
    wattron(welcome_window, A_BOLD);
    sprintf(printSentence, "Library Manage System");
    mvwprintw(welcome_window, 6, (MidWinW - strlen(printSentence)) / 2, printSentence);
    wattroff(welcome_window, A_BOLD);
    sprintf(printSentence, "Author: DHao2001");
    mvwprintw(welcome_window, 7, (MidWinW - strlen(printSentence)) / 2, printSentence);
    sprintf(printSentence, "Homepage: www.dhao.xyz");
    mvwprintw(welcome_window, 8, (MidWinW - strlen(printSentence)) / 2, printSentence);
    wattron(welcome_window, A_BOLD);
    sprintf(printSentence, "Press ANY Key!");
    mvwprintw(welcome_window, 11, (MidWinW - strlen(printSentence)) / 2, printSentence);
    wattroff(welcome_window, A_BOLD);
    wrefresh(welcome_window);
    wgetch(welcome_window);
    delwin(welcome_window);
    return 0;
}

int draw_opendb_window(WINDOW *opendb_window, char *dbpath)
{
    box(opendb_window, 0, 0);
    mvwprintw(opendb_window, MidWinH / 2 - 1, 2, "Open a Database File(Default ./LibInfo.db):");
    wattron(opendb_window, A_REVERSE);
    mvwhline(opendb_window, MidWinH / 2, 2, ' ', MidWinW - 5);
    wrefresh(opendb_window);
    wmove(opendb_window, MidWinH / 2, 2);
    wread_input(opendb_window, dbpath);
    wattroff(opendb_window, A_REVERSE);
    if (*dbpath == 0)
    {
        sprintf(dbpath, "LibInfo.db");
    }
    delwin(opendb_window);
    return 0;
}

int draw_select_work_mode(WINDOW *select_work_mode_win, int highlight_item)
{
    box(select_work_mode_win, 0, 0);
    char temp[200];
    sprintf(temp, "Select Your Task");
    mvwprintw(select_work_mode_win, 1, (MidWinW - strlen(temp)) / 2, temp);
    char *word_mode_chioces[] = {"Book Info Edit(B)", "Book Keep & Return(K)", "Exit/Quit(E/Q)"};

    int y = MidWinH / 2 - 2, x = 2;
    for (int i = 0; i < 3; i++)
    {
        if (i == highlight_item)
        {
            wattron(select_work_mode_win, A_REVERSE);
            mvwprintw(select_work_mode_win, y, x, word_mode_chioces[i]);
            y++;
            wattroff(select_work_mode_win, A_REVERSE);
        }
        else
        {
            mvwprintw(select_work_mode_win, y, x, word_mode_chioces[i]);
            y++;
        }
    }
    wrefresh(select_work_mode_win);
    return 0;
}

int bookedit_print_sheet_noisbn(char **queryResult, int nRow, int nColumn, int MainWinH, int MainWinW)
{
    char temp[100];
    WINDOW *sheet_window = newwin(MainWinH, MainWinW, 0, 0);
    sprintf(temp, "Search Result");
    mvwprintw(sheet_window, 1, (MainWinW - strlen(temp)) / 2, temp);
    mvwhline(sheet_window, 2, 0, '-', MainWinW);

    mvwhline(sheet_window, MainWinH - 3, 0, '-', MainWinW);
    sprintf(temp, "Press Any Key to Quit");
    mvwprintw(sheet_window, MainWinH - 2, (MainWinW - strlen(temp)) / 2, temp);

    const int startX[] = {1, MainWinW / 8, MainWinW / 2, MainWinW * 3 / 4, MainWinW * 7 / 8, 1};
    int cursorX = 1, cursorY = 3;
    for (int i = 0; i <= nRow; i++)
    {
        for (int j = 0; j < nColumn; j++)
        {
            mvwprintw(sheet_window, cursorY, cursorX, " %s", queryResult[i * nColumn + j]);
            cursorX = startX[j + 1];
        }
        cursorY += 2;
    }
    wrefresh(sheet_window);
    wgetch(sheet_window);
    wclear(sheet_window);
    delwin(sheet_window);
    return 0;
}

int bookedit_print_sheet_noisbn_askdel(char **queryResult, int nRow, int nColumn, int MainWinH, int MainWinW)
{
    char temp[100];
    WINDOW *sheet_window = newwin(MainWinH, MainWinW, 0, 0);
    sprintf(temp, "Search Result");
    mvwprintw(sheet_window, 1, (MainWinW - strlen(temp)) / 2, temp);
    mvwhline(sheet_window, 2, 0, '-', MainWinW);

    mvwhline(sheet_window, MainWinH - 3, 0, '-', MainWinW);
    wattron(sheet_window, A_BOLD);
    sprintf(temp, "Are You Sure to DELETE It?");
    mvwprintw(sheet_window, MainWinH - 2, (MainWinW - strlen(temp)) / 2, temp);
    wattroff(sheet_window, A_BOLD);
    sprintf(temp, "Yes(Y)   No(N)");
    mvwprintw(sheet_window, MainWinH - 1, (MainWinW - strlen(temp)) / 2, temp);

    const int startX[] = {1, MainWinW / 8, MainWinW / 2, MainWinW * 3 / 4, MainWinW * 7 / 8, 1};
    int cursorX = 1, cursorY = 3;
    for (int i = 0; i <= nRow; i++)
    {
        for (int j = 0; j < nColumn; j++)
        {
            mvwprintw(sheet_window, cursorY, cursorX, " %s", queryResult[i * nColumn + j]);
            cursorX = startX[j + 1];
        }
        cursorY += 2;
    }
    wrefresh(sheet_window);
    int rc;
    while (1)
    {
        rc = wgetch(sheet_window);
        if (rc == 'y' || rc == 'Y')
        {
            rc = 1;
            break;
        }
        else if (rc == 'n' || rc == 'N')
        {
            rc = 0;
            break;
        }
        else
        {
            sprintf(temp, "Unknown Command. Are You Sure to DELETE It?");
            mvwprintw(sheet_window, MainWinH - 2, (MainWinW - strlen(temp)) / 2, temp);
            wrefresh(sheet_window);
        }
    }

    wclear(sheet_window);
    delwin(sheet_window);
    return rc;
}

int keepreturn_print_sheet_noisbn_askkeep(char **queryResult, int nRow, int nColumn, int MainWinH, int MainWinW)
{
    char temp[100];
    WINDOW *sheet_window = newwin(MainWinH, MainWinW, 0, 0);
    sprintf(temp, "Search Result");
    mvwprintw(sheet_window, 1, (MainWinW - strlen(temp)) / 2, temp);
    mvwhline(sheet_window, 2, 0, '-', MainWinW);

    mvwhline(sheet_window, MainWinH - 3, 0, '-', MainWinW);
    wattron(sheet_window, A_BOLD);
    sprintf(temp, "Are You Sure to KEEP It?");
    mvwprintw(sheet_window, MainWinH - 2, (MainWinW - strlen(temp)) / 2, temp);
    wattroff(sheet_window, A_BOLD);
    sprintf(temp, "Yes(Y)   No(N)");
    mvwprintw(sheet_window, MainWinH - 1, (MainWinW - strlen(temp)) / 2, temp);

    const int startX[] = {1, MainWinW / 8, MainWinW / 2, MainWinW * 3 / 4, MainWinW * 7 / 8, 1};
    int cursorX = 1, cursorY = 3;
    for (int i = 0; i <= nRow; i++)
    {
        for (int j = 0; j < nColumn; j++)
        {
            mvwprintw(sheet_window, cursorY, cursorX, " %s", queryResult[i * nColumn + j]);
            cursorX = startX[j + 1];
        }
        cursorY += 2;
    }
    wrefresh(sheet_window);
    int rc;
    while (1)
    {
        rc = wgetch(sheet_window);
        if (rc == 'y' || rc == 'Y')
        {
            rc = 1;
            break;
        }
        else if (rc == 'n' || rc == 'N')
        {
            rc = 0;
            break;
        }
        else
        {
            wattron(sheet_window, A_BOLD);
            sprintf(temp, "Unknown Command. Are You Sure to KEEP It?");
            mvwprintw(sheet_window, MainWinH - 2, (MainWinW - strlen(temp)) / 2, temp);
            wattroff(sheet_window, A_BOLD);
            wrefresh(sheet_window);
        }
    }

    wclear(sheet_window);
    delwin(sheet_window);
    return rc;
}

int keepreturn_print_sheet_noisbn_askkr(char **studentQueryResult, char **bookQueryResult, int nRow, int nColumn, int MainWinH, int MainWinW)
{
    char temp[100];
    WINDOW *sheet_window = newwin(MainWinH, MainWinW, 0, 0);
    keypad(sheet_window, TRUE);
    sprintf(temp, "ID: %s   Name: %s", studentQueryResult[4], studentQueryResult[5]);
    mvwprintw(sheet_window, 1, (MainWinW - strlen(temp)) / 2, temp);
    mvwhline(sheet_window, 2, 0, '-', MainWinW);

    mvwhline(sheet_window, MainWinH - 3, 0, '-', MainWinW);
    wattron(sheet_window, A_BOLD);
    sprintf(temp, "Borrow: %d   Max: %s", nRow, studentQueryResult[7]);
    mvwprintw(sheet_window, MainWinH - 2, (MainWinW - strlen(temp)) / 2, temp);
    wattroff(sheet_window, A_BOLD);
    sprintf(temp, "Keep(K)   Quit(Q)   Return(R)");
    mvwprintw(sheet_window, MainWinH - 1, (MainWinW - strlen(temp)) / 2, temp);

    const int startX[] = {1, MainWinW / 8, MainWinW / 2, MainWinW * 3 / 4, MainWinW * 7 / 8, 1};
    int cursorX = 1, cursorY = 3;
    for (int i = 0; i <= nRow; i++)
    {
        for (int j = 0; j < nColumn; j++)
        {
            mvwprintw(sheet_window, cursorY, cursorX, " %s", bookQueryResult[i * nColumn + j]);
            cursorX = startX[j + 1];
        }
        cursorY += 2;
    }
    wrefresh(sheet_window);
    int rc;
    while (1)
    {
        int exit_while = 0;
        rc = wgetch(sheet_window);
        switch (rc)
        {
        case 'k':
        case 'K':
            rc = 2;
            exit_while = 1;
            break;
        case 'r':
        case 'R':
            rc = 1;
            exit_while = 1;
            break;
        case 'q':
        case 'Q':
            rc = 0;
            exit_while = 1;
            break;
        default:
            sprintf(temp, "Unknown Command.");
            mvwprintw(sheet_window, MainWinH - 2, (MainWinW - strlen(temp)) / 2, temp);
            wrefresh(sheet_window);
            break;
        }
        if (exit_while)
        {
            break;
        }
    }

    wclear(sheet_window);
    delwin(sheet_window);
    return rc;
}

int bookedit_add_book(int MainWinH, int MainWinW, sqlite3 *db)
{
    char sqlSentence[500], temp[200], *errMsg;
    char newISBN[20], newName[100], newAuthor[100], newPtime[10], newNumChar[10], newNote[100];
    WINDOW *add_book_window = newwin(MidWinH, MidWinW, (MainWinH - MidWinH) / 2, (MainWinW - MidWinW) / 2);
    box(add_book_window, 0, 0);

    sprintf(temp, "Add a Book");
    mvwprintw(add_book_window, 1, (MidWinW - strlen(temp)) / 2, temp);
    sprintf(temp, "ISBN:");
    mvwprintw(add_book_window, MidWinH / 7, 1, temp);
    sprintf(temp, "Book Name:");
    mvwprintw(add_book_window, MidWinH * 2 / 7, 1, temp);
    sprintf(temp, "Book Author:");
    mvwprintw(add_book_window, MidWinH * 3 / 7, 1, temp);
    sprintf(temp, "Publish Time:");
    mvwprintw(add_book_window, MidWinH * 4 / 7, 1, temp);
    sprintf(temp, "Book Number:");
    mvwprintw(add_book_window, MidWinH * 5 / 7, 1, temp);
    sprintf(temp, "Note:");
    mvwprintw(add_book_window, MidWinH * 6 / 7, 1, temp);
    wrefresh(add_book_window);

    wmove(add_book_window, MidWinH / 7, 7);
    if (wread_input(add_book_window, newISBN) == -1)
    {
        delwin(add_book_window);
        return -1;
    }
    wmove(add_book_window, MidWinH * 2 / 7, 12);
    if (wread_input(add_book_window, newName) == -1)
    {
        delwin(add_book_window);
        return -1;
    }
    wmove(add_book_window, MidWinH * 3 / 7, 14);
    if (wread_input(add_book_window, newAuthor) == -1)
    {
        delwin(add_book_window);
        return -1;
    }
    wmove(add_book_window, MidWinH * 4 / 7, 15);
    if (wread_input(add_book_window, newPtime) == -1)
    {
        delwin(add_book_window);
        return -1;
    }
    wmove(add_book_window, MidWinH * 5 / 7, 14);
    if (wread_input(add_book_window, newNumChar) == -1)
    {
        delwin(add_book_window);
        return -1;
    }
    wmove(add_book_window, MidWinH * 6 / 7, 7);
    if (wread_input(add_book_window, newNote) == -1)
    {
        delwin(add_book_window);
        return -1;
    }

    int rc;
    sprintf(sqlSentence, "INSERT INTO BookInfo VALUES (NULL,\'%s\',\'%s\',\'%s\',%s,%s,0,\'%s\')", newISBN, newName, newAuthor, newPtime, newNumChar, newNote);
    rc = sqlite3_exec(db, sqlSentence, NULL, NULL, &errMsg);
    if (rc != SQLITE_OK)
    {
        mvwprintw(add_book_window, MidWinH - 1, 1, errMsg);
        wrefresh(add_book_window);
        wgetch(add_book_window);
        sqlite3_free(errMsg);
        rc = 255;
    }
    else
    {
        char **queryResult;
        int sqlRow, sqlColumn;
        sprintf(sqlSentence, "SELECT ID,BookName,BookAuthor,BookNumber,LentNumber FROM BookInfo WHERE ISBN LIKE \'%s\';", newISBN);
        rc = sqlite3_get_table(db, sqlSentence, &queryResult, &sqlRow, &sqlColumn, &errMsg);
        if (rc != SQLITE_OK)
        {
            mvwprintw(add_book_window, MidWinH - 1, 1, errMsg);
            wrefresh(add_book_window);
            wgetch(add_book_window);
            sqlite3_free_table(queryResult);
            sqlite3_free(errMsg);
            rc = 255;
        }
        else
        {
            bookedit_print_sheet_noisbn(queryResult, sqlRow, sqlColumn, MainWinH, MainWinW);
            rc = 0;
        }
    }
    delwin(add_book_window);
    return rc;
}

int bookedit_find_book(int MainWinH, int MainWinW, sqlite3 *db)
{
    char sqlSentence[500], temp[200];
    WINDOW *search_key_win = newwin(MidWinH, MidWinW, (MainWinH - MidWinH) / 2, (MainWinW - MidWinW) / 2);
    box(search_key_win, 0, 0);

    sprintf(temp, "Search (Support SQL Widecard):");
    mvwprintw(search_key_win, 1, (MidWinW - strlen(temp)) / 2, temp);
    sprintf(temp, "Search Key Word:");
    mvwprintw(search_key_win, MidWinH / 2 - 1, 2, temp);
    wattron(search_key_win, A_REVERSE);
    mvwhline(search_key_win, MidWinH / 2, 2, ' ', MidWinW - 5);
    *temp = 0;
    wread_input(search_key_win, temp);
    if (*temp == 0)
    {
        wattroff(search_key_win, A_REVERSE);
        wattron(search_key_win, COLOR_PAIR(COLOR_RED));
        mvwprintw(search_key_win, MidWinH / 2 + 2, 1, "Error: Empty input!");
        mvwprintw(search_key_win, MidWinH / 2 + 3, 1, "Exit in 2s");
        wattroff(search_key_win, COLOR_PAIR(COLOR_RED));
        wrefresh(search_key_win);
    }
    else
    {
        int rc, sqlRow, sqlColumn;
        char *errMsg, **queryResult;
        sprintf(sqlSentence, "SELECT ID,BookName,BookAuthor,BookNumber,LentNumber FROM BookInfo WHERE BookName LIKE \'%%%s%%\' OR BookAuthor LIKE \'%%%s%%\' OR ISBN LIKE \'%%%s%%\' OR ID LIKE \'%%%s%%\' LIMIT %d;", temp, temp,temp,temp, (MainWinH - 8) / 2);
        rc = sqlite3_get_table(db, sqlSentence, &queryResult, &sqlRow, &sqlColumn, &errMsg);
        if (rc != SQLITE_OK)
        {
            mvwprintw(search_key_win, MidWinH / 2 + 2, 1, "Error: %s", errMsg);
            wrefresh(search_key_win);
        }
        else
        {
            bookedit_print_sheet_noisbn(queryResult, sqlRow, sqlColumn, MainWinH, MainWinW);
        }
        sqlite3_free_table(queryResult);
        sqlite3_free(errMsg);
    }
    delwin(search_key_win);
    return 0;
}

int bookedit_delete_book(int MainWinH, int MainWinW, sqlite3 *db)
{
    char temp[200], delID[20], sqlSentence[300], **queryResult, *errMsg;
    WINDOW *delbook_window = newwin(MidWinH, MidWinW, (MainWinH - MidWinH) / 2, (MainWinW - MidWinW) / 2);
    box(delbook_window, 0, 0);

    sprintf(temp, "Enter Book ID:");
    mvwprintw(delbook_window, MidWinH / 2 - 1, 1, temp);
    while (1)
    {
        wattron(delbook_window, A_REVERSE);
        mvwhline(delbook_window, MidWinH / 2, 2, ' ', MidWinW - 5);
        wrefresh(delbook_window);

        wmove(delbook_window, MidWinH / 2, 1);
        int rc = wread_input(delbook_window, delID);
        wattroff(delbook_window, A_REVERSE);
        if (rc == -1)
        {
            return -1;
        }
        else
        {
            int sqlRow, sqlColumn;
            sprintf(sqlSentence, "SELECT ID,BookName,BookAuthor,BookNumber,LentNumber FROM BookInfo WHERE ID LIKE \'%%%s%%\';", delID);
            int rc = sqlite3_get_table(db, sqlSentence, &queryResult, &sqlRow, &sqlColumn, &errMsg);
            if (sqlRow == 0)
            {
                sprintf(temp, "ID %s NOT Found.", delID);
                mvwprintw(delbook_window, MidWinH / 2 + 1, 1, temp);
                wrefresh(delbook_window);
            }
            else
            {
                int usrSure = bookedit_print_sheet_noisbn_askdel(queryResult, sqlRow, sqlColumn, MainWinH, MainWinW);
                if (usrSure == 1)
                {
                    sprintf(sqlSentence, "DELETE FROM BookInfo WHERE ID LIKE \'%s\';", delID);
                    rc = sqlite3_exec(db, sqlSentence, NULL, NULL, &errMsg);
                    if (rc == SQLITE_OK)
                    {
                        break;
                    }
                    else
                    {
                        sprintf(temp, "Error: %s", errMsg);
                        mvwprintw(delbook_window, MidWinH / 2 + 1, 1, temp);
                        wrefresh(delbook_window);
                        wgetch(delbook_window);
                        break;
                    }
                }
                else
                {
                    break;
                }
            }
        }
    }
    sqlite3_free(errMsg);
    sqlite3_free_table(queryResult);
    delwin(delbook_window);
    return 0;
}

int bookedit_edit_book(int MainWinH, int MainWinW, sqlite3 *db)
{
    int sqlRow, sqlColumn;
    char sqlSentence[500], temp[100], *errMsg, **queryResult, edit_bookID[20] = {0};
    WINDOW *get_editID_win = newwin(MidWinH, MidWinW, (MainWinH - MidWinH) / 2, (MainWinW - MidWinW) / 2);

    while (1)
    {
        box(get_editID_win, 0, 0);
        sprintf(temp, "Enter Book ID:");
        mvwprintw(get_editID_win, MidWinH / 2 - 1, 2, temp);
        wattron(get_editID_win, A_REVERSE);
        mvwhline(get_editID_win, MidWinH / 2, 2, ' ', MidWinW - 5);
        wrefresh(get_editID_win);
        int rc = wread_input(get_editID_win, edit_bookID);
        wattroff(get_editID_win, A_REVERSE);
        if (rc == -1)
        {
            delwin(get_editID_win);
            return -1;
        }
        if (*edit_bookID == 0)
        {
            mvwhline(get_editID_win, MidWinH / 2 + 1, 1, ' ', MidWinW - 1);
            sprintf(temp, "Empty Book ID");
            mvwprintw(get_editID_win, MidWinH / 2 + 1, MidWinW, temp);
            wrefresh(get_editID_win);
            continue;
        }
        sprintf(sqlSentence, "SELECT ISBN,BookName,BookAuthor,BookPublishTime,BookNumber,BookNote FROM BookInfo WHERE ID LIKE \'%s\';", edit_bookID);
        rc = sqlite3_get_table(db, sqlSentence, &queryResult, &sqlRow, &sqlColumn, &errMsg);
        if (rc != SQLITE_OK)
        {
            mvwhline(get_editID_win, MidWinH / 2 + 1, 1, ' ', MidWinW - 1);
            sprintf(temp, "Error: %s", errMsg);
            mvwprintw(get_editID_win, MidWinH / 2 + 1, MidWinW, temp);
            wrefresh(get_editID_win);
            continue;
        }
        if (sqlRow < 1)
        {
            mvwhline(get_editID_win, MidWinH / 2 + 1, 1, ' ', MidWinW - 1);
            sprintf(temp, "Book ID %s No Found!", edit_bookID);
            mvwprintw(get_editID_win, MidWinH / 2 + 1, MidWinW, temp);
            wrefresh(get_editID_win);
            continue;
        }
        break;
    }
    delwin(get_editID_win);

    WINDOW *edit_book_win = newwin(MidWinH, MidWinW, (MainWinH - MidWinH) / 2, (MainWinW - MidWinW) / 2);
    box(edit_book_win, 0, 0);

    char newISBN[20], newName[100], newAuthor[100], newPTime[20], newBookNum[5], newNote[80];
    strcpy(newISBN, queryResult[6]);
    strcpy(newName, queryResult[7]);
    strcpy(newAuthor, queryResult[8]);
    strcpy(newPTime, queryResult[9]);
    strcpy(newBookNum, queryResult[10]);
    strcpy(newNote, queryResult[11]);
    sprintf(temp, "Edit Book %s", edit_bookID);
    mvwprintw(edit_book_win, 1, (MidWinW - strlen(temp)) / 2, temp);
    sprintf(temp, "ISBN: %s", newISBN);
    mvwprintw(edit_book_win, MidWinH / 7, 1, temp);
    sprintf(temp, "Book Name: %s", newName);
    mvwprintw(edit_book_win, MidWinH * 2 / 7, 1, temp);
    sprintf(temp, "Book Author: %s", newAuthor);
    mvwprintw(edit_book_win, MidWinH * 3 / 7, 1, temp);
    sprintf(temp, "Publish Time: %s", newPTime);
    mvwprintw(edit_book_win, MidWinH * 4 / 7, 1, temp);
    sprintf(temp, "Book Number: %s", newBookNum);
    mvwprintw(edit_book_win, MidWinH * 5 / 7, 1, temp);
    sprintf(temp, "Note: %s", newNote);
    mvwprintw(edit_book_win, MidWinH * 6 / 7, 1, temp);
    wrefresh(edit_book_win);

    wmove(edit_book_win, MidWinH / 7, 7);
    if (wread_change_string(edit_book_win, newISBN) == -1)
    {
        delwin(edit_book_win);
        return -1;
    }
    wmove(edit_book_win, MidWinH * 2 / 7, 12);
    if (wread_change_string(edit_book_win, newName) == -1)
    {
        delwin(edit_book_win);
        return -1;
    }
    wmove(edit_book_win, MidWinH * 3 / 7, 14);
    if (wread_change_string(edit_book_win, newAuthor) == -1)
    {
        delwin(edit_book_win);
        return -1;
    }
    wmove(edit_book_win, MidWinH * 4 / 7, 15);
    if (wread_change_string(edit_book_win, newPTime) == -1)
    {
        delwin(edit_book_win);
        return -1;
    }
    wmove(edit_book_win, MidWinH * 5 / 7, 14);
    if (wread_change_string(edit_book_win, newBookNum) == -1)
    {
        delwin(edit_book_win);
        return -1;
    }
    wmove(edit_book_win, MidWinH * 6 / 7, 7);
    if (wread_change_string(edit_book_win, newNote) == -1)
    {
        delwin(edit_book_win);
        return -1;
    }

    sprintf(sqlSentence, "UPDATE BookInfo SET ISBN = \'%s\', BookName = \'%s\', BookAuthor = \'%s\', BookPublishTime = %s, BookNumber = %s, BookNote = \'%s\' WHERE ID LIKE \'%s\';", newISBN, newName, newAuthor, newPTime, newBookNum, newNote, edit_bookID);
    int rc = sqlite3_exec(db, sqlSentence, NULL, NULL, &errMsg);
    if (rc != SQLITE_OK)
    {
        mvhline(MidWinH / 2 + 1, 1, ' ', MidWinW - 1);
        sprintf(temp, "Error: %s", errMsg);
        mvprintw(MidWinH / 2 + 1, MidWinW, temp);
        refresh();
    }
    delwin(edit_book_win);
    sqlite3_free(errMsg);
    sqlite3_free_table(queryResult);
    return 0;
}

int workwin_book_edit(int MainWinH, int MainWinW, sqlite3 *db)
{
    keypad(stdscr, TRUE);
    int sqlRow, sqlColumn, offset = 0, sort_mode = 0;
    char temp[100], **queryResult, *errMsg, sqlSentence[300];
    sprintf(sqlSentence, "SELECT ID,BookName,BookAuthor,BookNumber,LentNumber FROM BookInfo LIMIT %d;", (MainWinH - 8) / 2);
    while (1)
    {
        switch (sort_mode)
        {
        case 0:
            sprintf(sqlSentence, "SELECT ID,BookName,BookAuthor,BookNumber,LentNumber FROM BookInfo ORDER BY ID ASC LIMIT %d OFFSET %d;", (MainWinH - 8) / 2, offset);
            break;
        case 1:
            sprintf(sqlSentence, "SELECT ID,BookName,BookAuthor,BookNumber,LentNumber FROM BookInfo ORDER BY BookName ASC LIMIT %d OFFSET %d;", (MainWinH - 8) / 2, offset);
            break;
        case 2:
            sprintf(sqlSentence, "SELECT ID,BookName,BookAuthor,BookNumber,LentNumber FROM BookInfo ORDER BY ISBN ASC LIMIT %d OFFSET %d;", (MainWinH - 8) / 2, offset);
            break;
        default:
            break;
        }
        int rc;
        rc = sqlite3_get_table(db, sqlSentence, &queryResult, &sqlRow, &sqlColumn, &errMsg);
        clear();
        sprintf(temp, "Book Information Edit");
        mvprintw(1, (MainWinW - strlen(temp)) / 2, temp);
        mvhline(2, 0, '-', MainWinW);
        mvhline(MainWinH - 3, 0, '-', MainWinW);
        mvhline(MainWinH - 2, 0, ' ', MainWinW);
        sprintf(temp, "Add(A)  Delete(D)  Edit(E)  Find(F)  Sort(S)  Quit(Q)");
        mvprintw(MainWinH - 2, (MainWinW - strlen(temp)) / 2, temp);
        mvhline(MainWinH - 1, 0, ' ', MainWinW);
        sprintf(temp, "Status: Ready.");
        mvprintw(MainWinH - 1, 1, temp);
        if (rc != SQLITE_OK)
        {
            sprintf(temp, "Error: %s", sqlite3_errmsg(db));
            mvprintw(MainWinH / 2, MainWinW / 2, temp);
            sqlite3_free_table(queryResult);
        }
        else
        {
            const int startX[] = {1, MainWinW / 8, MainWinW / 2, MainWinW * 3 / 4, MainWinW * 7 / 8, 1};
            int cursorX = 1, cursorY = 3;
            for (int i = 0; i <= sqlRow; i++)
            {
                for (int j = 0; j < sqlColumn; j++)
                {
                    mvprintw(cursorY, cursorX, "%s", queryResult[i * sqlColumn + j]);
                    cursorX = startX[j + 1];
                }
                cursorY += 2;
            }
            refresh();
        }
        int workmode = getch(), exit_while = 0;
        switch (workmode)
        {
        case KEY_UP:
            if (offset > 0)
            {
                offset--;
            }
            // sprintf(sqlSentence, "SELECT ID,BookName,BookAuthor,BookNumber,LentNumber FROM BookInfo LIMIT %d OFFSET %d;", (MainWinH - 10) / 2, offset);
            break;
        case KEY_DOWN:
            offset++;
            // sprintf(sqlSentence, "SELECT ID,BookName,BookAuthor,BookNumber,LentNumber FROM BookInfo LIMIT %d OFFSET %d;", (MainWinH - 10) / 2, offset);
            break;
        case 'a':
        case 'A':
            bookedit_add_book(MainWinH, MainWinW, db);
            break;
        case 'd':
        case 'D':
            bookedit_delete_book(MainWinH, MainWinW, db);
            break;
        case 'e':
        case 'E':
            bookedit_edit_book(MainWinH, MainWinW, db);
            break;
        case 'f':
        case 'F':
            bookedit_find_book(MainWinH, MainWinW, db);
            refresh();
            break;
        case 'q':
        case 'Q':
            exit_while = 1;
            break;
        case 's':
        case 'S':{
            mvhline(MainWinH - 1, 0, ' ', MainWinW);
            sprintf(temp, "Hint: Sort By BookName(N) or ISBN(I) or BookID(D). Cancel(C)");
            mvprintw(MainWinH - 1, 1, temp);
            int ch = getch();
            switch (ch)
            {
            case 'N':
            case 'n':

                sort_mode = 1;
                break;
            case 'I':
            case 'i':

                sort_mode = 2;
                break;
            case 'D':
            case 'd':

                sort_mode = 0;
                break;
            case 'C':
            case 'c':
                // sprintf(sqlSentence, "SELECT ID,BookName,BookAuthor,BookNumber,LentNumber FROM BookInfo LIMIT %d OFFSET %d;", (MainWinH - 10) / 2, offset);
                break;
            default:
                mvhline(MainWinH - 1, 0, ' ', MainWinW);
                sprintf(temp, "Error: Unknown Command");
                mvprintw(MainWinH - 1, 1, temp);
                break;
            }}
            break;
        default:
            mvhline(MainWinH - 1, 0, ' ', MainWinW);
            sprintf(temp, "Error: Unknown Command");
            mvprintw(MainWinH - 1, 1, temp);
            break;
        }
        if (exit_while)
        {
            break;
        }
    }
    sqlite3_free_table(queryResult);
    return 0;
}

int quote_process(char *input, int len, char *target)
{
    for (int i = 0; i <= len; i++)
    {
        if (input[i] == '\'')
        {
            *(target++) = '\'';
        }
        *(target++) = input[i];
    }
    return 0;
}

int keepreturn_keep_book(int MainWinH, int MainWinW, sqlite3 *db, char student_ID[])
{
    int sqlRow, sqlColumn;
    char sqlSentence[500], **bookQueryResult, **studentQueryResult, *errMsg, temp[100], keep_ID[30];
    WINDOW *keep_window = newwin(MidWinH, MidWinW, (MainWinH - MidWinH) / 2, (MainWinW - MidWinW) / 2);
    box(keep_window, 0, 0);

    sprintf(temp, "Enter Book ID to keep:");
    mvwprintw(keep_window, MidWinH / 2 - 1, 2, temp);
    while (1)
    {
        int exit_while = 0;
        *keep_ID = 0;
        wattron(keep_window, A_REVERSE);
        mvwhline(keep_window, MidWinH / 2, 2, ' ', MidWinW - 6);
        wrefresh(keep_window);

        wmove(keep_window, MidWinH / 2, 2);
        int isESC = wread_input(keep_window, keep_ID);
        wattroff(keep_window, A_REVERSE);
        if (isESC == -1)
        {
            break;
        }
        if (*keep_ID == 0)
        {
            sprintf(temp, "Empty Book ID!");
            mvwprintw(keep_window, MidWinH / 2 + 1, 2, temp);
            wrefresh(keep_window);
            continue;
        }
        else
        {
            sprintf(sqlSentence, "SELECT ID,BookName,BookAuthor,BookNumber,LentNumber FROM BookInfo WHERE ID LIKE \'%s\' LIMIT %d;", keep_ID, (MainWinH - 10) / 2);
            int rc;
            rc = sqlite3_get_table(db, sqlSentence, &bookQueryResult, &sqlRow, &sqlColumn, &errMsg);
            if (rc != SQLITE_OK)
            {
                sprintf(temp, "Error: %s", errMsg);
                mvwprintw(keep_window, MidWinH / 2 + 1, 2, temp);
                wrefresh(keep_window);
                continue;
            }
            else
            {
                if (sqlRow < 1)
                {
                    sprintf(temp, "Book ID %s No Found!", keep_ID);
                    mvwprintw(keep_window, MidWinH / 2 + 1, 2, temp);
                    wrefresh(keep_window);
                    continue;
                }
                else
                {
                    int totalBook = atoi(bookQueryResult[8]);
                    int lentBook = atoi(bookQueryResult[9]);
                    if (totalBook <= lentBook)
                    {
                        sprintf(temp, "Sorry, All book %s have been borrowed!", keep_ID);
                        mvwprintw(keep_window, MidWinH / 2 + 1, 2, temp);
                        wrefresh(keep_window);
                        continue;
                    }
                    else
                    {
                        int usrSure = keepreturn_print_sheet_noisbn_askkeep(bookQueryResult, sqlRow, sqlColumn, MainWinH, MainWinW);
                        if (usrSure)
                        {
                            sprintf(sqlSentence, "SELECT StudentID, StudentName, StudentBorrowedBookID, MaxBorrowNumber, BorrowedNumber FROM StudentInfo WHERE StudentID LIKE \'%s\';", student_ID);
                            rc = sqlite3_get_table(db, sqlSentence, &studentQueryResult, &sqlRow, &sqlColumn, &errMsg);
                            if (rc != SQLITE_OK)
                            {
                                sprintf(temp, "Error: %s", errMsg);
                                mvwprintw(keep_window, MidWinH / 2 + 1, 2, temp);
                                wrefresh(keep_window);
                                continue;
                            }
                            else
                            {
                                int stuMax = atoi(studentQueryResult[8]);
                                int stuLent = atoi(studentQueryResult[9]);
                                if (stuMax <= stuLent)
                                {
                                    sprintf(temp, "You have reach max book-keeping amount.");
                                    mvwprintw(keep_window, MidWinH / 2 + 1, 2, temp);
                                    wrefresh(keep_window);
                                    continue;
                                }
                                else
                                {
                                    sprintf(sqlSentence, "UPDATE BookInfo SET LentNumber = %d WHERE ID LIKE \'%s\';", lentBook + 1, keep_ID);
                                    rc = sqlite3_exec(db, sqlSentence, NULL, NULL, &errMsg);
                                    if (rc != SQLITE_OK)
                                    {
                                        sprintf(temp, "Error: %s", errMsg);
                                        mvwprintw(keep_window, MidWinH / 2 + 1, 2, temp);
                                        wrefresh(keep_window);
                                        continue;
                                    }
                                    if (studentQueryResult[7] == NULL || strlen(studentQueryResult[7]) == 0)
                                    {
                                        sprintf(temp, "\'%s\'", keep_ID);
                                    }
                                    else
                                    {
                                        sprintf(temp, "%s,\'%s\'", studentQueryResult[7], keep_ID);
                                    }

                                    char borTemp[200];
                                    quote_process(temp, strlen(temp), borTemp);
                                    sprintf(sqlSentence, "UPDATE StudentInfo SET BorrowedNumber = %d, StudentBorrowedBookID = \'%s\' WHERE StudentID = %s;", stuLent + 1, borTemp, student_ID);
                                    rc = sqlite3_exec(db, sqlSentence, NULL, NULL, &errMsg);
                                    if (rc != SQLITE_OK)
                                    {
                                        sprintf(temp, "Error: %s", errMsg);
                                        mvwprintw(keep_window, MidWinH / 2 + 1, 2, temp);
                                        wrefresh(keep_window);
                                        continue;
                                    }
                                    exit_while = 1;
                                }
                            }
                        }
                        else
                        {
                            continue;
                        }
                    }
                }
            }
        }
        if (exit_while)
        {
            break;
        }
    }
    sqlite3_free_table(studentQueryResult);
    sqlite3_free_table(bookQueryResult);
    sqlite3_free(errMsg);
    delwin(keep_window);
    return 0;
}

int keepreturn_return_book(int MainWinH, int MainWinW, sqlite3 *db, char student_ID[])
{
    int sqlRow, sqlColumn;
    char sqlSentence[500], **bookQueryResult, **studentQueryResult, *errMsg, temp[100], return_ID[30];
    WINDOW *return_window = newwin(MidWinH, MidWinW, (MainWinH - MidWinH) / 2, (MainWinW - MidWinW) / 2);

    while (1)
    {
        box(return_window, 0, 0);
        sprintf(temp, "Enter Book ID to Return:");
        mvwprintw(return_window, MidWinH / 2 - 1, 2, temp);
        int exit_while = 0;
        *return_ID = 0;
        wattron(return_window, A_REVERSE);
        mvwhline(return_window, MidWinH / 2, 2, ' ', MidWinW - 6);
        wrefresh(return_window);

        wmove(return_window, MidWinH / 2, 2);
        int isESC = wread_input(return_window, return_ID);
        wattroff(return_window, A_REVERSE);
        if (isESC == -1)
        {
            exit_while = 1;
        }
        if (*return_ID == 0)
        {
            sprintf(temp, "Empty Book ID!");
            mvwprintw(return_window, MidWinH / 2 + 1, 2, temp);
            wrefresh(return_window);
            continue;
        }
        else
        {
            sprintf(sqlSentence, "SELECT StudentBorrowedBookID FROM StudentInfo WHERE StudentID LIKE \'%s\';", student_ID);
            int rc;
            rc = sqlite3_get_table(db, sqlSentence, &studentQueryResult, &sqlRow, &sqlColumn, &errMsg);
            if (rc != SQLITE_OK)
            {
                sprintf(temp, "Error: %s", errMsg);
                mvwprintw(return_window, MidWinH / 2 + 1, 2, temp);
                wrefresh(return_window);
                continue;
            }
            else
            {
                char borrowedBookID[100];
                sprintf(borrowedBookID, "%s", studentQueryResult[1]);
                if (strlen(borrowedBookID) < 3)
                {
                    sprintf(temp, "It seems that no book you have borrowed.");
                    mvwprintw(return_window, MidWinH / 2 + 1, 2, temp);
                    wrefresh(return_window);
                    continue;
                }

                sprintf(sqlSentence, "SELECT LentNumber FROM BookInfo WHERE ID LIKE \'%s\';", return_ID);
                rc = sqlite3_get_table(db, sqlSentence, &bookQueryResult, &sqlRow, &sqlColumn, &errMsg);
                if (rc != SQLITE_OK)
                {
                    sprintf(temp, "Error: %s", errMsg);
                    mvwprintw(return_window, MidWinH / 2 + 1, 2, temp);
                    wrefresh(return_window);
                    continue;
                }
                if (sqlRow < 1)
                {
                    sprintf(temp, "Book ID %s No Found!", return_ID);
                    mvwprintw(return_window, MidWinH / 2 + 1, 2, temp);
                    wrefresh(return_window);
                    continue;
                }
                int bookLentNumber = atoi(bookQueryResult[1]);

                sprintf(sqlSentence, "SELECT StudentBorrowedBookID FROM StudentInfo WHERE StudentID = %s;", student_ID);
                rc = sqlite3_get_table(db, sqlSentence, &bookQueryResult, &sqlRow, &sqlColumn, &errMsg);
                if (rc != SQLITE_OK)
                {
                    sprintf(temp, "Error: %s", errMsg);
                    mvwprintw(return_window, MidWinH / 2 + 1, 2, temp);
                    wrefresh(return_window);
                    continue;
                }

                sprintf(sqlSentence, "SELECT \'%s\' IN (%s);", return_ID, bookQueryResult[1]);
                rc = sqlite3_get_table(db, sqlSentence, &bookQueryResult, &sqlRow, &sqlColumn, &errMsg);
                if (rc != SQLITE_OK)
                {
                    sprintf(temp, "Error: %s", errMsg);
                    mvwprintw(return_window, MidWinH / 2 + 1, 2, temp);
                    wrefresh(return_window);
                    continue;
                }
                if (atoi(bookQueryResult[1]) == 0)
                {
                    sprintf(temp, "You didn't borrowed Book %s.", return_ID);
                    mvwprintw(return_window, MidWinH / 2 + 1, 2, temp);
                    wrefresh(return_window);
                    continue;
                }

                sprintf(sqlSentence, "SELECT ID FROM BookInfo WHERE ID IN (%s) AND ID NOT LIKE \'%s\' LIMIT %d;", borrowedBookID, return_ID, (MainWinH - 10) / 2);
                rc = sqlite3_get_table(db, sqlSentence, &bookQueryResult, &sqlRow, &sqlColumn, &errMsg);
                if (rc != SQLITE_OK)
                {
                    sprintf(temp, "Error: %s", errMsg);
                    mvwprintw(return_window, MidWinH / 2 + 1, 2, temp);
                    wrefresh(return_window);
                    continue;
                }

                char newBorrowID[300] = {0};
                if (sqlRow < 1)
                {
                    *newBorrowID = 0;
                }
                else
                {
                    sprintf(newBorrowID, "\'\'%s\'\'", bookQueryResult[1]);
                    for (int i = 2; i <= sqlRow; i++)
                    {
                        sprintf(newBorrowID, "%s,\'\'%s\'\'", newBorrowID, bookQueryResult[i]);
                    }
                }

                sprintf(sqlSentence, "UPDATE BookInfo SET LentNumber = %d WHERE ID LIKE \'%s\';", --bookLentNumber, return_ID);
                rc = sqlite3_exec(db, sqlSentence, NULL, NULL, &errMsg);
                if (rc != SQLITE_OK)
                {
                    sprintf(temp, "Error: %s", errMsg);
                    mvwprintw(return_window, MidWinH / 2 + 1, 2, temp);
                    wrefresh(return_window);
                    continue;
                }
                sprintf(sqlSentence, "UPDATE StudentInfo SET StudentBorrowedBookID = \'%s\', BorrowedNumber = %d WHERE StudentID = \'%s\';", newBorrowID, sqlRow, student_ID);
                rc = sqlite3_exec(db, sqlSentence, NULL, NULL, &errMsg);
                if (rc != SQLITE_OK)
                {
                    sprintf(temp, "Error: %s", errMsg);
                    mvwprintw(return_window, MidWinH / 2 + 1, 2, temp);
                    wrefresh(return_window);
                    continue;
                }
                exit_while = 1;
            }
        }
        if (exit_while)
        {
            break;
        }
    }
    sqlite3_free_table(studentQueryResult);
    sqlite3_free_table(bookQueryResult);
    sqlite3_free(errMsg);
    delwin(return_window);
    return 0;
}

int workwin_keep_return(int MainWinH, int MainWinW, sqlite3 *db)
{
    char student_ID[20] = {0}, sqlSentence[500], **bookQueryResult, **studentQueryResult, *errMsg, temp[100];
    WINDOW *getID_window = newwin(MidWinH, MidWinW, (MainWinH - MidWinH) / 2, (MainWinW - MidWinW) / 2);
    box(getID_window, 0, 0);
    sprintf(temp, "Enter your ID:");
    mvwprintw(getID_window, MidWinH / 2 - 1, 2, temp);
    while (1)
    {
        int exit_while = 0;
        *student_ID = 0;
        wattron(getID_window, A_REVERSE);
        mvwhline(getID_window, MidWinH / 2, 2, ' ', MidWinW - 6);
        wrefresh(getID_window);

        wmove(getID_window, MidWinH / 2, 2);
        wread_input(getID_window, student_ID);
        wattroff(getID_window, A_REVERSE);
        if (*student_ID == 0)
        {
            sprintf(temp, "Empty Student ID!");
            mvwprintw(getID_window, MidWinH / 2 + 1, 2, temp);
            wrefresh(getID_window);
            continue;
        }
        else
        {
            sprintf(sqlSentence, "SELECT StudentID, StudentName, StudentBorrowedBookID, MaxBorrowNumber FROM StudentInfo WHERE StudentID LIKE \'%s\';", student_ID);
            int sqlRow, sqlColumn, rc;
            rc = sqlite3_get_table(db, sqlSentence, &studentQueryResult, &sqlRow, &sqlColumn, &errMsg);
            if (rc != SQLITE_OK)
            {
                sprintf(temp, "Error: %s", errMsg);
                mvwprintw(getID_window, MidWinH / 2 + 1, 2, temp);
                wrefresh(getID_window);
                continue;
            }
            else
            {
                if (sqlRow < 1)
                {
                    sprintf(temp, "ID %s No Found!", student_ID);
                    mvwprintw(getID_window, MidWinH / 2 + 1, 2, temp);
                    wrefresh(getID_window);
                    continue;
                }
                else
                {
                    while (1)
                    {
                        sprintf(sqlSentence, "SELECT StudentID, StudentName, StudentBorrowedBookID, MaxBorrowNumber FROM StudentInfo WHERE StudentID LIKE \'%s\';", student_ID);
                        rc = sqlite3_get_table(db, sqlSentence, &studentQueryResult, &sqlRow, &sqlColumn, &errMsg);
                        if (rc != SQLITE_OK)
                        {
                            sprintf(temp, "Error: %s", errMsg);
                            mvwprintw(getID_window, MidWinH / 2 + 1, 2, temp);
                            wrefresh(getID_window);
                            break;
                        }
                        sprintf(sqlSentence, "SELECT ID,BookName,BookAuthor,BookNumber,LentNumber FROM BookInfo WHERE ID IN ( %s ) LIMIT %d;", studentQueryResult[6], (MainWinH - 10) / 2);
                        rc = sqlite3_get_table(db, sqlSentence, &bookQueryResult, &sqlRow, &sqlColumn, &errMsg);
                        if (rc != SQLITE_OK)
                        {
                            sprintf(temp, "Error: %s", errMsg);
                            mvwprintw(getID_window, MidWinH / 2 + 1, 2, temp);
                            wrefresh(getID_window);
                            break;
                        }
                        else
                        {
                            int work_mode = keepreturn_print_sheet_noisbn_askkr(studentQueryResult, bookQueryResult, sqlRow, sqlColumn, MainWinH, MainWinW);
                            switch (work_mode)
                            {
                            case 0:
                                exit_while = 1;
                                break;
                            case 1:
                                keepreturn_return_book(MainWinH, MainWinW, db, student_ID);
                                break;
                            case 2:
                                keepreturn_keep_book(MainWinH, MainWinW, db, student_ID);
                                break;
                            default:
                                break;
                            }
                        }
                        if (exit_while)
                        {
                            break;
                        }
                    }
                }
            }
        }
        if (exit_while)
        {
            break;
        }
    }
    sqlite3_free_table(studentQueryResult);
    sqlite3_free_table(bookQueryResult);
    sqlite3_free(errMsg);
    delwin(getID_window);
    return 0;
}

int main(int argc, char *argv[])
{
    int MainWinH, MainWinW;
    int rc;
    char dbPath[300] = {0}, errMsg[300];
    initscr();
    clear();
    noecho();
    cbreak();
    getmaxyx(stdscr, MainWinH, MainWinW);
    //Init Setting

    WINDOW *welcome_window = newwin(MidWinH, MidWinW, (MainWinH - MidWinH) / 2, (MainWinW - MidWinW) / 2);
    draw_welcome_window(welcome_window);
    //Welcome Window Show

    WINDOW *opendb_window = newwin(MidWinH, MidWinW, (MainWinH - MidWinH) / 2, (MainWinW - MidWinW) / 2);
    draw_opendb_window(opendb_window, dbPath);
    //Open Database Window

    sqlite3 *db;
    rc = sqlite3_open_v2(dbPath, &db, SQLITE_OPEN_CREATE | SQLITE_OPEN_READWRITE, NULL);
    if (rc != SQLITE_OK)
    {
        WINDOW *errWin = newwin(SmallWinH, SmallWinW, (MainWinH - SmallWinH) / 2, (MainWinW - SmallWinW) / 2);
        box(errWin, 0, 0);
        mvwprintw(errWin, 2, 2, "Error occured: ");
        int x = 2, y = 6;
        sprintf(errMsg, sqlite3_errmsg(db));
        char *temp = errMsg;
        while (*temp != 0)
        {
            mvwaddch(errWin, y, x, *temp);
            if (x > 22)
            {
                y++;
                x = 2;
            }
        }
        wrefresh(errWin);
        wgetch(errWin);
        delwin(errWin);
        endwin();
        return 255;
    }
    //Load Database

    while (1)
    {
        WINDOW *select_work_mode_win = newwin(MidWinH, MidWinW, (MainWinH - MidWinH) / 2, (MainWinW - MidWinW) / 2);
        keypad(select_work_mode_win, TRUE);
        int work_mode = 0;
        while (1)
        {
            draw_select_work_mode(select_work_mode_win, work_mode);
            int input = wgetch(select_work_mode_win);
            int exit_while = 0;
            switch (input)
            {
            case KEY_UP:
                if (work_mode > 0)
                {
                    --work_mode;
                }
                break;
            case KEY_DOWN:
                if (work_mode < 2)
                {
                    ++work_mode;
                }
                break;
            case '0':
            case 'B':
            case 'b':
            {
                work_mode = 0;
                exit_while = 1;
                break;
            }
            case '1':
            case 'k':
            case 'K':
            {

                work_mode = 1;
                exit_while = 1;
            }
            break;
            case '2':
            case 's':
            case 'S':
            {
                work_mode = 2;
                exit_while = 1;
            }
            break;
            case 'e':
            case 'E':
            case 'q':
            case 'Q':
            {
                work_mode = 3;
                exit_while = 1;
            }
            break;
            case '\n':
                exit_while = 1;
                break;
            default:
                break;
            }
            if (exit_while)
            {
                break;
            }
        }
        delwin(select_work_mode_win);
        refresh();
        //Choose a Work Mode

        int exit_while = 0;
        switch (work_mode)
        {
        case 0:
            workwin_book_edit(MainWinH, MainWinW, db);
            break;
        case 1:
            workwin_keep_return(MainWinH, MainWinW, db);
            break;
        case 3:
            exit_while = 1;
            break;
        default:
            break;
        }
        if (exit_while)
        {
            break;
        }
    }
    sqlite3_close_v2(db);
    endwin();
    return 0;
}
