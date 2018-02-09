# 3350 lab1
# to compile your project, type make and press enter

all: lab1

lab1: lab1.cpp 
	g++ lab1.cpp libggfonts.a log.h -Wall -olab1 -lrt -pthread -lX11 -lGL -lGLU -lm  

clean:
	rm -f lab1

