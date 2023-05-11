#include <fcntl.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/stat.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <signal.h>
#include <time.h>

void getLines(char *line1, char *line2, char *line3, char *argv)
{
    int fd = open(argv, O_RDONLY);
    if (fd == -1)
    {
        perror("Error in: open");
    }

    char buff[450];
    int bitesRead = read(fd, buff, 450);
    if (bitesRead == -1)
    {
        perror("Error in: read");
    }

    int i;
    for (i = 0; i < 150; i++)
    {
        if (buff[i] == '\n')
        {
            line1[i] = '\0';
            i++;
            break;
        }
        line1[i] = buff[i];
    }
    for (int temp = i; i < 300; i++)
    {
        if (buff[i] == '\n')
        {
            line2[i - temp] = '\0';
            i++;
            break;
        }
        line2[i - temp] = buff[i];
    }
    for (int temp = i; i < 450; i++)
    {
        if (buff[i] == '\n')
        {
            line3[i - temp] = '\0';
            break;
        }
        line3[i - temp] = buff[i];
    }
}
/////////////////////////////////////////////////////////////////////////////////
void getPathFromLine(char *absPath, char *line)
{
    if (line[0] != '/')
    {
        if (getcwd(absPath, 150) == NULL)
        {
            perror("Error in: getcwd");
        }
        strcat(absPath, "/");
        strcat(absPath, line);
    }
    else
    {
        strcpy(absPath, line);
    }
}
/////////////////////////////////////////////////////////////////////////////////
int checkIfFolder(char *path1, char *path2, char *path3)
{
    struct stat sb;
    memset(&sb, 0, sizeof(sb));
    if (stat(path1, &sb) != 0)
    {
        write(2, "Not a valid directory\n", strlen("Not a valid directory\n"));
        exit(-1);   
    }
    if (!S_ISDIR(sb.st_mode))
    {
        write(2, "Not a valid directory\n", strlen("Not a valid directory\n"));
        exit(-1);
    }
    memset(&sb, 0, sizeof(sb));
    if (stat(path2, &sb) != 0){
        write(2, "Input file not exist\n", strlen("Output file not exist\n"));       
        exit(-1);
    }
    memset(&sb, 0, sizeof(sb));
    if (stat(path3, &sb) != 0){
        write(2, "Output file not exist\n", strlen("Output file not exist\n"));     
        exit(-1);
    }
    return 1;
}
/////////////////////////////////////////////////////////////////////////////////
int checkIfExistCFiles(char *path, char *cFileName)
{

    DIR *d;
    struct dirent *dir;
    struct stat st;
    int found_c_file = 0;

    if ((d = opendir(path)) == NULL)
    {

        perror("Error in: opendir");
        return 0;
    }
    else
    {
        while ((dir = readdir(d)) != NULL)
        {
            if (
                (strcmp(&dir->d_name[strlen(dir->d_name) - 2], ".c") == 0) &&
                (strlen(dir->d_name) > 2))
            {
                strcpy(cFileName, dir->d_name);
                int x = closedir(d);
                if (x == -1)
                {
                    perror("Error in: closedir");
                }
                return 1;
            }
        }
        int x = closedir(d);
        if (x == -1)
        {
            perror("Error in: closedir");
        }
        return 0;
    }
}

/////////////////////////////////////////////////////////////////////////////////
void alarm_hand (int sig) {
	// signal(SIGALRM, alarm_hand);
}

void compileAndRunCFiles(char *folderName, char *absPath2, int fdResults, char *cFileName, bool* compStatus, int errorFD)
{
    int in, out;

    int x = chdir(folderName);
    if (x == -1)
    {
        perror("Error in: chdir");
    }

    pid_t val;
    // fork
    val = fork();
    if (val == -1)
    {
        perror("Error in: fork");
    }
    else if (val > 0)
    {
            int status;
        waitpid(val, &status, 0);
        if (WIFEXITED(status) && WEXITSTATUS(status) == 0)
        {
            // runnig the compiled file with the input from the conf file (line2)

            pid_t pid;
            // fork
            pid = fork();
            if (pid == -1)
            {
                perror("Error in: fork");
            }
            else if (pid > 0)
            {
                int status2;
                waitpid(pid, &status2, 0);
                
                alarm(0);

                if (WIFSIGNALED(status2)){

                    *compStatus = false;
                    char strCompError[150];
                    memset(strCompError, 0, sizeof(strCompError)); // Initialize the array to all zeroes
                    strcpy(strCompError, folderName);
                    strcat(strCompError, ",20,TIMEOUT\n");
                    int b = write(fdResults, strCompError, sizeof(strCompError));
                    if (b == -1)
                    {
                        perror("Error in: write");
                    }
                }
                
                // delete the .out file from the compilation
                if (remove("output.out") != 0)
                {
                    perror("Error in: remove");
                }
            }
            // child - second fork - will execute
            else if (pid == 0)
            {

                if ((in = open(absPath2, O_RDONLY)) == -1)
                {
                    perror("Error in: open");
                }

                if ((out = open("output.txt", O_CREAT | O_WRONLY | O_TRUNC , S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP)) == -1)
                {
                    perror("Error in: open");
                }
                // if ((errorFD = open(absPathErr, O_CREAT | O_RDWR , 0777)) == -1)

                // {
                //     perror("Error in: open");
                // }

                dup2(in, 0);
                dup2(out, 1);
                dup2(errorFD, 2);

                close(in);
                close(out);
                // close(errorFD);

                signal(SIGALRM, alarm_hand);
                alarm(5);

                int ret = execlp("./output.out", "./output.out", NULL);

                if (ret == -1)
                {
                    perror("Error in: execlp");
                }
            }
        }
        // if there are comp errors
        else
        {
            *compStatus = false;
            char strCompError[150];
            memset(strCompError, 0, sizeof(strCompError)); // Initialize the array to all zeroes
            strcpy(strCompError, folderName);
            strcat(strCompError, ",10,COMPILATION_ERROR\n");
            int b = write(fdResults, strCompError, sizeof(strCompError));
            if (b == -1)
            {
                perror("Error in: write");
            }
        }
    }
    // child - first fork - will compile
    else if (val == 0)
    {
        // if ((errorFD = open(absPathErr, O_CREAT | O_RDWR , 0777)) == -1)
        // {
        //     perror("Error in: open");
        // }
        dup2(errorFD, 2);

        // close(errorFD);
        int ret = execlp("gcc", "gcc", "-o", "output.out", cFileName, NULL);

        if (ret == -1)
        {
            perror("Error in: execlp");
        }
    }
}
/////////////////////////////////////////////////////////////////////////////////


int compareOutput(char *compareProgram, char *correctOutput, int errorFD)
{
    pid_t pid;


    // fork
    pid = fork();
    if (pid == -1)
    {
        perror("Error in: fork");
    }
    else if (pid > 0)
    {

        
        int status;
        pid_t result = waitpid(pid, &status, 0);
        if (result == -1) {
            perror("Error in: waitpid");
        }
        else
        {
            int child_ret = WEXITSTATUS(status);
            return child_ret;
        }
    }
    // child
    else if (pid == 0)
    {
        // int errorFD;
        // if ((errorFD = open(absPathErr, O_CREAT | O_RDWR , 0777)) == -1)
        // {
        //     perror("Error in: open");
        // }
        dup2(errorFD, 2);
        // close(errorFD);

        int ret = execlp(compareProgram, compareProgram,correctOutput, "output.txt", NULL);
        if (ret == -1)
        {
            perror("Error in: execlp");
        }
    }
    return 0;
}

/////////////////////////////////////////////////////////////////////////////////
void loopCompile(char *absPath1, char *absPath2, char *absPath3, char *absPathCompare, int errorFD)
{
    char cFileName[150];
    // bool compStatus;
    // open resukt.csv
    int fdResults = open("results.csv", O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
    if (fdResults == -1)
    {
        perror("Error in: open");
    }

    int x = chdir(absPath1);
    if (x == -1)
    {
        perror("Error in: chdir");
    }

    DIR *dip;
    struct dirent *dit;
    if ((dip = opendir(absPath1)) == NULL)
    {
        perror("Error in: opendir");
    }
    while ((dit = readdir(dip)) != NULL)
    {
        bool compStatus = true;
        char strResult[150];
        memset(strResult, 0, sizeof(strResult)); // Initialize the array to all zeroes

        if (dit->d_type == DT_DIR && (strcmp(dit->d_name, ".") != 0) &&
            (strcmp(dit->d_name, "..") != 0))
        {
            if (checkIfExistCFiles(dit->d_name, cFileName))
            {
                // compile the c file in the folder (if exist)
                compileAndRunCFiles(dit->d_name, absPath2, fdResults, cFileName, &compStatus, errorFD);

                // comp succeeded
                //////////////////////////////
                if (compStatus)
                {
                    strcpy(strResult, dit->d_name);

                    int compareValue = compareOutput(absPathCompare, absPath3, errorFD);
                    switch (compareValue)
                    {
                    case 1:
                        strcat(strResult, ",100,EXCELLENT\n");
                        break;
                    case 2:
                        strcat(strResult, ",50,WRONG\n");
                        break;
                    case 3:
                        strcat(strResult, ",75,SIMILAR\n");
                        break;
                    case 4:
                        strcat(strResult, ",20,TIMEOUT\n");
                        break;
                    }
                    int a = write(fdResults, strResult, strlen(strResult));
                    if (a == -1)
                    {
                        perror("Error in: write");
                    }

                    // delete the output.txt file
                    if (remove("output.txt") != 0)
                    {
                        perror("Error in: remove");
                    }
                }
                //////////////////////////////

                int z = chdir("..");
                if (z == -1)
                {
                    perror("Error in: chdir");
                }
            }

            // there aren't any c files
            else
            {
                strcpy(strResult, dit->d_name);
                strcat(strResult, ",0,NO_C_FILE\n");
                int a = write(fdResults, strResult, strlen(strResult));
                if (a == -1)
                {
                    perror("Error in: write");
                }
            }
        }
    }
    close(fdResults);

    int y = closedir(dip);
    if (y == -1)
    {
        perror("Error in: closedir");
    }
}
/////////////////////////////////////////////////////////////////////////////////
int main(int argc, char *argv[])
{
    char line1[150];
    char line2[150];
    char line3[150];
    char absPath1[150];
    char absPath2[150];
    char absPath3[150];
    char absPathErr[150];

    char absPathCompare[150];
    int newfd;

    // get the abs path to the comp.out
    getPathFromLine(absPathCompare, "comp.out");
    // printf("%s\n", absPathCompare);

    // if ((newfd = open("errors.txt", O_CREAT | O_RDWR | O_TRUNC, S_IRUSR | S_IWUSR)) == -1)
    if ((newfd = open("errors.txt", O_CREAT | O_RDWR | O_TRUNC , 0777)) == -1)
    
    {
        perror("Error in: open");
    }
    // dup2(newfd, 2);
    // close(newfd);



    // get the 3 lines from the conf file
    getLines(line1, line2, line3, argv[1]);

    getPathFromLine(absPath1, line1);
    getPathFromLine(absPath2, line2);
    getPathFromLine(absPath3, line3);
    getPathFromLine(absPathErr, "errors.txt");
    
    checkIfFolder(line1, line2, line3);
    loopCompile(absPath1, absPath2, absPath3, absPathCompare, newfd);

    close(newfd);
}
