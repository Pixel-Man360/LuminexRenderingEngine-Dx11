#pragma once
#include <memory>
#include <string>

namespace Engine::Editor
{
    // Base class for all undoable commands
    class UndoCommand
    {
    public:
        virtual ~UndoCommand() = default;
        
        virtual void Execute() = 0;  // Do/Redo the action
        virtual void Undo() = 0;     // Undo the action
        virtual std::string GetDescription() const = 0;
    };
}
