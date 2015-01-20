#include "myinc.h"
#include "run.h"
#include <complex>
#include <sys/mman.h>
#include <fcntl.h>

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

void load_img(unsigned char*from, int w, int h, float*into)
{
  int i,j;
  // convert mono12p into real part of complex double float
  // i .. index for byte
  // j .. index for 12bit
  for(i=0,j=0;j< w*h;i+=3,j+=2) {
    unsigned char 
      ab = from[i],
      c = from[i+1] & 0x0f,
      d = (from[i+1] & 0xf0)>>4,
      ef = from[i+2];
    into[2*((j%w)+ w * (j/w))] = 1.0f*((ab<<4)+d);
    into[2*(((j+1)%w)+ w * ((j+1)/w))] = 1.0f*((ef<<4)+c);
  }

}


extern "C" void r_reload(struct run_state *state)
{
  state->count = 0;
  
  const int W=280, H=280, NW=79, NH=62; 
  const long long unsigned int len = W*H*NW*NH/8*12;
  state->img = new CImg<float>(W,H,NW,NH);
  //CImg<float> &im = state->img; 
  int fd = open("/media/sdd3/b/cam0",O_RDONLY);
  unsigned char*from = (unsigned char*)mmap(0,len,PROT_READ,MAP_SHARED,fd,0);
  if(from==MAP_FAILED)
    cout << "error mapping file" << endl;
  for(int i=0;i<NW;i++)
    for(int j=0;j<NH;j++)
      load_img(from,W,H,state->img->data());
  
  munmap(from,len);
  close(fd);
  
  //im[0].display(state->disp[0]);
  state->img->select(state->disp[0]);

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

  //CImgList<float> &im = state->img[0];   

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


