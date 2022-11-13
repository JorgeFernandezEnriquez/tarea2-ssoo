all: tarea
tarea: 
	g++ tarea.cpp -o tarea -w
clean:
	rm -f tarea