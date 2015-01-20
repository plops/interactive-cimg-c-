CFLAGS=-ggdb -Wall -Wextra -O2 -march=native -pedantic  -fPIC -D_BSD_SOURCE -std=c99
CXXFLAGS= -ggdb -O2 -march=native -fPIC -Dcimg_use_fftw3 -fopenmp

# -fuse-cxa-atexit

LDLIBS_MAIN=-ldl
LDFLAGS=
LDLIBS=

all: main librun1.so

main: main.c api.h
	$(CC) $(CFLAGS) -o $@ $< $(LDLIBS_MAIN)

%.h.gch: %.h
	$(CXX) $(CXXFLAGS)  -x c++-header -c $(LDFLAGS)  $< $(LDLIBS) -lX11 -lfftw3_threads -lfftw3 -lfftw3f -lfftw3f_threads

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<


librun-help.so: api.h myinc.h.gch run.o 
	$(CXX) $(CXXFLAGS) -shared  $(LDFLAGS) -o o.so run.o $(LDLIBS) -lX11 -lfftw3_threads -lfftw3 -lfftw3f -lfftw3f_threads
	touch librun-help.so
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

librun1.so: api.h myinc.h.gch run.o 
	$(CXX) $(CXXFLAGS) -shared  $(LDFLAGS) -o librun1.so run.o $(LDLIBS) -lX11 -lfftw3_threads -lfftw3 -lfftw3f -lfftw3f_threads
	killall -s SIGUSR1 main


clean:
	rm main run.o librun1.so myinc.h.gch librun2.so librun-help.so

test : main librun-help.so
	./$<
