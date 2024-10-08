CFLAGS = -std=c++17
LDFLAGS = -lglfw -lvulkan -ldl -lpthread -lX11 -lXxf86vm -lXrandr -lXi

main: src/main.cpp
	clang++ $(CFLAGS) -o main src/*.cpp src/*/*.cpp $(LDFLAGS)

clean:
	rm -rf main
