#pragma once
#include "Editor/EditorMode.hpp"

namespace TE {

    class SelectionMode : public EditorMode {
    public:
        virtual const char* GetName() const override { return "Selection Mode"; }
        virtual const char* GetIcon() const override { return "Q"; } // Placeholder for icon

        virtual void OnEnter() override {
            // Logic for entering selection mode
        }

        virtual void OnUpdate(float dt) override {
            // Logic for selection mode update
        }

        virtual void OnExit() override {
            // Logic for exiting selection mode
        }
    };

    T_REGISTER_EDITOR_MODE(SelectionMode);

    // Helper to initialize the default state
    inline void InitEditorModes() {
        if (EditorModeRegistry::GetActiveMode() == nullptr && !EditorModeRegistry::GetModes().empty()) {
            EditorModeRegistry::SetActiveMode(EditorModeRegistry::GetModes()[0]->GetName());
        }
    }

}
