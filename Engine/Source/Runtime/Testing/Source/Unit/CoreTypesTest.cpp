#include "PreRequisites.h"
#include "TestHarness.hpp"
#include "TestRegistry.hpp"
#include "GameplayUtils.hpp"
#include "TagComponent.hpp"
#include "TransformComponent.hpp"
#include "MathUtils.hpp"
#include "MathAPI.hpp"
#include "MathEngine.hpp"
#include "EngineTypes/TEString.hpp"
#include "AssetManager.hpp"
#include "FontAsset.hpp"
#include "Reflection/TEReflection.hpp"

TE_TEST_CASE(CoreTypes, TEStringOperations)
{
    TEString str = "Hello, TimeEngine!";
    TE_CHECK_EQ(str.length(), 18);
    TE_CHECK(!str.IsEmpty());
    TE_CHECK(str.Contains("TimeEngine"));
    TE_CHECK(str.StartsWith("Hello"));
    TE_CHECK(str.EndsWith("Engine!"));

    TEString sub = str.Substr(7, 10);
    TE_CHECK_EQ(sub, "TimeEngine");

    TEString formatted = TEString("Value: ") + TEString::FromInt(42);
    TE_CHECK_EQ(formatted, "Value: 42");

    TEString emptyStr;
    TE_CHECK(emptyStr.IsEmpty());
}

TE_TEST_CASE(CoreTypes, TEArrayOperations)
{
    TEArray<int> arr;
    TE_CHECK(arr.IsEmpty());
    TE_CHECK_EQ(arr.Num(), 0);

    for (int i = 0; i < 100; ++i)
    {
        arr.Add(i * 2);
    }

    TE_CHECK_EQ(arr.Num(), 100);
    TE_CHECK_EQ(arr[0], 0);
    TE_CHECK_EQ(arr[50], 100);
    TE_CHECK_EQ(arr[99], 198);

    arr.RemoveAt(0);
    TE_CHECK_EQ(arr.Num(), 99);
    TE_CHECK_EQ(arr[0], 2);

    arr.Clear();
    TE_CHECK(arr.IsEmpty());
    TE_CHECK_EQ(arr.Num(), 0);
}

TE_TEST_CASE(CoreTypes, TEOptionAndTEResult)
{
    TEOption<int> optSome = 123;
    TE_CHECK(optSome.has_value());
    TE_CHECK_EQ(optSome.value(), 123);

    TEOption<int> optNone;
    TE_CHECK(!optNone.has_value());
    TE_CHECK_EQ(optNone.value_or(999), 999);

    TEResult<TEString, int> okRes = TEString("Success");
    TE_CHECK(okRes.has_value());
    TE_CHECK_EQ(okRes.value(), "Success");

    TEResult<TEString, int> errRes = TEUnexpected<int>(404);
    TE_CHECK(!errRes.has_value());
    TE_CHECK_EQ(errRes.error(), 404);
}

TE_TEST_CASE(CoreTypes, MathUtilsTransformations)
{
    TEVector2 v2a(3.0f, 4.0f);
    TE_CHECK_EQ(v2a.x, 3.0f);
    TE_CHECK_EQ(v2a.y, 4.0f);
    TE_CHECK_EQ(v2a.Length(), 5.0f);

    TEVector v3a(1.0f, 0.0f, 0.0f);
    TEVector v3b(0.0f, 1.0f, 0.0f);
    float dot = Dot(TEVector2(v3a.x, v3a.y), TEVector2(v3b.x, v3b.y));
    TE_CHECK_EQ(dot, 0.0f);

    TEMatrix4 identity(1.0f);
    TEMatrix4 translation = TEMatrix4::Translate(identity, TEVector(10.0f, 20.0f, 30.0f));
    TEVector4 point(0.0f, 0.0f, 0.0f, 1.0f);
    TEVector4 transformed = translation * point;
    TE_CHECK_EQ(transformed.x, 10.0f);
    TE_CHECK_EQ(transformed.y, 20.0f);
    TE_CHECK_EQ(transformed.z, 30.0f);
}

#include "AnimatedSpriteComponent.hpp"
#include "EntityManager.hpp"
#include "Scene.hpp"
#include "SpriteComponent.hpp"
#include "Sprite.hpp"
#include "SpriteSerializer.hpp"
#include "SpriteSheet.hpp"
#include "SpriteSheetSerializer.hpp"
#include "Texture.hpp"
#include "TextureSerializer.hpp"
#include "Utils/TEFileSystem.hpp"

TE_TEST_CASE(TwoDAssetPipeline, SpritePropertiesAndSerialization)
{
    auto sprite = CreateRef<Sprite>();
    sprite->SetName("Hero_Idle");
    sprite->SetUVs(0.1f, 0.2f, 0.9f, 0.8f);
    sprite->SetPivot(0.5f, 0.0f);
    sprite->SetPixelsPerUnit(64.0f);

    float u0, v0, u1, v1;
    sprite->GetUVs(u0, v0, u1, v1);
    TE_CHECK_EQ(u0, 0.1f);
    TE_CHECK_EQ(v0, 0.2f);
    TE_CHECK_EQ(u1, 0.9f);
    TE_CHECK_EQ(v1, 0.8f);

    float px, py;
    sprite->GetPivot(px, py);
    TE_CHECK_EQ(px, 0.5f);
    TE_CHECK_EQ(py, 0.0f);
    TE_CHECK_EQ(sprite->GetPixelsPerUnit(), 64.0f);

    // Collider Points
    sprite->GenerateAutoContourCollider(0.1f);
    TE_CHECK_EQ(sprite->GetCustomColliderPoints().Num(), 4);

    // Serialization & Deserialization
    TEString testFile = "temp_test_hero.tesprite";
    SpriteSerializer serializer(sprite);
    bool serialized = serializer.Serialize(testFile);
    TE_CHECK(serialized);

    auto loadedSprite = CreateRef<Sprite>();
    SpriteSerializer deserializer(loadedSprite);
    bool deserialized = deserializer.Deserialize(testFile);
    TE_CHECK(deserialized);
    TE_CHECK_EQ(loadedSprite->GetName(), "Hero_Idle");
    TE_CHECK_EQ(loadedSprite->GetPixelsPerUnit(), 64.0f);

    float lu0, lv0, lu1, lv1;
    loadedSprite->GetUVs(lu0, lv0, lu1, lv1);
    TE_CHECK_EQ(lu0, 0.1f);
    TE_CHECK_EQ(lv0, 0.2f);
    TE_CHECK_EQ(lu1, 0.9f);
    TE_CHECK_EQ(lv1, 0.8f);

    if (TEFileSystem::Exists(testFile))
    {
        TEFileSystem::Remove(testFile);
    }
}

TE_TEST_CASE(TwoDAssetPipeline, SpriteSheetGridSlicingAndSerialization)
{
    auto sheet = CreateRef<SpriteSheet>();
    sheet->SetName("Dino_Atlas");
    sheet->SetGridSettings(32, 32, 2, 2, 0, 0);

    TE_CHECK_EQ(sheet->GetCellWidth(), 32);
    TE_CHECK_EQ(sheet->GetCellHeight(), 32);
    TE_CHECK_EQ(sheet->GetPaddingX(), 2);
    TE_CHECK_EQ(sheet->GetPaddingY(), 2);

    // Add Animation Sequences
    AnimSequence walkAnim;
    walkAnim.Name = "Walk";
    walkAnim.FPS = 12.0f;
    walkAnim.FrameIndices.Add(0);
    walkAnim.FrameIndices.Add(1);
    walkAnim.FrameIndices.Add(2);
    walkAnim.FrameIndices.Add(3);
    sheet->GetAnimations().Add(walkAnim);

    TE_CHECK_EQ(sheet->GetAnimations().Num(), 1);
    TE_CHECK_EQ(sheet->GetAnimations()[0].Name, "Walk");
    TE_CHECK_EQ(sheet->GetAnimations()[0].FPS, 12.0f);
    TE_CHECK_EQ(sheet->GetAnimations()[0].FrameIndices.Num(), 4);

    // Test Serialization
    TEString testSheetFile = "temp_test_dino.tespritesheet";
    SpriteSheetSerializer serializer(sheet);
    bool saved = serializer.Serialize(testSheetFile);
    TE_CHECK(saved);

    auto loadedSheet = CreateRef<SpriteSheet>();
    SpriteSheetSerializer deserializer(loadedSheet);
    bool loaded = deserializer.Deserialize(testSheetFile);
    TE_CHECK(loaded);
    TE_CHECK_EQ(loadedSheet->GetName(), "Dino_Atlas");
    TE_CHECK_EQ(loadedSheet->GetCellWidth(), 32);
    TE_CHECK_EQ(loadedSheet->GetCellHeight(), 32);
    TE_CHECK_EQ(loadedSheet->GetAnimations().Num(), 1);
    TE_CHECK_EQ(loadedSheet->GetAnimations()[0].Name, "Walk");

    if (TEFileSystem::Exists(testSheetFile))
    {
        TEFileSystem::Remove(testSheetFile);
    }
}

TE_TEST_CASE(TwoDAssetPipeline, AnimatedSpriteComponentLifecycleAndTicking)
{
    auto scene = CreateRef<Scene>("AnimTestScene");
    Entity entity = scene->CreateEntity("AnimatedHero");

    auto *animComp = entity.AddComponent<AnimatedSpriteComponent>();
    TE_CHECK(animComp != nullptr);

    AnimationClip runClip;
    runClip.Name = "Run";
    runClip.FrameDuration = 0.1f;
    runClip.FrameIndices.Add(0);
    runClip.FrameIndices.Add(1);
    runClip.FrameIndices.Add(2);
    runClip.FrameIndices.Add(3);
    runClip.Loop = true;

    animComp->AddClip(runClip);
    animComp->Play("Run");

    TE_CHECK(animComp->IsPlaying());
    TE_CHECK_EQ(animComp->CurrentClipName, "Run");
    TE_CHECK_EQ(animComp->GetCurrentFrameIndex(), 0);

    // Tick by 0.05s (less than 1 frame duration of 0.1s)
    animComp->Tick(0.05f);
    TE_CHECK_EQ(animComp->GetCurrentFrameIndex(), 0);

    // Tick by another 0.06s (total 0.11s > 0.1s -> advances to frame index 1)
    animComp->Tick(0.06f);
    TE_CHECK_EQ(animComp->GetCurrentFrameIndex(), 1);

    // Tick through to loop back
    animComp->Tick(0.1f); // frame index 2
    TE_CHECK_EQ(animComp->GetCurrentFrameIndex(), 2);
    animComp->Tick(0.1f); // frame index 3
    TE_CHECK_EQ(animComp->GetCurrentFrameIndex(), 3);
    animComp->Tick(0.1f); // loops back to frame index 0
    TE_CHECK_EQ(animComp->GetCurrentFrameIndex(), 0);

    animComp->Stop();
    TE_CHECK(!animComp->IsPlaying());
}

TE_TEST_CASE(TwoDAssetPipeline, SpriteComponentProperties)
{
    auto scene = CreateRef<Scene>("SpriteTestScene");
    Entity entity = scene->CreateEntity("StaticSpriteActor");

    auto *spriteComp = entity.AddComponent<SpriteComponent>();
    TE_CHECK(spriteComp != nullptr);

    spriteComp->TexturePath = "Assets/Sprites/Hero.png";
    spriteComp->Color = TEColor(1.0f, 0.5f, 0.2f, 1.0f);

    TE_CHECK_EQ(spriteComp->TexturePath, "Assets/Sprites/Hero.png");
    TE_CHECK_EQ(spriteComp->Color.r, 1.0f);
    TE_CHECK_EQ(spriteComp->Color.g, 0.5f);
    TE_CHECK_EQ(spriteComp->Color.b, 0.2f);
}

// ==========================================
// Spatial Sweep Tests
// ==========================================
#include "PhysicsWorld.hpp"

TE_TEST_CASE(SpatialSweep, SweepLineHitAndMiss)
{
    PhysicsWorld world;

    // Static AABB body centred at (5, 0) with half-extents (1, 1)
    RigidBody body;
    body.Position = TEVector2(5.0f, 0.0f);
    body.IsStatic = true;
    body.Shape = CollisionShape(BoundsAABB(TEVector2(-1.0f, -1.0f), TEVector2(1.0f, 1.0f)));
    world.AddBody(&body);

    // Sweep along X — should hit
    TESpatialHitResult hit = world.SweepLine(TEVector2(0.0f, 0.0f), TEVector2(10.0f, 0.0f));
    TE_CHECK(hit.Hit);
    TE_CHECK(hit.Fraction >= 0.0f);
    TE_CHECK(hit.Fraction <= 1.0f);
    TE_CHECK(static_cast<bool>(hit));

    // Sweep above the box — should miss
    TESpatialHitResult miss = world.SweepLine(TEVector2(0.0f, 5.0f), TEVector2(10.0f, 5.0f));
    TE_CHECK(!miss.Hit);
    TE_CHECK(!static_cast<bool>(miss));
}

TE_TEST_CASE(SpatialSweep, MultiSweepLineMultipleHits)
{
    PhysicsWorld world;

    // First box centred at Position=(3, 0) with extents [-1, 1]x[-0.5, 0.5] (spans x=2..4)
    RigidBody bodyA;
    bodyA.Position = TEVector2(3.0f, 0.0f);
    bodyA.IsStatic = true;
    bodyA.Shape = CollisionShape(BoundsAABB(TEVector2(-1.0f, -0.5f), TEVector2(1.0f, 0.5f)));
    world.AddBody(&bodyA);

    // Second box centred at Position=(7, 0) with extents [-1, 1]x[-0.5, 0.5] (spans x=6..8)
    RigidBody bodyB;
    bodyB.Position = TEVector2(7.0f, 0.0f);
    bodyB.IsStatic = true;
    bodyB.Shape = CollisionShape(BoundsAABB(TEVector2(-1.0f, -0.5f), TEVector2(1.0f, 0.5f)));
    world.AddBody(&bodyB);

    TEArray<TESpatialHitResult> hits = world.MultiSweepLine(TEVector2(0.0f, 0.0f), TEVector2(10.0f, 0.0f));

    TE_CHECK(hits.Num() >= 2);

    // Results must be sorted ascending by Fraction
    for (size_t i = 1; i < hits.Num(); ++i)
    {
        TE_CHECK(hits[i].Fraction >= hits[i - 1].Fraction);
    }

    // First hit must be closer than second
    TE_CHECK(hits[0].Fraction < hits[1].Fraction);
}

TE_TEST_CASE(SpatialSweep, MultiSweepLineMiss)
{
    PhysicsWorld world;

    RigidBody body;
    body.Position = TEVector2(5.0f, 10.0f);
    body.IsStatic = true;
    body.Shape = CollisionShape(BoundsAABB(TEVector2(-1.0f, -1.0f), TEVector2(1.0f, 1.0f)));
    world.AddBody(&body);

    // Sweep at y=0 — completely misses the box at y=10
    TEArray<TESpatialHitResult> hits = world.MultiSweepLine(TEVector2(0.0f, 0.0f), TEVector2(10.0f, 0.0f));

    TE_CHECK(hits.IsEmpty());
}

TE_TEST_CASE(SpatialSweep, SweepCircleHit)
{
    PhysicsWorld world;

    // Circle body centred at Position=(5, 0) with local center (0, 0) and radius 1
    RigidBody body;
    body.Position = TEVector2(5.0f, 0.0f);
    body.IsStatic = true;
    body.Shape = CollisionShape(BoundsCircle(TEVector2(0.0f, 0.0f), 1.0f));
    world.AddBody(&body);

    // Uniform circle sweep (radius 1) sweeping left-to-right
    TESpatialHitResult hit = world.SweepCircle(TEVector2(0.0f, 0.0f), TEVector2(10.0f, 0.0f), TEVector2(1.0f, 1.0f));

    TE_CHECK(hit.Hit);
    TE_CHECK(hit.Fraction >= 0.0f);
    TE_CHECK(hit.Fraction <= 1.0f);
}

TE_TEST_CASE(SpatialSweep, SweepCircleEllipseHit)
{
    PhysicsWorld world;

    // AABB target at Position=(5, 0)
    RigidBody body;
    body.Position = TEVector2(5.0f, 0.0f);
    body.IsStatic = true;
    body.Shape = CollisionShape(BoundsAABB(TEVector2(-1.0f, -1.0f), TEVector2(1.0f, 1.0f)));
    world.AddBody(&body);

    // Non-uniform (ellipse) radii — wider on X axis
    TESpatialHitResult hit = world.SweepCircle(TEVector2(0.0f, 0.0f), TEVector2(10.0f, 0.0f), TEVector2(2.0f, 1.0f));

    TE_CHECK(hit.Hit);
    TE_CHECK(hit.Fraction >= 0.0f);
    TE_CHECK(hit.Fraction <= 1.0f);
}

TE_TEST_CASE(SpatialSweep, MultiSweepCircleAllHits)
{
    PhysicsWorld world;

    RigidBody bodyA;
    bodyA.Position = TEVector2(3.0f, 0.0f);
    bodyA.IsStatic = true;
    bodyA.Shape = CollisionShape(BoundsCircle(TEVector2(0.0f, 0.0f), 0.5f));
    world.AddBody(&bodyA);

    RigidBody bodyB;
    bodyB.Position = TEVector2(7.0f, 0.0f);
    bodyB.IsStatic = true;
    bodyB.Shape = CollisionShape(BoundsCircle(TEVector2(0.0f, 0.0f), 0.5f));
    world.AddBody(&bodyB);

    TEArray<TESpatialHitResult> hits =
        world.MultiSweepCircle(TEVector2(0.0f, 0.0f), TEVector2(10.0f, 0.0f), TEVector2(0.5f, 0.5f));

    TE_CHECK(hits.Num() >= 2);

    for (size_t i = 1; i < hits.Num(); ++i)
    {
        TE_CHECK(hits[i].Fraction >= hits[i - 1].Fraction);
    }
}

TE_TEST_CASE(SpatialSweep, SweepBoxAxisAligned)
{
    PhysicsWorld world;

    RigidBody body;
    body.Position = TEVector2(5.0f, 0.0f);
    body.IsStatic = true;
    body.Shape = CollisionShape(BoundsAABB(TEVector2(-1.0f, -1.0f), TEVector2(1.0f, 1.0f)));
    world.AddBody(&body);

    // 0° rotation — pure AABB box sweep
    TESpatialHitResult hit = world.SweepBox(TEVector2(0.0f, 0.0f), TEVector2(10.0f, 0.0f), TEVector2(0.5f, 0.5f), 0.0f);

    TE_CHECK(hit.Hit);
    TE_CHECK(hit.Fraction >= 0.0f);
    TE_CHECK(hit.Fraction <= 1.0f);
}

TE_TEST_CASE(SpatialSweep, SweepBoxRotated45)
{
    PhysicsWorld world;

    RigidBody body;
    body.Position = TEVector2(5.0f, 0.0f);
    body.IsStatic = true;
    body.Shape = CollisionShape(BoundsAABB(TEVector2(-1.5f, -1.5f), TEVector2(1.5f, 1.5f)));
    world.AddBody(&body);

    constexpr float k45Rad = 0.7853981633974483f; // pi / 4
    TESpatialHitResult hit =
        world.SweepBox(TEVector2(0.0f, 0.0f), TEVector2(10.0f, 0.0f), TEVector2(1.0f, 1.0f), k45Rad);

    TE_CHECK(hit.Hit);
    TE_CHECK(hit.Fraction >= 0.0f);
    TE_CHECK(hit.Fraction <= 1.0f);
}

TE_TEST_CASE(SpatialSweep, SweepBoxRotated90)
{
    PhysicsWorld world;

    RigidBody body;
    body.Position = TEVector2(5.0f, 0.0f);
    body.IsStatic = true;
    body.Shape = CollisionShape(BoundsAABB(TEVector2(-1.0f, -2.0f), TEVector2(1.0f, 2.0f)));
    world.AddBody(&body);

    constexpr float k90Rad = 1.5707963267948966f; // pi / 2
    TESpatialHitResult hit =
        world.SweepBox(TEVector2(0.0f, 0.0f), TEVector2(10.0f, 0.0f), TEVector2(2.0f, 0.5f), k90Rad);

    TE_CHECK(hit.Hit);
    TE_CHECK(hit.Fraction >= 0.0f);
    TE_CHECK(hit.Fraction <= 1.0f);
}

TE_TEST_CASE(SpatialSweep, MultiSweepBoxAllHits)
{
    PhysicsWorld world;

    RigidBody bodyA;
    bodyA.Position = TEVector2(3.0f, 0.0f);
    bodyA.IsStatic = true;
    bodyA.Shape = CollisionShape(BoundsAABB(TEVector2(-0.5f, -0.5f), TEVector2(0.5f, 0.5f)));
    world.AddBody(&bodyA);

    RigidBody bodyB;
    bodyB.Position = TEVector2(7.0f, 0.0f);
    bodyB.IsStatic = true;
    bodyB.Shape = CollisionShape(BoundsAABB(TEVector2(-0.5f, -0.5f), TEVector2(0.5f, 0.5f)));
    world.AddBody(&bodyB);

    TEArray<TESpatialHitResult> hits =
        world.MultiSweepBox(TEVector2(0.0f, 0.0f), TEVector2(10.0f, 0.0f), TEVector2(0.5f, 0.5f), 0.0f);

    TE_CHECK(hits.Num() >= 2);

    for (size_t i = 1; i < hits.Num(); ++i)
    {
        TE_CHECK(hits[i].Fraction >= hits[i - 1].Fraction);
    }
}

TE_TEST_CASE(SpatialSweep, SweepHitResultFields)
{
    PhysicsWorld world;

    RigidBody body;
    body.Position = TEVector2(5.0f, 0.0f);
    body.IsStatic = true;
    body.EntityName = "TargetEntity";
    body.EntityTag = "Enemy";
    body.Shape = CollisionShape(BoundsAABB(TEVector2(-1.0f, -1.0f), TEVector2(1.0f, 1.0f)));
    world.AddBody(&body);

    TESpatialHitResult hit = world.SweepLine(TEVector2(0.0f, 0.0f), TEVector2(10.0f, 0.0f));

    TE_CHECK(hit.Hit);
    // Hit point must lie within the swept segment X range
    TE_CHECK(hit.Point.x >= 0.0f);
    TE_CHECK(hit.Point.x <= 10.0f);
    // Distance must be non-negative
    TE_CHECK(hit.Distance >= 0.0f);
    // Fraction in [0, 1]
    TE_CHECK(hit.Fraction >= 0.0f);
    TE_CHECK(hit.Fraction <= 1.0f);
    // Normal must have at least one non-zero component
    TE_CHECK(hit.Normal.x != 0.0f || hit.Normal.y != 0.0f);
    // Metadata propagation
    TE_CHECK_EQ(hit.EntityName, "TargetEntity");
    TE_CHECK_EQ(hit.EntityTag, "Enemy");
}

TE_TEST_CASE(SpatialSweep, SweepCustomPolygon)
{
    PhysicsWorld world;

    RigidBody body;
    body.Position = TEVector2(5.0f, 0.0f);
    body.IsStatic = true;
    body.Shape = CollisionShape(BoundsAABB(TEVector2(-1.0f, -1.0f), TEVector2(1.0f, 1.0f)));
    world.AddBody(&body);

    // Create custom triangle shape
    BoundsTriangle tri(TEVector2(-0.5f, -0.5f), TEVector2(0.5f, -0.5f), TEVector2(0.0f, 0.5f));
    CollisionShape customShape(tri);

    TESpatialHitResult hit = world.SweepCustom(TEVector2(0.0f, 0.0f), TEVector2(10.0f, 0.0f), customShape, 0.0f);

    TE_CHECK(hit.Hit);
    TE_CHECK(hit.Fraction >= 0.0f);
    TE_CHECK(hit.Fraction <= 1.0f);
}

// ==========================================
// Rendering Pipeline Tests
// ==========================================

TE_TEST_CASE(Rendering, TextureDefaultState)
{
    // CPU-side property checks only (no GPU / OpenGL context required).
    TERef<Texture> tex = CreateRef<Texture>();

    TE_CHECK_EQ(tex->GetWidth(), static_cast<uint32_t>(0));
    TE_CHECK_EQ(tex->GetHeight(), static_cast<uint32_t>(0));
    TE_CHECK_EQ(tex->GetChannels(), static_cast<uint32_t>(0));

    TE_CHECK_EQ(static_cast<int>(tex->GetFilterMode()), static_cast<int>(TextureFilterMode::Linear));
    TE_CHECK_EQ(static_cast<int>(tex->GetWrapMode()), static_cast<int>(TextureWrapMode::Repeat));

    TE_CHECK(!tex->GetGenerateMipmaps());
    TE_CHECK(!tex->GetPremultipliedAlpha());

    tex->SetName("UnitTestTexture");
    TE_CHECK_EQ(tex->GetName(), "UnitTestTexture");
}

TE_TEST_CASE(Rendering, TexturePropertyMutators)
{
    TERef<Texture> tex = CreateRef<Texture>();

    tex->SetFilterMode(TextureFilterMode::Nearest);
    TE_CHECK_EQ(static_cast<int>(tex->GetFilterMode()), static_cast<int>(TextureFilterMode::Nearest));

    tex->SetWrapMode(TextureWrapMode::ClampToEdge);
    TE_CHECK_EQ(static_cast<int>(tex->GetWrapMode()), static_cast<int>(TextureWrapMode::ClampToEdge));

    tex->SetPremultipliedAlpha(true);
    TE_CHECK(tex->GetPremultipliedAlpha());

    tex->SetPremultipliedAlpha(false);
    TE_CHECK(!tex->GetPremultipliedAlpha());

    tex->SetWrapMode(TextureWrapMode::MirroredRepeat);
    TE_CHECK_EQ(static_cast<int>(tex->GetFilterMode()), static_cast<int>(TextureFilterMode::Nearest));
    TE_CHECK_EQ(static_cast<int>(tex->GetWrapMode()), static_cast<int>(TextureWrapMode::MirroredRepeat));
}

// ==========================================
// Memory Architecture & Allocator Unit Tests
// ==========================================

TE_TEST_CASE(Memory, ArenaAllocatorBasicLifecycle)
{
    TEArenaAllocator arena(1024);
    TE_CHECK(arena.IsInitialized());
    TE_CHECK_EQ(arena.GetCapacity(), static_cast<size_t>(1024));
    TE_CHECK_EQ(arena.GetAllocatedBytes(), static_cast<size_t>(0));

    // Allocate integer
    int *val = arena.New<int>(42);
    TE_CHECK(val != nullptr);
    if (val)
    {
        TE_CHECK_EQ(*val, 42);
    }
    TE_CHECK(arena.GetAllocatedBytes() >= sizeof(int));

    // Allocate aligned array
    float *arr = arena.NewArray<float>(10);
    TE_CHECK(arr != nullptr);
    if (arr)
    {
        for (int i = 0; i < 10; ++i)
            arr[i] = static_cast<float>(i) * 1.5f;
        TE_CHECK_EQ(arr[9], 13.5f);
    }

    // Rewind marker test
    auto marker = arena.GetMarker();
    int *tempVal = arena.New<int>(999);
    TE_CHECK(tempVal != nullptr);
    TE_CHECK(arena.GetAllocatedBytes() > marker);

    arena.Rewind(marker);
    TE_CHECK_EQ(arena.GetAllocatedBytes(), marker);

    // Frame reset test
    arena.Reset();
    TE_CHECK_EQ(arena.GetAllocatedBytes(), static_cast<size_t>(0));
    TE_CHECK(arena.GetPeakUsage() > 0);
}

TE_TEST_CASE(Memory, PoolAllocatorBasicLifecycle)
{
    struct TestComponent
    {
        float x = 1.0f;
        float y = 2.0f;
        int id = 7;
    };

    TEPoolAllocator pool(sizeof(TestComponent), 16);
    TE_CHECK(pool.IsInitialized());
    TE_CHECK_EQ(pool.GetCapacity(), static_cast<size_t>(16));
    TE_CHECK_EQ(pool.GetAllocatedCount(), static_cast<size_t>(0));
    TE_CHECK_EQ(pool.GetFreeCount(), static_cast<size_t>(16));

    TestComponent *c1 = pool.New<TestComponent>();
    TE_CHECK(c1 != nullptr);
    if (c1)
    {
        TE_CHECK_EQ(c1->id, 7);
        c1->id = 42;
    }
    TE_CHECK_EQ(pool.GetAllocatedCount(), static_cast<size_t>(1));

    TestComponent *c2 = pool.New<TestComponent>();
    TE_CHECK(c2 != nullptr);
    TE_CHECK_EQ(pool.GetAllocatedCount(), static_cast<size_t>(2));

    pool.Delete(c1);
    TE_CHECK_EQ(pool.GetAllocatedCount(), static_cast<size_t>(1));

    pool.Delete(c2);
    TE_CHECK_EQ(pool.GetAllocatedCount(), static_cast<size_t>(0));

    // Templated Typed Pool Test
    TTypedPool<TestComponent, 8> typedPool;
    TE_CHECK_EQ(typedPool.GetCapacity(), static_cast<size_t>(8));
    TestComponent *tc = typedPool.Allocate();
    TE_CHECK(tc != nullptr);
    TE_CHECK_EQ(typedPool.GetAllocatedCount(), static_cast<size_t>(1));
    typedPool.Free(tc);
    TE_CHECK_EQ(typedPool.GetAllocatedCount(), static_cast<size_t>(0));
}

// ==========================================
// Math Engine & Custom SIMD Backend Tests
// ==========================================

TE_TEST_CASE(Math, CustomSIMDBackendMatrixMultiplication)
{
    auto customAPI = MathAPI::Create(MathBackendType::Custom);
    TE_CHECK(customAPI != nullptr);
    TE_CHECK_EQ(static_cast<int>(customAPI->GetType()), static_cast<int>(MathBackendType::Custom));

    TEMatrix4 identity(1.0f);
    TEMatrix4 translated = customAPI->Translate(identity, TEVector(10.0f, 20.0f, 30.0f));
    TEVector4 vec(0.0f, 0.0f, 0.0f, 1.0f);
    TEVector4 transformed = customAPI->MultiplyMat4Vec4(translated, vec);

    TE_CHECK_EQ(transformed.x, 10.0f);
    TE_CHECK_EQ(transformed.y, 20.0f);
    TE_CHECK_EQ(transformed.z, 30.0f);
    TE_CHECK_EQ(transformed.w, 1.0f);

    TEMatrix4 scaled = customAPI->Scale(identity, TEVector(2.0f, 3.0f, 4.0f));
    TEMatrix4 combined = customAPI->MultiplyMat4(translated, scaled);
    TEVector4 vec2(1.0f, 1.0f, 1.0f, 1.0f);
    TEVector4 res2 = customAPI->MultiplyMat4Vec4(combined, vec2);

    TE_CHECK_EQ(res2.x, 12.0f);
    TE_CHECK_EQ(res2.y, 23.0f);
    TE_CHECK_EQ(res2.z, 34.0f);
}

// ==========================================
// TEMap & TESet Container Tests
// ==========================================

TE_TEST_CASE(CoreTypes, TEMapOperations)
{
    TEMap<TEString, int> scores;
    TE_CHECK(scores.IsEmpty());
    TE_CHECK_EQ(scores.Num(), static_cast<size_t>(0));

    scores.Add("Player1", 100);
    scores.Add("Player2", 250);
    scores["Player3"] = 500;

    TE_CHECK_EQ(scores.Num(), static_cast<size_t>(3));
    TE_CHECK(scores.Contains("Player1"));
    TE_CHECK(scores.Contains("Player2"));
    TE_CHECK(!scores.Contains("PlayerUnknown"));

    TE_CHECK_EQ(scores["Player1"], 100);
    TE_CHECK_EQ(scores["Player2"], 250);
    TE_CHECK_EQ(scores["Player3"], 500);

    int *pVal = scores.Find("Player2");
    TE_CHECK(pVal != nullptr);
    if (pVal)
    {
        TE_CHECK_EQ(*pVal, 250);
    }

    auto keys = scores.GetKeys();
    TE_CHECK_EQ(keys.Num(), static_cast<size_t>(3));

    auto values = scores.GetValues();
    TE_CHECK_EQ(values.Num(), static_cast<size_t>(3));

    scores.Remove("Player1");
    TE_CHECK_EQ(scores.Num(), static_cast<size_t>(2));
    TE_CHECK(!scores.Contains("Player1"));

    scores.Clear();
    TE_CHECK(scores.IsEmpty());
}

TE_TEST_CASE(CoreTypes, TESetOperations)
{
    TESet<TEString> tags;
    TE_CHECK(tags.IsEmpty());
    TE_CHECK_EQ(tags.Num(), static_cast<size_t>(0));

    tags.Add("Enemy");
    tags.Add("Boss");
    tags.Add("Undead");

    TE_CHECK_EQ(tags.Num(), static_cast<size_t>(3));
    TE_CHECK(tags.Contains("Enemy"));
    TE_CHECK(tags.Contains("Boss"));
    TE_CHECK(!tags.Contains("Friendly"));

    // Duplicate addition check
    tags.Add("Boss");
    TE_CHECK_EQ(tags.Num(), static_cast<size_t>(3));

    tags.Remove("Enemy");
    TE_CHECK_EQ(tags.Num(), static_cast<size_t>(2));
    TE_CHECK(!tags.Contains("Enemy"));

    tags.Clear();
    TE_CHECK(tags.IsEmpty());
    TE_CHECK_EQ(tags.Num(), static_cast<size_t>(0));
}

TE_TEST_CASE(CoreTypes, TEArrayEmplaceAndReallocationIntegrity)
{
    struct ComplexObject
    {
        TEString Name;
        int Value = 0;
        ComplexObject() = default;
        ComplexObject(const TEString &name, int val) : Name(name), Value(val) {}
        bool operator==(const ComplexObject &other) const { return Name == other.Name && Value == other.Value; }
    };

    TEArray<ComplexObject> array;
    for (int i = 0; i < 50; ++i)
    {
        array.Emplace("Object_" + TEString::FromInt(i), i * 10);
    }

    TE_CHECK_EQ(array.Num(), static_cast<size_t>(50));
    TE_CHECK_EQ(array[0].Name, "Object_0");
    TE_CHECK_EQ(array[0].Value, 0);
    TE_CHECK_EQ(array[49].Name, "Object_49");
    TE_CHECK_EQ(array[49].Value, 490);

    // Test element insertion
    array.Insert(25, ComplexObject("Inserted", 9999));
    TE_CHECK_EQ(array.Num(), static_cast<size_t>(51));
    TE_CHECK_EQ(array[25].Name, "Inserted");
    TE_CHECK_EQ(array[25].Value, 9999);

    // Test removal
    array.RemoveAt(25);
    TE_CHECK_EQ(array.Num(), static_cast<size_t>(50));
    TE_CHECK_EQ(array[25].Name, "Object_25");

    // Test find by predicate
    ComplexObject *found = array.FindBy([](const ComplexObject &obj) { return obj.Value == 300; });
    TE_CHECK(found != nullptr);
    if (found)
    {
        TE_CHECK_EQ(found->Name, "Object_30");
    }
}

TE_TEST_CASE(CoreTypes, TEFileSystemBinaryReadWrite)
{
    TEString tempPath = "temp_binary_test.bin";
    TEArray<uint8_t> testData;
    for (int i = 0; i < 256; ++i)
    {
        testData.Add(static_cast<uint8_t>(i));
    }

    bool writeSuccess = TEFileSystem::WriteAllBytes(tempPath, testData.GetData(), testData.Num());
    TE_CHECK(writeSuccess);
    TE_CHECK(TEFileSystem::Exists(tempPath));
    TE_CHECK_EQ(TEFileSystem::FileSize(tempPath), static_cast<uint64_t>(256));

    TEArray<uint8_t> readData = TEFileSystem::ReadAllBytes(tempPath);
    TE_CHECK_EQ(readData.Num(), static_cast<size_t>(256));
    for (size_t i = 0; i < 256; ++i)
    {
        TE_CHECK_EQ(readData[i], static_cast<uint8_t>(i));
    }

    TEFileSystem::Remove(tempPath);
    TE_CHECK(!TEFileSystem::Exists(tempPath));
}

TE_TEST_CASE(AssetPipeline, DDSHeaderParsingAndFormatDetection)
{
    // 1. Construct synthetic BC1 (DXT1) DDS buffer
    TEArray<uint8_t> dxt1Data;
    dxt1Data.Resize(128, 0);

    // Magic 'DDS '
    *reinterpret_cast<uint32_t *>(dxt1Data.GetData() + 0) = 0x20534444;
    // Header size 124
    *reinterpret_cast<uint32_t *>(dxt1Data.GetData() + 4) = 124;
    // Height & Width
    *reinterpret_cast<uint32_t *>(dxt1Data.GetData() + 12) = 256;
    *reinterpret_cast<uint32_t *>(dxt1Data.GetData() + 16) = 512;
    // Linear size
    *reinterpret_cast<uint32_t *>(dxt1Data.GetData() + 20) = 65536;
    // Mip count
    *reinterpret_cast<uint32_t *>(dxt1Data.GetData() + 28) = 4;
    // Pixel format flags (FourCC = 0x4)
    *reinterpret_cast<uint32_t *>(dxt1Data.GetData() + 80) = 0x4;
    // FourCC 'DXT1'
    *reinterpret_cast<uint32_t *>(dxt1Data.GetData() + 84) = 0x31545844;

    DDSHeaderInfo bc1Info = AssetManager::ParseDDSHeader(dxt1Data);
    TE_CHECK(bc1Info.IsValid);
    TE_CHECK_EQ(bc1Info.Width, 512);
    TE_CHECK_EQ(bc1Info.Height, 256);
    TE_CHECK_EQ(bc1Info.MipMapCount, 4);
    TE_CHECK_EQ(static_cast<int>(bc1Info.Format), static_cast<int>(DDSCompressionFormat::BC1_DXT1));
    TE_CHECK_EQ(bc1Info.DataOffset, 128);

    // 2. Construct synthetic BC7 DX10 DDS buffer
    TEArray<uint8_t> bc7Data;
    bc7Data.Resize(148, 0);

    // Magic 'DDS '
    *reinterpret_cast<uint32_t *>(bc7Data.GetData() + 0) = 0x20534444;
    *reinterpret_cast<uint32_t *>(bc7Data.GetData() + 4) = 124;
    *reinterpret_cast<uint32_t *>(bc7Data.GetData() + 12) = 1024;
    *reinterpret_cast<uint32_t *>(bc7Data.GetData() + 16) = 1024;
    *reinterpret_cast<uint32_t *>(bc7Data.GetData() + 28) = 1;
    // Pixel format flags FourCC
    *reinterpret_cast<uint32_t *>(bc7Data.GetData() + 80) = 0x4;
    // FourCC 'DX10'
    *reinterpret_cast<uint32_t *>(bc7Data.GetData() + 84) = 0x30315844;
    // DX10 header dxgiFormat = 98 (BC7_UNORM)
    *reinterpret_cast<uint32_t *>(bc7Data.GetData() + 128) = 98;

    DDSHeaderInfo bc7Info = AssetManager::ParseDDSHeader(bc7Data);
    TE_CHECK(bc7Info.IsValid);
    TE_CHECK_EQ(bc7Info.Width, 1024);
    TE_CHECK_EQ(bc7Info.Height, 1024);
    TE_CHECK_EQ(static_cast<int>(bc7Info.Format), static_cast<int>(DDSCompressionFormat::BC7_UNORM));
    TE_CHECK_EQ(bc7Info.DataOffset, 148);
}

struct TestReflectedEntity
{
    int Health = 100;
    float Speed = 5.5f;
    uint32_t Level = 1;
};

TE_TEST_CASE(Reflection, DynamicRegistrationAndPropertyInspection)
{
    static TEArenaAllocator s_ReflectTestArena(4096);
    s_ReflectTestArena.Reset();

    TEClass entityCls("TestReflectedEntity", "TObject", sizeof(TestReflectedEntity),
                      []() -> void * { return static_cast<void *>(s_ReflectTestArena.New<TestReflectedEntity>()); });

    entityCls.AddProperty(TEProperty("Health", "int", offsetof(TestReflectedEntity, Health), sizeof(int),
                                     [](const void *inst) -> TEString
                                     {
                                         const auto *obj = reinterpret_cast<const TestReflectedEntity *>(inst);
                                         return TEString::FromInt(obj->Health);
                                     }));

    entityCls.AddProperty(TEProperty("Speed", "float", offsetof(TestReflectedEntity, Speed), sizeof(float),
                                     [](const void *inst) -> TEString
                                     {
                                         const auto *obj = reinterpret_cast<const TestReflectedEntity *>(inst);
                                         return TEString::FromFloat(obj->Speed);
                                     }));

    entityCls.AddProperty(TEProperty("Level", "uint32_t", offsetof(TestReflectedEntity, Level), sizeof(uint32_t)));

    TEReflectionRegistry::Get().RegisterType(entityCls);

    const TEClass *found = TEReflectionRegistry::Get().FindClass("TestReflectedEntity");
    TE_CHECK(found != nullptr);
    if (found)
    {
        TE_CHECK_EQ(found->GetName(), "TestReflectedEntity");
        TE_CHECK_EQ(found->GetParentName(), "TObject");
        TE_CHECK(found->IsChildOf("TObject"));
        TE_CHECK_EQ(found->GetProperties().Num(), 3);

        const TEProperty *healthProp = found->FindProperty("Health");
        TE_CHECK(healthProp != nullptr);
        if (healthProp)
        {
            TE_CHECK_EQ(healthProp->GetTypeName(), "int");
            TE_CHECK_EQ(healthProp->GetOffset(), offsetof(TestReflectedEntity, Health));
        }

        // Test dynamic instantiation and property modification
        void *inst = found->Instantiate();
        TE_CHECK(inst != nullptr);
        if (inst)
        {
            TestReflectedEntity *obj = reinterpret_cast<TestReflectedEntity *>(inst);
            TE_CHECK_EQ(obj->Health, 100);
            TE_CHECK_EQ(obj->Speed, 5.5f);

            // Read via property reflection
            int val = healthProp->GetValue<int>(inst);
            TE_CHECK_EQ(val, 100);

            // Write via property reflection
            healthProp->SetValue<int>(inst, 250);
            TE_CHECK_EQ(obj->Health, 250);

            TEString healthStr = healthProp->ToString(inst);
            TE_CHECK_EQ(healthStr, "250");
        }
    }
}

TE_TEST_CASE(Typography, FontMetricsAndKerningCalculations)
{
    auto font = CreateRef<FontAsset>("TestTypographyFont", 32.0f);
    TE_CHECK_EQ(font->GetName(), "TestTypographyFont");
    TE_CHECK_EQ(font->GetPixelSize(), 32.0f);

    // Set Kerning pairs
    font->SetKerning("A", "V", -2.5f);
    font->SetKerning("T", "o", -1.8f);

    TE_CHECK_EQ(font->GetKerning("A", "V"), -2.5f);
    TE_CHECK_EQ(font->GetKerning("T", "o"), -1.8f);
    TE_CHECK_EQ(font->GetKerning("X", "Y"), 0.0f);
}

TE_TEST_CASE(GameplayUtils, SpawnAndDestroyLifecycle)
{
    Scene scene("TestScene");

    // 1. Test SpawnEntity (Base Entity with Tag & Transform)
    Entity entity1 = GameplayUtils::SpawnEntity(scene, "PlayerHero", {10.0f, 25.0f});
    TE_CHECK(entity1.IsValid());

    auto *tag1 = entity1.GetComponent<TagComponent>();
    TE_CHECK(tag1 != nullptr);
    TE_CHECK_EQ(tag1->Tag, "PlayerHero");

    auto *tc1 = entity1.GetComponent<TransformComponent>();
    TE_CHECK(tc1 != nullptr);
    TE_CHECK_EQ(tc1->Transform.Position.x, 10.0f);
    TE_CHECK_EQ(tc1->Transform.Position.y, 25.0f);

    // 2. Test Destroy
    GameplayUtils::Destroy(scene, entity1);
    TE_CHECK(!entity1.IsValid());
}

TE_TEST_CASE(GameplayUtils, SpatialSweepRouting)
{
    Scene scene("PhysicsTestScene");

    // Test SweepLine invocation (safely runs even if physics world is empty)
    TESpatialHitResult hit = GameplayUtils::SweepLine({0.0f, 0.0f}, {100.0f, 0.0f}, &scene);
    TE_CHECK(!hit.Hit);
    TE_CHECK_EQ(hit.Fraction, 1.0f);
}
