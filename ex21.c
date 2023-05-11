// Naveh Ullman 327921094

#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdbool.h>
#include <unistd.h>

int main(int argc, char *argv[])
{
    char file1[150];
    char file2[150];

    char ch1;
    char ch2;

    bool eq = true;
    bool sim = false;

    strcpy(file1, argv[1]);
    strcpy(file2, argv[2]);

    int fd1 = open(file1, O_RDONLY);
    int fd2 = open(file2, O_RDONLY);

    int rt1 = read(fd1, &ch1, 1);
    int rt2 = read(fd2, &ch2, 1);
    while (rt1 && rt2)
    // while(read(fd1, &ch1, 1) && read(fd2, &ch2, 1))
    {
        // if the same char
        if (ch1 == ch2)
        {
            rt1 = read(fd1, &ch1, 1);
            rt2 = read(fd2, &ch2, 1);
            continue;
        }
        // it is not the same char -> can't be equal, maybe similar
        eq = false;
        sim = true;

        // check if it is like 'a' and 'A' - mean still similar
        if (ch1 == ch2 + 32 || ch2 == ch1 + 32)
        {
            rt1 = read(fd1, &ch1, 1);
            rt2 = read(fd2, &ch2, 1);
            continue;
        }
        // it is not like 'a' and 'A', check if ch1 is ' ' or '\n'
        if (ch1 == ' ' || ch1 == '\n')
        {
            while (ch1 == ' ' || ch1 == '\n')
            {
                if (!read(fd1, &ch1, 1))
                {
                    break;
                }
            }
        }
        // it is not like 'a' and 'A', check if ch1 is ' ' or '\n'
        if (ch2 == ' ' || ch2 == '\n')
        {
            while (ch2 == ' ' || ch2 == '\n')
            {
                if (!read(fd2, &ch2, 1))
                {
                    break;
                }
            }
        }
        // now ch1 is a regular char, ch2 too. check if ch1 and ch2 are similar
        if (ch1 == ch2 || ch1 == ch2 + 32 || ch2 == ch1 + 32)
        {
            continue;
        }
        // not similar
        else
        {
            eq = false;
            sim = false;
            break;
        }

        rt1 = read(fd1, &ch1, 1);
        rt2 = read(fd2, &ch2, 1);
    }
    // file1 finish, file2 doesnt
    if (rt1 != 0 && rt2 == 0)
    {
        if (ch1 == ' ' || ch1 == '\n')
        {
            eq = false;
            sim = true;
            while (ch1 == ' ' || ch1 == '\n')
            {
                if (!read(fd1, &ch1, 1))
                {
                    break;
                }
            }
            if (!(ch1 == ' ' || ch1 == '\n'))
            {
                sim = false;
            }
        }
        else{
            eq = false;
            sim = false;
        }
    }
    else if (rt1 == 0 && rt2 != 0)
    {

        if (ch2 == ' ' || ch2 == '\n')
        {
            eq = false;
            sim = true;
            while (ch2 == ' ' || ch2 == '\n')
            {
                if (!read(fd2, &ch2, 1))
                {
                    break;
                }
            }
            if (!(ch2 == ' ' || ch2 == '\n'))
            {
                sim = false;
            }
        }
        else{
            eq = false;
            sim = false;
        }

    }
    close(fd1);
    close(fd2);

    if (eq)
    {
        // printf("%d\n", 1);
        return 1;
    }
    else if (sim)
    {
        // printf("%d\n", 3);
        return 3;
    }
    else
    {
        // printf("%d\n", 2);
        return 2;
    }
}