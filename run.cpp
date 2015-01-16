#include "myinc.h"

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
// https://www.haskell.org/haskellwiki/Sinc_function
// http://www.boost.org/boost/math/special_functions/sinc.hpp
// i have to get used to boost, i guess
float sinc(float x)
{
  if(fabs(x) <= 1e-8){
    float x2=x*x;
    return 1-x2/6+x2*x2/120;
  }
  return sin(x)/x;
}

extern "C" void r_reload(struct run_state *state)
{
  state->count = 0;

  const int N=128;
  state->img = new CImg<float>(N,N,N);
  CImg<float> &im = state->img[0]; 
  float
    hx0=float(im.width()/2),
    hy0=float(im.height()/2),
    hz0=float(im.depth()/2);
  cimg_forXYZ(im,x,y,z){
    float a=x-hx0,b=y-hy0,c=z-hz0;
    im(x,y,z) = sinc(2*M_PI*.12*sqrt(a*a+b*b+c*c));
  }
  
  im.shift(im.width()/2,im.height()/2,im.depth()/2,0,2);
  CImgList<float> F = im.get_FFT(); 

  {
    cimglist_apply(F,shift)(im.width()/2,im.height()/2,im.depth()/2,0,2);
    
    im.assign(F[0]); // now im contains a spherical shell

    int gap = int(5/128.0*N);
    im.get_shared_slices(hz0-gap,hz0+gap).fill(0.0f); // delete the very high angles
  }
  
  if(1){
    im.shift(im.width()/2,im.height()/2,im.depth()/2,0,2); // prepare inverse fft
    F = im.get_FFT(true);
    cimglist_apply(F,shift)(im.width()/2,im.height()/2,im.depth()/2,0,2); // center asf
    //im.assign(F[0]); // this is the 4pi asf
    
    im.assign(F[0].get_pow(2) + F[1].get_pow(2)); // this is the 4pi psf (of the intensity in the focal plane)
    im.assign(im.get_pow(2)); // this is the 4pi psf (on the camera, assuming lambda_ex = lambda_em)
    
    //  im.assign(im.abs().mul(im.abs())); // calculate psf
  }

  if(1){
    F = im.get_FFT();
    cimglist_apply(F,shift)(im.width()/2,im.height()/2,im.depth()/2,0,2); // center asf
    im.assign(((F[0].get_pow(2) + F[1].get_pow(2)).sqrt() + .0001).pow(.000001)); // this is the 4pi otf
  }
  
  im.normalize(0,255);

  if(0)
    cimg_forZ(im,z){
      const unsigned char white[] = {255,255,255};
      char s[100];
      snprintf(s,100,"z=%d",z);
      im.get_shared_slice(z).draw_text(20,40,s,white);
    }
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

  CImg<float> &im = state->img[0];   
  static int z=float(im.depth()/2);
  if(state->disp->key()==cimg::keyN)
    z++;
  if(state->disp->key()==cimg::keyP)
    z--;
  state->disp->set_key(); // flush all key events

  
  CImg<unsigned char> a(512,512,1,3);
  a.fill(32);
  const unsigned char white[] = {255,255,255};
  char s[100];
  state->count++;
  int y = z%state->img[0].height();
  snprintf(s,100,"%d %d",y,state->count%state->img[0].depth());
  a.draw_text(80,80,s,white);

  //state->img[0].get_slice(z%state->img[0].depth()).draw_text(4,90,s,white).display(state->disp[0]);


  //(state->img[0]<'y')[y].display(state->disp[0]);

  CImg<float> xz(im.width(),im.depth());
  cimg_forXY(xz,x,z){
    xz(x,z) = im(x,y,z);
  }
  
  xz.draw_text(4,90,s,white).display(state->disp[0]);
  //state->img[0].get_crop(0,z,0,0,
  //			 state->img[0].width(),z,state->img[0].depth(),0).normalize(0,255).draw_text(4,90,s,white).display(state->disp[0]);
  state->disp->wait();
  state->disp->wait(32);

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


