default:
	g++ *.cpp -std=c++17 -L lib/ -I include/ -framework CoreVideo -framework IOKit -framework Cocoa -framework GLUT -framework OpenGL lib/libraylib.a -o d
