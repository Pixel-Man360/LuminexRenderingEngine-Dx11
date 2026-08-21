#include "SceneHierarchyPanel.h"
#include "UndoManager.h"
#include "DeleteObjectCommand.h"
#include "../Editor/DuplicateObjectCommand.h"
#include "imgui.h"
#include "../Graphics/Renderer.h"
#include "../Scene/LightComponent.h"
#include <regex>

using namespace Engine::Editor;


static std::string GenerateUniqueName(Engine::Scene::Scene* scene, const std::string& baseName)
{
    if (!scene) return baseName;
    
    int highestNumber = 0;
    bool baseNameExists = false;
    
    std::regex pattern("^" + baseName + "(?: \\((\\d+)\\))?$");
    
    for (auto& obj : scene->GetObjects())
    {
        std::smatch match;
        std::string objName = obj->GetName();
        
        if (std::regex_match(objName, match, pattern))
        {
            if (match[1].matched)
            {
                int num = std::stoi(match[1].str());
                if (num > highestNumber)
                    highestNumber = num;
            }
            else
            {
                baseNameExists = true;
            }
        }
    }
    
    if (!baseNameExists && highestNumber == 0)
        return baseName;
    else if (!baseNameExists)
        return baseName;
    else
    {
        int nextNumber = highestNumber + 1;
        if (nextNumber < 2) nextNumber = 2;
        return baseName + " (" + std::to_string(nextNumber) + ")";
    }
}

void SceneHierarchyPanel::Draw(EditorContext& context)
{
    ImGui::Begin("Scene Hierarchy");

    if (!context.ActiveScene)
    {
        ImGui::Text("No active scene");
        ImGui::End();
        return;
    }

    Engine::Scene::SceneObject* objectToDelete = nullptr;
    Engine::Scene::SceneObject* objectToDuplicate = nullptr;

    for (auto& obj : context.ActiveScene->GetObjects())
    {
        bool selected = context.SelectedObject == obj.get();

        if (ImGui::Selectable(obj->GetName().c_str(), selected, ImGuiSelectableFlags_AllowDoubleClick))
            context.SelectedObject = obj.get();

        if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
        {
            context.SelectedObject = obj.get();

            if (m_renderer) m_renderer->FocusCameraOn(obj.get());
        }

        if (ImGui::BeginPopupContextItem())
        {
            context.SelectedObject = obj.get();

            if (ImGui::MenuItem("Duplicate", "Ctrl+D"))
                objectToDuplicate = obj.get();

            ImGui::Separator();

            if (ImGui::MenuItem("Delete", "Delete"))
                objectToDelete = obj.get();

            ImGui::EndPopup();
        }
    }

    if (ImGui::IsWindowHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Left) && !ImGui::IsAnyItemHovered())
        context.SelectedObject = nullptr;

    if (objectToDuplicate)
    {
        auto command = std::make_unique<DuplicateObjectCommand>(context, objectToDuplicate);
        UndoManager::Get().ExecuteCommand(std::move(command));
    }

    if (objectToDelete)
    {
        auto command = std::make_unique<DeleteObjectCommand>(context, objectToDelete);
        UndoManager::Get().ExecuteCommand(std::move(command));
    }


    if (ImGui::BeginPopupContextWindow(nullptr, ImGuiPopupFlags_NoOpenOverItems | ImGuiPopupFlags_MouseButtonRight))
    {
        if (ImGui::BeginMenu("3D Object"))
        {
            if (ImGui::MenuItem("Cube"))
            {
                if (context.ActiveScene && m_renderer)
                {
                    auto* obj = context.ActiveScene->CreateObject(GenerateUniqueName(context.ActiveScene, "Cube"));
                    obj->SetMesh(m_renderer->GetCubeMesh());
                    context.SelectedObject = obj;
                }
            }
            if (ImGui::MenuItem("Sphere"))
            {
                if (context.ActiveScene && m_renderer)
                {
                    auto* obj = context.ActiveScene->CreateObject(GenerateUniqueName(context.ActiveScene, "Sphere"));
                    obj->SetMesh(m_renderer->GetSphereMesh());
                    context.SelectedObject = obj;
                }
            }
            if (ImGui::MenuItem("Cylinder"))
            {
                if (context.ActiveScene && m_renderer)
                {
                    auto* obj = context.ActiveScene->CreateObject(GenerateUniqueName(context.ActiveScene, "Cylinder"));
                    obj->SetMesh(m_renderer->GetCylinderMesh());
                    context.SelectedObject = obj;
                }
            }
            if (ImGui::MenuItem("Capsule"))
            {
                if (context.ActiveScene && m_renderer)
                {
                    auto* obj = context.ActiveScene->CreateObject(GenerateUniqueName(context.ActiveScene, "Capsule"));
                    obj->SetMesh(m_renderer->GetCapsuleMesh());
                    context.SelectedObject = obj;
                }
            }
            if (ImGui::MenuItem("Plane"))
            {
                if (context.ActiveScene && m_renderer)
                {
                    auto* obj = context.ActiveScene->CreateObject(GenerateUniqueName(context.ActiveScene, "Plane"));
                    obj->SetMesh(m_renderer->GetPlaneMesh());
                    context.SelectedObject = obj;
                }
            }
            ImGui::EndMenu();
        }

       
        if (ImGui::BeginMenu("Light"))
        {
            if (ImGui::MenuItem("Directional Light"))
            {
                if (context.ActiveScene)
                {
                    auto* obj = context.ActiveScene->CreateObject(GenerateUniqueName(context.ActiveScene, "Directional Light"));
                    obj->AddLightComponent();
                    obj->GetLightComponent()->SetType(Engine::Scene::LightType::Directional);
                    obj->GetTransform().SetRotationEuler({ 50.0f, -30.0f, 0.0f });
                    context.SelectedObject = obj;
                }
            }
            if (ImGui::MenuItem("Point Light"))
            {
                if (context.ActiveScene)
                {
                    auto* obj = context.ActiveScene->CreateObject(GenerateUniqueName(context.ActiveScene, "Point Light"));
                    obj->AddLightComponent();
                    obj->GetLightComponent()->SetType(Engine::Scene::LightType::Point);
                    obj->GetLightComponent()->SetRange(10.0f);
                    obj->GetTransform().SetPosition({ 0.0f, 3.0f, 0.0f });
                    context.SelectedObject = obj;
                }
            }
            ImGui::EndMenu();
        }

        ImGui::Separator();


        if (ImGui::MenuItem("Create Empty"))
        {
            auto* newObj = context.ActiveScene->CreateObject(GenerateUniqueName(context.ActiveScene, "Empty"));
            context.SelectedObject = newObj;
        }

        ImGui::EndPopup();
    }

    ImGui::End();
}
