#include "parser.h"
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h> 
#include <fcntl.h>
#include <signal.h>
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

void	comprobacionBg(tline *linea){
	if (linea->background)
		signal (SIGINT, SIG_IGN);
	else
		signal (SIGINT, SIG_DFL);
}

void leerUno(tline *linea) {
	pid_t  pid; //En esta variable se guardará lo que devuelva el fork(). Si == 0, es el hijo. Si es > 0 es el padre. Si es < 0, error
	int status;

	comprobacionBg(linea);
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
		if (linea->background)
			waitpid(pid, &status, WNOHANG);
		else {
			waitpid(pid, &status, 0);
			if (WIFEXITED(status) != 0) //El valor devuelto por WIFEXITED será 0 cuando el hijo ha terminado de una manera anormal.
				if (WEXITSTATUS(status) != 0)//WEXITSTATUS devuelve el valor que ha pasado el hijo a la función exit()
					printf("ERROR: El comando no se ejecutó correctamente\n");
		}
	}
}

void	variosComandos(tline *linea){
	pid_t	pid;
	int		**p_hijos;
	int		status;
	int		i = -1;	//i es un contador
	int		j, k; //j es un limite para crear los pipes y k un contador que usaremos para ir cerrando pipes

	j = linea->ncommands - 2;
	k = 0;
	p_hijos = (int **)malloc(sizeof(int *) * j);
	while (++i <= j)
		p_hijos[i] = (int *)malloc(sizeof(int) * 3);
	i = 0;
	comprobacionBg(linea);
	pipe(p_hijos[0]);
	pid = fork();
	if (pid < 0) {
		fprintf(stderr, "ERROR: %s\n", strerror(errno));
		exit (1);
	}
	if (pid == 0) {
		close(p_hijos[0][0]);
		dup2(p_hijos[0][1], STDOUT_FILENO);
		/*while(++k <= j){ //Cerramos el resto de pipes que no nos interesan	
			close (p_hijos[k][0]);
			close (p_hijos[k][1]);
		}*/
		execvp(linea -> commands[0].argv[0], linea -> commands[0].argv);
		printf("ERROR: El mandato %s no existe\n" ,linea -> commands[0].argv[0]); //En caso de error
		exit(1);
	} else {
		/*if (linea->ncommands > 2) {
			while (++i <= j)
				pipe(p_hijos[i]);
			i = 0;
			k = -1;
			while (++i <= j){
				pid = fork();

				if (pid < 0) {
					fprintf(stderr, "ERROR: %s\n", strerror(errno));
					exit (1);
				}
				if (pid == 0) {
					close(p_hijos[i-1][1]);
					dup2(p_hijos[i-1][0], STDIN_FILENO);
					close(p_hijos[i][0]);
					dup2(p_hijos[i][1], STDOUT_FILENO);
					while(++k <= j)	//Cerramos el resto de pipes que no nos interesan
						if (k != i && k != i - 1){
							close (p_hijos[k][0]);
							close (p_hijos[k][1]);
						}
					execvp(linea -> commands[i].argv[0], linea -> commands[i].argv);
					printf("ERROR: El mandato %s no existe\n" ,linea -> commands[i].argv[0]); //En caso de error
					exit(1);
				}
			}
		}*/
		pid = fork(); // Último hijo

		if (pid < 0) {
			fprintf(stderr, "ERROR: %s\n", strerror(errno));
			exit (1);
		}
		else if (pid == 0) {
			k = -1;
			close (p_hijos[j][1]);
			dup2(p_hijos[j][0], STDIN_FILENO);
			while(++k < j){ //Cerramos el resto de pipes que no nos interesan	
				close (p_hijos[k][0]);
				close (p_hijos[k][1]);
			}
			execvp(linea -> commands[j + 1].argv[0], linea -> commands[j + 1].argv);
			printf("ERROR: El mandato %s no existe\n" ,linea -> commands[j + 1].argv[0]); //En caso de error
			exit(1);
		}
	}	//Padre
	i = 0;
	k = -1;
	while (++k < linea -> ncommands)
		wait(&status);
	free (p_hijos);
	while (++i <= j)
		free(p_hijos[i]);
}

int	entrada(tline *linea){
	int	fd;

	fd = open(linea->redirect_input, O_RDONLY);
	if (fd == -1)
		fprintf(stderr, "ERROR: %s\n", strerror(errno));
	return (fd);
}

int	main() {
	char	buf[BSIZE];
	tline	*linea;

	int		in_error = 0; //Nos servirá de flag para evitar ejecutar un comando con una entrada no válida y que se nos quede colgado

	int 	fd_in = dup(0);
	int		fd_out = dup(1);	//Creas un file descriptor para la salida y lo duplicas, 1 para salida
	int		fd_error = dup(2);

	signal(SIGINT, SIG_IGN);
	prompt();
	while (fgets(buf, BSIZE, stdin))
	{
		linea = tokenize(buf); //Leemos linea del teclado
		if (linea == NULL)
			continue;

		//	Redireccion de entrada
		if (linea -> redirect_input) {
			in_error = entrada(linea);
			dup2(in_error, 0);	//Cuando grep falla no vuelve a mostrar el prompt después
		}
		// Redireccion de salida
		if (linea -> redirect_output)
			dup2(open(linea->redirect_output, O_CREAT | O_WRONLY, 0642), 1); 
		//Redireccion de error
		if (linea -> redirect_error)
			dup2(open(linea->redirect_error, O_CREAT | O_WRONLY, 0642), 2);

		//Leer 1 comando de la linea
		if (linea -> ncommands == 1 && in_error != -1)
		{
			if (strcmp(linea -> commands[0].argv[0], "exit") == 0)
				exit (0);
			if (strcmp(linea -> commands[0].argv[0], "cd") == 0)
				cd(linea);
			else
				leerUno(linea);
		} else if (linea -> ncommands >= 2 && in_error != -1)
			variosComandos(linea);

		if (linea -> redirect_input)
			dup2(fd_in, 0);
		if (linea -> redirect_output)
			dup2(fd_out, 1);	//Se vuelve a reestablecer la salida a su valor por defecto, 1
		if (linea -> redirect_error)
			dup2(fd_error, 2);

		prompt();
	}
}