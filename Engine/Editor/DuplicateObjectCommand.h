#pragma once

#include "UndoCommand.h"
#include "EditorContext.h"
#include "../Scene/Scene.h"
#include "../Scene/SceneObject.h"
#include "../Scene/LightComponent.h"
#include "../Graphics/Material.h"

#include <algorithm>
#include <cctype>
#include <memory>
#include <string>

namespace Engine::Editor
{
    class DuplicateObjectCommand : public UndoCommand
    {
    public:
        DuplicateObjectCommand(EditorContext& context, Engine::Scene::SceneObject* source)
            : m_context(context)
        {
            if (!source || !context.ActiveScene) return;

            m_name = GenerateDuplicateName(context.ActiveScene, source->GetName());

            m_position = source->GetTransform().GetPosition();
            m_rotation = source->GetTransform().GetRotationEuler();
            m_scale = source->GetTransform().GetScale();

            m_mesh = source->GetMesh();
            m_texture = source->GetTexture();
            m_parent = source->GetParent();

            if (source->GetMaterial())
                m_material = std::make_shared<Engine::Graphics::Material>(*source->GetMaterial());

            if (source->HasLight())
            {
                m_hasLight = true;

                Engine::Scene::LightComponent* light = source->GetLightComponent();

                m_lightType = light->GetType();
                m_lightColor = light->GetColor();
                m_lightIntensity = light->GetIntensity();
                m_lightRange = light->GetRange();
                m_lightEnabled = light->IsEnabled();
            }
        }

        void Execute() override
        {
            if (!m_context.ActiveScene) return;

            Engine::Scene::SceneObject* object = m_context.ActiveScene->CreateObject(m_name);

            object->GetTransform().SetPosition(m_position);
            object->GetTransform().SetRotationEuler(m_rotation);
            object->GetTransform().SetScale(m_scale);

            object->SetMesh(m_mesh);
            object->SetTexture(m_texture);

            if (m_material)
                object->SetMaterial(std::make_shared<Engine::Graphics::Material>(*m_material));

            if (m_parent)
                object->SetParent(m_parent);

            if (m_hasLight)
            {
                object->AddLightComponent();

                Engine::Scene::LightComponent* light = object->GetLightComponent();

                light->SetType(m_lightType);
                light->SetColor(m_lightColor);
                light->SetIntensity(m_lightIntensity);
                light->SetRange(m_lightRange);
                light->SetEnabled(m_lightEnabled);
            }

            m_duplicate = object;
            m_context.SelectedObject = object;
        }

        void Undo() override
        {
            if (!m_context.ActiveScene || !m_duplicate) return;

            if (m_context.SelectedObject == m_duplicate)
                m_context.SelectedObject = nullptr;

            m_context.ActiveScene->DestroyObject(m_duplicate);
            m_duplicate = nullptr;
        }

        std::string GetDescription() const override
        {
            return "Duplicate " + m_name;
        }

    private:
        static std::string GetBaseName(const std::string& name)
        {
            size_t openBracket = name.rfind(" (");

            if (openBracket == std::string::npos || name.empty() || name.back() != ')')
                return name;

            std::string number = name.substr(openBracket + 2, name.size() - openBracket - 3);

            if (number.empty()) return name;

            bool numeric = std::all_of(number.begin(), number.end(), [](unsigned char c)
                                       {
                                           return std::isdigit(c) != 0;
                                       });

            return numeric ? name.substr(0, openBracket) : name;
        }

        static int GetNameIndex(const std::string& name, const std::string& baseName)
        {
            if (name == baseName) return 1;

            std::string prefix = baseName + " (";

            if (name.size() <= prefix.size() + 1) return 0;
            if (name.compare(0, prefix.size(), prefix) != 0) return 0;
            if (name.back() != ')') return 0;

            std::string number = name.substr(prefix.size(), name.size() - prefix.size() - 1);

            if (number.empty()) return 0;

            bool numeric = std::all_of(number.begin(), number.end(), [](unsigned char c)
                                       {
                                           return std::isdigit(c) != 0;
                                       });

            if (!numeric) return 0;

            return std::stoi(number);
        }

        static std::string GenerateDuplicateName(Engine::Scene::Scene* scene, const std::string& sourceName)
        {
            std::string baseName = GetBaseName(sourceName);
            int highestIndex = 1;

            for (const auto& object : scene->GetObjects())
            {
                int index = GetNameIndex(object->GetName(), baseName);
                if (index > highestIndex) highestIndex = index;
            }

            return baseName + " (" + std::to_string(highestIndex + 1) + ")";
        }

    private:
        EditorContext& m_context;

        Engine::Scene::SceneObject* m_duplicate = nullptr;
        Engine::Scene::SceneObject* m_parent = nullptr;

        std::string m_name;

        DirectX::XMFLOAT3 m_position = {};
        DirectX::XMFLOAT3 m_rotation = {};
        DirectX::XMFLOAT3 m_scale = { 1.0f, 1.0f, 1.0f };

        Engine::Graphics::Mesh* m_mesh = nullptr;
        ID3D11ShaderResourceView* m_texture = nullptr;
        std::shared_ptr<Engine::Graphics::Material> m_material;

        bool m_hasLight = false;

        Engine::Scene::LightType m_lightType = Engine::Scene::LightType::Point;
        DirectX::XMFLOAT3 m_lightColor = { 1.0f, 1.0f, 1.0f };

        float m_lightIntensity = 1.0f;
        float m_lightRange = 10.0f;

        bool m_lightEnabled = true;
    };
}