#pragma once
#include "Core/Asset/Asset.hpp"
#include "EntityManager.hpp"

class PhysicsWorld;

class TE_API Scene : public Asset, public std::enable_shared_from_this<Scene>
{
public:
    Scene();
    Scene(const TEString &name);
    ~Scene() = default;
    Scene(const Scene &) = delete;
    Scene &operator=(const Scene &) = delete;

    virtual AssetHandle GetHandle() const override { return m_Handle; }
    void SetHandle(AssetHandle handle) { m_Handle = handle; }

    virtual const TEString &GetType() const override
    {
        static TEString type = "Scene";
        return type;
    }
    virtual const TEString &GetName() const override { return m_Name; }
    void SetName(const TEString &name) { m_Name = name; }

    virtual const TEString &GetHoverDescription() const override
    {
        static TEString desc;
        desc = "TimeEngine Scene: " + m_Name;
        return desc;
    }

    // Asset Metadata Overrides
    virtual TEString GetDefaultExtension() const override { return ".tescene"; }
    virtual TEString GetDefaultIconPath() const override { return "Resources/Editor/SceneIcon.png"; }

    // ISavable Overrides
    virtual TEString GetSavableID() const override { return "ActiveScene"; }
    virtual TEString GetSavableDisplayName() const override { return m_Name.empty() ? "Untitled Scene" : m_Name; }
    virtual TEString GetSavableType() const override { return "Scene"; }
    virtual TEString GetSavablePath() const override { return GetAssetPath(); }
    virtual void MarkDirty(bool dirty = true) override;
    virtual bool Save() override;

    virtual void OnContentBrowserCreate(const TEString &path) override;

    Entity CreateEntity(const TEString &name = "Entity");
    void DestroyEntity(Entity entity);
    void SetParent(Entity child, Entity parent);

    void OnRuntimeStart();
    void OnRuntimeStop();
    void OnUpdateRuntime(float dt);

    static TERef<Scene> Copy(TERef<Scene> other);

    EntityManager &GetEntityManager() { return m_EntityManager; }
    TERef<PhysicsWorld> GetPhysicsWorld() const { return m_PhysicsWorld; }

    static class ComponentRegistry &GetGlobalComponentRegistry();

private:
    EntityManager m_EntityManager;
    TERef<PhysicsWorld> m_PhysicsWorld;
    AssetHandle m_Handle;
    TEString m_Name;
};

