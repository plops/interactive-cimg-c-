#include "myinc.h"

struct run_state * global_state;

const int w=512,h=512;

extern "C" void signalHandler(int a)
{
  // clean up in case i press Ctrl+c 
  r_finalize(global_state);
}

  
extern "C" struct run_state * r_init()
{
  struct run_state *state = (run_state*)malloc(sizeof(*state));
  state->count = 0;  
  
  global_state = state;
  signal(SIGTERM, signalHandler);
  
  return state;
}

extern "C" void r_reload(struct run_state *state)
{
  state->count = 0;

  state->img = new CImg<unsigned char>(512,512,1,3);
  state->img->fill(32).noise(128).blur(8);
  const unsigned char white[] = {255,255,255};
  state->img->draw_text(80,80,"Hello World",white);
  
  state->disp=new CImgDisplay(512,512,"Hello");
  state->img->display(state->disp[0]);
}

// assign forces deallocation
// CImgDisplay has its own pixel buffer

extern "C" int r_step(struct run_state *state)
{
  if(state->disp->is_closed())
    return 0;
  if(state->disp->is_resized())
    state->disp->resize();

  //CImg<unsigned char> a(512,512,1,3);
  //a.fill(32).noise(128).blur(8);
  const unsigned char white[] = {255,255,255};
  char s[100];
  snprintf(s,100,"hello %d",state->count);
  state->img->draw_text(80,80,s,white);

  state->count++;

  state->img->display(state->disp[0]);
  state->disp->wait(20);
  //  CImg<float>=

  //CImgList<float> F = img.get_FFT();
  //cimglist_apply(F,shift)(img.width()/2,img.height()/2,0,0,2);
  // //	cout << "min " << ((F[0].get_pow(2) + F[1].get_pow(2)).sqrt() + 1).log().min()
  // //     << " max "  << (((F[0].get_pow(2) + F[1].get_pow(2)).sqrt() + 1).log()*-1).min()*-1 << endl;
  //CImg<float> fmag = ((F[0].get_pow(2) + F[1].get_pow(2)).sqrt() + 1).blur_median(3).log().normalize(0,255);
  
  //cimg_rof(fmag,p,float) {
  //  const float m=8.3f, M=14.0f;
  //  float v = (float) 255.0f*(*p-m)/(M-m);
  //  *p = (v<0.0f)?0.0f:(v>255.0)?255.0:v;
  //};
  
  return 1; 
}


extern "C" void r_unload(struct run_state *state)
{
  delete state->disp;
  delete state->img;
}

extern "C" void r_finalize(struct run_state *state)
{
  r_unload(state);
  
  free(state);
}



const struct run_api RUN_API = {
  r_init,
  r_finalize,
  r_reload,
  r_unload,
  r_step
};


