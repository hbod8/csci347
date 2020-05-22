/* Definitions */

#define WAIT 1
#define NOWAIT 0
#define LINELEN 1024

/* Prototypes */

int processline(char *line, int infd, int outfd, int flags);
char **arg_parse(char *line, int *argcptr);
int expand(char *orig, char *new, int newsize);
int shellcommand(char **args, int argc);
int removeComments(char *line);
void strmode(mode_t mode, char *p);
void handlesig(int signal);
void find_pipe(char *str, char *pipe, int fd);