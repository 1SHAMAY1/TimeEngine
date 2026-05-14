#pragma once
#include "EntityManager.hpp"
#include "Core/Asset/Asset.hpp"
#include <memory>
#include <string>

namespace TE
{

class Scene : public Asset
{
public:
    Scene();
    Scene(const std::string& name);
    ~Scene() = default;

    virtual AssetHandle GetHandle() const override { return m_Handle; }
    virtual const std::string& GetType() const override { static std::string type = "Scene"; return type; }
    virtual const std::string& GetName() const override { return m_Name; }
    virtual const std::string& GetHoverDescription() const override 
    { 
        static std::string desc;
        desc = "TimeEngine Scene: " + m_Name;
        return desc; 
    }

    // Asset Metadata Overrides
    virtual std::string GetDefaultExtension() const override { return ".tescene"; }
    virtual std::string GetDefaultIconPath() const override { return "Resources/Editor/SceneIcon.png"; }
    
    virtual void OnContentBrowserCreate(const std::filesystem::path& path) override;

    Entity CreateEntity(const std::string &name = "Entity");
    void DestroyEntity(Entity entity);
    void SetParent(Entity child, Entity parent);

    EntityManager &GetEntityManager() { return m_EntityManager; }

private:
    EntityManager m_EntityManager;
    AssetHandle m_Handle;
    std::string m_Name;
};

} // namespace TE
