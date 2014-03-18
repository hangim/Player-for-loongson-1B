mipsel-linux-gcc ./src/musicList.c -c -o ./src/musicList.o
mipsel-linux-gcc ./src/control.c -c -o ./src/control.o
mipsel-linux-gcc -I./sdllib/include ./src/player.c -c  -o ./src/player.o

mipsel-linux-gcc -I./sdllib/include -L./sdllib/lib -lSDL -lSDL_ttf -lts -lfreetype -lz ./src/musicList.o ./src/control.o ./src/player.o -o ./bin/player-mipsel
rm ./src/*.o
