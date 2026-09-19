#pragma once

#include "EngineTypes/EngineTypes.hpp"
#include <cstddef>
#include <functional>

enum class TETypeKind
{
    Primitive,
    Class,
    Struct,
    Enum
};

class TE_API TEProperty
{
public:
    TEProperty() = default;
    TEProperty(const TEString &name, const TEString &typeName, size_t offset, size_t size,
               std::function<TEString(const void *)> toStringFn = nullptr,
               std::function<bool(void *, const TEString &)> fromStringFn = nullptr)
        : m_Name(name), m_TypeName(typeName), m_Offset(offset), m_Size(size), m_ToStringFn(toStringFn),
          m_FromStringFn(fromStringFn)
    {
    }

    const TEString &GetName() const { return m_Name; }
    const TEString &GetTypeName() const { return m_TypeName; }
    size_t GetOffset() const { return m_Offset; }
    size_t GetSize() const { return m_Size; }

    template <typename T> const T &GetValue(const void *instance) const
    {
        const uint8_t *bytePtr = reinterpret_cast<const uint8_t *>(instance);
        return *reinterpret_cast<const T *>(bytePtr + m_Offset);
    }

    template <typename T> void SetValue(void *instance, const T &val) const
    {
        uint8_t *bytePtr = reinterpret_cast<uint8_t *>(instance);
        *reinterpret_cast<T *>(bytePtr + m_Offset) = val;
    }

    TEString ToString(const void *instance) const
    {
        if (m_ToStringFn)
            return m_ToStringFn(instance);
        return "";
    }

    bool FromString(void *instance, const TEString &str) const
    {
        if (m_FromStringFn)
            return m_FromStringFn(instance, str);
        return false;
    }

private:
    TEString m_Name;
    TEString m_TypeName;
    size_t m_Offset = 0;
    size_t m_Size = 0;
    std::function<TEString(const void *)> m_ToStringFn;
    std::function<bool(void *, const TEString &)> m_FromStringFn;
};

class TE_API TEClass
{
public:
    TEClass() = default;
    TEClass(const TEString &name, const TEString &parentName, size_t size,
            std::function<void *()> defaultConstructor = nullptr)
        : m_Name(name), m_ParentName(parentName), m_Size(size), m_Constructor(defaultConstructor)
    {
    }

    const TEString &GetName() const { return m_Name; }
    const TEString &GetParentName() const { return m_ParentName; }
    size_t GetSize() const { return m_Size; }

    void AddProperty(const TEProperty &prop) { m_Properties.Add(prop); }

    const TEArray<TEProperty> &GetProperties() const { return m_Properties; }

    const TEProperty *FindProperty(const TEString &name) const
    {
        for (size_t i = 0; i < m_Properties.Num(); ++i)
        {
            if (m_Properties[i].GetName() == name)
            {
                return &m_Properties[i];
            }
        }
        return nullptr;
    }

    bool IsChildOf(const TEString &parentClassName) const
    {
        if (m_ParentName.IsEmpty())
            return false;
        if (m_ParentName == parentClassName)
            return true;
        return false;
    }

    void *Instantiate() const
    {
        if (m_Constructor)
            return m_Constructor();
        return nullptr;
    }

private:
    TEString m_Name;
    TEString m_ParentName;
    size_t m_Size = 0;
    TEArray<TEProperty> m_Properties;
    std::function<void *()> m_Constructor;
};

class TE_API TEReflectionRegistry
{
public:
    static TEReflectionRegistry &Get()
    {
        static TEReflectionRegistry s_Instance;
        return s_Instance;
    }

    void RegisterType(const TEClass &cls) { m_Classes[cls.GetName()] = cls; }
    void AddClass(const TEClass &cls) { m_Classes[cls.GetName()] = cls; }

    const TEClass *FindClass(const TEString &name) const { return m_Classes.Find(name); }

    const TEMap<TEString, TEClass> &GetAllClasses() const { return m_Classes; }

    void Clear() { m_Classes.Clear(); }

private:
    TEMap<TEString, TEClass> m_Classes;
};

#define TE_REFLECT_TYPE_NAME(Type) #Type

#define TE_REGISTER_CLASS_BEGIN(ClassType)                                                                             \
    struct ClassType##_Reflector                                                                                       \
    {                                                                                                                  \
        ClassType##_Reflector()                                                                                        \
        {                                                                                                              \
            TEClass cls(#ClassType, "", sizeof(ClassType),                                                             \
                        []() -> void * { return static_cast<void *>(new ClassType()); });

#define TE_REGISTER_CLASS_BEGIN_DERIVED(ClassType, ParentType)                                                         \
    struct ClassType##_Reflector                                                                                       \
    {                                                                                                                  \
        ClassType##_Reflector()                                                                                        \
        {                                                                                                              \
            TEClass cls(#ClassType, #ParentType, sizeof(ClassType),                                                    \
                        []() -> void * { return static_cast<void *>(new ClassType()); });

#define TE_REGISTER_PROPERTY(ClassType, MemberName, MemberType)                                                        \
    cls.AddProperty(TEProperty(                                                                                        \
        #MemberName, #MemberType, offsetof(ClassType, MemberName), sizeof(MemberType),                                 \
        [](const void *inst) -> TEString                                                                               \
        {                                                                                                              \
            const ClassType *obj = reinterpret_cast<const ClassType *>(inst);                                          \
            return TEString::Format("{0}", obj->MemberName);                                                           \
        },                                                                                                             \
        [](void *inst, const TEString &str) -> bool                                                                    \
        {                                                                                                              \
            ClassType *obj = reinterpret_cast<ClassType *>(inst);                                                      \
            return true;                                                                                               \
        }));

#define TE_REGISTER_CLASS_END(ClassType)                                                                               \
    TEReflectionRegistry::Get().RegisterType(cls);                                                                     \
    }                                                                                                                  \
    }                                                                                                                  \
    ;                                                                                                                  \
    static inline ClassType##_Reflector s_##ClassType##_Reflector_Instance;
