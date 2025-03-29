#include "comenzi.h"
#include <fcntl.h>
const int P_READ = 0;
const int P_WRITE = 1;
void redir(int red, char* filename, char* wd)
{
    char ch[50], pa[2]="/";
    strcpy(ch,wd);
    strcat(ch,pa);
    strcat(ch,filename);
    switch (red)
    {
        case outp:
        {
            int fd=open(ch,O_WRONLY);
            close(fileno(stdout));
            dup(fd);
            close(fd);
            break;
        }
        case inp:
        {
            int fd=open(ch,O_RDONLY);
            close(fileno(stdin));
            dup(fd);
            close(fd);
            break;
        }
        case errp:
        {
            int fd=open(ch,O_WRONLY);
            close(fileno(stderr));
            dup(fd);
            close(fd);
            break;
        }
        case outpa:
        {
            int fd=open(ch,O_WRONLY | O_APPEND);
            close(fileno(stdout));
            dup(fd);
            close(fd);
            break;
        }
        case errpa:
        {
            int fd=open(ch,O_WRONLY | O_APPEND);
            close(fileno(stderr));
            dup(fd);
            close(fd);
            break;
        }
        case erraoutp:
        {
            int fd=open(ch,O_WRONLY);
            close(fileno(stderr));
            dup(fd);
            close(fileno(stdout));
            dup(fd);
            close(fd);
            break;
        }
        case erraoutpa:
        {
            int fd=open(ch,O_WRONLY | O_APPEND);
            close(fileno(stderr));
            dup(fd);
            close(fileno(stdout));
            dup(fd);
            close (fd);
            break;
        }
    }
}
void pipe_ord (int op,int prevop,int pdold[], int pdnew[])
{
        if (prevop==orr)
        {
          close(fileno(stdin));
          dup(pdold[P_READ]);
          close(pdold[0]);
          close(pdold[1]);
        }
        if (op==orr)
        {
          close(fileno(stdout));
          dup(pdnew[P_WRITE]);
          close(pdnew[0]);
          close(pdnew[1]);
         }

}
void redir_outerr (int idth)
{
    char temp_file[20]="";
    sprintf(temp_file,"temp_%d.txt",idth);
    int fd=open(temp_file,O_RDWR);
    close (fileno(stdout));
    dup(fd);
    close (fileno(stderr));
    dup(fd);
    close(fd);    
}
