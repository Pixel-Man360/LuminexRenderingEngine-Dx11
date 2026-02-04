#pragma once
#include "UndoCommand.h"
#include "../Scene/SceneObject.h"
#include "../Scene/Transform.h"
#include <DirectXMath.h>

namespace Engine::Editor
{
    // Command for Transform changes (Position, Rotation, Scale)
    class TransformChangeCommand : public UndoCommand
    {
    public:
        enum class ChangeType { Position, Rotation, Scale };

        TransformChangeCommand(
            Engine::Scene::SceneObject* object,
            ChangeType type,
            const DirectX::XMFLOAT3& oldValue,
            const DirectX::XMFLOAT3& newValue)
            : m_object(object)
            , m_type(type)
            , m_oldValue(oldValue)
            , m_newValue(newValue)
        {
        }

        void Execute() override
        {
            ApplyValue(m_newValue);
        }

        void Undo() override
        {
            ApplyValue(m_oldValue);
        }

        std::string GetDescription() const override
        {
            std::string typeName;
            switch (m_type)
            {
            case ChangeType::Position: typeName = "Position"; break;
            case ChangeType::Rotation: typeName = "Rotation"; break;
            case ChangeType::Scale: typeName = "Scale"; break;
            }
            return "Change " + typeName + " of " + m_object->GetName();
        }

    private:
        void ApplyValue(const DirectX::XMFLOAT3& value)
        {
            if (!m_object) return;
            
            switch (m_type)
            {
            case ChangeType::Position:
                m_object->GetTransform().SetPosition(value);
                break;
            case ChangeType::Rotation:
                m_object->GetTransform().SetRotationEuler(value);
                break;
            case ChangeType::Scale:
                m_object->GetTransform().SetScale(value);
                break;
            }
        }

        Engine::Scene::SceneObject* m_object;
        ChangeType m_type;
        DirectX::XMFLOAT3 m_oldValue;
        DirectX::XMFLOAT3 m_newValue;
    };
}
