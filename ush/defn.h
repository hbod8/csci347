/* Prototypes */

void processline(char *line);
char **arg_parse(char *line, int *argcptr);
int expand(char *orig, char *new, int newsize);
int shellcommand(char **args, int argc);
int removeComments(char *line);