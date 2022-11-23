#include "parser.h"
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

int BUFFER_SIZE = 1024;

int	ft_strcmp(const char *s1, const char *s2)
{
	unsigned int	i;

	i = 0;
	while ((s1[i] != '\0' || s2[i] != '\0'))
	{
		if ((unsigned char)s1[i] > (unsigned char)s2[i] || s2[i] == '\0')
			return (1);
		else if ((unsigned char)s1[i] < (unsigned char)s2[i] || s1[i] == '\0')
			return (-1);
		i++;
	}
	return (0);
}

void	leerUno(tline *linea) {

}

int	main() {
	char buf[BUFFER_SIZE];
	tline *linea; 

	printf ("msh> ");	//Añadir el directorio??
	while (fgets(buf, BUFFER_SIZE, stdin))	//Lee una linea que introduzca el usuario y la tokeniza? para procesarla
	{
		printf ("msh> ");	//Añadir el directorio??
		linea = tokenize(buf); //Leemos linea del teclado
		if (linea == NULL)
			continue;

		//Leer 1 comando de la linea
		if (linea -> ncommands == 1)
		{
			if (ft_strcmp(linea -> commands[0].argv[0], "exit") == 0)
				exit (0);
			else
				leerUno(linea);
		}

		
	}
}

