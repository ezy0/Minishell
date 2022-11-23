
typedef struct {
	char * filename;
	int argc;
	char ** argv;
} tcommand;

typedef struct {
	int ncommands;	//Numero de comandos
	tcommand * commands;	//Puntero de comandos
	char * redirect_input;	//Redir entrada
	char * redirect_output;	//Redir de salida
	char * redirect_error;	//Redir error
	int background;	//Indicador segundo plano
} tline;

extern tline * tokenize(char *str);
