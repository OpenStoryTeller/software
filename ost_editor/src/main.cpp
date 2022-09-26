
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
#include "code_editor.h"
#include "console_window.h"
#include "chip32_assembler.h"

#define NODE_TYPE_IMAGE     1
#define NODE_TYPE_SOUND     2
#define NODE_TYPE_ADDER     3
#define NODE_TYPE_WAIT_EVENT  4

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

class WaitForEvent : public INode
{
public:

    virtual void draw() override {

        ImNodes::BeginNode(NODE_TYPE_WAIT_EVENT);

        int attr_id = 1;

        ImNodes::BeginNodeTitleBar();
        ImGui::TextUnformatted("Wait event");
        ImNodes::EndNodeTitleBar();

        ImNodes::BeginInputAttribute(attr_id++);
        ImGui::Text("Begin wait");
        ImNodes::EndInputAttribute();

        ImNodes::BeginInputAttribute(attr_id++);

        static ImGuiComboFlags flags = 0;
        const char* items[] = { "Button", "Delay", "Low battery" };
        static int item_current_idx = 0; // Here we store our selection data as an index.
        const char* combo_preview_value = items[item_current_idx];  // Pass in the preview value visible before opening the combo (it could be anything)
        if (ImGui::BeginCombo("event_type", combo_preview_value, flags))
        {
            for (int n = 0; n < IM_ARRAYSIZE(items); n++)
            {
                const bool is_selected = (item_current_idx == n);
                if (ImGui::Selectable(items[n], is_selected))
                    item_current_idx = n;

                // Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
                if (is_selected)
                    ImGui::SetItemDefaultFocus();
            }
            ImGui::EndCombo();
        }

        ImNodes::EndInputAttribute();



        ImNodes::BeginOutputAttribute(attr_id++);
        // in between Begin|EndAttribute calls, you can call ImGui
        // UI functions
        ImGui::Indent(40);
        ImGui::Text("Button");
        ImNodes::EndOutputAttribute();

        ImNodes::BeginOutputAttribute(attr_id++);
        // in between Begin|EndAttribute calls, you can call ImGui
        // UI functions
        ImGui::Indent(40);
        ImGui::Text("Delay");
        ImNodes::EndOutputAttribute();

        ImNodes::BeginOutputAttribute(attr_id++);
        // in between Begin|EndAttribute calls, you can call ImGui
        // UI functions
        ImGui::Indent(40);
        ImGui::Text("Low battery");
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


void draw_node_editor()
{
    ImGui::SetNextWindowSize(ImVec2(600, 400));
    ImGui::Begin("node editor");

    static std::vector<std::unique_ptr<INode>> nodes;
    static bool one_time = true;

    if (one_time)
    {
        one_time = false;
        nodes.push_back(std::make_unique<Adder>());
        nodes.push_back(std::make_unique<ShowImage>());
        nodes.push_back(std::make_unique<WaitForEvent>());
    }


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

class App : public SdlWrapper
{
public:

    int Update(SDL_Renderer *renderer, double deltaTime) override
    {
        draw_editor_layout();
        draw_memory_editor();
        draw_node_editor();
        draw_ost_device(renderer, deltaTime);
        code_editor_draw();

        m_console.Draw("Console", nullptr);

        return 0;
    }

private:
    ConsoleWindow m_console;
};


static const std::string test1 = R"(; jump over the data, to our entry label
    jump         entry

$hello          bytes  "Hello world!", 0x0A  ; data

; label definition
entry:   ;; comment here should work
    mov      r0, r2  ; copy R2 into R0 (blank space between , and R2)
mov R0,R2  ; copy R2 into R0 (NO blank space between , and R2)

    jump entry
    halt
)";

#include "chip32.h"

int main()
{
    std::vector<uint8_t> program;
    Chip32Assembler assembler;
    assembler.Parse(test1);
    assembler.BuildBinary(program);

    uint16_t progSize = program.size();

    // Add RAM after program
    program.resize(8*1024);

    chip32_initialize(program.data(), program.size(), 256);
    chip32_run(progSize, 1000);

    return 0;
    /*
    App app;

    app.Initialize();

    ImNodes::CreateContext();

    code_editor_initialize();

    int ev = 0;
    do
    {
       ev = app.Process();
    }
    while (ev != SDL_EV_QUIT);

    ImNodes::DestroyContext();

    app.Close();

    std::cout << "OST Editor exit" << std::endl;

    return 0;
    */
}
