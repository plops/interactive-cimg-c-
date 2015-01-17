extern "C" struct run_state{
  int count;
  CImgDisplay *disp;
  CImgList<float> *img;
};

extern "C" void r_finalize(struct run_state *state);
extern "C" void r_reload(struct run_state *state);
extern "C" void r_unload(struct run_state *state);
extern "C" int r_step(struct run_state *state);

