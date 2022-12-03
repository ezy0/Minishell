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

typedef struct Job{
    pid_t	pid;
    char	*nombre;
}tJob;

typedef struct s_list{
	tJob			*job;
	struct s_list	*next;
} t_list;

tJob 	*CrearJob(int pid, char *buf);
t_list	*CrearListaJobs();
void	addJob(t_list *lst, tJob *new_job);
void	mostrarJobs(t_list *lista);
void	freeLista(t_list *lista);

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

void leerUno(tline *linea, t_list *lista_jobs, char *buf) {
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
		if(linea->background ==1){  
            waitpid(pid,&status,WNOHANG);
            printf("[%d]\n",pid);
            addJob(lista_jobs, CrearJob(pid, buf));
        }else{
            waitpid (pid,&status,0);
            if (WIFEXITED(status) != 0) 
                if (WEXITSTATUS(status) != 0)
                    printf("ERROR: El comando no se ejecutó correctamente\n");  
        }
	}
}

void	variosComandos(tline *linea, t_list *lista_jobs, char *buf){
	pid_t	pid;
	int		**p_hijos;
	int		status;
	int		i, j, k; //j es un limite para crear los pipes, i y k son contadores

	j = linea -> ncommands - 2;
	// Asignamos memoria para el array de pipes
	p_hijos = (int **)malloc(sizeof(int *) * j);
	for (i = 0; i <= j; i++)
		p_hijos[i] = (int *)malloc(sizeof(int) * 3);
	
	comprobacionBg(linea);
	pipe(p_hijos[0]);

	pid = fork();	//Primer hijo
	if (pid < 0) {
		fprintf(stderr, "ERROR: %s\n", strerror(errno));
		exit (1);
	}
	if (pid == 0) {
		close(p_hijos[0][0]);
		dup2(p_hijos[0][1], STDOUT_FILENO);
		execvp(linea -> commands[0].argv[0], linea -> commands[0].argv);
		printf("ERROR: El mandato %s no existe\n" ,linea -> commands[0].argv[0]); //En caso de error
		exit(1);
	} else {
		if (linea->ncommands > 2) {
			for (i = 1; i <= j; i++)
				pipe(p_hijos[i]);
			for (i = 1; i <= j; i++){
				pid = fork();

				if (pid < 0) {
					fprintf(stderr, "ERROR: %s\n", strerror(errno));
					exit (1);
				}
				if (pid == 0) {
					comprobacionBg(linea);
					close(p_hijos[i-1][1]);
					dup2(p_hijos[i-1][0], STDIN_FILENO);
					close(p_hijos[i][0]);
					dup2(p_hijos[i][1], STDOUT_FILENO);
					for (k = 0; k <= j; k++)	//Cerramos el resto de pipes que no nos interesan
						if (k != i && k != i - 1){
							close (p_hijos[k][0]);
							close (p_hijos[k][1]);
						}
					execvp(linea -> commands[i].argv[0], linea -> commands[i].argv);
					printf("ERROR: El mandato %s no existe\n" ,linea -> commands[i].argv[0]); //En caso de error
					exit(1);
				}
			}
		}
		pid = fork(); // Último hijo
		if (pid < 0) {
			fprintf(stderr, "ERROR: %s\n", strerror(errno));
			exit (1);
		}
		else if (pid == 0) {
			comprobacionBg(linea);
			close (p_hijos[j][1]);
			dup2(p_hijos[j][0], STDIN_FILENO);
			for (k = 0; k < j; k++){ //Cerramos el resto de pipes que no nos interesan	
				close (p_hijos[k][0]);
				close (p_hijos[k][1]);
			}
			execvp(linea -> commands[j + 1].argv[0], linea -> commands[j + 1].argv);
			printf("ERROR: El mandato %s no existe\n" ,linea -> commands[j + 1].argv[0]); //En caso de error
			exit(1);
		}
	}	//Padre
	for(k = 0; k <= j; k++){ //Cerramos todos los pipes	
		close (p_hijos[k][0]);
		close (p_hijos[k][1]);
	}
	if(linea->background ==1){  
        waitpid(pid,&status,WNOHANG);
        printf("[%d]\n",pid);
        addJob(lista_jobs, CrearJob(pid, buf));
	}else
		for (k = 0; k < linea -> ncommands; k++)// Esperamos a que acaben todos los hijos
			wait(&status);
	// Liberamos toda la memoria que hemos reservado con malloc
	for (i = 1; i <= j; i++)
		free(p_hijos[i]);
	free(p_hijos);
}

int	entrada(tline *linea){
	int	fd;

	fd = open(linea->redirect_input, O_RDONLY);
	if (fd == -1)
		fprintf(stderr, "ERROR: %s\n", strerror(errno));
	return (fd);
}

void	foreground(tline *linea, t_list	*lista_jobs){
	int num = atoi(linea->commands[0].argv[1]);
	int i = 0;
	while (lista_jobs[i].job != NULL)
		i++;
	if (i + 1 < num)
	{
		fprintf(stderr, "%s: %s\n", linea->commands[0].argv[0], strerror(errno));
		return;
	}
	pid_t pid= lista_jobs[num - 1].job->pid;
	int status;
	pid_t fg_wait = waitpid(pid, &status, 0);
	int fg_return = kill(fg_wait, SIGCONT);
	if(fg_return < 0 || fg_wait < 0)
		printf("Snap error, cannot bring process to foreground\n");
}

int	main() {
	char	buf[BSIZE];
	tline	*linea;

	int		in_error = 0; //Nos servirá de flag para evitar ejecutar un comando con una entrada no válida y que se nos quede colgado

	int 	fd_in = dup(0);
	int		fd_out = dup(1);	//Creas un file descriptor para la salida y lo duplicas, 1 para salida
	int		fd_error = dup(2);

	t_list	*lista_jobs = CrearListaJobs();	//Creamos una lista vacía para ir guardando los mandatos en bg

	signal (SIGINT, SIG_IGN);

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
			if (strcmp(linea -> commands[0].argv[0], "exit") == 0){
				freeLista(lista_jobs);
				exit (0);
			}
			else if (strcmp(linea -> commands[0].argv[0], "cd") == 0)
				cd(linea);
			else if (strcmp(linea -> commands[0].argv[0], "jobs") == 0){
				if (lista_jobs->job != NULL)
					mostrarJobs(lista_jobs);
			}
			else if (strcmp(linea -> commands[0].argv[0], "fg") == 0)
				foreground(linea, lista_jobs);
			else
				leerUno(linea, lista_jobs, buf);
		} else if (linea -> ncommands >= 2 && in_error != -1)
			variosComandos(linea, lista_jobs, buf);

		if (linea -> redirect_input)
			dup2(fd_in, 0);
		if (linea -> redirect_output)
			dup2(fd_out, 1);	//Se vuelve a reestablecer la salida a su valor por defecto, 1
		if (linea -> redirect_error)
			dup2(fd_error, 2);

		signal (SIGINT, SIG_IGN);
		prompt();
	}
}

tJob *CrearJob(int pid, char *buf){ //La funcion podria ser void si queremos modificar un job que se pasa, sino deberíamos hacer un malloc y no pasar ningun job. La memoria reservada se liberaria en una funcion de quitar jobs
    tJob	*job;
	char	*aux;
	int		i = 0;

	aux = buf;
	while (aux[i] != '&')
		i++;
	aux[strlen(buf) - (strlen(buf) - i)] = '\0';
	job = (tJob *)malloc(sizeof(tJob));
	job->pid=pid;
	job->nombre = strdup(aux);
    return job;
}

t_list	*CrearListaJobs()
{
	t_list	*list;

	list = (t_list *)malloc(sizeof(t_list));
	if (!list)
		return (0);
	list->job = NULL;
	list->next = NULL;
	return (list);
}

/*tJob *addJob(tJob *lista,int pid){  //Puntero a funcion tJob y no devuelve nada, podria ser void y modificar la lista que se pasa
    tJob *nuevoJob;
    tJob *buscador;

    nuevoJob=(tJob*)malloc(sizeof(tJob));
    nuevoJob->pid=pid;
    nuevoJob->next=NULL;
    if(lista=NULL)
        lista=nuevoJob;
    else{
        buscador = lista;
        while(buscador->next=NULL){
            buscador=buscador->next;
        }
        buscador->next =nuevoJob;
    }
}*/

void	addJob(t_list *lst, tJob *new_job)
{
	t_list	*aux;
	t_list	*new;

	new = (t_list *)malloc(sizeof(t_list));
	if (lst->job == NULL){
		lst->job = new_job;
		free(new);
	}
	else
	{
		aux = lst;
		while (aux->next != NULL)
			aux = aux->next;
		new->job = new_job;
		new->next = NULL;
		aux->next = new;
	}
}

void	mostrarJobs(t_list *lista){
    int num = 1;
	t_list *aux = lista;

    while(aux->next!=NULL){
        printf("[%d]	running		%s\n",num++, aux->job->nombre);
        aux=aux->next;
    }
	printf("[%d]	running		%s\n",num++, aux->job->nombre);
}

void	freeLista(t_list *lista_jobs){
	t_list *aux = lista_jobs;
	while (aux){
		free(lista_jobs->job);
		lista_jobs = lista_jobs->next;
		free(aux);
		aux = lista_jobs;
	}
	free (lista_jobs);
	free(aux);
}
