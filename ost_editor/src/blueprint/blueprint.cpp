
#include "builders.h"
#include "widgets.h"

#include <imgui_node_editor.h>

#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui_internal.h>

#include <string>
#include <vector>
#include <map>
#include <algorithm>
#include <utility>

#include "blueprint.h"

static inline ImRect ImGui_GetItemRect()
{
    return ImRect(ImGui::GetItemRectMin(), ImGui::GetItemRectMax());
}

static inline ImRect ImRect_Expanded(const ImRect& rect, float x, float y)
{
    auto result = rect;
    result.Min.x -= x;
    result.Min.y -= y;
    result.Max.x += x;
    result.Max.y += y;
    return result;
}

namespace ed = ax::NodeEditor;
namespace util = ax::NodeEditor::Utilities;

using namespace ax;

using ax::Widgets::IconType;


//extern "C" __declspec(dllimport) short __stdcall GetAsyncKeyState(int vkey);
//extern "C" bool Debug_KeyPress(int vkey)
//{
//    static std::map<int, bool> state;
//    auto lastState = state[vkey];
//    state[vkey] = (GetAsyncKeyState(vkey) & 0x8000) != 0;
//    if (state[vkey] && !lastState)
//        return true;
//    else
//        return false;
//}

static bool Splitter(bool split_vertically, float thickness, float* size1, float* size2, float min_size1, float min_size2, float splitter_long_axis_size = -1.0f)
{
    using namespace ImGui;
    ImGuiContext& g = *GImGui;
    ImGuiWindow* window = g.CurrentWindow;
    ImGuiID id = window->GetID("##Splitter");
    ImRect bb;
    bb.Min = window->DC.CursorPos + (split_vertically ? ImVec2(*size1, 0.0f) : ImVec2(0.0f, *size1));
    bb.Max = bb.Min + CalcItemSize(split_vertically ? ImVec2(thickness, splitter_long_axis_size) : ImVec2(splitter_long_axis_size, thickness), 0.0f, 0.0f);
    return SplitterBehavior(bb, id, split_vertically ? ImGuiAxis_X : ImGuiAxis_Y, size1, size2, min_size1, min_size2, 0.0f);
}

Blueprint::Blueprint(Application &app)
    : m_app(app)
{

}

void ImGuiEx_BeginColumn()
{
    ImGui::BeginGroup();
}

void ImGuiEx_NextColumn()
{
    ImGui::EndGroup();
    ImGui::SameLine();
    ImGui::BeginGroup();
}

void ImGuiEx_EndColumn()
{
    ImGui::EndGroup();
}

void Blueprint::OnStop()
{
    if (m_editorContext)
    {
        ed::DestroyEditor(m_editorContext);
        m_editorContext = nullptr;
    }
}


void Blueprint::OnFrame()
{
    auto& io = ImGui::GetIO();

//    ImGui::Text("FPS: %.2f (%.2gms)", io.Framerate, io.Framerate ? 1000.0f / io.Framerate : 0.0f);

//    ImGui::Separator();

    ed::SetCurrentEditor(m_editorContext);

    // Start interaction with editor.
    ed::Begin("My Editor", ImVec2(0.0, 0.0f));

    int uniqueId = 1;

    //
    // 1) Commit known data to editor
    //
/*
    // Submit Node A
    ed::NodeId nodeA_Id = uniqueId++;
    ed::PinId  nodeA_InputPinId = uniqueId++;
    ed::PinId  nodeA_OutputPinId = uniqueId++;

    if (m_FirstFrame)
        ed::SetNodePosition(nodeA_Id, ImVec2(10, 10));
    ed::BeginNode(nodeA_Id);
        ImGui::Text("Node A");
        ed::BeginPin(nodeA_InputPinId, ed::PinKind::Input);
            ImGui::Text("-> In");
        ed::EndPin();
        ImGui::SameLine();
        ed::BeginPin(nodeA_OutputPinId, ed::PinKind::Output);
            ImGui::Text("Out ->");
        ed::EndPin();
    ed::EndNode();

    // Submit Node B
    ed::NodeId nodeB_Id = uniqueId++;
    ed::PinId  nodeB_InputPinId1 = uniqueId++;
    ed::PinId  nodeB_InputPinId2 = uniqueId++;
    ed::PinId  nodeB_OutputPinId = uniqueId++;

    if (m_FirstFrame)
        ed::SetNodePosition(nodeB_Id, ImVec2(210, 60));
    ed::BeginNode(nodeB_Id);
        ImGui::Text("Node B");
        ImGuiEx_BeginColumn();
            ed::BeginPin(nodeB_InputPinId1, ed::PinKind::Input);
                ImGui::Text("-> In1");
            ed::EndPin();
            ed::BeginPin(nodeB_InputPinId2, ed::PinKind::Input);
                ImGui::Text("-> In2");
            ed::EndPin();
        ImGuiEx_NextColumn();
            ed::BeginPin(nodeB_OutputPinId, ed::PinKind::Output);
                ImGui::Text("Out ->");
            ed::EndPin();
        ImGuiEx_EndColumn();
    ed::EndNode();

    // Submit Links
    for (auto& linkInfo : m_Links)
        ed::Link(linkInfo.Id, linkInfo.InputId, linkInfo.OutputId);
*/
    //
    // 2) Handle interactions
    //

    // Handle creation action, returns true if editor want to create new object (node or link)
    if (ed::BeginCreate())
    {
        ed::PinId inputPinId, outputPinId;
        if (ed::QueryNewLink(&inputPinId, &outputPinId))
        {
            // QueryNewLink returns true if editor want to create new link between pins.
            //
            // Link can be created only for two valid pins, it is up to you to
            // validate if connection make sense. Editor is happy to make any.
            //
            // Link always goes from input to output. User may choose to drag
            // link from output pin or input pin. This determine which pin ids
            // are valid and which are not:
            //   * input valid, output invalid - user started to drag new ling from input pin
            //   * input invalid, output valid - user started to drag new ling from output pin
            //   * input valid, output valid   - user dragged link over other pin, can be validated

            if (inputPinId && outputPinId) // both are valid, let's accept link
            {
                // ed::AcceptNewItem() return true when user release mouse button.
                if (ed::AcceptNewItem())
                {
                    // Since we accepted new link, lets add one to our list of links.
                    m_Links.push_back({ ed::LinkId(m_NextLinkId++), inputPinId, outputPinId });

                    // Draw new link.
                    ed::Link(m_Links.back().Id, m_Links.back().InputId, m_Links.back().OutputId);
                }

                // You may choose to reject connection between these nodes
                // by calling ed::RejectNewItem(). This will allow editor to give
                // visual feedback by changing link thickness and color.
            }
        }
    }
    ed::EndCreate(); // Wraps up object creation action handling.


    // Handle deletion action
    if (ed::BeginDelete())
    {
        // There may be many links marked for deletion, let's loop over them.
        ed::LinkId deletedLinkId;
        while (ed::QueryDeletedLink(&deletedLinkId))
        {
            // If you agree that link can be deleted, accept deletion.
            if (ed::AcceptDeletedItem())
            {
                // Then remove link from your data.
                for (auto& link : m_Links)
                {
                    if (link.Id == deletedLinkId)
                    {
                        m_Links.erase(&link);
                        break;
                    }
                }
            }

            // You may reject link deletion by calling:
            // ed::RejectDeletedItem();
        }
    }
    ed::EndDelete(); // Wrap up deletion action



    // End of interaction with editor.
    ed::End();

    if (m_FirstFrame)
        ed::NavigateToContent(0.0f);

    ed::SetCurrentEditor(nullptr);

    m_FirstFrame = false;

} // OnFrame

void Blueprint::OnStart()
{
    ed::Config config;

    config.SettingsFile = "Blueprints.json";

    m_editorContext = ed::CreateEditor(&config);
}
