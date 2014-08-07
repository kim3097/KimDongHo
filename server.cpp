//Cinema Server source code 

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <wiringPi.h>
#include <sqlite3.h>
#include "SDL/SDL.h"

#define LED1 1
#define LED2 4
#define LED3 5
#define LED4 6
#define SW1 7
#define SW2 0
#define SW3 2
#define SW4 3
#define BUF_SIZE 1024
#define SQLite_PATH "seat_table.db"

void seat_led(char* seat_num);
char* sqlite(char *ID);
static int callback(void *NotUsed, int argc, char **argv, char **azColName);
void SDL(char *ID);
void read_childproc(int sig);
void error_handling(char *message);

int main(int argc, char *argv[])
{
        int serv_sock, clnt_sock;
        char message[BUF_SIZE];
        int str_len, state;
        char* id;
        pid_t pid;
        struct sigaction act;
        socklen_t adr_sz;
        if( argc != 2 ){
                printf("Usage : %s <port>\n", argv[0]);
                exit(1);
        }

        /* Protect Zombie process */
        act.sa_handler = read_childproc;
        sigemptyset(&act.sa_mask);
        act.sa_flags = 0;
        state = sigaction(SIGCHLD, &act, 0);

        /* Socket */
        serv_sock = socket(PF_INET, SOCK_STREAM, 0);
        if(serv_sock == -1)
                error_handling("socket() error");

        memset(&serv_adr, 0, sizeof(serv_adr));
        serv_adr.sin_family = AF_INET;
        serv_adr.sin_addr.s_addr = htonl(INADDR_ANY);
        serv_adr.sin_port = htons(atoi(argv[1]));

        if(bind(serv_sock, (struct sockaddr*)&serv_adr, sizeof(serv_adr)) == -1)
                error_handling("bind() error");

        if(listen(serv_sock, 5) == -1)
                error_handling("listen() error");

        while(1)
        {
                clnt_adr_sz = sizeof(clnt_adr);
                clnt_sock = accept(serv_sock, (struct sockaddr*)&clnt_adr,
                                &clnt_adr_sz);
                if(clnt_sock == -1)
                        continue;
                else
                        printf("new client connected...\n");
                /* Create fork() */
                pid = fork();

                if(pid == -1)
                {
                        close(clnt_sock);
                        continue;
                }

                /* Child process */
                if(pid == 0)
                {
                        close(serv_sock);
                        while((str_len = read(clnt_sock, message, BUF_SIZE))!=0){

                                /* SQLite Searching*/
                                id = sqlite(message);

                                printf("%s\n",id);

                                /* SDL Function */
                                SDL(id);

                                /* SEAT_LED Function*/
                                seat_led(id);
                        }
                close(clnt_sock);
                printf("client disconnected...\n");
                return 0;
                }

                /* Parent process */
                else
                {
                        close(clnt_sock);
                }
        }
        close(serv_sock);
        return 0;
}

void seat_led(char* seat_num){
        if( wiringPiSetup() == -1)
                error_handling("wiringPi() error");

        /* PinMode Setup */
        pinMode( LED1, OUTPUT ); pinMode( SW1, INPUT );
        pinMode( LED2, OUTPUT ); pinMode( SW2, INPUT );
        pinMode( LED3, OUTPUT ); pinMode( SW3, INPUT );
        pinMode( LED4, OUTPUT ); pinMode( SW4, INPUT );

        /* LEd ON/OFF */
        if(!strcmp(seat_num, "A-1")){
                while(1){
                        digitalWrite(LED1, 1);

                        /* Switch on -> Led off */
                        if(digitalRead( SW1 ) == 0 ){
                                printf("seat number(A-1) led off\n");
                                digitalWrite(LED1, 0);
                                break;
                        }
                }
        }

        else if(!strcmp(seat_num, "A-2")){
                while(1){
                        digitalWrite(LED2, 1);

                        /* Switch on -> Led off */
                        if(digitalRead( SW2 ) == 0 ){
                                printf("seat number(A-2) led off\n");
                                digitalWrite(LED2, 0);
                                break;
                        }
                }
        }

        else if(!strcmp(seat_num, "B-1")){
                while(1){
                        digitalWrite(LED3, 1);

                        /* Switch on -> Led off */
                        if(digitalRead( SW3 ) == 0 ){
                                printf("seat number(B-1) led off\n");
                                digitalWrite(LED3, 0);
                                break;
                        }
                }
        }

        else if(!strcmp(seat_num, "B-2")){
                while(1){
                        digitalWrite(LED4, 1);

                        /* Switch on -> Led off */
                        if(digitalRead( SW4 ) == 0 ){
                                printf("seat number(B-2) led off\n");
                                digitalWrite(LED4, 0);
                                break;
                        }
                }
        }

        else
                printf("Wrong ID : %s\n", seat_num);

}

char* sqlite(char *ID){
        sqlite3 *db;
        char *zErrMsg = 0;
        int rc;
        static char num[20];
        char zipcode[50];

        sprintf(zipcode, "select * from seat_table where ID='%s'", ID);

        rc=sqlite3_open(SQLite_PATH, &db);

        if(rc){
                fprintf(stderr, "Can't open databases:%s\n", sqlite3_errmsg(db));
                sqlite3_close(db);
                return(0);
        }
        rc=sqlite3_exec(db, zipcode , callback, num, &zErrMsg);

        if(rc!=SQLITE_OK){
                fprintf(stderr, "SQL error: %s\n",zErrMsg);
                sqlite3_free(zErrMsg);
        }

        sqlite3_close(db);
        return num;
}

static int callback(void *NotUsed, int argc, char **argv, char **azColName){
  int i;
  for(i=0;i<argc;i++){
         strcpy((char*)NotUsed, argv[i]);
   }
  return 0;
}

void read_childproc(int sig)
{
        pid_t pid;
        int status;
        pid = waitpid(-1, &status, WNOHANG);
        printf("removed proc id: %d\n", pid);
}

void SDL(char *seat_num)
{
         //The images
        SDL_Surface* image = NULL;
        SDL_Surface* screen = NULL;

        //Start SDL
        SDL_Init( SDL_INIT_EVERYTHING );

        //Set up screen
        screen = SDL_SetVideoMode( 1000, 600, 32, SDL_SWSURFACE );

        //Load image
        if(!strcmp(seat_num, "A-1"))
                image = SDL_LoadBMP( "A-1.bmp" );

        else if(!strcmp(seat_num, "A-2"))
                image = SDL_LoadBMP( "A-2.bmp" );

        else if(!strcmp(seat_num, "B-1"))
                image = SDL_LoadBMP( "B-1.bmp" );

        else if(!strcmp(seat_num, "B-2"))
                image = SDL_LoadBMP( "B-2.bmp" );

        else if(!strcmp(seat_num, "MASTER"))
                printf("MASTER!\n");

        else
                image = SDL_LoadBMP( "exuser.bmp" );

        //Apply image to screen
        SDL_BlitSurface( image, NULL, screen, NULL );

        //Update Screen
        SDL_Flip( screen );

        //Pause
        SDL_Delay( 3000 );

        //Free the loaded image
        SDL_FreeSurface( image );

        //Quit SDL
        SDL_Quit();
}

void error_handling(char *message)
{
        fputs(message, stderr);
        fputc('\n', stderr);
        exit(1);
}



