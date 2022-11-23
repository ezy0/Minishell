#include "parser.h"
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h> 

int BSIZE = 1024;


void	prompt() {
	char	dir[BSIZE];

	getcwd(dir, BSIZE); //Nombramos el directorio
	printf ("\033[1;32m");
	printf ("%s ", dir);
	printf ("\033[1;33m");
	printf ("msh> ");
	printf ("\033[0m");
}

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

int	cd(tline *linea) {
	char *dir;
	char buffer[BSIZE];
	
	if(linea -> commands[0].argc > 2)
	{
	  fprintf(stderr,"Debes escribir una sola ruta\n");
	  return 1;
	}
	
	if (linea -> commands[0].argc == 1)
	{
		dir = getenv("HOME");
		if(dir == NULL)
		  fprintf(stderr,"No existe la variable $HOME\n");
	} else
		dir = linea -> commands[0].argv[1];
	
	// Comprobar si es un directorio
	if (chdir(dir) != 0)
		fprintf(stderr,"Error al cambiar de directorio: %s\n", strerror(errno));  

	return 0;
}


void	leerUno(tline *linea) {

}

int	main() {
	char	buf[BSIZE];
	tline	*linea;

	prompt();
	while (fgets(buf, BSIZE, stdin))	//Lee una linea que introduzca el usuario y la tokeniza? para procesarla
	{
		linea = tokenize(buf); //Leemos linea del teclado
		if (linea == NULL)
			continue;

		//Leer 1 comando de la linea
		if (linea -> ncommands == 1)
		{
			if (ft_strcmp(linea -> commands[0].argv[0], "exit") == 0)
				exit (0);
			if (ft_strcmp(linea -> commands[0].argv[0], "cd") == 0)
				cd(linea);
			else
				leerUno(linea);
		}

		prompt();
	}
}
