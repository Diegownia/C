/*** includes ***/

#include<ctype.h>
#include<errno.h>
#include<stdio.h>
#include<stdlib.h>
#include<sys/ioctl.h>
#include<termios.h>
#include<unistd.h>

/*** defines ***/

#define CTRL_KEY(k) ((k) & 0x1f)

/*** data ***/

struct editorConfig
{
    int screenrows;
    int screencols;
    struct termios orig_termios;
};

struct editorConfig E;

/*** terminal ***/

void Die(const char *s)
{
    write(STDOUT_FILENO, "\x1b[2J", 4);
    write(STDOUT_FILENO, "\x1b[H", 3);

    perror(s);
    exit(1);
}

void DisableRawMode()
{
    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &E.orig_termios) == -1)
    Die("tcsetattr");
}

void EnableRawMode()
{
    if (tcgetattr(STDIN_FILENO, &E.orig_termios) == -1) Die("tcgetattr");
    atexit(DisableRawMode);

    struct termios raw = E.orig_termios;
    raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
    raw.c_oflag &= ~(OPOST);
    raw.c_cflag &= ~(CS8);
    raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);

    raw.c_cc[VMIN] = 0;
    raw.c_cc[VTIME] = 1;

    //tcgetattr(STDIN_FILENO, &raw);

    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1) Die("tcsetattr");
}

char EditorReadKey()
{
    int nread;
    char c;
    while ((nread = read(STDIN_FILENO, &c, 1)) !=1)
    {
        if (nread == -1 && errno != EAGAIN) Die("read");
    }
    return c;
}

int GetWindowSize(int *rows, int *cols)
{
    struct winsize ws;

    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == -1 || ws.ws_col == 0)
    {
        return -1; //Look David! I'm doing defensive programing! 
    }
    else
    {
        *cols = ws.ws_col;
        *rows = ws.ws_row;
        return 0;
    }
}

/*** output ***/

void EditorDrawRows()
{
    int y;
    for ( y =0; y < 24; y++)
    {
        write(STDOUT_FILENO, "~\r\n", 3);
    }
}

void EditorRefreshScreen()
{
    write(STDOUT_FILENO, "\x1b[2J", 4);
    write(STDOUT_FILENO, "\x1b[H", 3);

    EditorDrawRows();

    write(STDOUT_FILENO, "\x1b[H", 3);
}

/*** input ***/

void EditorProcessKeypress()
{
    char c = EditorReadKey();

    switch (c) 
    {
        
        case CTRL_KEY('q'):
            write(STDOUT_FILENO, "\x1b[2J", 4);
            write(STDOUT_FILENO, "\x1b[H", 3);
            exit(0);
        break;
    }

}

/*** init ***/

void InitEditor()
{
    if (GetWindowSize(&E.screenrows, &E.screencols) == -1) Die("GetWindowSize");
}

int main() 
{
    EnableRawMode();
    InitEditor();

    while (1)
    {
        EditorRefreshScreen();
        EditorProcessKeypress();
    }
    return 0;
}