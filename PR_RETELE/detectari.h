#include <cstring>
#include <stdio.h>
enum redirection
{
    outp,inp,outpa,errp,errpa,erraoutp,erraoutpa,nred
};
char red[9][4]={">","<",">>","<<","2>","2>>","&>","&>>"};
enum operatori
{
    andd,orr,ne,ending
};
enum cmd
{
    login,pwd,cd,exiting,duckoff,echo,other
};
char comenzi[6][7]={"login","pwd","cd","exit","false","echo"};
int find_red(char *ch)
{
    for (int i=0;i<7;i++)
    {
        if (strcmp(ch,red[i])==0)
            return i;
    }
    return nred;
}
int find_cmd(char cmd[])
{
    for (int i=0;i<6;i++)
    {
        if (strcmp(cmd,comenzi[i])==0)
            return i;
    }
    return 6;
}
int det_op (char* ch)
{
    if (strcmp(ch,"||")==0)
        return orr;
    if (strcmp(ch,"&&")==0)
        return andd;
    return ne;
}
int det_op_2 (char* ch)
{
    if (strcmp(ch,"|")==0)
        return orr;
    if (strcmp(ch,"&")==0)
        return andd;
    return ne;
}