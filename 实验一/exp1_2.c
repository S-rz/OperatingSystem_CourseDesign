#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

int main(int argc, char *argv[])
{
     pid_t p1,p2,p3;
     if((p1=fork())==0)
     {
         execv("/home/srz/QT_test/build-Add-Desktop_Qt_5_8_0_GCC_64bit-Debug/Add",NULL);
         exit(0);
     }
     else
     {
         if((p2=fork())==0)
         {
             execv("/home/srz/QT_test/build-Display-Desktop_Qt_5_8_0_GCC_64bit-Debug/Display",NULL);
             exit(0);
         }
         else
         {
             if(( p3=fork())==0)
             {
                 execv("/home/srz/QT_test/build-gettime-Desktop_Qt_5_8_0_GCC_64bit-Debug/gettime",NULL);
                 exit(0);
             }
         }
     }
     waitpid(p1,NULL,0);
     waitpid(p2,NULL,0);
     waitpid(p3,NULL,0);
     return 0;
}
