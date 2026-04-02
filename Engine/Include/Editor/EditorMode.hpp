#pragma once
#include <string>
#include <vector>
#include <memory>
#include <functional>

namespace TE {

    class EditorMode {
    public:
        virtual ~EditorMode() = default;
        virtual void OnEnter() {}
        virtual void OnUpdate(float dt) {}
        virtual void OnImGuiRender() {}
        virtual void OnExit() {}
        virtual const char* GetName() const = 0;
        virtual const char* GetIcon() const { return ""; }
    };

    class EditorModeRegistry {
    public:
        static void RegisterMode(std::unique_ptr<EditorMode> mode) {
            Instance().m_Modes.push_back(std::move(mode));
        }

        static void SetActiveMode(const std::string& name) {
            auto& instance = Instance();
            if (instance.m_ActiveMode && instance.m_ActiveMode->GetName() == name)
                return;

            for (auto& mode : instance.m_Modes) {
                if (mode->GetName() == name) {
                    if (instance.m_ActiveMode) instance.m_ActiveMode->OnExit();
                    instance.m_ActiveMode = mode.get();
                    instance.m_ActiveMode->OnEnter();
                    return;
                }
            }
        }

        static EditorMode* GetActiveMode() { return Instance().m_ActiveMode; }
        
        static const std::vector<std::unique_ptr<EditorMode>>& GetModes() { return Instance().m_Modes; }

    private:
        static EditorModeRegistry& Instance() {
            static EditorModeRegistry instance;
            return instance;
        }

        std::vector<std::unique_ptr<EditorMode>> m_Modes;
        EditorMode* m_ActiveMode = nullptr;
    };

    // Auto-registration helper
    template<typename T>
    struct EditorModeRegisterer {
        EditorModeRegisterer() {
            EditorModeRegistry::RegisterMode(std::make_unique<T>());
        }
    };

#define T_REGISTER_EDITOR_MODE(Type) \
    static ::TE::EditorModeRegisterer<Type> Type##_Registerer;

}
