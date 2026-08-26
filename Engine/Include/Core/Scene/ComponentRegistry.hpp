#pragma once
#include "Core/Scene/EntityManager.hpp"
#include "Utils/TimeGUI.hpp"
#include <algorithm>
#include <cstddef>
#include <functional>
#include <glm/glm.hpp>
#include <typeindex>

template <typename T> struct TEPropertyDrawer;

class TComponent;
using EntityID = uint64_t;

using PropertyDrawFunc = std::function<bool(void *, const TEString &)>;

struct PropertyMetadata
{
    TEString Name;
    TEString DisplayName;
    PropertyDrawFunc DrawFunc;
    size_t Offset;
    std::function<bool(void *)> Condition = nullptr; // Returns true if property should be visible
    TEString EnumName;                               // If set, this property is an enum
    std::function<TEString(void *)> SerializeFunc = nullptr;
    std::function<void(void *, const TEString &)> DeserializeFunc = nullptr;
};

using ComponentFactory = std::function<TComponent *(EntityManager *, EntityID)>;

// Factory that is given the newly created entity and can add / configure any components
using EntityPresetFactory = std::function<void(EntityID, EntityManager *)>;

struct EntityPreset
{
    TEString Name;     // Display name in the menu (e.g. "Point Light")
    TEString Category; // Optional category header (e.g. "Lights", "Shapes")
    EntityPresetFactory Create;
};

struct ComponentMetadata
{
    TEString ClassName;
    TEString DisplayName;
    ComponentFactory Factory;
    TEArray<PropertyMetadata> Properties;
    std::type_index TypeIndex = std::type_index(typeid(void));
    bool IsInternal = false; // If true, hidden from Hierarchy component tree (shown in Properties only)
};

struct EnumMetadata
{
    TEString Name;
    TEArray<std::pair<TEString, int>> Values;
};

class ComponentRegistry
{
public:
    static ComponentRegistry &Get()
    {
        static ComponentRegistry instance;
        return instance;
    }

    template <typename T> bool RegisterComponent(const TEString &className, const TEString &displayName)
    {
        auto &meta = m_Components[className];
        meta.ClassName = className;
        meta.DisplayName = displayName;
        meta.TypeIndex = std::type_index(typeid(T));
        meta.Factory = [](EntityManager *em, EntityID id) { return (TComponent *)em->AddComponent<T>(id); };
        m_TypeToName[meta.TypeIndex] = className;
        return true;
    }

    template <typename Class, typename Type>
    bool RegisterProperty(const TEString &className, const TEString &propName, const TEString &displayName,
                          Type Class::*member, std::function<bool(void *)> condition = nullptr)
    {
        auto &meta = m_Components[className];
        for (const auto &p : meta.Properties)
        {
            if (p.Name == propName)
                return true;
        }
        meta.Properties.push_back({propName, displayName,
                                   [member, propName](void *instance, const TEString &label) -> bool
                                   {
                                       TEString uiLabel = label + "###" + propName;
                                       Type *ptr = &(static_cast<Class *>(instance)->*member);
                                       return TEPropertyDrawer<Type>::Draw(ptr, uiLabel);
                                   },
                                   0, // Offset no longer used directly, but kept in struct for ABI or other uses
                                   condition, "",
                                   [member](void *instance) -> TEString
                                   {
                                       Type *ptr = &(static_cast<Class *>(instance)->*member);
                                       return TEPropertyDrawer<Type>::Serialize(ptr);
                                   },
                                   [member](void *instance, const TEString &data)
                                   {
                                       Type *ptr = &(static_cast<Class *>(instance)->*member);
                                       TEPropertyDrawer<Type>::Deserialize(ptr, data);
                                   }});
        return true;
    }

    template <typename Class, typename Type>
    bool RegisterProperty(const TEString &className, const TEString &propName, const TEString &displayName,
                          std::function<Type *(void *)> getPtr, std::function<bool(void *)> condition = nullptr)
    {
        auto &meta = m_Components[className];
        for (const auto &p : meta.Properties)
        {
            if (p.Name == propName)
                return true;
        }
        meta.Properties.push_back({propName, displayName,
                                   [getPtr, propName](void *instance, const TEString &label) -> bool
                                   {
                                       TEString uiLabel = label + "###" + propName;
                                       Type *ptr = getPtr(instance);
                                       if (ptr)
                                           return TEPropertyDrawer<Type>::Draw(ptr, uiLabel);
                                       return false;
                                   },
                                   0, condition, "",
                                   [getPtr](void *instance) -> TEString
                                   {
                                       Type *ptr = getPtr(instance);
                                       return ptr ? TEPropertyDrawer<Type>::Serialize(ptr) : "";
                                   },
                                   [getPtr](void *instance, const TEString &data)
                                   {
                                       Type *ptr = getPtr(instance);
                                       if (ptr)
                                           TEPropertyDrawer<Type>::Deserialize(ptr, data);
                                   }});
        return true;
    }

    template <typename Class, typename EnumType>
    bool RegisterEnumProperty(const TEString &className, const TEString &propName, const TEString &displayName,
                              EnumType Class::*member, const TEString &enumName)
    {
        auto &meta = m_Components[className];
        for (const auto &p : meta.Properties)
        {
            if (p.Name == propName)
                return true;
        }
        meta.Properties.push_back(
            {propName, displayName,
             [member, enumName, propName](void *instance, const TEString &label) -> bool
             {
                 bool changed = false;
                 EnumType *valPtr = &(static_cast<Class *>(instance)->*member);
                 int *val = reinterpret_cast<int *>(valPtr); // Enums are ints in our property system
                 auto enumMeta = ComponentRegistry::Get().GetEnumMetadata(enumName);
                 if (enumMeta)
                 {
                     const char *current = "Unknown";
                     for (const auto &pair : enumMeta->Values)
                     {
                         if (pair.second == *val)
                         {
                             current = pair.first.c_str();
                             break;
                         }
                     }
                     // Use ### for unique ID while keeping display label
                     TEString imguiLabel = label + "###" + propName;
                     if (TimeGUI::BeginCombo(imguiLabel.c_str(), current))
                     {
                         for (const auto &pair : enumMeta->Values)
                         {
                             bool isSelected = (pair.second == *val);
                             // Use ### with value to ensure unique ID even if names match (or are
                             // duplicated)
                             TEString selectableId = pair.first + "###" + TEString::FromInt(pair.second);
                             if (TimeGUI::Selectable(selectableId.c_str(), isSelected))
                             {
                                 *val = pair.second;
                                 changed = true;
                             }
                             if (isSelected)
                                 TimeGUI::SetItemDefaultFocus();
                         }
                         TimeGUI::EndCombo();
                     }
                 }
                 else
                 {
                     TimeGUI::Text(TEString::Format("%s: Enum %s not found", label.c_str(), enumName.c_str()));
                 }
                 return changed;
             },
             0, nullptr, enumName,
             [member](void *instance) -> TEString
             {
                 // Enums will be stored as integers
                 EnumType *valPtr = &(static_cast<Class *>(instance)->*member);
                 return TEString::FromInt(static_cast<int>(*valPtr));
             },
             [member](void *instance, const TEString &data)
             {
                 if (data.empty())
                     return;
                 EnumType *valPtr = &(static_cast<Class *>(instance)->*member);
                 *valPtr = static_cast<EnumType>(std::stoi(data));
             }});
        return true;
    }

    template <typename E> bool RegisterEnum(const TEString &name, const TEArray<std::pair<TEString, E>> &values)
    {
        auto &meta = m_Enums[name];
        if (!meta.Values.empty())
            return true; // Already registered

        meta.Name = name;
        for (const auto &[vName, value] : values)
        {
            meta.Values.push_back({vName, (int)value});
        }
        return true;
    }

    const TEMap<TEString, ComponentMetadata> &GetComponents() const { return m_Components; }

    const ComponentMetadata *GetMetadata(const TEString &className) const
    {
        auto it = m_Components.find(className);
        if (it != m_Components.end())
            return &it->second;
        return nullptr;
    }

    const ComponentMetadata *GetMetadata(std::type_index type) const
    {
        auto itN = m_TypeToName.find(type);
        if (itN != m_TypeToName.end())
            return GetMetadata(itN->second);
        return nullptr;
    }

    const EnumMetadata *GetEnumMetadata(const TEString &name) const
    {
        auto it = m_Enums.find(name);
        if (it != m_Enums.end())
            return &it->second;
        return nullptr;
    }

    TEMap<TEString, ComponentFactory> GetEntries() const
    {
        TEMap<TEString, ComponentFactory> entries;
        for (auto const &[name, meta] : m_Components)
        {
            entries[meta.DisplayName] = meta.Factory;
        }
        return entries;
    }

    bool RegisterEntityPreset(const TEString &name, const TEString &category, EntityPresetFactory factory)
    {
        // Check for duplicates (occurs if headers are included in multiple DLL/EXE modules)
        for (const auto &p : m_EntityPresets)
        {
            if (p.Name == name)
                return true;
        }

        m_EntityPresets.push_back({name, category, std::move(factory)});

        // Sort by category then name for clean grouping in the UI
        std::sort(m_EntityPresets.begin(), m_EntityPresets.end(),
                  [](const EntityPreset &a, const EntityPreset &b)
                  {
                      if (a.Category != b.Category)
                          return a.Category < b.Category;
                      return a.Name < b.Name;
                  });

        return true;
    }

    const TEArray<EntityPreset> &GetEntityPresets() const { return m_EntityPresets; }

    bool MarkInternal(const TEString &className)
    {
        auto it = m_Components.find(className);
        if (it != m_Components.end())
            it->second.IsInternal = true;
        return true;
    }

private:
    ComponentRegistry() : m_Components(), m_TypeToName(), m_Enums() {}
    TEMap<TEString, ComponentMetadata> m_Components;
    TEMap<std::type_index, TEString> m_TypeToName;
    TEMap<TEString, EnumMetadata> m_Enums;
    TEArray<EntityPreset> m_EntityPresets;
};

// Mark a component as internal (hidden from Scene Hierarchy component tree)
struct TEComponentInternalRegistrar
{
    TEComponentInternalRegistrar(const char *className) { ComponentRegistry::Get().MarkInternal(className); }
};

// Self-registering preset helper - registers once at startup
struct TEPresetRegistrar
{
    TEPresetRegistrar(const char *displayName, const TEString &category, EntityPresetFactory factory)
    {
        ComponentRegistry::Get().RegisterEntityPreset(displayName, category, std::move(factory));
    }
};

// Marker for Editor UI and base class helpers
#define GENERATED_BODY(Class)                                                                                          \
    using SelfType = Class;                                                                                            \
    inline static const TEString StaticClassName = #Class;

// Property declaration
#define T_PROPERTY(Type, Var, DName, Default, ...)                                                                     \
    Type Var = Default;                                                                                                \
    T_PROP_ACCESSORS(Type, Var, __VA_ARGS__)

// Internal helper for accessors
#define T_PROP_ACCESSORS(Type, Var, ...)                                                                               \
    T_PROP_DISPATCH(__VA_ARGS__, T_PROP_GETSET_CONST, T_PROP_GETSET, T_PROP_NONE)(Type, Var, __VA_ARGS__)

#define T_PROP_NONE(Type, Var, ...)
#define T_PROP_GETSET(Type, Var, Get, Set, ...)                                                                        \
    Type Get##Get() { return Var; }                                                                                    \
    void Set##Set(Type val) { Var = val; }
#define T_PROP_GETSET_CONST(Type, Var, Get, Set, Const, ...)                                                           \
    Type Get##Get() Const { return Var; }                                                                              \
    void Set##Set(Type val) { Var = val; }

#define T_PROP_DISPATCH(_1, _2, _3, NAME, ...) NAME

// Registration macro (must be used INSIDE the component's namespace)
#define T_REGISTER_COMPONENT(Class, DisplayName)                                                                       \
    inline static bool s_##Class##_Reg = ComponentRegistry::Get().RegisterComponent<Class>(#Class, DisplayName);

#define T_REGISTER_PROPERTY(Class, Type, Var, DName)                                                                   \
    inline static bool s_##Class##_##Var##_Reg =                                                                       \
        ComponentRegistry::Get().RegisterProperty<Class, Type>(#Class, #Var, DName, &Class::Var);

#define T_REGISTER_PROPERTY_COND(Class, Type, Var, DName, Cond)                                                        \
    inline static bool s_##Class##_##Var##_Reg =                                                                       \
        ComponentRegistry::Get().RegisterProperty<Class, Type>(#Class, #Var, DName, &Class::Var, Cond);

#define T_REGISTER_ENUM_PROPERTY(Class, EnumType, Var, DName)                                                          \
    inline static bool s_##Class##_##Var##_Reg =                                                                       \
        ComponentRegistry::Get().RegisterEnumProperty<Class, EnumType>(#Class, #Var, DName, &Class::Var, #EnumType);

// Enum registration (must be used INSIDE a namespace)
#define T_ENUM(EnumType, ...)                                                                                          \
    inline static bool EnumType##_Reg = ComponentRegistry::Get().RegisterEnum<EnumType>(#EnumType, {__VA_ARGS__});

// Mark a component as internal (hidden from Scene Hierarchy component tree)
// Use this for engine-internal components like TagComponent and TransformComponent
#define T_COMPONENT_INTERNAL(Class) inline static TEComponentInternalRegistrar s_##Class##_InternalReg(#Class);

// Register a named entity preset that appears in the Add Entity (+) popup.
// UniqueName: a globally-unique C++ identifier (used for the static var name, no quotes)
// DisplayName: readable string shown in the menu (e.g. "Point Light")
// Category: group header (e.g. "Lights", "Shapes"). Use "" for none.
// ...: lambda factory (EntityID id, EntityManager *em) -> void
#define T_REGISTER_PRESET(UniqueName, DisplayName, Category, ...)                                                      \
    inline static TEPresetRegistrar s_preset_##UniqueName##_Reg(DisplayName, Category, __VA_ARGS__);

#include "PropertyDrawers.hpp"
