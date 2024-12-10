all: compile link

compile:
	g++ -c main.cpp -ID:\Code\C++\Lib\SFML-2.6.1\include -DSFML_STATIC

link:
	g++ main.o -o main -LD:\Code\C++\Lib\SFML-2.6.1\lib -lsfml-graphics-s -lsfml-window-s -lsfml-system-s -lsfml-main -lopengl32 -lfreetype -lwinmm -lgdi32

clean:
	del -f main *.o *.exe