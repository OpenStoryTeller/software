
#include <iostream>
#include <memory>
#include "sdl_wrapper.h"
#include "imgui.h"
#include "imgui_internal.h"
#include "imgui_memory_editor.h"
#include "imnodes.h"
#include "IconsFontAwesome5.h"
#include "SDL2/SDL.h"
#include "imgui-knobs.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "ost_wrapper.h"

#include "ost_common.h"

#define NODE_TYPE_IMAGE     1
#define NODE_TYPE_SOUND     2
#define NODE_TYPE_ADDER     3




void draw_memory_editor()
{
    // Create a window and draw memory editor inside it:
   static MemoryEditor mem_edit_1;
   static char data[0x10000];
   size_t data_size = 0x10000;
   mem_edit_1.DrawWindow("Memory Editor", data, data_size);

}

class INode
{
public:

    virtual void draw() = 0;
};

class Adder : public INode
{
public:

    virtual void draw() override {

        ImNodes::BeginNode(NODE_TYPE_ADDER);

        ImNodes::BeginNodeTitleBar();
        ImGui::TextUnformatted("ADD");
        ImNodes::EndNodeTitleBar();

        ImNodes::BeginInputAttribute(1);
        ImGui::Text("IN 1");
        ImNodes::EndInputAttribute();

        ImNodes::BeginInputAttribute(3);
        ImGui::Text("IN 2");
        ImNodes::EndInputAttribute();


        const int output_attr_id = 4;
        ImNodes::BeginOutputAttribute(output_attr_id);
        // in between Begin|EndAttribute calls, you can call ImGui
        // UI functions
        ImGui::Indent(40);
        ImGui::Text("OUT");
        ImNodes::EndOutputAttribute();

        ImNodes::EndNode();
    }
};

class ShowImage : public INode
{
public:

    virtual void draw() override {

        ImNodes::BeginNode(NODE_TYPE_IMAGE);

        ImNodes::BeginNodeTitleBar();
        ImGui::TextUnformatted("IMAGE");
        ImNodes::EndNodeTitleBar();

        ImNodes::BeginInputAttribute(1);
        ImGui::Text("Show");
        static char str0[128] = "Choose image file";
        ImGui::InputText("##input text", str0, IM_ARRAYSIZE(str0));
        ImNodes::EndInputAttribute();

        ImNodes::EndNode();
    }
};

class PlaySound : public INode
{
public:

    virtual void draw() override {

        ImNodes::BeginNode(NODE_TYPE_SOUND);

        ImNodes::BeginNodeTitleBar();
        ImGui::TextUnformatted("SOUND");
        ImNodes::EndNodeTitleBar();

        ImNodes::BeginInputAttribute(1);
        ImGui::Text("Show");
        static char str0[128] = "Choose sound file";
        ImGui::InputText("##input text", str0, IM_ARRAYSIZE(str0));
        ImNodes::EndInputAttribute();

        ImNodes::EndNode();
    }
};
/*

class ImGuiImage
{
public:
    ~ImGuiImage() {
        if (m_texture != nullptr)
        {
            SDL_DestroyTexture(m_texture);
        }
    }

    void Create(SDL_Renderer *renderer)
    {
        m_texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGB565, SDL_TEXTUREACCESS_TARGET, 320, 240);

        // Switch the renderer to the texture
        SDL_SetRenderTarget(renderer, m_texture);


        // Switch back to main screen renderer
        SDL_SetRenderTarget(renderer, NULL);


    }

    void Load(SDL_Renderer *renderer, const char* filename)
    {
        // Read data
        int32_t width, height, bytesPerPixel;
        void* data = stbi_load(filename, &width, &height, &bytesPerPixel, 0);

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
        m_texture = nullptr;
        if (surface)
        {
            m_texture = SDL_CreateTextureFromSurface(renderer, surface);
        }
        else
        {
            // default texture (image loading problem)
            m_texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, 20, 20);
        }

        STBI_FREE(data);
        SDL_FreeSurface(surface);


        int w, h = 0;
        // get the width and height of the texture
        if (SDL_QueryTexture(m_texture, NULL, NULL, &w, &h) == 0)
        {
            m_w = w;
            m_h = h;
        }
        else
        {
            m_w = 20;
            m_h = 20;
        }
    }

    void draw() {
        if (m_texture != nullptr)
        {
            // Actually display the image
            ImGui::Image(m_texture, ImVec2((float)m_w, (float)m_h));
        }
    }

private:
    SDL_Texture *m_texture{nullptr};
    int m_w{0};
    int m_h{0};
};
*/






void draw_node_editor()
{
    ImGui::SetNextWindowSize(ImVec2(600, 400));
    ImGui::Begin("node editor");

    std::vector<std::unique_ptr<INode>> nodes;

    nodes.push_back(std::make_unique<Adder>());
    nodes.push_back(std::make_unique<ShowImage>());

    ImNodes::BeginNodeEditor();

    for (auto & n : nodes)
    {
        n->draw();
    }


    ImNodes::MiniMap(0.2f, ImNodesMiniMapLocation_TopRight);

    ImNodes::EndNodeEditor();

    ImGui::End();
}


void draw_tool_bar()
{

    ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_MenuBar;
    float height = ImGui::GetFrameHeight();

    if (ImGui::BeginViewportSideBar("##ToolBar", NULL, ImGuiDir_Up, height, window_flags)) {
        if (ImGui::BeginMenuBar())
        {
            ImVec2 sz = { 28, 28};
            if (ImGui::Button(ICON_FA_COG, sz))
            {
                ImGui::OpenPopup("Options");
            }
            ImGui::NewLine();

            if (ImGui::Button(ICON_FA_SIGN_OUT_ALT))
            {
           //
            }


            ImGui::EndMenuBar();
        }
    }
    ImGui::End();
}

void draw_status_bar()
{
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_MenuBar;
    float height = ImGui::GetFrameHeight();

    if (ImGui::BeginViewportSideBar("##MainStatusBar", NULL, ImGuiDir_Down, height, window_flags)) {
        if (ImGui::BeginMenuBar()) {
            ImGui::Text("Happy status bar");
            ImGui::EndMenuBar();
        }
    }
    ImGui::End();
}

float draw_menu_bar()
{
    float menuHeight = 0;
    if(ImGui::BeginMainMenuBar())
    {
        menuHeight = ImGui::GetWindowSize().y;

        if (ImGui::BeginMenu("Actions"))
        {
            if(ImGui::MenuItem("Quit"))
            {
                // mEvent.ExitGame();
            }
            ImGui::EndMenu();
        }

        ImGui::EndMainMenuBar();
    }
    return menuHeight;
}


void draw_editor_layout()
{
    static bool resetDockspace = true;

    float menuHeight = draw_menu_bar();

    draw_status_bar();
    draw_tool_bar();


    static ImGuiWindowFlags window_flags = ImGuiWindowFlags_None;

    ImGui::DockSpaceOverViewport();
    if (resetDockspace)
    {
        resetDockspace = false;

        ImGuiID id = ImGui::GetID("Dock Main");


//        ImGui::DockBuilderRemoveNode(id);
//        ImGui::DockBuilderAddNode(id, ImGuiDockNodeFlags_CentralNode);//ImGuiDockNodeFlags_CentralNode | ImGuiDockNodeFlags_NoResize);

//        ImGui::DockBuilderSetNodePos(id, { 0, menuHeight });
//        ImGui::DockBuilderSetNodeSize(id, { (float)GetSystem().GetWindowSize().w, (float)GetSystem().GetWindowSize().h - menuHeight});

////        ImGuiID dockMainID = id;
////        const ImGuiID dockLeft = ImGui::DockBuilderSplitNode(dockMainID, ImGuiDir_Left, 1.0, nullptr, nullptr);

//        ImGui::DockBuilderDockWindow("EditorView", id);


//        ImGui::DockBuilderFinish(id);

        // ----------------- other exmple
        /*
        ImGuiDockNodeFlags dockspaceFlags = ImGuiDockNodeFlags_None;
        ImGuiID dockspaceID = ImGui::GetID(ID().c_str());
        if (!ImGui::DockBuilderGetNode(dockspaceID)) {
            ImGui::DockBuilderRemoveNode(dockspaceID);
            ImGui::DockBuilderAddNode(dockspaceID, ImGuiDockNodeFlags_None);

            ImGuiID dock_main_id = dockspaceID;
            ImGuiID dock_up_id = ImGui::DockBuilderSplitNode(dock_main_id, ImGuiDir_Up, 0.05f, nullptr, &dock_main_id);
            ImGuiID dock_right_id = ImGui::DockBuilderSplitNode(dock_main_id, ImGuiDir_Right, 0.2f, nullptr, &dock_main_id);
            ImGuiID dock_left_id = ImGui::DockBuilderSplitNode(dock_main_id, ImGuiDir_Left, 0.2f, nullptr, &dock_main_id);
            ImGuiID dock_down_id = ImGui::DockBuilderSplitNode(dock_main_id, ImGuiDir_Down, 0.2f, nullptr, &dock_main_id);
            ImGuiID dock_down_right_id = ImGui::DockBuilderSplitNode(dock_down_id, ImGuiDir_Right, 0.6f, nullptr, &dock_down_id);

            ImGui::DockBuilderDockWindow("Actions", dock_up_id);
            ImGui::DockBuilderDockWindow("Hierarchy", dock_right_id);
            ImGui::DockBuilderDockWindow("Inspector", dock_left_id);
            ImGui::DockBuilderDockWindow("Console", dock_down_id);
            ImGui::DockBuilderDockWindow("Project", dock_down_right_id);
            ImGui::DockBuilderDockWindow("Scene", dock_main_id);

                // Disable tab bar for custom toolbar
            ImGuiDockNode* node = ImGui::DockBuilderGetNode(dock_up_id);
            node->LocalFlags |= ImGuiDockNodeFlags_NoTabBar;

            ImGui::DockBuilderFinish(dock_main_id);
        }

        */

    }

}




void sdl_draw_callback(SDL_Renderer *renderer, double deltaTime)
{
    draw_editor_layout();
    draw_memory_editor();
    draw_node_editor();
    draw_ost_device(renderer, deltaTime);
}


int main()
{
    sdl_wrapper_init();

    ImNodes::CreateContext();


    int ev = 0;
    do
    {
       ev = sdl_wrapper_process(sdl_draw_callback);
    }
    while (ev != SDL_EV_QUIT);

    ImNodes::DestroyContext();
    sdl_wrapper_close();

    std::cout << "OST Editor exit" << std::endl;

    return 0;
}
