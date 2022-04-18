#include "Book.h"

void getInt(uint64_t *val, char *str) {
    int len = strlen(str);
    uint64_t placeVal = 1;
    for(int i = len-1; i >= 0; i --) {
        *val += placeVal * (str[i] - 48);
        placeVal *= 10;
    }
}

uint16_t getMoveFromBook(ZobristKey *key, FILE *book) {
    while(1) {
        char keyStr[32];
        char moveStr[32];
        int i = 0;

        int readingKey = TRUE;
        while(1) {
            char c = fgetc(book);
            if(c == EOF) return 0;

            if(readingKey) {
                if(c == ' ') {
                    readingKey = FALSE;
                    keyStr[i++] = '\0';
                    i = 0;
                    c = fgetc(book);
                } else {
                    keyStr[i++] = c;
                }
            }
            if(!readingKey) {
                if(c == '\n') {
                    moveStr[i++] = '\0';
                    break;
                }
                moveStr[i++] = c;
            }
        }
        
        uint64_t bookKey = 0;
        getInt(&bookKey, keyStr);
        if(bookKey == *key) {
            uint64_t move = 0;
            getInt(&move, moveStr);
            return (uint16_t) move;
        }
    }
    return 0;
}

uint16_t bk_getMove(ZobristKey *key) {
    FILE *book = fopen("/home/sam/GitHub/Carlsim/Book/Book.txt", "r");
    uint16_t move = getMoveFromBook(key, book);
    fclose(book);
    return move;
}

int readPly(FILE *pgn, char *whiteMove, char *blackMove) {
    char c = fgetc(pgn);
    while(c < 'A' || c > 'z') {
        if(c == EOF) return -1;
        if(c == '-') return 0;
        c = fgetc(pgn);
    }

    int i = 0;

    while(1) {
        if(c == ' ' || c == '\n') break;
        if(c != 13 && c != '\n' && c != ' ') whiteMove[i++] = c;
        c = fgetc(pgn);
    }
    whiteMove[i] = '\0';

    while(c != ' ' && c != '\n')
        c = fgetc(pgn);

    i = 0;

    while(1) {
        c = fgetc(pgn);
        if(c == '/' || c == '0') {
            blackMove[0] = '\0';
            return 2;
        }
        if(c == ' ' || c == '\n') break;
        if(c != 13 && c != '\n' && c != ' ') blackMove[i++] = c;
    }
    blackMove[i] = '\0';

    return 1;
}

void getStr(uint64_t val, char *str) {
    uint64_t placeVal = 1;

    char temp[32];
    int len = 0;
    while(val) {
        int digit = (val % (10*placeVal)) / placeVal;
        val -= digit * placeVal;
        placeVal *= 10;
        temp[len++] = digit + 48;
    }
    for(int i = 0; i < len; i ++) {
        str[len - i - 1] = temp[i];
    }
    str[len] = '\0';
}

void getBookLine(ZobristKey *key, uint16_t move, char *line) {
    char keyStr[32];
    getStr(*key, keyStr);
    char moveStr[32];
    getStr(move, moveStr);
    char c;
    int i = 0;
    int j = 0;
    while((c = keyStr[i++]) != '\0') line[j++] = c;
    line[j++] = ' ';
    i = 0;
    while((c = moveStr[i++]) != '\0') line[j++] = c;
    line[j++] = '\n';
    line[j++] = '\0';
}

void bk_parsePGN(char *path) {
    FILE *book = fopen("/home/sam/GitHub/Carlsim/Book/Book.txt", "r+");
    FILE *pgn = fopen(path, "r");

    while(1) {
        char c;
        while((c = fgetc(pgn)) != '\n');
        if(fgetc(pgn) == '\n') break;
        if(fgetc(pgn) == '\n') break;
    }
    char whiteMoveStr[16];
    char blackMoveStr[16];
    int gameEnded = FALSE;
    while(1) {
        //printf("\n\n\n\n");
        loadFENStr(&g_board, STARTING_FEN);
        while(1) {
            int state = readPly(pgn, whiteMoveStr, blackMoveStr);
            if(state == -1) {
                fclose(pgn);
                fclose(book);
                return;
            } else if(state == 2) {
                gameEnded = TRUE;
            } else if(state == 0 || gameEnded) {
                gameEnded = FALSE;
                break;
            }  

            //printf("%s %s\n", whiteMoveStr, blackMoveStr);
            uint16_t move = 0;
            if(whiteMoveStr[0] != '\0') move = parseAlgebraicMove(&g_board, whiteMoveStr);
            if(move != 0) {
                char bookLine[64];
                if(getMoveFromBook(&g_board.zobrist, book) == 0) {
                    getBookLine(&g_board.zobrist, move, bookLine);
                    if(fputs(bookLine, book) == EOF) printf("Err\n");
                }
                makeMove(&g_board, move);
            }
            move = 0;
            if(blackMoveStr[0] != '\0')  move = parseAlgebraicMove(&g_board, blackMoveStr);
            if(move != 0) {
                char bookLine[64];
                if(getMoveFromBook(&g_board.zobrist, book) == 0) {
                    getBookLine(&g_board.zobrist, move, bookLine);
                    if(fputs(bookLine, book) == EOF) printf("Err\n");
                }
                makeMove(&g_board, move);
            }
            //printBoard(&g_board);
        }

        int iters = 0;
        while(1) {
            int c = fgetc(pgn);
            while(c != '\n') {
                if(c == EOF) {
                    fclose(book);
                    fclose(pgn);
                    return;
                }
                c = fgetc(pgn);
            }
            if(iters > 2 && fgetc(pgn) == '\n') break;
            if(iters > 2 && fgetc(pgn) == '\n') break;
            iters ++;
        }
    }

    fclose(book);
    fclose(pgn);
}

void bk_parseAll(char *dir) {
    DIR* FD;
    struct dirent* in_file;

    /* Openiing common file for writing */
    FD = opendir(dir);
    while ((in_file = readdir(FD))) 
    {
        /* On linux/Unix we don't want current and parent directories
         * On windows machine too, thanks Greg Hewgill
         */
        if (!strcmp (in_file->d_name, "."))
            continue;
        if (!strcmp (in_file->d_name, ".."))    
            continue;
        /* Open directory entry file for common operation */
        /* TODO : change permissions to meet your need! */
        char file[265];
        file[0] = '\0';
        strcat(file, dir);
        strcat(file, in_file->d_name); 
        int len = strlen(file);
        char ext[5];
        int j = len-1;
        int i = 3;
        for(int k = 0; k < 4; k ++) {
            ext[i--] = file[j--];
        }
        ext[4] = '\0';

        if(strcmp(ext, ".pgn") == 0) {
            printf("Loading pgn: %s\n", file);
            bk_parsePGN(file);
        } else if(ext[0] != '.') {
            strcat(file, "/");
            printf("Opening directory: %s\n", file);
            bk_parseAll(file);
        }
    }

    /* Don't forget to close common file before leaving */
}