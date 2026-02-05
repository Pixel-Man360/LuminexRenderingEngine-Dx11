#pragma once
#include "UndoCommand.h"
#include <vector>
#include <stack>
#include <memory>

namespace Engine::Editor
{
    class UndoManager
    {
    public:
        static UndoManager& Get()
        {
            static UndoManager instance;
            return instance;
        }

  
        void ExecuteCommand(std::unique_ptr<UndoCommand> command)
        {
            command->Execute();
            m_undoStack.push(std::move(command));
            
            while (!m_redoStack.empty())
                m_redoStack.pop();
        }

        void AddCommand(std::unique_ptr<UndoCommand> command)
        {
            m_undoStack.push(std::move(command));
            
            while (!m_redoStack.empty())
                m_redoStack.pop();
        }

        void Undo()
        {
            if (m_undoStack.empty()) return;
            
            auto& command = m_undoStack.top();
            command->Undo();
            m_redoStack.push(std::move(m_undoStack.top()));
            m_undoStack.pop();
        }

        void Redo()
        {
            if (m_redoStack.empty()) return;
            
            auto& command = m_redoStack.top();
            command->Execute();
            m_undoStack.push(std::move(m_redoStack.top()));
            m_redoStack.pop();
        }

        bool CanUndo() const { return !m_undoStack.empty(); }
        bool CanRedo() const { return !m_redoStack.empty(); }

        std::string GetUndoDescription() const
        {
            if (m_undoStack.empty()) return "";
            return m_undoStack.top()->GetDescription();
        }

        std::string GetRedoDescription() const
        {
            if (m_redoStack.empty()) return "";
            return m_redoStack.top()->GetDescription();
        }

        void Clear()
        {
            while (!m_undoStack.empty()) m_undoStack.pop();
            while (!m_redoStack.empty()) m_redoStack.pop();
        }

    private:
        UndoManager() = default;
        
        std::stack<std::unique_ptr<UndoCommand>> m_undoStack;
        std::stack<std::unique_ptr<UndoCommand>> m_redoStack;
    };
}
