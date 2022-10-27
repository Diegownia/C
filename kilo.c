/*** includes ***/

#include<ctype.h>
#include<errno.h>
#include<stdio.h>
#include<stdlib.h>
#include<termios.h>
#include<unistd.h>

/*** defines ***/

#define CTRL_KEY(k) ((k) & 0x1f)

/*** data ***/

struct termios orig_termios;

/*** terminal ***/

void Die(const char *s)
{
    perror(s);
    exit(1);
}

void DisableRawMode()
{
    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios) == -1)
    Die("tcsetattr");
}

void EnableRawMode()
{
    if (tcgetattr(STDIN_FILENO, &orig_termios) == -1) Die("tcgetattr");
    atexit(DisableRawMode);

    struct termios raw = orig_termios;
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

/*** output ***/

void EditorRefreshScreen()
{
    write(STDOUT_FILENO, "\x1b[2J", 4);
    write(STDOUT_FILENO, "\x1b[H", 3);
}

/*** input ***/

void EditorProcessKeypress()
{
    char c = EditorReadKey();

    switch (c) 
    {
        case CTRL_KEY('q'):
        exit(0);
        break;
    }

}

/*** init ***/

int main() 
{
    EnableRawMode();

    while (1)
    {
        EditorRefreshScreen();
        EditorProcessKeypress();
    }
    return 0;
}