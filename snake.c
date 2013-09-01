#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <termios.h>
#include <signal.h>
#include <time.h>

#include <sys/time.h>
#include <sys/ioctl.h>
#include "snake.h"

char* gameover = "Good game, brah!";
int* body[BODY_MAX_LEN];
FILE* fd, *urand;
int snackX, snackY;
char snackDeployed = 0, lengthen, paused = 0;

void main(){
    int i,j,k,termWidth,termHeight;
    struct timeval tv;
    struct timeval ito;
    struct timespec req;
    struct timespec rem;
    time_t last;
    struct termios termios_p;
    char input = 0, print = 1,
        again = 1, justread = 0,
        dir = RIGHT;
    fd_set set;
    struct winsize w;
    char** array;
    
    ito.tv_sec = 0;
    ito.tv_usec = 0;
    
    //intialize body pointers
    i = 0;
    body[i] = malloc(BODY_MAX_LEN * 2 * sizeof(int) );
    while(i < BODY_MAX_LEN-1){
        i++;
        body[i] = body[i-1]+2 * sizeof(int);
    }
    initBody();
    i = 0;
    while(i < BODY_MAX_LEN){
        body[i][0] = -1;
        body[i][1] = -1;
        i++;
    }
    
    
    urand = fopen("/dev/urandom","r");
    printf("yo3\n");
    fflush(stdout);
    
    ioctl(0, TIOCGWINSZ, &w);
    
    //malloc(1);
    
    fd = fopen("dbg", "w");
    fprintf(fd,"*** START ***\n");
    
    gettimeofday(&tv,NULL);
    last = tv.tv_sec;
    tcgetattr(1,&termios_p);
    termios_p.c_lflag &= ~ECHO;
    termios_p.c_lflag &= ~ICANON;
    tcsetattr(1,TCSANOW,&termios_p);
    termWidth=w.ws_col;
    termHeight=w.ws_row;
    fprintf(fd,"row: %i\n", w.ws_row);
    fprintf(fd,"col: %i\n", w.ws_col);
    req.tv_nsec = 500000000;
    req.tv_sec = 0;
    initBody();
    fflush(fd);
    while(1){
        if(print){
            for(j = 0; j < w.ws_row; j++){
                for(i = 0; i < w.ws_col; i++){
                    if(bodyContains(i,j)){
                        printf("+");
                    }else if(snackX == i && snackY == j) {
                        printf("@");
                    }else{
                        printf(".");
                    }
                }
            }
        }
        fflush(stdout);
        nanosleep(&req, &rem);
        if(!paused) slither(lengthen);
        lengthen = 0;
        if(snackX == body[0][0] && snackY == body[0][1]){
            snackDeployed = 0;
            lengthen = 1;
        }
        if(!snackDeployed){
            deploySnack(termWidth, termHeight);
        }
        
        
        again = 1;
        while(again){
            again = 0;
            FD_ZERO( &set );
            FD_SET( 0, &set);
            select(3, &set, NULL, NULL, &ito);
            if(FD_ISSET(0, &set)){
                read(0, &input, 1);
                fprintf(fd,"readin a byte\n");
                justread = 1;
                again = 1;
            }
        }
        if(input == 'q'){
            quit(&termios_p);
        } else if(input == 'p'){
            paused=!paused;
        }
        if(justread && !paused){
            fprintf(fd,"reacting to a readddddddddddddd\n");
            if(input == 'a' && dir != RIGHT){
                dir = LEFT;
            }else if(input == 's' && dir != UP){
                dir = DOWN;
            }else if(input == 'w' && dir != DOWN){
                dir = UP;
            }else if(input == 'd' && dir != LEFT){
                dir = RIGHT;
            }
        }
        if(dir == LEFT){
            body[0][0] = body[1][0] -1;
            body[0][1] = body[1][1];
        }else if(dir == DOWN){
            body[0][0] = body[1][0];
            body[0][1] = body[1][1]+1;
        }else if(dir == UP){
            body[0][0] = body[1][0];
            body[0][1] = body[1][1]-1;
        }else if(dir == RIGHT){
            body[0][0] = body[1][0] +1;
            body[0][1] = body[1][1];
        }
        justread = 0;
        checkGameOver(&w,body[0][0],body[0][1],&termios_p);
        fprintf(fd,"loop\n");
        fflush(fd);
    }
}

void initBody(){
    body[0][0]=5;
    body[0][1]=5;
    body[1][0]=5;
    body[1][1]=4;
    body[2][0]=5;
    body[2][1]=3;
    body[3][0]=5;
    body[3][1]=2;
    body[4][0]=5;
    body[4][1]=1;
}

void slither(char lengthen){
    fprintf(fd,"Slitherin\n");
    fflush(fd);
    int i=1;
    int stage[2], deck[2];
    stage[0] = body[0][0];
    stage[1] = body[0][1];
    while(body[i-lengthen][0] != -1){
        deck[0] = body[i][0];
        deck[1] = body[i][1];
        body[i][0] = stage[0];
        body[i][1] = stage[1];
        stage[0] = deck[0];
        stage[1] = deck[1];
        i++;
    }
}

char bodyContains(int x, int y){
    int i=0;
    while(body[i][0] != -1){
        if(body[i][0] == x && body[i][1] == y){
            return 1;
        }
        i++;
    }
    //fprintf(fd,"body[%i][0] = %i \n",i,body[i][0]);
    return 0;
}

void quit(struct termios* termios_p){
    fflush(0);
    //free(body[0]);
    termios_p->c_lflag |= ECHO;
    termios_p->c_lflag |= ICANON;
    tcsetattr(1,TCSANOW,termios_p);
    exit(0);
}


void checkGameOver(struct winsize *w, int x, int y, struct termios* termios_p){
    int i;
    if(x<0 || x > w->ws_col || y < 0 || y > w->ws_row){
        printf("\n");
        for(i = 0; i < w->ws_col; i++){
            printf("*");
        }
        printf("\n%s\n",gameover);
        for(i = 0; i < w->ws_col; i++){
            printf("*");
        }
        printf("\n");
        quit(termios_p);
    }
}

void deploySnack(int height, int width){
    //read from urandom
    do{
        fprintf(fd,"deploying snakc\n");
        fread(&snackX, sizeof (snackX), 1, urand);
        fread(&snackY, sizeof (snackY), 1, urand);
        snackX%=width;
        snackY%=height;
    }while(bodyContains(snackX,snackY));
    snackDeployed = 1;
}


void printBody(){
    int i = 0;
    fprintf(fd, "printing body\n");
    while( body[i][0] != -1){
        fprintf(fd,"[%i,%i]\n",body[i][0],body[i][1]);
        i++;
    }
    fflush(fd);
}


