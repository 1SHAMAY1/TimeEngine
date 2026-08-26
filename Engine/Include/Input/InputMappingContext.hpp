#pragma once
#include "Core/Asset/Asset.hpp"
#include "Core/Asset/AssetManager.hpp"
#include "InputAction.hpp"
#include "Utils/TEFileSystem.hpp"

// Custom Input Binding codes including Keyboard, Mouse Buttons, and Mouse Axes
enum class EInputBindingType : uint8_t
{
    Keyboard = 0,
    MouseButton = 1,
    MouseAxis = 2
};

struct InputModifier
{
    bool ScaleNegate = false;
    float Multiplier = 1.0f;
    float Deadzone = 0.05f;
};

struct ActionKeyMapping
{
    InputAction Action;
    EInputBindingType BindingType = EInputBindingType::Keyboard;
    uint32_t Code = 0; // KeyCode or MouseCode or MouseAxisIndex (0: Mouse X, 1: Mouse Y, 2: Mouse Scroll)
    bool IsRemappable = true;
    InputModifier Modifier;
};

class TE_API InputMappingContext : public Asset
{
public:
    InputMappingContext() = default;
    InputMappingContext(const TEString &name) : ContextName(name) {}
    virtual ~InputMappingContext() override = default;

    TEString ContextName = "NewInputMappingContext";
    TEArray<ActionKeyMapping> Mappings;

    void AddMapping(const ActionKeyMapping &mapping) { Mappings.Add(mapping); }
    void RemoveMapping(size_t index) { Mappings.RemoveAt(index); }

    // Asset Interface Overrides
    virtual AssetHandle GetHandle() const override { return m_Handle; }
    virtual const TEString &GetType() const override
    {
        static TEString type = "InputMappingContext";
        return type;
    }
    virtual const TEString &GetName() const override { return ContextName; }
    virtual const TEString &GetHoverDescription() const override { return ContextName; }
    virtual TEString GetDefaultExtension() const override { return ".teimc"; }
    virtual TERef<Asset> Clone() const override { return CreateRef<InputMappingContext>(*this); }

    bool SaveToFile(const TEString &filepath) const;
    virtual bool LoadFromFile(const TEString &filepath) override;

    virtual void OnContentBrowserCreate(const TEString &path) override
    {
        TEString basePath = path / "NewInputMappingContext.teimc";
        TEString finalPath = basePath;
        int counter = 1;
        while (TEFileSystem::Exists(finalPath))
        {
            finalPath = path / ("NewInputMappingContext (" + TEString::FromInt(counter) + ").teimc");
            counter++;
        }
        ContextName = finalPath.GetStem();
        SaveToFile(finalPath);
    }

private:
    AssetHandle m_Handle = 0;
};
