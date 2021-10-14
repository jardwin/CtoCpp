all:
	# -I specify the include folders
	# -L specify where to load the library
	g++ -ISDL2/include -ISDL2_image/include -LSDL2/lib -LSDL2_image/lib -o ProjetEpitaSDL *.cpp -lSDL2main -lSDL2 -lSDL2_image
