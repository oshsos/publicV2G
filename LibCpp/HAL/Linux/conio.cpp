/**
 By Doug Smythies
 https://askubuntu.com/questions/1185926/how-can-i-use-kbhit-function-while-using-gcc-compiler-as-it-does-not-contains
*/

#include <string.h>
#include <termios.h>
#include <sys/time.h>
#include <cstdio>

int _kbhit()
{
    static bool initialized = false;
    static fd_set set;
    static struct timeval tv;
    static struct termios t;

    if (!initialized)
    {
        memset(&tv, 0, sizeof(tv));
        tv.tv_usec = 1;
        tcgetattr(0, &t);
        t.c_lflag &= ~ICANON;
        //cfmakeraw(&t);
        tcsetattr(0, TCSANOW, &t);
        initialized = true;
    }

    FD_ZERO(&set);
    FD_SET(0, &set);
    select(1, &set, 0, 0, &tv);
    return FD_ISSET(0, &set);
}

char getch()
{
    return getchar();
}

//    #include <stdio.h>
//    #include "BibCpp/HAL/Linux/conio.h"
//
//    int main()
//    {
//        printf("press 'q'\n");
//        while (1)
//        {
//            if (_kbhit())
//            {
//                input = getchar();
//                printf("%d", (int)input);
//                if (input == 'q') break;
//            }
//            printf(".");
//            fflush(stdout); // if the printf has no new line the sleep() function prevents printing to the screen
//            sleep(1);
//        }
//        printf("\nend\n");
//        return 0;
//    }
