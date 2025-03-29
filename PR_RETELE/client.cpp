#include <iostream>
#include <pthread.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <netdb.h>
#include <string>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "encription.h"
#define PRIME1 38707
#define PRIME2 34729
int PORT=2728;
int rp=0;
using namespace std;
int sockfd;
en_keys cl_k=en_keys(PRIME1,PRIME2);
en_keys sv_k=en_keys();
struct sockaddr_in srv;
void rpt_err (int rp)
{
        //will be added later
}
void kill_cl(int err)
{
    perror("The error detected on client is: ");  //will be added later
    exit(err);
}
void prep_cl()
{
    if ((sockfd=socket(AF_INET,SOCK_STREAM,0))==-1)
    {
        perror ("[server] Eroare la socket().\n");
        kill_cl(errno);
    }
    srv.sin_family = AF_INET;
    char addr[]="127.0.0.1";
    srv.sin_addr.s_addr = inet_addr(addr);
    srv.sin_port = htons (PORT);
        // stabilim conexiunea cu serverul
        if (connect (sockfd, (struct sockaddr*) &srv, sizeof (struct sockaddr))==-1)
        {
            perror("[client] Eroare la connect");
            kill_cl(errno);
        }
    cl_k.give_pbk(sockfd);
    sv_k.get_pbk(sockfd);
    printf("Client pregatit, va rog sa tastati comanda ceruta urmata de de un ENTER\n");
    
}
void quit_cl()
{
    printf("Closing client, have a nice day\n");
    close(sockfd);
    exit(0);
}
void show_rst();
void send_cmd()
{
    char cmd[200]="";
    cin.get(cmd,200);
    cin.get();
    for (int i=0;cmd[i];i++)
    {
        unsigned int copie=sv_k.encrypt_text(cmd[i]);
        write(sockfd,&copie,sizeof(copie));
    }
    char ch='\n';
    unsigned int copie=sv_k.encrypt_text(ch);
    write(sockfd,&copie,sizeof(copie));
    //trimit prin socket comanda
    show_rst();
}
void show_rst()
{
    //citim output de la server
    unsigned int ch=0;
    char ch2='1';
    int r=read(sockfd,&ch,sizeof(ch));
    ch2=cl_k.decrypt_text(ch);
    if (ch2=='7')
    {
        quit_cl();
    }
    r=read(sockfd,&ch,sizeof(ch)); // trebuie modificat pentru int
    while ((ch2=cl_k.decrypt_text(ch))!='\r')
    {
        if (r==-1)
        {
            fprintf(stderr,"Eroare la read client");
            kill_cl(errno);
        }
        printf("%c",ch2);
        r=read(sockfd,&ch,sizeof(ch));
        ch2=cl_k.decrypt_text(ch);
    }
    printf("\n");
    //citim restul mesajului trimis (ar trebui sa contina si cmd trimis la server)
}
int main()
{
    prep_cl();
    while (1)
    {
       send_cmd(); 
    }
}