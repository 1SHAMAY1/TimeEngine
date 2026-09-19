#include "PreRequisites.h"
#include "Editor/DragDrop/StandardDragDropHandlers.hpp"
#include "Editor/DragDrop/DragDropRegistry.hpp"
#include "Editor/DragDrop/DragDropManager.hpp"
#include "EditorSaveManager.hpp"
#include "Layers/EditorLayer.hpp"

#include "Scene.hpp"
#include "SceneSerializer.hpp"
#include "EntityManager.hpp"
#include "SpriteComponent.hpp"
#include "TransformComponent.hpp"
#include "Utils/TEFileSystem.hpp"
#include "Log.h"

// Auto-register built-in standard handlers with the registry
TE_REGISTER_DRAG_DROP_HANDLER(SceneViewportDropHandler)
TE_REGISTER_DRAG_DROP_HANDLER(Sprite2DViewportDropHandler)
TE_REGISTER_DRAG_DROP_HANDLER(HierarchyReparentDropHandler)
TE_REGISTER_DRAG_DROP_HANDLER(OSFileImportDropHandler)

// =============================================================================
// Scene Viewport Drop Handler
// =============================================================================
bool SceneViewportDropHandler::CanHandle(const DragDropContext &context, const DragDropPayload &payload) const
{
    if (!payload.IsType(TE_DND_ASSET_PATH))
        return false;

    TEString path = payload.AsString();
    return DragDropManager::ValidateExtension(path, ".tescene");
}

TEString SceneViewportDropHandler::GetTooltipText(const DragDropContext &context, const DragDropPayload &payload) const
{
    TEString path = payload.AsString();
    return "Open Scene: " + path.GetFilename();
}

bool SceneViewportDropHandler::OnDrop(const DragDropContext &context, const DragDropPayload &payload)
{
    if (!context.Editor)
        return false;

    TEString path = payload.AsString();
    TE_CORE_INFO("SceneViewportDropHandler: Loading scene from drop '{0}'", path);

    auto loadSceneAction = [editor = context.Editor, path]()
    {
        auto newScene = CreateRef<Scene>();
        SceneSerializer serializer(newScene);
        if (serializer.Deserialize(path))
        {
            newScene->SetName(path.GetStem());
            newScene->SetAssetPath(path);
            editor->SetActiveScene(newScene);
            editor->ClearSelection();
            EditorSaveManager::RegisterSavable(newScene);
            TE_CORE_INFO("SceneViewportDropHandler: Successfully loaded scene '{0}'", path);
        }
        else
        {
            TE_CORE_ERROR("SceneViewportDropHandler: Failed to deserialize scene '{0}'", path);
        }
    };

    auto activeScene = context.Editor->GetActiveScene();
    if (activeScene && activeScene->IsDirty())
    {
        EditorSaveManager::RequestSavePromptWithAction(loadSceneAction);
    }
    else
    {
        loadSceneAction();
    }

    return true;
}

// =============================================================================
// Sprite 2D Viewport Drop Handler
// =============================================================================
bool Sprite2DViewportDropHandler::CanHandle(const DragDropContext &context, const DragDropPayload &payload) const
{
    if (!payload.IsType(TE_DND_ASSET_PATH))
        return false;

    TEString path = payload.AsString();
    return DragDropManager::ValidateExtension(path, ".png,.jpg,.jpeg,.tga,.bmp");
}

TEString Sprite2DViewportDropHandler::GetTooltipText(const DragDropContext &context,
                                                     const DragDropPayload &payload) const
{
    TEString path = payload.AsString();
    return "Spawn Sprite: " + path.GetFilename();
}

bool Sprite2DViewportDropHandler::OnDrop(const DragDropContext &context, const DragDropPayload &payload)
{
    auto scene = context.ActiveScene;
    if (!scene)
        return false;

    TEString path = payload.AsString();
    TEString entityName = path.GetStem();
    if (entityName.empty())
        entityName = "Sprite";

    Entity entity = scene->CreateEntity(entityName);
    if (!entity.IsValid())
        return false;

    // Add / configure sprite component
    auto *sprite = entity.AddComponent<SpriteComponent>();
    if (sprite)
    {
        sprite->TexturePath = path;
    }

    // Position at camera center if available
    if (context.Editor && entity.HasComponent<TransformComponent>())
    {
        auto &transform = entity.GetTransform();
        TEVector camPos = context.Editor->GetCameraPosition();
        transform.Position = TEVector(camPos.x, camPos.y, 0.0f);
    }

    if (context.Editor)
    {
        context.Editor->SelectEntity(entity);
    }

    TE_CORE_INFO("Sprite2DViewportDropHandler: Created sprite entity '{0}' for '{1}'", entityName, path);
    return true;
}

// =============================================================================
// Hierarchy Reparent Drop Handler
// =============================================================================
bool HierarchyReparentDropHandler::CanHandle(const DragDropContext &context, const DragDropPayload &payload) const
{
    if (!payload.IsType(TE_DND_ENTITY_ID) && !payload.IsType(TE_DND_ENTITY_LIST))
        return false;

    auto scene = context.ActiveScene;
    if (!scene)
        return false;

    uint64_t targetEntityId = (uint64_t)(uintptr_t)context.TargetUserData;

    if (payload.IsType(TE_DND_ENTITY_ID))
    {
        uint64_t draggedId = payload.AsEntityID();
        if (draggedId == 0 || draggedId == targetEntityId)
            return false;

        // Prevent circular parenting (cannot parent into its own descendant)
        if (targetEntityId != 0 && DragDropManager::IsDescendantOf(scene, targetEntityId, draggedId))
            return false;
    }
    else if (payload.IsType(TE_DND_ENTITY_LIST))
    {
        auto entityList = payload.AsEntityIDArray();
        for (size_t i = 0; i < entityList.Size(); ++i)
        {
            uint64_t draggedId = entityList[i];
            if (draggedId == 0 || draggedId == targetEntityId)
                return false;
            if (targetEntityId != 0 && DragDropManager::IsDescendantOf(scene, targetEntityId, draggedId))
                return false;
        }
    }

    return true;
}

TEString HierarchyReparentDropHandler::GetTooltipText(const DragDropContext &context,
                                                      const DragDropPayload &payload) const
{
    uint64_t targetEntityId = (uint64_t)(uintptr_t)context.TargetUserData;
    if (targetEntityId == 0)
        return "Move to Scene Root";

    return "Parent under Entity " + TEString::FromInt((int)targetEntityId);
}

bool HierarchyReparentDropHandler::OnDrop(const DragDropContext &context, const DragDropPayload &payload)
{
    auto scene = context.ActiveScene;
    if (!scene)
        return false;

    EntityManager &em = scene->GetEntityManager();
    uint64_t targetEntityId = (uint64_t)(uintptr_t)context.TargetUserData;
    auto entityList = payload.AsEntityIDArray();

    Entity target = targetEntityId != 0 ? Entity(targetEntityId, &em) : Entity();

    for (size_t i = 0; i < entityList.Size(); ++i)
    {
        uint64_t draggedId = entityList[i];
        if (!em.IsValid(draggedId))
            continue;

        Entity dragged(draggedId, &em);
        scene->SetParent(dragged, target);
    }

    TE_CORE_INFO("HierarchyReparentDropHandler: Reparented {0} entity/entities under target {1}", entityList.Size(),
                 targetEntityId);
    return true;
}

// =============================================================================
// OS File Import Drop Handler
// =============================================================================
bool OSFileImportDropHandler::CanHandle(const DragDropContext &context, const DragDropPayload &payload) const
{
    if (!payload.IsType(TE_DND_OS_FILES))
        return false;

    auto paths = payload.AsPathArray();
    return !paths.IsEmpty();
}

TEString OSFileImportDropHandler::GetTooltipText(const DragDropContext &context, const DragDropPayload &payload) const
{
    auto paths = payload.AsPathArray();
    return "Import " + TEString::FromInt((int)paths.Size()) + " file(s)";
}

bool OSFileImportDropHandler::OnDrop(const DragDropContext &context, const DragDropPayload &payload)
{
    auto paths = payload.AsPathArray();
    if (paths.IsEmpty())
        return false;

    TEString targetFolder = "";
    if (context.TargetUserData)
    {
        targetFolder = *(const TEString *)context.TargetUserData;
    }

    if (targetFolder.empty() || !TEFileSystem::Exists(targetFolder))
    {
        targetFolder = "Assets";
    }

    for (size_t i = 0; i < paths.Size(); ++i)
    {
        TEString src = paths[i];
        if (!TEFileSystem::Exists(src))
            continue;

        TEString dest = targetFolder / src.GetFilename();
        TEFileSystem::CopyFile(src, dest, true);
        TE_CORE_INFO("OSFileImportDropHandler: Imported '{0}' -> '{1}'", src, dest);
    }

    return true;
}

// Auto-register FolderAssetMoveDropHandler
TE_REGISTER_DRAG_DROP_HANDLER(FolderAssetMoveDropHandler)

// =============================================================================
// Folder Asset Move Drop Handler
// =============================================================================
bool FolderAssetMoveDropHandler::CanHandle(const DragDropContext &context, const DragDropPayload &payload) const
{
    if (!payload.IsType(TE_DND_ASSET_PATH) && !payload.IsType(TE_DND_CONTENT_BROWSER_ITEMS) &&
        !payload.IsType(TE_DND_OS_FILES))
        return false;

    if (!context.TargetUserData)
        return false;

    TEString targetFolder = *(const TEString *)context.TargetUserData;
    if (targetFolder.empty() || !TEFileSystem::Exists(targetFolder) || !TEFileSystem::IsDirectory(targetFolder))
        return false;

    if (payload.IsType(TE_DND_ASSET_PATH))
    {
        TEString srcPath = payload.AsString();
        if (srcPath.empty() || srcPath == targetFolder)
            return false;
        // Don't drop folder into itself or its immediate parent
        if (srcPath.GetParentPath() == targetFolder)
            return false;
        if (targetFolder.StartsWith(srcPath))
            return false;
    }

    return true;
}

TEString FolderAssetMoveDropHandler::GetTooltipText(const DragDropContext &context,
                                                    const DragDropPayload &payload) const
{
    TEString targetFolder = context.TargetUserData ? *(const TEString *)context.TargetUserData : "";
    return "Move into " + targetFolder.GetFilename();
}

bool FolderAssetMoveDropHandler::OnDrop(const DragDropContext &context, const DragDropPayload &payload)
{
    if (!context.TargetUserData)
        return false;

    TEString targetFolder = *(const TEString *)context.TargetUserData;
    if (targetFolder.empty() || !TEFileSystem::Exists(targetFolder))
        return false;

    TEArray<TEString> pathsToMove;
    if (payload.IsType(TE_DND_ASSET_PATH))
    {
        pathsToMove.Add(payload.AsString());
    }
    else if (payload.IsType(TE_DND_CONTENT_BROWSER_ITEMS) || payload.IsType(TE_DND_OS_FILES))
    {
        pathsToMove = payload.AsPathArray();
    }

    for (size_t i = 0; i < pathsToMove.Size(); ++i)
    {
        TEString src = pathsToMove[i];
        if (!TEFileSystem::Exists(src))
            continue;

        TEString dest = targetFolder / src.GetFilename();
        if (src == dest)
            continue;

        if (TEFileSystem::IsDirectory(src))
        {
            // Directory move: create dest dir, copy contents, remove src
            TEFileSystem::CreateDirectories(dest);
            auto files = TEFileSystem::GetFiles(src, "", true);
            for (const auto &f : files)
            {
                TEString rel = f.Mid(src.Length());
                while (rel.StartsWith("/") || rel.StartsWith("\\"))
                    rel = rel.Mid(1);
                TEString subDest = dest / rel;
                TEFileSystem::CreateDirectories(subDest.GetParentPath());
                TEFileSystem::CopyFile(f, subDest, true);
            }
            TEFileSystem::RemoveAll(src);
        }
        else
        {
            TEFileSystem::CopyFile(src, dest, true);
            TEFileSystem::Remove(src);
        }

        TE_CORE_INFO("FolderAssetMoveDropHandler: Moved '{0}' -> '{1}'", src, dest);
    }

    return true;
}
