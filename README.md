# MiniShell

## Descripción

**MiniShell** es una práctica que consiste en implementar un intérprete de comandos básico, capaz de ejecutar comandos en Linux mediante pipes, redirecciones y ejecución en segundo plano (background). Este proyecto simula el comportamiento de una shell y utiliza la librería `libparser` para analizar las líneas de comandos.

---
## Funcionalidad principal
1. **Comandos y secuencias**:
   - Ejecuta uno o varios comandos separados por `|` (pipes).
   - Soporta redirecciones:
     - Entrada (`< fichero`): Solo para el primer comando.
     - Salida (`> fichero`): Solo para el último comando.
     - Error (`>& fichero`): Solo para el último comando.
   - Permite ejecución en **background** si la línea termina con `&`.

2. **Interacción con el usuario**:
   - Muestra un prompt para recibir comandos.
   - Lee, analiza y ejecuta los comandos.
   - Maneja errores:
     - Comando inexistente
     - Error en redirección
   - Los procesos en background y la minishell no terminan al recibir la señal `SIGINT` (Ctrl-C), pero los procesos en foreground sí lo hacen.

3. **Manejo de procesos**:
   - Crea procesos hijo para cada comando en una secuencia.
   - Utiliza tuberías para comunicar los procesos.
   - Espera la finalización de los procesos en foreground antes de mostrar el prompt.

## Objetivos parciales
1. **Comandos básicos**:
   - Ejecutar un único comando con 0 o más argumentos.
   - Redirecciones de entrada y salida.
2. **Pipes**:
   - Ejecutar secuencias de 2 o más comandos conectados con `|`.
   - Soporte para redirecciones combinadas con pipes.
3. **Comandos internos**:
   - `cd`: Cambia el directorio de trabajo.
     - Soporta rutas absolutas y relativas.
     - Accede al directorio de la variable `HOME` si no se proporcionan argumentos.
   - `exit`: Finaliza la minishell.
   - `jobs`: Lista procesos en segundo plano.
   - `fg`: Reanuda la ejecución de un proceso en background.
   - `umask`: Cambia los permisos predeterminados de creación de ficheros (acepta números octales).

4. **Ejecución en segundo plano**:
   - Ejecuta comandos en background y muestra el `PID` del proceso en ejecución.
   - Usa la flag `WNOHANG` con `waitpid` para no bloquear el prompt.

5. **Manejo de señales**:
   - `SIGINT`:
     - La minishell y procesos en background no terminan.
     - Los procesos en foreground responden con la acción por defecto.

---

## Compilación

Para compilar el programa, utiliza el siguiente comando:

```bash
gcc -Wall -Wextra myshell.c libparser.a -o myshell
```

## Autores
- https://github.com/ezy0
- https://github.com/a-martinma
 
