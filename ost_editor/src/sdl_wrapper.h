#ifndef SDL_WRAPPER_H
#define SDL_WRAPPER_H

#include <functional>
#include <SDL.h>

#define SDL_EV_QUIT     10000

void sdl_wrapper_get_window_size(int *w, int *h);
int sdl_wrapper_init();
int sdl_wrapper_process(std::function<void (SDL_Renderer *, double)> sdl_draw_callback);
void sdl_wrapper_close();

#endif // SDL_WRAPPER_H
