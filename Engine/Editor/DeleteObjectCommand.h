#pragma once
#include "UndoCommand.h"
#include "EditorContext.h"
#include "../Scene/Scene.h"
#include "../Scene/SceneObject.h"
#include "../Scene/LightComponent.h"
#include <memory>
#include <string>

namespace Engine::Graphics { class Mesh; }

namespace Engine::Editor
{
    class DeleteObjectCommand : public UndoCommand
    {
    public:
        DeleteObjectCommand(
            EditorContext& context,
            Engine::Scene::SceneObject* object)
            : m_context(context)
            , m_deletedObject(nullptr)
        {

            m_name = object->GetName();
            m_position = object->GetTransform().GetPosition();
            m_rotation = object->GetTransform().GetRotationEuler();
            m_scale = object->GetTransform().GetScale();
            m_mesh = object->GetMesh();
            m_texture = object->GetTexture();
            
            if (object->HasLight())
            {
                m_hasLight = true;
                auto* light = object->GetLightComponent();
                m_lightType = light->GetType();
                m_lightColor = light->GetColor();
                m_lightIntensity = light->GetIntensity();
                m_lightRange = light->GetRange();
                m_lightEnabled = light->IsEnabled();
            }
            
            m_objectPtr = object;
        }

        void Execute() override
        {
            if (m_context.SelectedObject == m_objectPtr)
                m_context.SelectedObject = nullptr;
            
            m_context.ActiveScene->DestroyObject(m_objectPtr);
            m_deletedObject = nullptr;
        }

        void Undo() override
        {
            auto* obj = m_context.ActiveScene->CreateObject(m_name);
            obj->GetTransform().SetPosition(m_position);
            obj->GetTransform().SetRotationEuler(m_rotation);
            obj->GetTransform().SetScale(m_scale);
            obj->SetMesh(m_mesh);
            obj->SetTexture(m_texture);
            
            if (m_hasLight)
            {
                obj->AddLightComponent();
                auto* light = obj->GetLightComponent();
                light->SetType(m_lightType);
                light->SetColor(m_lightColor);
                light->SetIntensity(m_lightIntensity);
                light->SetRange(m_lightRange);
                light->SetEnabled(m_lightEnabled);
            }
            
            m_objectPtr = obj;
            m_context.SelectedObject = obj;
        }

        std::string GetDescription() const override
        {
            return "Delete " + m_name;
        }

    private:
        EditorContext& m_context;
        Engine::Scene::SceneObject* m_objectPtr;
        Engine::Scene::SceneObject* m_deletedObject;
        
        std::string m_name;
        DirectX::XMFLOAT3 m_position;
        DirectX::XMFLOAT3 m_rotation;
        DirectX::XMFLOAT3 m_scale;
        Engine::Graphics::Mesh* m_mesh = nullptr;
        ID3D11ShaderResourceView* m_texture = nullptr;
        
        bool m_hasLight = false;
        Engine::Scene::LightType m_lightType = Engine::Scene::LightType::Point;
        DirectX::XMFLOAT3 m_lightColor = { 1, 1, 1 };
        float m_lightIntensity = 1.0f;
        float m_lightRange = 10.0f;
        bool m_lightEnabled = true;
    };
}
