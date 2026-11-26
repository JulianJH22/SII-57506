// Mundo.cpp: implementation of the CMundo class.
//
//////////////////////////////////////////////////////////////////////
#include <fstream>
#include "MundoCliente.h"
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

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CMundoCliente::CMundoCliente()
{
	fd_sc = -1;
    fd_cs = -1;
    fd_bot = -1;

    n_j = 0;
    dat = dat2 = NULL;
    puntos1 = 0;
    puntos2 = 0;
    k = '0';

    Init();
}

CMundoCliente::~CMundoCliente()
{
	if (fd_sc >= 0) close(fd_sc);
	if (fd_cs >= 0) close(fd_cs);
	unlink("F_cs");
}

void CMundoCliente::InitGL()
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
void CMundoCliente::OnDraw()
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

void CMundoCliente::OnTimer(int value)
{	
	//Actualizar la posicion de los objetos en pantalla
	char buf[200];
	ssize_t n = read(fd_sc, buf, sizeof(buf));
	if (n <= 0) exit(0);
	sscanf(buf, "%f %f %f %f %f %f %f %f %f %f %d %d", &esfera.centro.x,&esfera.centro.y, &jugador1.x1,&jugador1.y1,&jugador1.x2,&jugador1.y2, &jugador2.x1,&jugador2.y1,&jugador2.x2,&jugador2.y2, &puntos1, &puntos2);
	
	//Actualizar la posicion de los objetos para el bot
	dat->esfera=esfera;
	dat->raqueta=jugador2;
	dat->puntos_1=puntos1;
	dat->puntos_2=puntos2;

	dat2->esfera=esfera;
	dat2->raqueta=jugador2;
	dat2->puntos_1=puntos1;
	dat2->puntos_2=puntos2;

	n_j = dat->j_id + dat2->j_id;

	if (puntos1 >= 3 || puntos2 >= 3) { fprintf(stderr, "cliente sale con puntos %d-%d\n", puntos1, puntos2); exit(0);}

	ssize_t nn = 0;
	char bbot[100];
	memset(bbot, 0, sizeof(bbot));
	sprintf(bbot, "%c %d %d %d", k, n_j, dat->accion, dat2->accion);
	nn = write(fd_cs, bbot, sizeof(bbot));
	if (nn < 0) {
        perror("Error escribiendo en F_cs");
        close(fd_cs);
        fd_cs = -1;
    }
	k = '0';

}

void CMundoCliente::OnKeyboardDown(unsigned char key, int x, int y)
{
	k = key;
}

void CMundoCliente::Init()
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

	unlink("F_cs");
	if (mkfifo("F_cs", 0600) < 0) { perror("mkfifo F_cs"); return; }

	if ((fd_sc = open("F_sc", O_RDONLY)) < 0) { perror("open F_sc"); return; }	
	if ((fd_cs = open("F_cs", O_WRONLY)) < 0) { perror("open F_cs"); return; }

	//Proyeccion de memoria compartida para el bot
	if ((fd_bot = open("F_bot", O_RDWR | O_CREAT | O_TRUNC, 0666)) < 0) {
		perror("No puede abrirse el bot");
	}

	ftruncate(fd_bot, 2*sizeof(DatosMemCompartida));

	dat = (DatosMemCompartida*)mmap(NULL, 2*sizeof(DatosMemCompartida), PROT_READ | PROT_WRITE, MAP_SHARED, fd_bot, 0);

	if (dat == MAP_FAILED)
	{
		perror("Error en la proyeccion mmap"); 
		exit (1);
	}

	dat2 = dat + 1;

	dat ->j_id = 0;
	dat2 ->j_id = 0;

	close(fd_bot);
}
