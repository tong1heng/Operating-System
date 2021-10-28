#include <readline/readline.h>
#include <readline/history.h>
#include <stdio.h>
#include <stdlib.h>

int main() {
    read_history(NULL);
    while (1)
    {
        char * str = readline("Myshell $ ");
        for(int i=0;i<10;i++) {
            printf("%c",str[i]);
        }
        add_history(str);
        write_history(NULL);
        free(str);
    }
    return 0;
}