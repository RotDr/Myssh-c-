#include <stdio.h>
#include <iostream>
#include <unistd.h>
#include <iostream>
#include <unistd.h>
unsigned int euclid(unsigned int a,unsigned int b)
{
    while (b!=0)
    {
        unsigned int r=a%b;
        a=b;
        b=r;
    }
    return a;
}
unsigned int rapid_exp_mod (unsigned int nr, unsigned int e, unsigned int mod)
{
    unsigned long long nrcpy=(unsigned long long)nr;
    unsigned long long result=1;
    unsigned long long modcpy=(unsigned long long)mod;
    nrcpy%=modcpy;
    if (nrcpy==0) return 0;

    while (e>0)
    {
        if (e%2==1)
        {
            result=(result*nrcpy)%modcpy;
            e--;
        }
        nrcpy=(nrcpy*nrcpy)%modcpy;
        e/=2;
    }
    return (unsigned int)result;
}
unsigned int euclid_dlc(unsigned int e,unsigned int phi, unsigned int mod)
{
   unsigned int inv, u1, u3, v1, v3, t1, t3, q;
    int64_t iter;
    u1 = 1;
    u3 = e;
    v1 = 0;
    v3 = phi;
    iter = 1;
    while(v3 != 0)
    {
        q = u3 / v3;
        t3 = u3 % v3;
        t1 = u1 + q * v1;
        u1 = v1;
        v1 = t1;
        u3 = v3;
        v3 = t3;
        iter = -iter;
    }
    if(u3 != 1)
        return  0;
    if(iter < 0)
        inv = phi - u1;
    else
        inv = u1;
    return inv;
}
class en_keys
{
    public:
    unsigned int pr_kphi;
    unsigned int pr_kd;
    int pu_ke;
    unsigned int pu_kn;
    en_keys (unsigned int,unsigned int);
    en_keys ();
    void give_pbk(int fd)
    {
        int sz=sizeof(this->pu_ke);
        if (write(fd,&this->pu_ke,sz)!=sz)
        {
            perror("error when giving the public key, shutting down");
            exit (1);
        }
        if (write(fd,&this->pu_kn,sz)!=sz)
        {
            perror("error when giving the public key, shutting down");
            exit (1);
        }
    }
    void get_pbk(int fd)
    {
        int sz=sizeof(unsigned int);
        if (read(fd,&this->pu_ke,sz)!=sz)
        {
            perror("error when receiving the public key, shutting down");
            exit (1);
        }
        if (read(fd,&this->pu_kn,sz)!=sz)
        {
            perror("error when receiving the public key, shutting down");
            exit (1);
        }
    }
    unsigned int encrypt_text(char ch)
    {
        unsigned int ch2=(unsigned int)ch;
        ch2=rapid_exp_mod(ch2,this->pu_ke,this->pu_kn);
        return ch2;
    }
    char decrypt_text(unsigned int ch)
    {
        char ch2=(char)rapid_exp_mod(ch,this->pr_kd,this->pu_kn);
        return ch2;

    }
};
en_keys::en_keys(unsigned int prim1,unsigned int prim2)
{
    this->pu_kn=prim1*prim2;
    this->pu_ke=65537;
    this->pr_kphi=(prim1-1)*(prim2-1);
    this->pr_kd=euclid_dlc(this->pu_ke,this->pr_kphi,this->pu_kn);
}
en_keys::en_keys()
{
    this->pr_kphi=0;
    this->pu_ke=0;
    this->pu_kn=0;
    this->pr_kd=0;
}