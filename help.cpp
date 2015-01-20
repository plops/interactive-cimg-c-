// the following code is my first attempts with wigner distributions

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

inline float over_wc2(float lc,float w0)
{
  //  return 1/(w0*w0)+1/(lc*lc); 
  float lc2=lc*lc, w02=w0*w0;
  return (w02+lc2)/(lc2*w02);
}

inline float wc(float lc,float w0)
{
  // return sqrt(1/(1/(w0*w0)+1/(lc*lc)));
  float lc2=lc*lc, w02=w0*w0;
  return sqrt(lc2*w02/(w02+lc2));
}

inline float beam_waist(float lambda,float lc,float w0, float z)
{
  return w0*sqrt(1+pow((lambda*z)/(M_PI*w0),2)*over_wc2(lc,w0));
}

inline float curvature(float lambda, float lc, float w0, float z)
{
  return z*(1+pow(M_PI*w0*wc(lc,w0)/(lambda*z),2));
}


// 1982 friberg prop param gaussian shell beam
complex<float> gaussian_shell2(float lambda, float lc, float r1, float r2, float w0, float z)
{
  float r1pr22=pow(r1+r2,2), r1mr22=pow(r1-r2,2), wz=beam_waist(lambda,lc,w0,z),
    wz2=wz*wz, lc2=lc*lc, w02=w0*w0, k=2*M_PI/lambda;
  float
    a = w02/wz2,
    b = exp(-r1pr22/(2*wz2)),
    c = exp(-w02*r1mr22/(2*lc2*wz2));
  std::complex<float> im(0,1);
  std::complex<float> res = a*b*c;
  if(abs(z)<1e-8)
    return res;
  return res*exp(-im*k*r1mr22/(2*curvature(lambda,lc,w0,z)));
}


extern "C" void r_reload(struct run_state *state)
{
  state->count = 0;
  
  const int N=512;
  state->img = new CImgList<float>(2,N,N);
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





// the following code is for simulating psf of 4pi microscopes

  // if(0){
  //   const int N=64;
  //   state->img = new CImg<float>(N,N,N);
  //   CImg<float> &im = state->img[0]; 
  //   float
  //     hx0=float(im.width()/2),
  //     hy0=float(im.height()/2),
  //     hz0=float(im.depth()/2);

  //   if(0){
  //     cimg_forXYZ(im,x,y,z){
  // 	float a=x-hx0,b=y-hy0,c=z-hz0;
  // 	im(x,y,z) = sinc(2*M_PI*.12*sqrt(a*a+b*b+c*c));
  //     }
  //   }
    
  
  //   if(0) {
  //     im.shift(im.width()/2,im.height()/2,im.depth()/2,0,2);
  //     CImgList<float> F = im.get_FFT(); 

  //     {
  // 	cimglist_apply(F,shift)(im.width()/2,im.height()/2,im.depth()/2,0,2);
	
  // 	im.assign(F[0]); // now im contains a spherical shell
	
  // 	int gap = int(5/128.0*N);
  // 	im.get_shared_slices(hz0-gap,hz0+gap).fill(0.0f); // delete the very high angles
  //     }
      
  //     if(1){
  // 	im.shift(im.width()/2,im.height()/2,im.depth()/2,0,2); // prepare inverse fft
  // 	F = im.get_FFT(true);
  // 	cimglist_apply(F,shift)(im.width()/2,im.height()/2,im.depth()/2,0,2); // center asf
  // 	//im.assign(F[0]); // this is the 4pi asf
      
  // 	im.assign(F[0].get_pow(2) + F[1].get_pow(2)); // this is the 4pi psf (of the intensity in the focal plane)
  // 	//im.assign(im.get_pow(2)); // this is the 4pi psf (on the camera, assuming lambda_ex = lambda_em)
	
  // 	//  im.assign(im.abs().mul(im.abs())); // calculate psf
  //     }
      
  //     if(1){
  // 	F = im.get_FFT();
  // 	cimglist_apply(F,shift)(im.width()/2,im.height()/2,im.depth()/2,0,2); // center asf
  // 	im.assign(((F[0].get_pow(2) + F[1].get_pow(2)).sqrt() + .0001).pow(.000001)); // this is the 4pi otf
  //     }
  //   }
  // }
  
  //   im.normalize(0,255);

  // if(0)
  //   cimg_forZ(im,z){
  //     const unsigned char white[] = {255,255,255};
  //     char s[100];
  //     snprintf(s,100,"z=%d",z);
  //     im.get_shared_slice(z).draw_text(20,40,s,white);
  //   }
