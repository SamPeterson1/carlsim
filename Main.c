#include <stdio.h>
#include <string.h> 
#include <stdlib.h>
#include <sys/time.h>
#include "Board.h"
#include "Commands.h"
#include "StrUtil.h"
#include "MoveGenerator.h"
#include "Book.h"
#include "Log.h"
#include "CGI.h"

int main() {

    printf("Content-Type: text/json\n\n");
    
    clearLog();
    initMoveGenerator();
    z_init();
    bk_parseAll("Book/PGN/");

    loadFENStr(STARTING_FEN);
    
    printf("-----------------------------------------------------------------------------------------------------------------------------\n\n");
    printf("        CCCCCCCCCCCCC                                    lllllll    SSSSSSSSSSSSSSS IIIIIIIIIIMMMMMMMM               MMMMMMMM\n");
    printf("     CCC::::::::::::C                                    l:::::l  SS:::::::::::::::SI::::::::IM:::::::M             M:::::::M\n");
    printf("   CC:::::::::::::::C                                    l:::::l S:::::SSSSSS::::::SI::::::::IM::::::::M           M::::::::M\n");
    printf("  C:::::CCCCCCCC::::C                                    l:::::l S:::::S     SSSSSSSII::::::IIM:::::::::M         M:::::::::M\n");
    printf(" C:::::C       CCCCCC  aaaaaaaaaaaaa  rrrrr   rrrrrrrrr   l::::l S:::::S              I::::I  M::::::::::M       M::::::::::M\n");
    printf("C:::::C                a::::::::::::a r::::rrr:::::::::r  l::::l S:::::S              I::::I  M:::::::::::M     M:::::::::::M\n");
    printf("C:::::C                aaaaaaaaa:::::ar:::::::::::::::::r l::::l  S::::SSSS           I::::I  M:::::::M::::M   M::::M:::::::M\n");
    printf("C:::::C                         a::::arr::::::rrrrr::::::rl::::l   SS::::::SSSSS      I::::I  M::::::M M::::M M::::M M::::::M\n");
    printf("C:::::C                  aaaaaaa:::::a r:::::r     r:::::rl::::l     SSS::::::::SS    I::::I  M::::::M  M::::M::::M  M::::::M\n");
    printf("C:::::C                aa::::::::::::a r:::::r     rrrrrrrl::::l        SSSSSS::::S   I::::I  M::::::M   M:::::::M   M::::::M\n");
    printf("C:::::C               a::::aaaa::::::a r:::::r            l::::l             S:::::S  I::::I  M::::::M    M:::::M    M::::::M\n");
    printf(" C:::::C       CCCCCCa::::a    a:::::a r:::::r            l::::l             S:::::S  I::::I  M::::::M     MMMMM     M::::::M\n");
    printf("  C:::::CCCCCCCC::::Ca::::a    a:::::a r:::::r           l::::::lSSSSSSS     S:::::SII::::::IIM::::::M               M::::::M\n");
    printf("   CC:::::::::::::::Ca:::::aaaa::::::a r:::::r           l::::::lS::::::SSSSSS:::::SI::::::::IM::::::M               M::::::M\n");
    printf("     CCC::::::::::::C a::::::::::aa:::ar:::::r           l::::::lS:::::::::::::::SS I::::::::IM::::::M               M::::::M\n");
    printf("        CCCCCCCCCCCCC  aaaaaaaaaa  aaaarrrrrrr           llllllll SSSSSSSSSSSSSSS   IIIIIIIIIIMMMMMMMM               MMMMMMMM\n");
    printf("\n-----------------------------------------------------------------------------------------------------------------------------\n\n\n");

    char *command = NULL;
    size_t len = 0;
    

    while(1) {

        while(len == 0) {
            ssize_t a = getline(&command, &len, stdin);
        }
        len = 0;

        char **args = initBuffer();
        int argc;
        parse(command, args, &argc);

        int quit = cmd_execute(args, argc);
        if(quit) return 0;      
    }

    return 0;
}