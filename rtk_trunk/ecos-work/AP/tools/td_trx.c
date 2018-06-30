#include<stdio.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>
#define TD_REALTEK_MAGIC    0x304b5452/* "RTK0" */
int main(int argc, char **argv)
{
    int fd;
    unsigned int magic;
    if (argc != 2) {
        printf("help: td_trx [filepath]\n");
        return 0;
    }

    magic = TD_REALTEK_MAGIC;
    fd = open(argv[1],O_WRONLY);
    
    if (fd < 0) {
        printf("open file err!");
        return -1;
    }
    write(fd, &magic,sizeof(magic));
    close(fd);
    return 0;
}