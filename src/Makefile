LDFLAGS = -lsfml-graphics -lsfml-window -lsfml-system
CFLAGS = -Wall -g -o2

OBJ = ClothSim.o

ClothSim: $(OBJ)
	g++ $(CFLAGS) -o ClothSim $(OBJ) $(LDFLAGS)

ClothSim.o: Main_Verlet.cpp
	g++ $(CFLAGS) -c -o ClothSim.o Main_Verlet.cpp
