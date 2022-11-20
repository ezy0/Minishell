#include "parser.h"
#include <stdio.h>

int BUFFER_SIZE = 1024;

int main() {
	char buf[BUFFER_SIZE];
	tline * line; //QUE ES ESTO LOL

	while (fgets(buf, BUFFER_SIZE, stdin > 0))
	{
		printf ("msh> ");	//Añadir el directorio??
		line = tokenize(buf); //Leemos linea del teclado
		if (line == NULL)
			continue;
		execline(line, buf);
	}
}