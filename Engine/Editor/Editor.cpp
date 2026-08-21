#include "Editor.h"
#include "EditorPanel.h"
#include "SceneHierarchyPanel.h"
#include "InspectorPanel.h"
#include "StatsPanel.h"
#include "ViewportPanel.h"
#include "MenuBarPanel.h"
#include "MousePicker.h"
#include "UndoManager.h"
#include "TransformChangeCommand.h"
#include "DeleteObjectCommand.h"
#include "../Editor/DuplicateObjectCommand.h"
#include "../Graphics/Renderer.h"

#include <Windows.h>
#include <imgui.h>
#include <imgui_impl_dx11.h>
#include <imgui_impl_win32.h>

using namespace Engine::Editor;

void SetupImGuiStyle()
{
    ImGuiStyle& style = ImGui::GetStyle();
    ImVec4* colors = style.Colors;

    colors[ImGuiCol_Text] = ImVec4(0.90f, 0.90f, 0.90f, 1.00f);
    colors[ImGuiCol_TextDisabled] = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);

    colors[ImGuiCol_WindowBg] = ImVec4(0.22f, 0.22f, 0.22f, 1.00f);
    colors[ImGuiCol_ChildBg] = ImVec4(0.22f, 0.22f, 0.22f, 1.00f);
    colors[ImGuiCol_PopupBg] = ImVec4(0.24f, 0.24f, 0.24f, 1.00f);

    colors[ImGuiCol_Border] = ImVec4(0.16f, 0.16f, 0.16f, 1.00f);
    colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);

    colors[ImGuiCol_FrameBg] = ImVec4(0.16f, 0.16f, 0.16f, 1.00f);
    colors[ImGuiCol_FrameBgHovered] = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
    colors[ImGuiCol_FrameBgActive] = ImVec4(0.24f, 0.24f, 0.24f, 1.00f);

    colors[ImGuiCol_TitleBg] = ImVec4(0.16f, 0.16f, 0.16f, 1.00f);
    colors[ImGuiCol_TitleBgActive] = ImVec4(0.16f, 0.16f, 0.16f, 1.00f);
    colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.16f, 0.16f, 0.16f, 1.00f);

    colors[ImGuiCol_MenuBarBg] = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);

    colors[ImGuiCol_ScrollbarBg] = ImVec4(0.16f, 0.16f, 0.16f, 1.00f);
    colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.40f, 0.40f, 0.40f, 1.00f);
    colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
    colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.60f, 0.60f, 0.60f, 1.00f);

    colors[ImGuiCol_CheckMark] = ImVec4(0.35f, 0.54f, 0.81f, 1.00f);

    colors[ImGuiCol_SliderGrab] = ImVec4(0.35f, 0.54f, 0.81f, 1.00f);
    colors[ImGuiCol_SliderGrabActive] = ImVec4(0.45f, 0.64f, 0.91f, 1.00f);

    colors[ImGuiCol_Button] = ImVec4(0.30f, 0.30f, 0.30f, 1.00f);
    colors[ImGuiCol_ButtonHovered] = ImVec4(0.40f, 0.40f, 0.40f, 1.00f);
    colors[ImGuiCol_ButtonActive] = ImVec4(0.35f, 0.54f, 0.81f, 1.00f);

    colors[ImGuiCol_Header] = ImVec4(0.28f, 0.28f, 0.28f, 1.00f);
    colors[ImGuiCol_HeaderHovered] = ImVec4(0.35f, 0.54f, 0.81f, 0.80f);
    colors[ImGuiCol_HeaderActive] = ImVec4(0.35f, 0.54f, 0.81f, 1.00f);

    colors[ImGuiCol_Separator] = ImVec4(0.16f, 0.16f, 0.16f, 1.00f);
    colors[ImGuiCol_SeparatorHovered] = ImVec4(0.35f, 0.54f, 0.81f, 1.00f);
    colors[ImGuiCol_SeparatorActive] = ImVec4(0.35f, 0.54f, 0.81f, 1.00f);

    colors[ImGuiCol_ResizeGrip] = ImVec4(0.35f, 0.54f, 0.81f, 0.25f);
    colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.35f, 0.54f, 0.81f, 0.67f);
    colors[ImGuiCol_ResizeGripActive] = ImVec4(0.35f, 0.54f, 0.81f, 0.95f);

    colors[ImGuiCol_Tab] = ImVec4(0.18f, 0.18f, 0.18f, 1.00f);
    colors[ImGuiCol_TabHovered] = ImVec4(0.35f, 0.54f, 0.81f, 0.80f);
    colors[ImGuiCol_TabActive] = ImVec4(0.28f, 0.28f, 0.28f, 1.00f);
    colors[ImGuiCol_TabUnfocused] = ImVec4(0.18f, 0.18f, 0.18f, 1.00f);
    colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.22f, 0.22f, 0.22f, 1.00f);

    colors[ImGuiCol_DockingPreview] = ImVec4(0.35f, 0.54f, 0.81f, 0.70f);
    colors[ImGuiCol_DockingEmptyBg] = ImVec4(0.16f, 0.16f, 0.16f, 1.00f);

    colors[ImGuiCol_PlotLines] = ImVec4(0.61f, 0.61f, 0.61f, 1.00f);
    colors[ImGuiCol_PlotLinesHovered] = ImVec4(0.35f, 0.54f, 0.81f, 1.00f);
    colors[ImGuiCol_PlotHistogram] = ImVec4(0.35f, 0.54f, 0.81f, 1.00f);
    colors[ImGuiCol_PlotHistogramHovered] = ImVec4(0.45f, 0.64f, 0.91f, 1.00f);

    colors[ImGuiCol_TableHeaderBg] = ImVec4(0.19f, 0.19f, 0.19f, 1.00f);
    colors[ImGuiCol_TableBorderStrong] = ImVec4(0.16f, 0.16f, 0.16f, 1.00f);
    colors[ImGuiCol_TableBorderLight] = ImVec4(0.23f, 0.23f, 0.23f, 1.00f);
    colors[ImGuiCol_TableRowBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    colors[ImGuiCol_TableRowBgAlt] = ImVec4(1.00f, 1.00f, 1.00f, 0.03f);

    colors[ImGuiCol_TextSelectedBg] = ImVec4(0.35f, 0.54f, 0.81f, 0.35f);
    colors[ImGuiCol_DragDropTarget] = ImVec4(0.35f, 0.54f, 0.81f, 1.00f);

    colors[ImGuiCol_NavHighlight] = ImVec4(0.35f, 0.54f, 0.81f, 1.00f);
    colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
    colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.20f);
    colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.20f, 0.20f, 0.20f, 0.35f);

    style.WindowRounding = 0.0f;
    style.ChildRounding = 0.0f;
    style.FrameRounding = 2.0f;
    style.GrabRounding = 2.0f;
    style.PopupRounding = 0.0f;
    style.ScrollbarRounding = 2.0f;
    style.TabRounding = 0.0f;

    style.WindowBorderSize = 1.0f;
    style.ChildBorderSize = 1.0f;
    style.PopupBorderSize = 1.0f;
    style.FrameBorderSize = 0.0f;
    style.TabBorderSize = 0.0f;

    style.WindowPadding = ImVec2(8.0f, 8.0f);
    style.FramePadding = ImVec2(4.0f, 3.0f);
    style.ItemSpacing = ImVec2(8.0f, 4.0f);
    style.ItemInnerSpacing = ImVec2(4.0f, 4.0f);
    style.IndentSpacing = 20.0f;
    style.ScrollbarSize = 14.0f;
    style.GrabMinSize = 10.0f;
}

void Editor::Initialize(HWND__* hwnd, ID3D11Device* device, ID3D11DeviceContext* context, EditorContext editorContext)
{
    IMGUI_CHECKVERSION();

    ImGui::CreateContext();

    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

    ImGui::StyleColorsDark();
    SetupImGuiStyle();

    ImGui_ImplWin32_Init(hwnd);
    ImGui_ImplDX11_Init(device, context);

    m_context = editorContext;
    m_deviceContext = context;

    m_gizmo.Initialize(device);

    auto menuBarPanel = std::make_unique<MenuBarPanel>();
    m_menuBar = menuBarPanel.get();
    m_panels.emplace_back(std::move(menuBarPanel));

    auto hierarchyPanel = std::make_unique<SceneHierarchyPanel>();
    m_hierarchyPanel = hierarchyPanel.get();
    m_panels.emplace_back(std::move(hierarchyPanel));

    auto inspectorPanel = std::make_unique<InspectorPanel>();
    m_inspectorPanel = inspectorPanel.get();
    m_panels.emplace_back(std::move(inspectorPanel));

    m_panels.emplace_back(std::make_unique<StatsPanel>());

    auto viewportPanel = std::make_unique<ViewportPanel>();
    m_viewportPanel = viewportPanel.get();
    m_panels.emplace_back(std::move(viewportPanel));
}

void Editor::SetRenderer(Engine::Graphics::Renderer* renderer)
{
    m_renderer = renderer;

    if (m_menuBar) m_menuBar->SetRenderer(renderer);
    if (m_inspectorPanel) m_inspectorPanel->SetRenderer(renderer);
    if (m_hierarchyPanel) m_hierarchyPanel->SetRenderer(renderer);
    if (m_viewportPanel) m_viewportPanel->SetRenderer(renderer);
}

void Editor::BeginFrame()
{
    ImGui_ImplDX11_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();

    ImGuiWindowFlags windowFlags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;

    ImGuiViewport* viewport = ImGui::GetMainViewport();

    ImGui::SetNextWindowPos(viewport->WorkPos);
    ImGui::SetNextWindowSize(viewport->WorkSize);
    ImGui::SetNextWindowViewport(viewport->ID);

    windowFlags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse;
    windowFlags |= ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
    windowFlags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
    windowFlags |= ImGuiWindowFlags_NoBackground;

    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

    ImGui::Begin("DockSpaceWindow", nullptr, windowFlags);

    ImGui::PopStyleVar(3);

    ImGuiID dockspaceId = ImGui::GetID("MainDockSpace");
    ImGui::DockSpace(dockspaceId, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_PassthruCentralNode);

    ImGui::End();
}

void Editor::Render()
{
    if (m_showDemoWindow) ImGui::ShowDemoWindow(&m_showDemoWindow);

    for (auto& panel : m_panels) panel->Draw(m_context);

    ImGui::SetNextWindowPos(ImVec2(10, 30), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(200, 40), ImGuiCond_FirstUseEver);

    if (ImGui::Begin("Gizmo", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize))
    {
        bool isTranslate = m_context.CurrentGizmoMode == GizmoMode::Translate;
        bool isRotate = m_context.CurrentGizmoMode == GizmoMode::Rotate;
        bool isScale = m_context.CurrentGizmoMode == GizmoMode::Scale;

        if (ImGui::Selectable("W Move", isTranslate, 0, ImVec2(50, 0))) m_context.CurrentGizmoMode = GizmoMode::Translate;

        ImGui::SameLine();

        if (ImGui::Selectable("E Rotate", isRotate, 0, ImVec2(55, 0))) m_context.CurrentGizmoMode = GizmoMode::Rotate;

        ImGui::SameLine();

        if (ImGui::Selectable("R Scale", isScale, 0, ImVec2(50, 0))) m_context.CurrentGizmoMode = GizmoMode::Scale;
    }

    ImGui::End();

    if (!ImGui::GetIO().WantCaptureKeyboard)
    {
        ImGuiIO& io = ImGui::GetIO();

        if (ImGui::IsKeyPressed(ImGuiKey_W)) m_context.CurrentGizmoMode = GizmoMode::Translate;
        if (ImGui::IsKeyPressed(ImGuiKey_E)) m_context.CurrentGizmoMode = GizmoMode::Rotate;
        if (ImGui::IsKeyPressed(ImGuiKey_R)) m_context.CurrentGizmoMode = GizmoMode::Scale;

        if (io.KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_D) && m_context.SelectedObject && m_context.ActiveScene)
        {
            auto command = std::make_unique<DuplicateObjectCommand>(m_context, m_context.SelectedObject);
            UndoManager::Get().ExecuteCommand(std::move(command));
        }

        if (ImGui::IsKeyPressed(ImGuiKey_Delete) && m_context.SelectedObject && m_context.ActiveScene)
        {
            auto command = std::make_unique<DeleteObjectCommand>(m_context, m_context.SelectedObject);
            UndoManager::Get().ExecuteCommand(std::move(command));
        }
    }

    m_gizmo.SetMode(m_context.CurrentGizmoMode);
}

void Editor::RenderGizmo(const DirectX::XMMATRIX& view,
                         const DirectX::XMMATRIX& projection,
                         const DirectX::XMFLOAT3& cameraPos)
{
    if (!m_context.SelectedObject || !m_deviceContext) return;

    m_gizmo.Render(m_deviceContext, m_context.SelectedObject, view, projection, cameraPos);
}

void Editor::HandleInput(int mouseX, int mouseY, bool leftButtonDown, bool leftButtonPressed,
                         const DirectX::XMMATRIX& view, const DirectX::XMMATRIX& projection,
                         const DirectX::XMFLOAT3& cameraPos, int screenWidth, int screenHeight)
{
    static Engine::Scene::SceneObject* lastClickedObject = nullptr;
    static ULONGLONG lastObjectClickTime = 0;
    static int lastObjectClickX = 0;
    static int lastObjectClickY = 0;

    if (ImGui::GetIO().WantCaptureMouse)
    {
        m_wasLeftButtonDown = leftButtonDown;
        m_lastMouseX = mouseX;
        m_lastMouseY = mouseY;
        return;
    }

    if (leftButtonPressed && !m_wasLeftButtonDown)
    {
        Engine::Scene::SceneObject* picked = MousePicker::Pick(m_context.ActiveScene, mouseX, mouseY,
                                                               view, projection, cameraPos, screenWidth, screenHeight);

        ULONGLONG now = GetTickCount64();
        int maxDoubleClickX = GetSystemMetrics(SM_CXDOUBLECLK);
        int maxDoubleClickY = GetSystemMetrics(SM_CYDOUBLECLK);

        bool sameObject = picked && picked == lastClickedObject;
        bool withinTime = now - lastObjectClickTime <= GetDoubleClickTime();
        bool withinDistance = abs(mouseX - lastObjectClickX) <= maxDoubleClickX &&
            abs(mouseY - lastObjectClickY) <= maxDoubleClickY;

        bool doubleClickedObject = sameObject && withinTime && withinDistance;

        if (doubleClickedObject)
        {
            m_context.SelectedObject = picked;

            if (m_renderer) m_renderer->FocusCameraOn(picked);

            lastClickedObject = nullptr;
            lastObjectClickTime = 0;
        }
        else
        {
            bool gizmoHit = false;

            if (m_context.SelectedObject)
            {
                gizmoHit = m_gizmo.OnMouseDown(mouseX, mouseY, m_context.SelectedObject,
                                               view, projection, cameraPos, screenWidth, screenHeight, m_context.UniformScale);
            }

            if (!gizmoHit)
            {
                m_context.SelectedObject = picked;

                if (picked && m_context.CurrentGizmoMode == GizmoMode::Translate)
                {
                    m_gizmo.OnMouseDown(mouseX, mouseY, picked,
                                        view, projection, cameraPos, screenWidth, screenHeight, m_context.UniformScale);
                }
            }

            bool clickedObjectBody = picked && (!gizmoHit || m_gizmo.GetActiveAxis() == GizmoAxis::All);

            if (clickedObjectBody)
            {
                lastClickedObject = picked;
                lastObjectClickTime = now;
                lastObjectClickX = mouseX;
                lastObjectClickY = mouseY;
            }
            else
            {
                lastClickedObject = nullptr;
                lastObjectClickTime = 0;
            }
        }
    }

    if (leftButtonDown && m_gizmo.IsDragging())
    {
        int deltaX = mouseX - m_lastMouseX;
        int deltaY = mouseY - m_lastMouseY;

        m_gizmo.OnMouseMove(mouseX, mouseY, deltaX, deltaY, m_context.SelectedObject,
                            view, projection, cameraPos, screenWidth, screenHeight, m_context.UniformScale);
    }

    if (!leftButtonDown && m_wasLeftButtonDown)
    {
        GizmoMode dragMode = m_gizmo.GetMode();

        XMFLOAT3 startPos = m_gizmo.GetDragStartPos();
        XMFLOAT3 startRot = m_gizmo.GetDragStartRot();
        XMFLOAT3 startScale = m_gizmo.GetDragStartScale();

        bool transformChanged = m_gizmo.OnMouseUp(m_context.SelectedObject);

        if (transformChanged && m_context.SelectedObject)
        {
            XMFLOAT3 endPos = m_context.SelectedObject->GetTransform().GetPosition();
            XMFLOAT3 endRot = m_context.SelectedObject->GetTransform().GetRotationEuler();
            XMFLOAT3 endScale = m_context.SelectedObject->GetTransform().GetScale();

            if (dragMode == GizmoMode::Translate)
            {
                auto command = std::make_unique<TransformChangeCommand>(
                    m_context.SelectedObject,
                    TransformChangeCommand::ChangeType::Position,
                    startPos,
                    endPos
                );

                UndoManager::Get().AddCommand(std::move(command));
            }
            else if (dragMode == GizmoMode::Rotate)
            {
                auto command = std::make_unique<TransformChangeCommand>(
                    m_context.SelectedObject,
                    TransformChangeCommand::ChangeType::Rotation,
                    startRot,
                    endRot
                );

                UndoManager::Get().AddCommand(std::move(command));
            }
            else if (dragMode == GizmoMode::Scale)
            {
                auto command = std::make_unique<TransformChangeCommand>(
                    m_context.SelectedObject,
                    TransformChangeCommand::ChangeType::Scale,
                    startScale,
                    endScale
                );

                UndoManager::Get().AddCommand(std::move(command));
            }
        }
    }

    m_wasLeftButtonDown = leftButtonDown;
    m_lastMouseX = mouseX;
    m_lastMouseY = mouseY;
}

void Editor::EndFrame()
{
    ImGui::Render();
    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

    ImGuiIO& io = ImGui::GetIO();

    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        ImGui::UpdatePlatformWindows();
        ImGui::RenderPlatformWindowsDefault();
    }
}

void Editor::AddPanel(std::unique_ptr<EditorPanel> panel)
{
    m_panels.emplace_back(std::move(panel));
}

void Editor::Shutdown()
{
    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();
}