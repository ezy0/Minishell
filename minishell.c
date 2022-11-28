#include "parser.h"
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h> 
#include <fcntl.h>

#include <sys/types.h> //wait
#include <sys/wait.h>  //wait

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

int	cd(tline *linea) {
	char *dir;
	
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


void leerUno(tline *linea) {
	pid_t  pid; //En esta variable se guardará lo que devuelva el fork(). Si == 0, es el hijo. Si es > 0 es el padre. Si es < 0, error
	int status;

	pid = fork();

	if (pid < 0) { //Error en el fork
		linea -> redirect_error = "ERROR: El fork ha fallado" ;
		printf("ERROR: El fork ha fallado");
		exit(1);
	}

	else if(pid == 0){
		execvp(linea -> commands -> argv[0], linea->commands -> argv); //Ejecuta el comando y los argumentos, empezando por 1 más ya que el primero en el array es el propio comando
		//Saltará a la siguiente linea si se ha producido un error
		linea -> redirect_error = "ERROR: El comando/argumentos ejecutados no existen\n" ;
		printf("ERROR: El mandato %s no existe\n" ,linea -> commands -> argv[0]);
		exit(1);
	}

	else{
		wait (&status);
		if (WIFEXITED(status) != 0) //El valor devuelto por WIFEXITED será 0 cuando el hijo ha terminado de una manera anormal.
			if (WEXITSTATUS(status) != 0)//WEXITSTATUS devuelve el valor que ha pasado el hijo a la función exit()
				printf("ERROR: El comando no se ejecutó correctamente\n");
	}

}

int	main() {
	char	buf[BSIZE];
	tline	*linea;

	int		fd_out = dup(1);	//Creas un file descriptor para la salida y lo duplicas, 1 para salida
	int		fd_error = dup(1);

	prompt();
	while (fgets(buf, BSIZE, stdin))	//Lee una linea que introduzca el usuario y la tokeniza? para procesarla
	{
		linea = tokenize(buf); //Leemos linea del teclado
		if (linea == NULL)
			continue;
		// Redireccion de salida
		if (linea -> redirect_output)
			dup2(open(linea->redirect_output, O_CREAT | O_WRONLY, 0642), 1); //Crea una copia del fd del fichero que se ha creado pero usa el fd = 1
		//Redireccion de error
		if (linea -> redirect_error)
			dup2(open(linea->redirect_error, O_CREAT | O_WRONLY, 0642), 1);

		//Leer 1 comando de la linea
		if (linea -> ncommands == 1)
		{
			if (strcmp(linea -> commands[0].argv[0], "exit") == 0)
				exit (0);
			if (strcmp(linea -> commands[0].argv[0], "cd") == 0)
				cd(linea);
			else
				leerUno(linea);
		}

		if (linea -> redirect_output)
			dup2(fd_out, 1);	//Se vuelve a reestablecer la salida a su valor por defecto, 1
		if (linea -> redirect_error)
			dup2(fd_error, 1);

		prompt();
	}
}