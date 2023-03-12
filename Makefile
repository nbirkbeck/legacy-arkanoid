.SUFFIXES: .cc. o

all: arkanoid ${OBJS}

OBJS=arkanoid.o ball.o  brick.o  compat.o  console.o  font.o  globals.o  list.o  paddle.o  pup.o dummy_sound.o
FLAGS=-g


%.o: %.cc
	g++ ${FLAGS} -c -o $@ $<


clean:
	rm -fv arkanoid *.o

arkanoid: ${OBJS}
	g++ ${FLAGS} -o $@ ${OBJS} -lSDL2

arkanoid.html: Makefile arkanoid.cc ball.cc  brick.cc  compat.cc  console.cc  font.cc  globals.cc  list.cc  paddle.cc  pup.cc
	emcc -O2 -o arkanoid.html -sASYNCIFY -sSDL2_IMAGE_FORMATS='["bmp"]' -s USE_SDL_IMAGE=2 -sSDL2_MIXER_FORMATS='["wav"]' -sUSE_SDL=2 --embed-file fonts --embed-file sounds --embed-file textures  --embed-file levels arkanoid.cc ball.cc  brick.cc  compat.cc  console.cc  font.cc  globals.cc  list.cc  paddle.cc  pup.cc dummy_sound.cc

