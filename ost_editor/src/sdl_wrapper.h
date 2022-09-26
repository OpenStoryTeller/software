#ifndef SDL_WRAPPER_H
#define SDL_WRAPPER_H

#include <functional>
#include <SDL.h>
#include "imgui.h"

#define SDL_EV_QUIT     10000

class SdlWrapper
{
public:

    virtual int Update(SDL_Renderer *renderer, double deltaTime) = 0;

    void GetWindowSize(int *w, int *h);
    int Initialize();
    int Process();
    void Close();

private:

    const int FPS = 60;
    const int frameDelay = 1000 / FPS;
    Uint32 frameStart;
    int frameTime;

    uint32_t mWidth = 1152;
    uint32_t mHeight = 648;

    const uint32_t mMinimumWidth = 1152;
    const uint32_t mMinimumHeight = 648;

    Uint64 currentTick = 0;
    Uint64 lastTick = 0;
    double deltaTime = 0;

    ImFont* mNormalFont = nullptr;
    ImFont* mBigFont = nullptr;

    SDL_Window *mWindow = nullptr;
    SDL_Renderer *mRenderer = nullptr;

};


#endif // SDL_WRAPPER_H
