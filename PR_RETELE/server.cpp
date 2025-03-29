#include <iostream>
#include <cstring>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <stdio.h>
#include <vector>
#include <fcntl.h>
#include "redirectionari.h"
#include "encription.h"
#define PRIME1 32999
#define PRIME2 31627

int PORT=2728;
struct sockaddr_in addr;
int sockfd;
char home_dir[]="/home/under";
struct sockaddr_in server;
struct sockaddr_in from;
extern int errno;
en_keys sv_k=en_keys(PRIME1,PRIME2);
en_keys cl_k=en_keys();
void thr_cl(int cl_sk);
void send_ans(int cl_sk, char* temp_file, int idth);
void* recv_cmd(void* td);
typedef struct thData{
    int idth;
    int cl; //fd dat de accept
    char addr[200]; // adresa absoluta a wd
    bool log;
}thData;

pthread_t ptid[100];
int nr_th;
int nr_cl;
void rm_th (int i)
{
    for (int j=i+1;j<nr_th;j++)
    {
        ptid[j-1]=ptid[j];
    }
    nr_th--;
}
void prep_srv()
{
        // creeam un socket care sa primeasca informatii initiale+conectare 
    if ((sockfd=socket(AF_INET,SOCK_STREAM,0))==-1)
    {
        perror ("[server] Eroare la socket().\n");
        kill_srv(errno);
    }
    int on=1;

    setsockopt(sockfd,SOL_SOCKET,SO_REUSEADDR,&on,sizeof(on));
    bzero (&server, sizeof (server));
    bzero (&from, sizeof (from));
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = htonl (INADDR_ANY);
    server.sin_port = htons (PORT);
    if (bind(sockfd,(struct sockaddr *)& server, sizeof (struct sockaddr))==-1)
    {
        perror("[server] Eroare la bind().\n");
        kill_srv(errno);
    }
    printf("Bind succesful\n");
    if (listen (sockfd,6)==-1)
    {
        perror("[server] Eroare la listen().\n");
        kill_srv(errno);
    }
    printf("Sever ready to accept\n");
}
void wait_for_client()  //asteptam un client sa se conecteze
{
    //accept()
    //socklen_t ln=sizeof(from);
    int client;
    if ( (client = accept (sockfd, (struct sockaddr *) &from,(socklen_t *)&from)) < 0)
	    {
	    perror ("[server]Eroare la accept().\n");
            return;
        }
    thr_cl(client);//acceptam si trecem la crearea unui thread pentru el
}
void thr_cl(int cl_sk)
{
    //creeam un thread pentru client
    thData* td=new(struct thData);  //retinem intr-un struct id la thread , client_fd si working directory
    td->idth=nr_th++;
    td->cl=cl_sk;
    td->log=false;
    strcpy(td->addr,home_dir); //tbr with get home dir
    cl_k.get_pbk(cl_sk);
    sv_k.give_pbk(cl_sk);
    pthread_create(&ptid[td->idth],NULL,&recv_cmd,td); //creeam un thread pt client
}
void divide_cmd (char* cmdarg[],int nrarg, thData& tdc);
void clear_vec();
void* recv_cmd(void* td)
{
    struct thData tdcpy=*((struct thData*)td);
    pthread_detach(pthread_self());
    while (1)
    {
        char temp_file[20]="";
        sprintf(temp_file,"temp_%d.txt",tdcpy.idth);
        mode_t mode = S_IRUSR | S_IWUSR ;
        int f=creat(temp_file,mode);
        if (f==-1)
        { 
           kill_srv(ftl_err);
        }

        redir_outerr(tdcpy.idth);

        char cmd[202]="";
        char ch2='1';
        unsigned int ch=0;
        int r=0;
        int cnt=0;
        while ((r=read(tdcpy.cl,&ch,sizeof(ch)))!=0 && (ch2=sv_k.decrypt_text(ch))!='\n')
        {
            if (r==-1)
            {
                rm_th(tdcpy.idth);
                kill_thr(errno,tdcpy.idth);
                exit (ftl_err);
            }
            if (ch2!='\n')
                cmd[cnt++]=ch2;
        }

        char *cmdarg[50];
        int beg=0,en=0;
        int nrarg=0;
        bool is_string=false;

        if (ver_d(tdcpy.addr)!=true)
        {
            error_det(ne_dir);
            strcpy(tdcpy.addr,home_dir);
            send_ans(tdcpy.cl,temp_file,tdcpy.idth);
        }
        else
        {
            for (int i=0;cmd[i];i++)
            {
                if (cmd[i]=='\"')
                {
                    if (is_string==true)
                    {
                        en=i-1;
                        char aux[50]="";
                        strncpy(aux,cmd+beg,en-beg+1);
                        aux[en-beg+1]='\0';
                        cmdarg[nrarg]=new char[50];
                        strcpy(cmdarg[nrarg++],aux);
                        beg=i+1;
                        is_string=false;
                    }
                    else
                    {
                        beg=i+1;
                        is_string=true;
                    }
                    continue;
                }
                if (cmd[beg]==' ' && is_string==false)
                    beg=i;
                if (is_string==false)
                {
                    if (cmd[i]==';')
                    {
                        char aux[50]=";";
                        cmdarg[nrarg]=new char[50];
                        strcpy(cmdarg[nrarg++],aux);
                        beg=i+1;
                        continue;
                    }
                    if (cmd[i+1]==' ' || cmd[i+1]==0|| cmd[i+1]==';')
                    {
                        en=i;
                        char aux[50]="";
                        strncpy(aux,cmd+beg,en-beg+1);
                        aux[en-beg+1]='\0';
                        cmdarg[nrarg]=new char[50];
                        strcpy(cmdarg[nrarg++],aux);
                        beg=i+1;
                    }
                }
            }
            if (is_string==true)
            {
                error_det(wr_arg);
                for (int i=0;i<nrarg;i++)
                    delete[] cmdarg[i];
                send_ans(tdcpy.cl,temp_file,tdcpy.idth);
            }
            else
            {
                if (nrarg==1 && strcmp(cmdarg[0],"exit")==0)
                {
                    char ms='7';  //quitt la erori
                    for (int i=0;i<nrarg;i++)
                        delete[] cmdarg[i];
                    unsigned int msg=cl_k.encrypt_text(ms);
                    if (write (tdcpy.cl,&msg,sizeof(msg))<0)
                    {
                        printf("Eroare la server");
                        rm_th(tdcpy.idth);
                        kill_thr(errno,tdcpy.idth);
                    }
                    rm_th(tdcpy.idth);
                    return NULL;
                }
                if (nrarg==3 && strcmp(cmdarg[0],"login")==0)
                {
                    bool lg=login_try(cmdarg[1],cmdarg[2]);
                    if (lg!=true)
                    {
                        printf("Login Failed\n");fflush(stdout);
                    }
                    else
                    {
                        printf("login Succesful\n");fflush(stdout);
                        tdcpy.log=true;
                    }
                    
                }
                else
                {
                    if (tdcpy.log==true)
                    {
                        divide_cmd(cmdarg,nrarg,tdcpy);
                    }
                    else
                    {
                        fprintf(stderr,"You are not logged in\n");
                    }
                }
            }
            for (int i=0;i<nrarg;i++)
                delete[] cmdarg[i];
            nrarg=0;
            send_ans(tdcpy.cl,temp_file,tdcpy.idth);
        }

    }

    //impartim cmd in functie de ; & si | si facem mai multe forkuri corespunzatoare
    // va fi folosit execvp pentru fiecare comanda de tip unit (nu contine ; | sau &) si stdout (impreuna cu stderr) va fi redirectionat pe un fisier temporar
    // for ( comenzi impartite de ; ) { cream un proces fiu pt orice (sub)comanda impartita de & } (sub(sub))comenzi care sunt impartite de | vor fi legate de fork fork(fork(fork(.....)) de fiecare data este pasat stdout de la prima la stdin din urmatoarea 
}
void send_ans(int cl_sk, char* temp_file, int idth)
{
    char ch='0';
    unsigned int ch2=0;
    close(fileno(stdout));
    close(fileno(stderr));
    int fd=open(temp_file,O_RDONLY);
    if (fd==-1)
    {
        rm_th(idth);
        kill_thr(ftl_err,idth);
    }
    int r=0;
    ch2=cl_k.encrypt_text(ch);
    int w=write(cl_sk,&ch2,sizeof(ch2));
    while ((r=read(fd,&ch,1))!=0)
    {
        if (r==-1)
        {
            rm_th(idth);
            kill_thr(ftl_err,idth);
        }
        ch2=cl_k.encrypt_text(ch);
        w=write(cl_sk,&ch2,sizeof(ch2));
        if (w==-1)
        {
            rm_th(idth);
            kill_thr(ftl_err,idth);
        }
    }
    // remove(temp_file);
    ch='\r';
     ch2=cl_k.encrypt_text(ch);
    write(cl_sk,&ch2,sizeof(ch2));
}
void analize_cmd(char* cmd[],int beg,int en, thData &th);
void divide_cmd (char* cmdarg[],int nrarg, thData& tdc)  // ;
{
    int beg=0,en=0;
    for (int i=0;i<nrarg;i++)
    {
        if (strcmp(cmdarg[i],";")==0 || i==nrarg-1)
        {
            if (i==nrarg-1)
                en=i;
            else
                en=i-1;
            analize_cmd(cmdarg,beg,en,tdc);
            beg=i+1;
        }
    }
}
int procces_cmd (char* cmd[],int b, int e, thData &th);
void analize_cmd(char* cmd[],int beg,int en, thData &th) // && si ||
{
    int begg=beg,enn=begg;
    int r=0;
    int prevop=ne;
    int op=ne;
    for (int i=beg;i<=en;i++)
    {
        if (i==en)
            op=ending;
        else
            op=det_op(cmd[i]);
        switch (op)
        {
            case ne:
            {
                break;
            }
            default :
            {
                if ((prevop==orr && r!=0) || (prevop==andd && r==0) || prevop==ne)
                {
                    enn=i-1;
                    if (op==ending)
                        enn++;
                    if (begg>enn)
                    {
                        redir_outerr(th.idth);
                        error_det(com_ne);
                        return ;
                    }  
                    
                    redir_outerr(th.idth);                
                    r=procces_cmd(cmd,begg,enn,th);
                }
                 prevop=op;
                begg=i+1;
            }   
        }
    }
}
void execute_cmd(char* ch[],int argc, thData &th);
int procces_cmd (char* cmd[],int b, int e, thData &th)  // & si |
{
    int pd1[2],pd2[2];
    int op=ne,beg=b,en,prevop=ne;
    int exit_stat=0;
    for (int i=b;i<=e;i++)
    {
        if (i!=e)
            op=det_op_2(cmd[i]);
        else
            op=ending;
        if (op!=ne)
        {
            en=i-1;
            if (op==ending)
                en++;
            char* cmdd[50];
            for (int j=beg;j<=en;j++)
            {
                cmdd[j-beg]=new char[50];
                char aux[50]="";
                strcpy(aux,cmd[j]);
                strcpy(cmdd[j-beg],aux);
            }
            if (op==ending && prevop==ne && strcmp(cmdd[0],"cd")==0) // cazul special cd dir;
            {
                 int argc=en-beg+1;
                if (argc!=2)
                {
                    for (int i=argc-1;i>=0;i--)
                    {
                        delete[] cmdd[i];
                    }
                    error_det(wr_arg);
                    return wr_arg;
                }
                else
                {
                    int r=cd_ro(th.addr,cmdd[1]);
                    for (int i=argc-1;i>=0;i--)
                    {
                        delete[] cmdd[i];
                    }
                    return r;
                }
            }
            if (op==orr)
            {
                if (pipe(pd2)==-1)
                {
                    rm_th(th.idth);
                    kill_thr(errno,th.idth);
                    exit(errno);
                }
            }
            int ppid=fork();

                switch (ppid)
                {
                    case -1:
                    {
                        kill_thr(errno,th.idth);
                        break;
                    }
                    case 0:
                    {
                        pipe_ord (op,prevop,pd1,pd2);
                        int argc=en-beg+1;
                        execute_cmd(cmdd,argc,th);
                        fprintf(stderr,"eroare imposibila");fflush(stderr);
                        break;
                    }
                    default:
                    {
                        if (prevop==orr)
                        {
                            close(pd1[0]);
                            close(pd1[1]);
                        }
                        std::swap(pd1,pd2);
                        int argc=en-beg+1;
                        for (int i=argc-1;i>=0;i--)
                        {
                            delete[] cmdd[i];
                        }
                        if (op==ending)
                        {
                            int stat;
                            if (waitpid(ppid, &stat, 0)==-1) 
                            {
                                rm_th(th.idth);
                                kill_thr(errno,th.idth);
                            }
                            if ( WIFEXITED(stat) ) 
                            {
                                exit_stat = WEXITSTATUS(stat);
                            }
                            return (exit_stat);      
                        }
                        prevop=op;
                        i=en+1;
                        beg=i+1;
                    }
                }
            }
    }
}
void execute_cmd(char* ch[],int argc, thData &th)    //the last part, executing the code stdout,stdin and stderr should have been redirected to a temp file/socket_pair
{                                                   //toate comenzile care ajung aici sunt apelati in procese fii iar exit o sa indice eroarea (cu exceptie la eror fatale)
    switch (find_cmd(ch[0]))
    {
        case duckoff:
        {
            if (argc==1)
            {
                exit(ftl_err);
            }
            else
            {
                error_det(wr_arg);
                exit(wr_arg);
            }
        }
        case echo:
        {
            if (argc>=3)
            {
                int red=find_red(ch[argc-2]);
                if (red!=nred)
                {
                    redir(red,ch[argc-1],th.addr);
                    delete [] ch[argc-1];
                    delete [] ch[argc-2];
                    argc-=2;
                }
            }
            if (argc==1)
            {
                error_det(wr_arg);
                exit(wr_arg);
            }
            for (int i=1;i<argc;i++)
            {
                printf("%s ",ch[i]);
            }
            exit(0);
        }
        case exiting:
        {
            error_det(wr_ex);
            exit(wr_ex);
        }
        case login:
        {
            if (th.log==true)
            {
                error_det(al_li);
                exit(al_li);
            }
            if (argc!=3)
            {
                error_det(wr_arg);
                exit (wr_arg);
            }
            else
                if  (login_try(ch[2],ch[3])==false)
                {
                    error_det(lg_fail);
                    exit(lg_fail);
                }
                else
                {
                    th.log=true;
                    printf("logged in\n");
                    exit (0);
                }
            break;
        }
        case pwd:
        {
            if (argc!=1 && argc!=3)
            {
                error_det(wr_arg);
                exit (wr_arg);
            }
            else
            {
                if (argc==3)
                {
                    int red=find_red(ch[argc-2]);   // verify if there is a redirection operator 
                    if (red!=nred)  //nred is not a redirection operator
                    {
                        redir(red,ch[argc-1],th.addr);  
                        delete [] ch[argc-1];
                        delete [] ch[argc-2];
                        argc-=2;
                    }
                    else
                    {
                        error_det(wr_arg);
                        exit (wr_arg);
                    }   
                }
                int r=pwd_ro(th.addr);
                exit (r);
            }
        }
        case cd:
        {
            if (argc!=2)
            {
                error_det(wr_arg);
                exit (wr_arg);
            }
            else
            {
                int r=cd_ro(th.addr,ch[1]);
                exit (r);
            }
            break;
        }
        default:
        {
            if (argc>=3)
            {
                int red=find_red(ch[argc-2]);
                if (red!=nred)
                {
                    redir(red,ch[argc-1],th.addr);
                    delete [] ch[argc-1];
                    delete [] ch[argc-2];
                    argc-=2;
                }
            }
            chdir(th.addr);
            ch[argc]=NULL;
            execvp(ch[0],ch);
            fprintf(stderr,"command failed!\n");
            exit (ftl_err);
        }
    }

}
int main()
{
    prep_srv();
    while (1)
    {
        wait_for_client();
    }
}