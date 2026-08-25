#pragma once

#include "Core/PreRequisites.h"
#include "GameFrameWork/GameplayUtils.hpp"
#include <typeindex>
#include <functional>
#include "Core/Settings/EngineSettings.hpp"

// ====================================================================================
// EngineSettingsRegistry (Master Registry for all Developer & Engine Settings classes)
// ====================================================================================
class TE_API EngineSettingsRegistry
{
public:
    static EngineSettingsRegistry &Get()
    {
        static EngineSettingsRegistry instance;
        return instance;
    }

    template <typename T>
    void RegisterSettingsClass()
    {
        std::type_index typeIdx = std::type_index(typeid(T));
        if (m_SettingsInstances.Find(typeIdx) == nullptr)
        {
            TERef<EngineSettings> inst = CreateRef<T>();
            inst->OnInitialized();
            m_SettingsInstances[typeIdx] = inst;

            TEString cat = inst->GetCategoryName();
            if (!m_Categories.Contains(cat))
            {
                m_Categories.Add(cat);
            }
        }
    }

    template <typename T>
    static TERef<T> GetSettings()
    {
        std::type_index typeIdx = std::type_index(typeid(T));
        auto *found = Get().m_SettingsInstances.Find(typeIdx);
        if (found && *found)
        {
            return std::static_pointer_cast<T>(*found);
        }
        return nullptr;
    }

    template <typename T>
    static T &GetMutable()
    {
        auto ptr = GetSettings<T>();
        if (!ptr)
        {
            Get().RegisterSettingsClass<T>();
            ptr = GetSettings<T>();
        }
        return *ptr;
    }

    const TEArray<TEString> &GetCategories() const { return m_Categories; }
    const TEMap<std::type_index, TERef<EngineSettings>> &GetAllSettings() const { return m_SettingsInstances; }

    TEArray<TERef<EngineSettings>> GetSettingsByCategory(const TEString &category) const
    {
        TEArray<TERef<EngineSettings>> result;
        for (const auto &pair : m_SettingsInstances)
        {
            if (pair.second && pair.second->GetCategoryName() == category)
            {
                result.Add(pair.second);
            }
        }
        return result;
    }

    TERef<EngineSettings> GetSettingsBySection(const TEString &category, const TEString &section) const
    {
        for (const auto &pair : m_SettingsInstances)
        {
            if (pair.second && pair.second->GetCategoryName() == category && pair.second->GetSectionName() == section)
            {
                return pair.second;
            }
        }
        return nullptr;
    }

    void SaveAll(const TEString &configDir)
    {
        for (auto it = m_SettingsInstances.begin(); it != m_SettingsInstances.end(); ++it)
        {
            if (it->second)
            {
                it->second->Save(configDir);
            }
        }
    }

    void LoadAll(const TEString &configDir)
    {
        for (auto it = m_SettingsInstances.begin(); it != m_SettingsInstances.end(); ++it)
        {
            if (it->second)
            {
                it->second->Load(configDir);
            }
        }
    }

    void Clear()
    {
        m_SettingsInstances.Clear();
        m_Categories.Clear();
    }

    static void ClearAll()
    {
        Get().Clear();
    }

private:
    EngineSettingsRegistry() = default;
    ~EngineSettingsRegistry() = default;

    TEMap<std::type_index, TERef<EngineSettings>> m_SettingsInstances;
    TEArray<TEString> m_Categories;
};

namespace Internal
{
    template <typename T>
    struct SettingsClassAutoRegistrar
    {
        SettingsClassAutoRegistrar()
        {
            EngineSettingsRegistry::Get().RegisterSettingsClass<T>();
        }
    };
} // namespace Internal


// ====================================================================================
// TE_REGISTER_SETTINGS Macro
// Automatically registers any derived EngineSettings subclass at static initialization.
// ====================================================================================
#define TE_REGISTER_SETTINGS(SettingsClass) \
    static Internal::SettingsClassAutoRegistrar<SettingsClass> _reg_settings_class_##SettingsClass;

