#include "parser.h"

int BUFFER_SIZE = 1024;

int main() {
	char buf[BUFFER_SIZE];
	tline * line; //QUE ES ESTO LOL

	while (fgets(buf, BUFFER_SIZE, stdin > 0))
	{
		line = tokenize(buf) //Leemos linea del teclado
		if (line == NULL)
			continue;
		execline(line, buf);
	}
}