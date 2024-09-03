default:
	g++ *.cpp -O3 -std=c++17 -L lib/ -I include/ -framework CoreVideo -framework IOKit -framework Cocoa -framework GLUT -framework OpenGL lib/libraylib.a -o d
