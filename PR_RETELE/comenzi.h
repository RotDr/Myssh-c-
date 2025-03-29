#include "detectari.h"
#include <sys/stat.h>
#include "errorhandle.h"
#include "sqlite3.h"
#include <unistd.h>
bool ver_d(char addr[])
{
     struct stat sb;
     if (stat(addr, &sb)!=0)
     {
        return false;
     }
     return true;
}
bool ver_f(char addr[])
{
     return access(addr,F_OK)!=-1;
}
bool login_try(char usr[],char psd[])
{
    bool ok=false;
    char db_name[]="ids.db";
    sqlite3* db;
    sqlite3_stmt *state;
    int rc=sqlite3_open(db_name,&db);
    if (rc!= SQLITE_OK)
    {
        error_det(ftl_err);
        exit(ftl_err);
    }
    char ch[200]="";
    sprintf(ch,"SELECT 69 FROM identity i where i.name like '%s' and i.password like '%s' ;",usr,psd);
    const char * sql=ch;
    rc=sqlite3_prepare_v2(db,sql,-1,&state,NULL);
    if (rc!=SQLITE_OK)
    {
        error_det(ftl_err);
        sqlite3_close(db);
        exit(ftl_err);
    }
    else
    {
        rc=sqlite3_step(state);
        if (rc!=SQLITE_DONE)
        {
            ok=true;
        }
        else
        {
             sqlite3_clear_bindings(state);
            sqlite3_reset(state);
        }   
    }
   
    sqlite3_finalize(state);
    sqlite3_close(db);
    return ok;
        
}
int cd_ro(char addr[],char fld[])
{
    if (!ver_d(addr))
    {
        error_det(ivd_addr);
        return ivd_addr;
    }
    char ch[200]="";
    strcpy(ch,addr);
    char fldd[30]="";
    int beg=0,en=0;
    for (int i=0;fld[i];i++)
    {
        if (fld[i+1]=='/' || fld[i+1]==0)
        {
            en=i;
            i++;
            strncpy(fldd,fld+beg,en-beg+1);
            fldd[en-beg+1]=0;
            int j=strlen(ch);
            if (strcmp(fldd,"..")==0)
            {
                j--;
                while (ch[j]!='/')
                    ch[j--]=0;
                if (j!=0)
                    ch[j]=0;
            }
            else
            {
                ch[j]='/';
                strcat(ch,fldd);
                if (!ver_d(ch))
                {
                    error_det(ne_dir);
                    return ne_dir;
                }

            }
            beg=i+1;
        }
    }
    strcpy(addr,ch);
    return 0;

}
int pwd_ro(char addr[])
{
    if (ver_d(addr)==true)
        printf("%s\n",addr);
    else
    {
        error_det(ivd_addr);
        return ivd_addr;
    }
    return 0;
}