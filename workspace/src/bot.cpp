#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "DatosMemCompartida.h"
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>

int main(int argc,char* argv[])
{
	int i;
	DatosMemCompartida* dat;
	char *direccion;
	int fd;
	
	if ((fd = open("F_bot", O_RDWR, 0666)) < 0) {
		perror("No puede abrirse el bot");
	}

	dat = (DatosMemCompartida*)mmap(NULL, sizeof(DatosMemCompartida), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

	if (dat == MAP_FAILED)
	{
		perror("Error en la proyeccion mmap"); 
		return 1;
	}
	close(fd);
	

	
	while(1)
	{
		usleep(2500);
		float centro = (dat->raqueta.y1 + dat->raqueta.y2) / 2.0f;
		float objetivo = dat->esfera.centro.y;

		if      (objetivo > centro + 0.2f) dat->accion = 1;
		else if (objetivo < centro - 0.2f) dat->accion = -1;
		else dat->accion = 0;

		if (dat->puntos_2 >= 3) {
			printf("Bot: He ganado!\n");
			break;
		} else if (dat->puntos_1 >= 3) {
			printf("Bot: He perdido!\n");
			break;
		}
	}
	return 0;
}