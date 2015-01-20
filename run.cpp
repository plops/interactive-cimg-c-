#include "myinc.h"
#include "run.h"
#include <complex>

// _get_select is the function printing stuff about a pixel under the cursor in CImgDisplay

struct run_state * global_state;

const int w=512,h=512;

extern "C" void signalHandler(int a)
{
  // clean up in case i press Ctrl+c 
  r_finalize(global_state);
}

//int fftw_init_threads(void);
//void fftw_plan_with_nthreads(int nthreads);

  
extern "C" struct run_state * r_init()
{

  fftw_init_threads();
  int n=cimg::nb_cpus();
  fftw_plan_with_nthreads(n);
  std::cout << "initialized fftw for " << n << " cpus."<< std::endl;
  struct run_state *state = (run_state*)malloc(sizeof(*state));
  state->count = 0;  
  
  global_state = state;
  signal(SIGTERM, signalHandler);

  state->disp=new CImgDisplay(512,512,"Hello");
  return state;
}

extern "C" void r_reload(struct run_state *state)
{
  state->count = 0;
  
  const int W=280, H=280;
  state->img = new CImgList<float>(2,W,H);
  CImgList<float> &im = state->img[0]; 
  const float lambda = .5, lc = 2000, w0 = 2000, z = 0.1;
  complex<float> transmission[N];
  std::complex<float> imag(0,1);
  float pi = atan2f(0,-1);
  for(int i;i<N;i++)
    transmission[i] = exp(imag*pi*(((i%16)>7)?1.0f:0.0f));
  cimg_forXY(im[0],x,y){
    float xx = 32000*((x-N/2)*1.0f/N), yy = 32000*((y-N/2)*1.0f/N),wx = .5*(xx+yy),wxprime=xx-yy;
    complex<float> val=gaussian_shell2(lambda,lc,wx,wxprime,w0,z);
    //val *= transmission[x]*conj(transmission[y]);
    im[0](x,y) = val.real();
    im[1](x,y) = 0.0f; // val.imag();
  }
  
  cimglist_apply(im,shift)(0,im[0].height()/2,0,0,2);
  CImgList<float> F = im.get_FFT('y');

  cimglist_apply(F,shift)(0,im[0].height()/2,0,0,2);
  
  im.assign(F);
  

  //im[0].display(state->disp[0]);
  im[1].select(state->disp[0]);

  // selecting rectangle with mouse moves in, clicking again moves out again
  // C-left C-right moves around
  // C-pageup or down
  // C-a assign
  // C-d zoom 2x
  // C-c zoom .5x
  // C-r reset zoom 
  // C-f fullscreen (i don't know how to get rid of it)
  // C-v ?
  // C-s save BMP screenshot
  // C-0 save cimgz screenshot
  // mouse wheel or C-left right moves through slices of volume
  // right button zoom
  // middle button shift
}


// assign forces deallocation
// CImgDisplay has its own pixel buffer

extern "C" int r_step(struct run_state *state)
{
  if(state->disp->is_closed())
    return 0;
  if(state->disp->is_resized())
    state->disp->resize();

  CImgList<float> &im = state->img[0];   

  static int z=0; //float(im.depth()/2);
  if(state->disp->key()==cimg::keyN)
    z++;
  if(state->disp->key()==cimg::keyP)
    z--;
  state->disp->set_key(); // flush all key events
  

  //im.display(state->disp[0]);
  
  // CImg<float> xz(im.width(),im.depth());
  // cimg_forXY(xz,x,z){
  //   xz(x,z) = im(x,y,z);
  // }
  
  // xz.draw_text(4,90,s,white).display(state->disp[0]);
  // //			 state->img[0].width(),z,state->img[0].depth(),0).normalize(0,255).draw_text(4,90,s,white).display(state->disp[0]);
  

  state->disp->wait();
  state->disp->wait(32);
  
  return 1; 
}


extern "C" void r_unload(struct run_state *state)
{
  
  delete state->img;
}

extern "C" void r_finalize(struct run_state *state)
{
  r_unload(state);
  delete state->disp;
  free(state);
}



const struct run_api RUN_API = {
  r_init,
  r_finalize,
  r_reload,
  r_unload,
  r_step
};


