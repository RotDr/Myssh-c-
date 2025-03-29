#include <stdio.h>
#include <errno.h>
enum erori
{
    suc,ivd_addr,ne_dir,ftl_err,wr_arg,com_ne,na_con,lg_fail,al_li,wr_ex
};
void kill_srv (int er)
{
    //fortam inchidere eroare fatala din server
    // will be added later 
    perror ("An error on main thread has been detected, the error is");
    fprintf(stderr,"Shutting down server");
    exit(er);
}
void exit_ro()
{
    exit(0);
}
void kill_thr (int er, int idth)
{
    if (er!=com_ne)
    {
        perror("A major error has occured in the thread:");
    }
    else
    {
        fprintf (stderr,"A major error has occured in the thread: the command hasn't been found or is non existent\n");
    }
    fprintf(stderr,"\nShutting down the client number %d\n",idth);
    exit(er);
    //fortam inchidere eroare letala thread 
    //will be added later
}
void error_det (int err)
{
    fprintf (stderr,"An error has been detected, the error is... ");
    switch (err)
    {
        case ivd_addr:
        {
            fprintf (stderr,"the address is invalid\n");
            break;
        }
        case ne_dir:
        {
            fprintf (stderr,"director provided is nonexistent\n");
            break;
        }
        case wr_arg:
        {
            fprintf (stderr,"director address is invalid\n");
            break;
        }
        case na_con:
        {
            fprintf (stderr,"You are not allowed to put command without lbeing logged in first\n");
            break;
        }
        case lg_fail:
        {
            fprintf (stderr,"log in failed, please try again\n");
            break;
        }
        case al_li:
        {
            fprintf (stderr,"you are already logged in\n");
            break;
        }
        case wr_ex:
        {
            fprintf (stderr,"Wrongful exit: The command exit can't be chained by other commands\n");
            break;
        }
    }
}