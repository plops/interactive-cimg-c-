#include "/home/martin/src/CImg/CImg.h" // in order to use as precompiled header this
		  // inclusion must come before the first line of c
#undef None
#undef Status
#include "api.h"

#include <iostream>

#include <stdio.h>
#include <malloc.h>
#include <signal.h>
#include <stdlib.h>


using namespace std;
using namespace cimg_library;


// this is for setenv, only works in c99
#define e(q) do{if(0!=(q)) printf("error in %s:%d",__func__,__LINE__);}while(0)

// this is to optionally comment out code
#define f(e) do{if(0)(e);}while(0)


