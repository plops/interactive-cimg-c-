CFLAGS=-ggdb -Wall -Wextra -O0 -march=native -pedantic  -fPIC -D_BSD_SOURCE -std=c99
CXXFLAGS= -ggdb -fuse-cxa-atexit -O2 -march=native -fPIC -Dcimg_use_fftw3 

LDLIBS_MAIN=-ldl
LDFLAGS=
LDLIBS=

all: main librun-help.so

main: main.c api.h
	$(CC) $(CFLAGS) -o $@ $< $(LDLIBS_MAIN)

%.h.gch: %.h
	$(CXX) $(CXXFLAGS) -shared  $(LDFLAGS)  $< $(LDLIBS) -lX11 -lfftw3_threads -lfftw3 -lfftw3f -lfftw3f_threads

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<


librun-help.so: api.h myinc.h.gch run.o 
	$(CXX) $(CXXFLAGS) -shared  $(LDFLAGS) -o o.so run.o $(LDLIBS) -lX11 -lfftw3_threads -lfftw3 -lfftw3f -lfftw3f_threads
	if [ -a librun2.so ]; then \
	rm librun2.so; \
	mv o.so librun1.so; \
	killall -s SIGUSR1 main; \
	else \
	touch librun1.so; \
	rm librun1.so; \
	mv o.so librun2.so; \
	killall -s SIGUSR2 main; \
	fi;
	touch librun-help.so


clean:
	rm main run.o librun1.so myinc.h.gch librun2.so librun-help.so

test : main librun-help.so
	./$<
