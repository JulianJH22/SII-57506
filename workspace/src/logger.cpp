#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>

int main(int argc, char **argv) {
    int fd;
    char buffer[1024];
    ssize_t n;

    // borrar FIFO por si existiera previamente
    unlink("F_logger");

    /* crear el FIFO */
    if (mkfifo("F_logger", 0600)<0) {
        perror("No puede crearse el FIFO");
        return(1);
    }
    /* Abrir el FIFO */
    if ((fd = open("F_logger", O_RDONLY)) < 0) {
        perror("No puede abrirse el FIFO");
        return(1);
    }
    /* Leer de la FIFO */
    while ((n = read(fd, buffer, sizeof(buffer))) > 0) {
	    write(1, buffer, n);
        write(1, "\n", 1);
    }
    close(fd);
    unlink("F_logger");
    return(0);
}