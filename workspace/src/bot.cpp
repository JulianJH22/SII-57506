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
	DatosMemCompartida* dat, *dat2;
	char *direccion;
	int fd;
	bool expectador = false;
	
	if ((fd = open("F_bot", O_RDWR, 0666)) < 0) {
		perror("No puede abrirse el bot");
	}

	dat = (DatosMemCompartida*)mmap(NULL, 2*sizeof(DatosMemCompartida), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

	if (dat == MAP_FAILED)
	{
		perror("Error en la proyeccion mmap"); 
		return 1;
	}
	close(fd);
	if (dat->j_id == 0) {
		dat2 = dat;
		dat->j_id = 1;
	} else {
		dat2 = dat + 1;
		if (dat2->j_id >= 1) expectador = true; 
		else dat2->j_id = 1;
	}
	

	while(1)
	{
		if (expectador) {
			usleep(10000);
			continue;
		}

		usleep(2500);
		float centro = (dat2->raqueta.y1 + dat2->raqueta.y2) / 2.0f;
		float objetivo = dat2->esfera.centro.y;

		if      (objetivo > centro + 0.2f) dat2->accion = 1;
		else if (objetivo < centro - 0.2f) dat2->accion = -1;
		else dat2->accion = 0;

		if (dat2->puntos_2 >= 3) {
			printf("Bot: He ganado!\n");
			break;
		} else if (dat2->puntos_1 >= 3) {
			printf("Bot: He perdido!\n");
			break;
		}
	}
	return 0;
}