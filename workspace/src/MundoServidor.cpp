// Mundo.cpp: implementation of the CMundo class.
//
//////////////////////////////////////////////////////////////////////
#include <fstream>
#include "MundoServidor.h"
#include "glut.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/mman.h>
#include <string.h>
#include <math.h>
#include <pthread.h>

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

void* hilo_comandos(void* d)
{
      CMundoServidor* p=(CMundoServidor*) d;
      p->RecibeComandosJugador();
	  return NULL;
}

void CMundoServidor::RecibeComandosJugador()
{
     while (1) {
            usleep(10);
            char cad[100];
			ssize_t n = read(fd_cs, cad, sizeof(cad) - 1);
			if (n <= 0) break;      // el cliente cerró o hubo error
            cad[n] = '\0';
            unsigned char key;
            sscanf(cad,"%c %d %d %d",&key, &n_j, &accion1, &accion2);

			if (n_j < 2) { 
				switch(key){ 
					case 's': jugador1.velocidad.y=-4; 	break; 
					case 'w': jugador1.velocidad.y=4; break; 
				} 
			}

			if (n_j == 0) { 
				switch(key){ 
					case 'l': jugador2.velocidad.y=-4; break; 
					case 'o': jugador2.velocidad.y=4; break; 
				} 
			}
      }
}

CMundoServidor::CMundoServidor()
{
	fd = -1;
	fd_sc = -1;
	fd_cs = -1;
	n_j = accion1 = accion2 = 0;
	puntos1 = 0;
	puntos2 = 0;
	Init();
}

CMundoServidor::~CMundoServidor()
{
	pthread_cancel(thid1);
	pthread_join(thid1, NULL);
	if (fd >= 0) close(fd);
	if (fd_sc >= 0) close(fd_sc);
	if (fd_cs >= 0) close(fd_cs);
	unlink("F_sc");
	
}

void CMundoServidor::InitGL()
{
	//Habilitamos las luces, la renderizacion y el color de los materiales
	glEnable(GL_LIGHT0);
	glEnable(GL_LIGHTING);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_COLOR_MATERIAL);
	
	glMatrixMode(GL_PROJECTION);
	gluPerspective( 40.0, 800/600.0f, 0.1, 150);
}

static void print(char *mensaje, int x, int y, float r, float g, float b)
{
	glDisable (GL_LIGHTING);

	glMatrixMode(GL_TEXTURE);
	glPushMatrix();
	glLoadIdentity();

	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	gluOrtho2D(0, glutGet(GLUT_WINDOW_WIDTH), 0, glutGet(GLUT_WINDOW_HEIGHT) );

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);
	glColor3f(r,g,b);
	glRasterPos3f(x, glutGet(GLUT_WINDOW_HEIGHT)-18-y, 0);
	int len = strlen (mensaje );
	for (int i = 0; i < len; i++) 
		glutBitmapCharacter (GLUT_BITMAP_HELVETICA_18, mensaje[i] );
		
	glMatrixMode(GL_TEXTURE);
	glPopMatrix();

	glMatrixMode(GL_PROJECTION);
	glPopMatrix();

	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();

	glEnable( GL_DEPTH_TEST );
}

void CMundoServidor::OnDraw()
{
	//Borrado de la pantalla	
   	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//Para definir el punto de vista
	glMatrixMode(GL_MODELVIEW);	
	glLoadIdentity();
	
	gluLookAt(0.0, 0, 17,  // posicion del ojo
		0.0, 0.0, 0.0,      // hacia que punto mira  (0,0,0) 
		0.0, 1.0, 0.0);      // definimos hacia arriba (eje Y)    

	/////////////////
	///////////
	//		AQUI EMPIEZA MI DIBUJO
	char cad[100];
	sprintf(cad,"Jugador1: %d",puntos1);
	print(cad,10,0,1,1,1);
	sprintf(cad,"Jugador2: %d",puntos2);
	print(cad,650,0,1,1,1);
	int i;
	for(i=0;i<paredes.size();i++)
		paredes[i].Dibuja();

	fondo_izq.Dibuja();
	fondo_dcho.Dibuja();
	jugador1.Dibuja();
	jugador2.Dibuja();
	esfera.Dibuja();

	/////////////////
	///////////
	//		AQUI TERMINA MI DIBUJO
	////////////////////////////

	//Al final, cambiar el buffer
	glutSwapBuffers();
}

void CMundoServidor::OnTimer(int value)
{	
	char cad[100];

	jugador1.Mueve(0.025f);
	jugador2.Mueve(0.025f);
	esfera.Mueve(0.025f);

	if (n_j == 2) {

		if(accion2==1) jugador1.velocidad.y=8;
		else if(accion2==-1) jugador1.velocidad.y=-8;
		else jugador1.velocidad.y=0;

		if(accion1==1) jugador2.velocidad.y=8;
		else if(accion1==-1) jugador2.velocidad.y=-8;
		else jugador2.velocidad.y=0;

	} else if (n_j == 1) {

		if(accion1==1) jugador2.velocidad.y=8;
		else if(accion1==-1) jugador2.velocidad.y=-8;
		else jugador2.velocidad.y=0;

	}


	int i;
	for(i=0;i<paredes.size();i++)
	{
		paredes[i].Rebota(esfera);
		paredes[i].Rebota(jugador1);
		paredes[i].Rebota(jugador2);
	}

	jugador1.Rebota(esfera);
	jugador2.Rebota(esfera);
	if(fondo_izq.Rebota(esfera))
	{
		esfera.centro.x=0;
		esfera.centro.y=rand()/(float)RAND_MAX;
		esfera.velocidad.x=5+2*rand()/(float)RAND_MAX;
		esfera.velocidad.y=8+2*rand()/(float)RAND_MAX;
		puntos2++;
		if (puntos2 >= 3)
		{
			sprintf(cad,"Jugador 2 gana la partida.\n");
			if (fd >= 0) write(fd, cad, strlen(cad));
		} else {
			sprintf(cad,"Jugador 2 marca 1 punto, lleva un total de %d puntos.\n",puntos2);
			if (fd >= 0) write(fd, cad, strlen(cad));
		}
	}

	if(fondo_dcho.Rebota(esfera))
	{
		esfera.centro.x=0;
		esfera.centro.y=rand()/(float)RAND_MAX;
		esfera.velocidad.x=-5-2*rand()/(float)RAND_MAX;
		esfera.velocidad.y=-8-2*rand()/(float)RAND_MAX;
		puntos1++;
		if (puntos1 >= 3)
		{
			sprintf(cad,"Jugador 1 gana la partida.\n");
			if (fd >= 0) write(fd, cad, strlen(cad));
		} else {
			sprintf(cad,"Jugador 1 marca 1 punto, lleva un total de %d puntos.\n",puntos1);
			if (fd >= 0) write(fd, cad, strlen(cad));
		}

	}

	char buf[200];
	memset(buf, 0, sizeof(buf));
	sprintf(buf, "%f %f %f %f %f %f %f %f %f %f %d %d", esfera.centro.x,esfera.centro.y, jugador1.x1,jugador1.y1,jugador1.x2,jugador1.y2,jugador2.x1,jugador2.y1,jugador2.x2,jugador2.y2, puntos1, puntos2);
	if (fd_sc >= 0) write(fd_sc, buf, sizeof(buf));

	if (puntos1 >= 3 || puntos2 >= 3) {usleep(250000); exit(0);}
}

void CMundoServidor::OnKeyboardDown(unsigned char key, int x, int y)
{
}

void CMundoServidor::Init()
{
	Plano p;
//pared inferior
	p.x1=-7;p.y1=-5;
	p.x2=7;p.y2=-5;
	paredes.push_back(p);

//superior
	p.x1=-7;p.y1=5;
	p.x2=7;p.y2=5;
	paredes.push_back(p);

	fondo_izq.r=0;
	fondo_izq.x1=-7;fondo_izq.y1=-5;
	fondo_izq.x2=-7;fondo_izq.y2=5;

	fondo_dcho.r=0;
	fondo_dcho.x1=7;fondo_dcho.y1=-5;
	fondo_dcho.x2=7;fondo_dcho.y2=5;

	//a la izq
	jugador1.g=0;
	jugador1.x1=-6;jugador1.y1=-1;
	jugador1.x2=-6;jugador1.y2=1;

	//a la dcha
	jugador2.g=0;
	jugador2.x1=6;jugador2.y1=-1;
	jugador2.x2=6;jugador2.y2=1;

	if ((fd = open("F_logger", O_WRONLY)) < 0) {
		perror("No puede abrirse el FIFO");
	}

	//FIFO Servidor-Cliente
	unlink("F_sc");
	if (mkfifo("F_sc", 0600) < 0) { perror("mkfifo F_sc"); return; }

	if ((fd_sc = open("F_sc", O_WRONLY)) < 0) { perror("open F_sc"); return; }
	if ((fd_cs = open("F_cs", O_RDONLY)) < 0) { perror("open F_cs"); return; }

	pthread_create(&thid1, NULL, hilo_comandos, this);
}

