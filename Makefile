
all: main
main: main.cpp 
	g++ -o main main.cpp `pkg-config --cflags opencv` `pkg-config --libs opencv` -lv4l2
