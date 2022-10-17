
#include "application.h"
#include <SDL.h>
#include "imgui.h"
#include "imgui_impl_sdl.h"
#include "imgui_impl_sdlrenderer.h"
#include "imgui_internal.h"
#include "IconsFontAwesome5.h"
#include <functional>


#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

void Application::GetWindowSize(int *w, int *h)
{
    SDL_GetWindowSize(mWindow, w, h);
}

SDL_Texture *Application::LoadTexture(const char* path)
{
    // Read data
    int32_t width, height, bytesPerPixel;
    void* data = stbi_load(path, &width, &height, &bytesPerPixel, 0);

    // Calculate pitch
    int pitch;
    pitch = width * bytesPerPixel;
    pitch = (pitch + 3) & ~3;

    // Setup relevance bitmask
    int32_t Rmask, Gmask, Bmask, Amask;
#if SDL_BYTEORDER == SDL_LIL_ENDIAN
    Rmask = 0x000000FF;
    Gmask = 0x0000FF00;
    Bmask = 0x00FF0000;
    Amask = (bytesPerPixel == 4) ? 0xFF000000 : 0;
#else
    int s = (bytesPerPixel == 4) ? 0 : 8;
    Rmask = 0xFF000000 >> s;
    Gmask = 0x00FF0000 >> s;
    Bmask = 0x0000FF00 >> s;
    Amask = 0x000000FF >> s;
#endif
    SDL_Surface* surface = SDL_CreateRGBSurfaceFrom(data, width, height, bytesPerPixel*8, pitch, Rmask, Gmask, Bmask, Amask);
    SDL_Texture* t = nullptr;
    if (surface)
    {
        t = SDL_CreateTextureFromSurface(mRenderer, surface);
    }
    else
    {
        t = SDL_CreateTexture(mRenderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, 20, 20);
    }

    STBI_FREE(data);
    SDL_FreeSurface(surface);
    return t;
}

void Application::DestroyTexture(SDL_Texture *texture)
{
    SDL_DestroyTexture(texture);
}


int Application::Initialize()
{
    // initiate SDL
     if (SDL_Init(SDL_INIT_TIMER | SDL_INIT_AUDIO | SDL_INIT_VIDEO | SDL_INIT_EVENTS |
                  SDL_INIT_JOYSTICK | SDL_INIT_HAPTIC | SDL_INIT_GAMECONTROLLER) != 0)
     {
         printf("[ERROR] %s\n", SDL_GetError());
         return -1;
     }

     SDL_WindowFlags window_flags = (SDL_WindowFlags)(
         SDL_WINDOW_OPENGL
         | SDL_WINDOW_RESIZABLE
         | SDL_WINDOW_ALLOW_HIGHDPI
         );
     mWindow = SDL_CreateWindow(
         "OST Editor",
         SDL_WINDOWPOS_CENTERED,
         SDL_WINDOWPOS_CENTERED,
         mWidth,
         mHeight,
         window_flags
         );
     // limit to which minimum size user can resize the window
     SDL_SetWindowMinimumSize(mWindow, mMinimumWidth, mMinimumHeight);

     // setup Dear ImGui context
     IMGUI_CHECKVERSION();
     ImGui::CreateContext();
     ImGuiIO& io = ImGui::GetIO();

     io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
     io.WantCaptureMouse = true;
     io.WantCaptureKeyboard = true;
     io.IniFilename = "ost_editor.ini";
     mNormalFont = io.Fonts->AddFontFromFileTTF( "assets/fonts/roboto.ttf", 20);

     ImFontConfig config;
     config.MergeMode = true; // ATTENTION, MERGE AVEC LA FONT PRECEDENTE !!
 //    config.GlyphMinAdvanceX = 20.0f; // Use if you want to make the icon monospaced
 //    config.GlyphOffset.y += 1.0;
     static const ImWchar icon_ranges[] = { ICON_MIN_FA, ICON_MAX_FA, 0 };
     io.Fonts->AddFontFromFileTTF("assets/fonts/fa-solid-900.ttf", 16.0f, &config, icon_ranges);

     mBigFont = io.Fonts->AddFontFromFileTTF( "assets/fonts/roboto.ttf", 60);

     io.Fonts->Build();

     ImGuiStyle& style = ImGui::GetStyle();

     ImVec4* colors = style.Colors;

     style.WindowRounding    = 2.0f;             // Radius of window corners rounding. Set to 0.0f to have rectangular windows
     style.ScrollbarRounding = 3.0f;             // Radius of grab corners rounding for scrollbar
     style.GrabRounding      = 2.0f;             // Radius of grabs corners rounding. Set to 0.0f to have rectangular slider grabs.
     style.AntiAliasedLines  = true;
     style.AntiAliasedFill   = true;
     style.WindowRounding    = 2;
     style.ChildRounding     = 2;
     style.ScrollbarSize     = 16;
     style.ScrollbarRounding = 3;
     style.GrabRounding      = 2;
     style.ItemSpacing.x     = 10;
     style.ItemSpacing.y     = 4;
     style.IndentSpacing     = 22;
     style.FramePadding.x    = 6;
     style.FramePadding.y    = 4;
     style.Alpha             = 1.0f;
     style.FrameRounding     = 3.0f;

     colors[ImGuiCol_Text]                   = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
     colors[ImGuiCol_TextDisabled]          = ImVec4(0.60f, 0.60f, 0.60f, 1.00f);
     colors[ImGuiCol_WindowBg]              = ImVec4(0.86f, 0.86f, 0.86f, 1.00f);
     //colors[ImGuiCol_ChildWindowBg]         = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
     colors[ImGuiCol_ChildBg]                = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
     colors[ImGuiCol_PopupBg]                = ImVec4(0.93f, 0.93f, 0.93f, 0.98f);
     colors[ImGuiCol_Border]                = ImVec4(0.71f, 0.71f, 0.71f, 0.08f);
     colors[ImGuiCol_BorderShadow]          = ImVec4(0.00f, 0.00f, 0.00f, 0.04f);
     colors[ImGuiCol_FrameBg]               = ImVec4(0.71f, 0.71f, 0.71f, 0.55f);
     colors[ImGuiCol_FrameBgHovered]        = ImVec4(0.94f, 0.94f, 0.94f, 0.55f);
     colors[ImGuiCol_FrameBgActive]         = ImVec4(0.71f, 0.78f, 0.69f, 0.98f);
     colors[ImGuiCol_TitleBg]               = ImVec4(0.85f, 0.85f, 0.85f, 1.00f);
     colors[ImGuiCol_TitleBgCollapsed]      = ImVec4(0.82f, 0.78f, 0.78f, 0.51f);
     colors[ImGuiCol_TitleBgActive]         = ImVec4(0.78f, 0.78f, 0.78f, 1.00f);
     colors[ImGuiCol_MenuBarBg]             = ImVec4(0.86f, 0.86f, 0.86f, 1.00f);
     colors[ImGuiCol_ScrollbarBg]           = ImVec4(0.20f, 0.25f, 0.30f, 0.61f);
     colors[ImGuiCol_ScrollbarGrab]         = ImVec4(0.90f, 0.90f, 0.90f, 0.30f);
     colors[ImGuiCol_ScrollbarGrabHovered]  = ImVec4(0.92f, 0.92f, 0.92f, 0.78f);
     colors[ImGuiCol_ScrollbarGrabActive]   = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
     colors[ImGuiCol_CheckMark]             = ImVec4(0.184f, 0.407f, 0.193f, 1.00f);
     colors[ImGuiCol_SliderGrab]            = ImVec4(0.26f, 0.59f, 0.98f, 0.78f);
     colors[ImGuiCol_SliderGrabActive]      = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
     colors[ImGuiCol_Button]                = ImVec4(0.71f, 0.78f, 0.69f, 0.40f);
     colors[ImGuiCol_ButtonHovered]         = ImVec4(0.725f, 0.805f, 0.702f, 1.00f);
     colors[ImGuiCol_ButtonActive]          = ImVec4(0.793f, 0.900f, 0.836f, 1.00f);
     colors[ImGuiCol_Header]                = ImVec4(0.71f, 0.78f, 0.69f, 0.31f);
     colors[ImGuiCol_HeaderHovered]         = ImVec4(0.71f, 0.78f, 0.69f, 0.80f);
     colors[ImGuiCol_HeaderActive]          = ImVec4(0.71f, 0.78f, 0.69f, 1.00f);
 //     colors[ImGuiCol_Column]                = ImVec4(0.39f, 0.39f, 0.39f, 1.00f);
 //     colors[ImGuiCol_ColumnHovered]         = ImVec4(0.26f, 0.59f, 0.98f, 0.78f);
 //     colors[ImGuiCol_ColumnActive]          = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
     colors[ImGuiCol_Separator]              = ImVec4(0.39f, 0.39f, 0.39f, 1.00f);
     colors[ImGuiCol_SeparatorHovered]       = ImVec4(0.14f, 0.44f, 0.80f, 0.78f);
     colors[ImGuiCol_SeparatorActive]        = ImVec4(0.14f, 0.44f, 0.80f, 1.00f);
     colors[ImGuiCol_ResizeGrip]            = ImVec4(1.00f, 1.00f, 1.00f, 0.00f);
     colors[ImGuiCol_ResizeGripHovered]     = ImVec4(0.26f, 0.59f, 0.98f, 0.45f);
     colors[ImGuiCol_ResizeGripActive]      = ImVec4(0.26f, 0.59f, 0.98f, 0.78f);
     colors[ImGuiCol_PlotLines]             = ImVec4(0.39f, 0.39f, 0.39f, 1.00f);
     colors[ImGuiCol_PlotLinesHovered]      = ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
     colors[ImGuiCol_PlotHistogram]         = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
     colors[ImGuiCol_PlotHistogramHovered]  = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
     colors[ImGuiCol_TextSelectedBg]        = ImVec4(0.26f, 0.59f, 0.98f, 0.35f);
 //     colors[ImGuiCol_ModalWindowDarkening]  = ImVec4(0.20f, 0.20f, 0.20f, 0.35f);
     colors[ImGuiCol_DragDropTarget]         = ImVec4(0.26f, 0.59f, 0.98f, 0.95f);
     colors[ImGuiCol_NavHighlight]           = colors[ImGuiCol_HeaderHovered];
     colors[ImGuiCol_NavWindowingHighlight]  = ImVec4(0.70f, 0.70f, 0.70f, 0.70f);

     // Setup renderer
     mRenderer =  SDL_CreateRenderer( mWindow, -1, SDL_RENDERER_ACCELERATED);

     // setup platform/renderer bindings
     ImGui_ImplSDL2_InitForSDLRenderer(mWindow, mRenderer);
     ImGui_ImplSDLRenderer_Init(mRenderer);

     return mRenderer != nullptr;
}

int Application::Process()
{
    // Stores the number of ticks at the start of the loop
    frameStart = SDL_GetTicks();

    SDL_RenderClear(mRenderer);

    static SDL_Event event;

    while (SDL_PollEvent(&event))
    {
        // without it you won't have keyboard input and other things
        ImGui_ImplSDL2_ProcessEvent(&event);
        // you might also want to check io.WantCaptureMouse and io.WantCaptureKeyboard
        // before processing events

        switch (event.type)
        {
        case SDL_QUIT:
            break;

        case SDL_WINDOWEVENT:
            switch (event.window.event)
            {
            case SDL_WINDOWEVENT_CLOSE:
                return SDL_EV_QUIT;
                break;
            case SDL_WINDOWEVENT_RESIZED:
                mWidth = event.window.data1;
                mHeight = event.window.data2;
                // std::cout << "[INFO] Window size: "
                //           << windowWidth
                //           << "x"
                //           << windowHeight
                //           << std::endl;
                break;
            }
            break;
        }
    }

    // start the Dear ImGui frame
    ImGui_ImplSDLRenderer_NewFrame();
    ImGui_ImplSDL2_NewFrame(mWindow);
    ImGui::NewFrame();

    lastTick = currentTick;
    currentTick = SDL_GetPerformanceCounter();
    deltaTime = (double)((currentTick - lastTick)*1000 / (double)SDL_GetPerformanceFrequency() );


    Update(mRenderer, deltaTime);

    // rendering
    ImGui::Render();
    ImGui_ImplSDLRenderer_RenderDrawData(ImGui::GetDrawData());
    SDL_RenderPresent(mRenderer);

    // This measures how long this iteration of the loop took
    frameTime = SDL_GetTicks() - frameStart;

    // This keeps us from displaying more frames than 60/Second
    if(frameDelay > frameTime)
    {
        SDL_Delay(frameDelay - frameTime);
//        SDL_WaitEventTimeout(&event, 1);
    }

    return 0;
}

void Application::Close()
{
    ImGui_ImplSDLRenderer_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();

    SDL_DestroyRenderer(mRenderer);
    SDL_DestroyWindow(mWindow);
    SDL_Quit();
}

int Application::GetTextureWidth(SDL_Texture *t)
{
    int w = 0;
    int h = 0;
    // get the width and height of the texture
    SDL_QueryTexture(t, NULL, NULL, &w, &h);
    return w;
}

int Application::GetTextureHeight(SDL_Texture *t)
{
    int w = 0;
    int h = 0;
    // get the width and height of the texture
    SDL_QueryTexture(t, NULL, NULL, &w, &h);
    return h;
}
