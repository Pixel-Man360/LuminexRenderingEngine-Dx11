#include "InspectorPanel.h"
#include "UndoManager.h"
#include "TransformChangeCommand.h"
#include "imgui.h"
#include "../Scene/Transform.h"
#include "../Scene/LightComponent.h"
#include "../Graphics/Renderer.h"
#include "../Graphics/Material.h"

#include <DirectXMath.h>
#include <windows.h>
#include <commdlg.h>
#include <string>
#include <vector>
#include <memory>

using namespace DirectX;
using namespace Engine::Editor;
using namespace Engine::Scene;

static XMFLOAT3 s_dragStartPos;
static XMFLOAT3 s_dragStartRot;
static XMFLOAT3 s_dragStartScale;

static bool s_isDraggingPos = false;
static bool s_isDraggingRot = false;
static bool s_isDraggingScale = false;

static ID3D11ShaderResourceView* DrawTextureSelector(const char* label, ID3D11ShaderResourceView* currentTexture,
                                                     Engine::Graphics::Renderer* renderer, Engine::Graphics::Renderer::TextureColorSpace colorSpace)
{
    auto textures = renderer->GetAvailableTextures();

    int currentIndex = 0;

    for (size_t i = 0; i < textures.size(); ++i)
    {
        if (textures[i].srv == currentTexture)
        {
            currentIndex = static_cast<int>(i);
            break;
        }
    }

    std::vector<std::string> names;
    names.reserve(textures.size() + 1);

    for (const auto& texture : textures) names.push_back(texture.name);

    names.push_back("Import...");

    std::vector<const char*> items;
    items.reserve(names.size());

    for (const auto& name : names) items.push_back(name.c_str());

    if (!ImGui::Combo(label, &currentIndex, items.data(), static_cast<int>(items.size()))) return currentTexture;

    if (currentIndex < static_cast<int>(textures.size())) return textures[currentIndex].srv;

    wchar_t filename[MAX_PATH] = {};

    OPENFILENAMEW fileDialog = {};
    fileDialog.lStructSize = sizeof(fileDialog);
    fileDialog.lpstrFile = filename;
    fileDialog.nMaxFile = MAX_PATH;
    fileDialog.lpstrFilter = L"Image Files\0*.png;*.jpg;*.jpeg;*.bmp;*.tga\0All Files\0*.*\0";
    fileDialog.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_NOCHANGEDIR;

    if (!GetOpenFileNameW(&fileDialog)) return currentTexture;

    ID3D11ShaderResourceView* texture = renderer->ImportTextureFromFile(filename, colorSpace);
    return texture ? texture : currentTexture;
}

void InspectorPanel::Draw(EditorContext& context)
{
    ImGui::Begin("Inspector");

    if (!context.SelectedObject)
    {
        ImGui::Text("No object selected");
        ImGui::End();
        return;
    }

    auto& transform = context.SelectedObject->GetTransform();

    ImGui::Text("Transform");
    ImGui::Separator();

    XMFLOAT3 pos = transform.GetPosition();

    if (ImGui::DragFloat3("Position", &pos.x, 0.1f))
    {
        if (!s_isDraggingPos)
        {
            s_dragStartPos = transform.GetPosition();
            s_isDraggingPos = true;
        }

        transform.SetPosition(pos);
    }

    if (s_isDraggingPos && ImGui::IsItemDeactivatedAfterEdit())
    {
        auto cmd = std::make_unique<TransformChangeCommand>(
            context.SelectedObject,
            TransformChangeCommand::ChangeType::Position,
            s_dragStartPos,
            pos
        );

        UndoManager::Get().AddCommand(std::move(cmd));
        s_isDraggingPos = false;
    }

    XMFLOAT3 eulerDeg = transform.GetRotationEuler();

    if (ImGui::DragFloat3("Rotation", &eulerDeg.x, 1.0f))
    {
        if (!s_isDraggingRot)
        {
            s_dragStartRot = transform.GetRotationEuler();
            s_isDraggingRot = true;
        }

        transform.SetRotationEuler(eulerDeg);
    }

    if (s_isDraggingRot && ImGui::IsItemDeactivatedAfterEdit())
    {
        auto cmd = std::make_unique<TransformChangeCommand>(
            context.SelectedObject,
            TransformChangeCommand::ChangeType::Rotation,
            s_dragStartRot,
            eulerDeg
        );

        UndoManager::Get().AddCommand(std::move(cmd));
        s_isDraggingRot = false;
    }

    ImGui::Spacing();

    ImGui::PushID("ScaleLinkToggle");

    if (context.UniformScale)
    {
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.6f, 0.2f, 1.0f));

        if (ImGui::SmallButton("[=]")) context.UniformScale = false;

        ImGui::PopStyleColor();

        if (ImGui::IsItemHovered()) ImGui::SetTooltip("Uniform Scale (click to unlink axes)");
    }
    else
    {
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.5f, 0.5f, 0.5f, 1.0f));

        if (ImGui::SmallButton("[ ]")) context.UniformScale = true;

        ImGui::PopStyleColor();

        if (ImGui::IsItemHovered()) ImGui::SetTooltip("Per-Axis Scale (click to link axes)");
    }

    ImGui::PopID();
    ImGui::SameLine();

    XMFLOAT3 scale = transform.GetScale();

    if (context.UniformScale)
    {
        float uniformValue = scale.x;

        if (ImGui::DragFloat("Scale", &uniformValue, 0.1f, 0.01f, 100.0f))
        {
            if (!s_isDraggingScale)
            {
                s_dragStartScale = transform.GetScale();
                s_isDraggingScale = true;
            }

            scale.x = uniformValue;
            scale.y = uniformValue;
            scale.z = uniformValue;

            transform.SetScale(scale);
        }
    }
    else
    {
        if (ImGui::DragFloat3("Scale", &scale.x, 0.1f, 0.01f, 100.0f))
        {
            if (!s_isDraggingScale)
            {
                s_dragStartScale = transform.GetScale();
                s_isDraggingScale = true;
            }

            transform.SetScale(scale);
        }
    }

    if (s_isDraggingScale && ImGui::IsItemDeactivatedAfterEdit())
    {
        auto cmd = std::make_unique<TransformChangeCommand>(
            context.SelectedObject,
            TransformChangeCommand::ChangeType::Scale,
            s_dragStartScale,
            scale
        );

        UndoManager::Get().AddCommand(std::move(cmd));
        s_isDraggingScale = false;
    }

    ImGui::Spacing();
    ImGui::Text("Mesh Renderer");
    ImGui::Separator();

    if (m_renderer)
    {
        const char* meshNames[] =
        {
            "None",
            "Cube",
            "Sphere",
            "Cylinder",
            "Capsule",
            "Plane"
        };

        Engine::Graphics::Mesh* meshes[] =
        {
            nullptr,
            m_renderer->GetCubeMesh(),
            m_renderer->GetSphereMesh(),
            m_renderer->GetCylinderMesh(),
            m_renderer->GetCapsuleMesh(),
            m_renderer->GetPlaneMesh()
        };

        int currentMesh = 0;
        Engine::Graphics::Mesh* objMesh = context.SelectedObject->GetMesh();

        for (int i = 0; i < 6; ++i)
        {
            if (meshes[i] == objMesh)
            {
                currentMesh = i;
                break;
            }
        }

        if (ImGui::Combo("Mesh", &currentMesh, meshNames, IM_ARRAYSIZE(meshNames)))
        {
            context.SelectedObject->SetMesh(meshes[currentMesh]);

            if (meshes[currentMesh] && !context.SelectedObject->GetMaterial())
                context.SelectedObject->SetMaterial(m_renderer->CreateDefaultMaterial());
        }

        Engine::Graphics::Material* mat = context.SelectedObject->GetMaterial();

        if (!mat)
        {
            if (context.SelectedObject->GetMesh())
            {
                if (ImGui::Button("Add Material"))
                    context.SelectedObject->SetMaterial(m_renderer->CreateDefaultMaterial());
            }
        }
        else
        {
            ImGui::Spacing();
            ImGui::Text("Material (PBR)");
            ImGui::Indent();

            ID3D11ShaderResourceView* albedoMap = DrawTextureSelector(
                "Albedo Map",
                mat->GetAlbedoMap(),
                m_renderer,
                Engine::Graphics::Renderer::TextureColorSpace::SRGB
            );

            mat->SetAlbedoMap(albedoMap);

            XMFLOAT4 albedo = mat->GetData().Albedo;
            float albedoColor[3] = { albedo.x, albedo.y, albedo.z };

            if (ImGui::ColorEdit3("Albedo Tint", albedoColor))
                mat->SetAlbedo({ albedoColor[0], albedoColor[1], albedoColor[2], 1.0f });

            ImGui::Spacing();

            ID3D11ShaderResourceView* normalMap = DrawTextureSelector("Normal Map", mat->GetNormalMap(), m_renderer, Engine::Graphics::Renderer::TextureColorSpace::Linear );

            mat->SetNormalMap(normalMap);

            ID3D11ShaderResourceView* previousMetallicMap = mat->GetMetallicMap();

            ID3D11ShaderResourceView* metallicMap = DrawTextureSelector(
                "Metallic Map",
                previousMetallicMap,
                m_renderer,
                Engine::Graphics::Renderer::TextureColorSpace::Linear
            );

            if (metallicMap != previousMetallicMap)
            {
                mat->SetMetallicMap(metallicMap);

                if (metallicMap && !previousMetallicMap) mat->SetMetallic(1.0f);
            }

            float metallic = mat->GetData().Metallic;
            const char* metallicLabel = metallicMap ? "Metallic Multiplier" : "Metallic";

            if (ImGui::SliderFloat(metallicLabel, &metallic, 0.0f, 1.0f))
                mat->SetMetallic(metallic);

            ImGui::Spacing();

            ID3D11ShaderResourceView* previousRoughnessMap = mat->GetRoughnessMap();

            ID3D11ShaderResourceView* roughnessMap = DrawTextureSelector(
                "Roughness Map",
                previousRoughnessMap,
                m_renderer,
                Engine::Graphics::Renderer::TextureColorSpace::Linear
            );

            if (roughnessMap != previousRoughnessMap)
            {
                mat->SetRoughnessMap(roughnessMap);

                if (roughnessMap && !previousRoughnessMap) mat->SetRoughness(1.0f);
            }

            float roughness = mat->GetData().Roughness;
            const char* roughnessLabel = roughnessMap ? "Roughness Multiplier" : "Roughness";

            if (ImGui::SliderFloat(roughnessLabel, &roughness, 0.0f, 1.0f))
                mat->SetRoughness(roughness);

            ImGui::Spacing();

            ID3D11ShaderResourceView* previousAOMap = mat->GetAOMap();

            ID3D11ShaderResourceView* aoMap = DrawTextureSelector(
                "AO Map",
                previousAOMap,
                m_renderer,
                Engine::Graphics::Renderer::TextureColorSpace::Linear
            );

            if (aoMap != previousAOMap)
            {
                mat->SetAOMap(aoMap);

                if (aoMap && !previousAOMap) mat->SetAO(1.0f);
            }

            float ao = mat->GetData().AO;
            const char* aoLabel = aoMap ? "AO Strength" : "AO";

            if (ImGui::SliderFloat(aoLabel, &ao, 0.0f, 1.0f))
                mat->SetAO(ao);

            ImGui::Spacing();

            XMFLOAT2 tiling = mat->GetTiling();
            float tilingValues[2] = { tiling.x, tiling.y };

            if (ImGui::DragFloat2("Tiling", tilingValues, 0.05f, 0.01f, 64.0f))
                mat->SetTiling({ tilingValues[0], tilingValues[1] });

            XMFLOAT2 offset = mat->GetOffset();
            float offsetValues[2] = { offset.x, offset.y };

            if (ImGui::DragFloat2("Offset", offsetValues, 0.01f, -10.0f, 10.0f))
                mat->SetOffset({ offsetValues[0], offsetValues[1] });

            ImGui::Unindent();
            ImGui::Spacing();

            if (ImGui::Button("Remove Material"))
                context.SelectedObject->SetMaterial(nullptr);
        }
    }

    LightComponent* light = context.SelectedObject->GetLightComponent();

    if (light)
    {
        ImGui::Spacing();
        ImGui::Text("Light");
        ImGui::Separator();

        const char* lightTypes[] = { "Directional", "Point", "Spot" };
        int currentType = static_cast<int>(light->GetType());

        if (ImGui::Combo("Type", &currentType, lightTypes, IM_ARRAYSIZE(lightTypes)))
            light->SetType(static_cast<Engine::Scene::LightType>(currentType));

        XMFLOAT3 color = light->GetColor();

        if (ImGui::ColorEdit3("Color", &color.x))
            light->SetColor(color);

        float intensity = light->GetIntensity();

        if (ImGui::DragFloat("Intensity", &intensity, 0.1f, 0.0f, 100.0f))
            light->SetIntensity(intensity);

        if (light->GetType() != Engine::Scene::LightType::Directional)
        {
            float range = light->GetRange();

            if (ImGui::DragFloat("Range", &range, 0.5f, 0.1f, 1000.0f))
                light->SetRange(range);
        }
    }

    ImGui::End();
}