#include "ParticleStackEditorPanel.hpp"
#include "ParticleWidgets.hpp"
#include "Utils/TimeGUI.hpp"


ParticleStackEditorPanel::ParticleStackEditorPanel()
    : IEditorPanel("Particle FX Stack Editor")
{
    m_StandaloneEmitter.ApplyFirePreset();
}

void ParticleStackEditorPanel::OnTimeGUIRender(Ref<EditorLayer> editor)
{
    OnTimeGUIRender();
}

void ParticleStackEditorPanel::OnTimeGUIRender()
{
    if (!m_Visible)
        return;

    if (TimeGUI::Begin("Particle FX Stack Editor", &m_Visible))
    {
        ParticleEmitterComponent *emitter = m_TargetEmitter ? m_TargetEmitter : &m_StandaloneEmitter;

        // Transport Controls Bar
        DrawTransportBar();
        TimeGUI::Separator();

        // Presets Quick Bar
        DrawPresetsBar();
        TimeGUI::Separator();

        // 1. Emitter Properties
        DrawEmitterPropertiesSection();

        // 2. Emitter Spawn Stack
        DrawEmitterSpawnSection();

        // 3. Particle Spawn Stack
        DrawParticleSpawnSection();

        // 4. Particle Update Stack
        DrawParticleUpdateSection();

        // 5. Particle Render Stack
        DrawParticleRenderSection();

        // Live preview ticking when in standalone mode
        if (!m_TargetEmitter)
        {
            m_StandaloneEmitter.Update(0.016f, TEVector(0.0f, 0.0f, 0.0f), nullptr);
        }
    }
    TimeGUI::End();
}

void ParticleStackEditorPanel::DrawTransportBar()
{
    ParticleEmitterComponent *emitter = m_TargetEmitter ? m_TargetEmitter : &m_StandaloneEmitter;

    if (emitter->Playing)
    {
        if (TimeGUI::Button("Pause (||)"))
            emitter->Playing = false;
    }
    else
    {
        if (TimeGUI::Button("Play (>)"))
            emitter->Playing = true;
    }

    TimeGUI::SameLine();
    if (TimeGUI::Button("Reset (<<)"))
    {
        emitter->Reset();
    }

    TimeGUI::SameLine();
    bool loop = emitter->Loop;
    if (TimeGUI::Checkbox("Loop", &loop))
        emitter->Loop = loop;

    TimeGUI::SameLine();
    TimeGUI::Text(" | Active: %zu / %zu", emitter->GetPool().GetActiveCount(), emitter->GetPool().GetCapacity());
}

void ParticleStackEditorPanel::DrawPresetsBar()
{
    ParticleEmitterComponent *emitter = m_TargetEmitter ? m_TargetEmitter : &m_StandaloneEmitter;

    TimeGUI::Text("Presets:");
    TimeGUI::SameLine();
    if (TimeGUI::Button("Fire"))
        emitter->ApplyFirePreset();

    TimeGUI::SameLine();
    if (TimeGUI::Button("Sparks"))
        emitter->ApplySparksPreset();

    TimeGUI::SameLine();
    if (TimeGUI::Button("Explosion"))
        emitter->ApplyExplosionPreset();

    TimeGUI::SameLine();
    if (TimeGUI::Button("Smoke"))
        emitter->ApplySmokePreset();

    TimeGUI::SameLine();
    if (TimeGUI::Button("Rain"))
        emitter->ApplyRainPreset();
}

void ParticleStackEditorPanel::DrawEmitterPropertiesSection()
{
    ParticleEmitterComponent *emitter = m_TargetEmitter ? m_TargetEmitter : &m_StandaloneEmitter;

    if (ParticleWidgets::DrawModuleHeader("Emitter Properties", nullptr, true))
    {
        float cap = emitter->MaxCapacity;
        if (TimeGUI::DragFloat("Max Capacity", &cap, 10.0f, 10.0f, 50000.0f, "%.0f"))
            emitter->MaxCapacity = cap;

        const char *simSpaceNames[] = {"World", "Local"};
        int currentSpace = (int)emitter->SimSpace;
        if (TimeGUI::Combo("Sim Space", &currentSpace, simSpaceNames, 2))
            emitter->SimSpace = (ESimulationSpace)currentSpace;
    }
}

void ParticleStackEditorPanel::DrawEmitterSpawnSection()
{
    ParticleEmitterComponent *emitter = m_TargetEmitter ? m_TargetEmitter : &m_StandaloneEmitter;

    bool enableRate = emitter->EnableRate;
    if (ParticleWidgets::DrawModuleHeader("Emitter Spawn - Continuous Rate", &enableRate, true))
    {
        emitter->EnableRate = enableRate;
        float rate = emitter->EmitRate;
        if (TimeGUI::DragFloat("Spawn Rate (p/s)", &rate, 1.0f, 0.0f, 5000.0f, "%.1f"))
            emitter->EmitRate = rate;
    }

    bool enableBurst = emitter->EnableBurst;
    if (ParticleWidgets::DrawModuleHeader("Emitter Spawn - Burst Instantaneous", &enableBurst, false))
    {
        emitter->EnableBurst = enableBurst;
        float count = emitter->BurstCount;
        if (TimeGUI::DragFloat("Burst Count", &count, 1.0f, 1.0f, 1000.0f, "%.0f"))
            emitter->BurstCount = count;

        float interval = emitter->BurstInterval;
        if (TimeGUI::DragFloat("Burst Interval (s)", &interval, 0.05f, 0.05f, 60.0f, "%.2f"))
            emitter->BurstInterval = interval;
    }
}

void ParticleStackEditorPanel::DrawParticleSpawnSection()
{
    ParticleEmitterComponent *emitter = m_TargetEmitter ? m_TargetEmitter : &m_StandaloneEmitter;

    if (ParticleWidgets::DrawModuleHeader("Particle Spawn - Shape & Location", nullptr, true))
    {
        const char *shapeNames[] = {"Point", "Circle 2D", "Box 2D", "Cone 2D", "Sphere 3D", "Box 3D"};
        int currentShape = (int)emitter->EmissionShape;
        if (TimeGUI::Combo("Emission Shape", &currentShape, shapeNames, 6))
            emitter->EmissionShape = (EEmissionShape)currentShape;

        if (emitter->EmissionShape == EEmissionShape::Circle2D || emitter->EmissionShape == EEmissionShape::Sphere3D ||
            emitter->EmissionShape == EEmissionShape::Cone2D)
        {
            float r = emitter->ShapeRadius;
            if (TimeGUI::DragFloat("Radius", &r, 0.05f, 0.01f, 100.0f, "%.2f"))
                emitter->ShapeRadius = r;
        }

        if (emitter->EmissionShape == EEmissionShape::Cone2D)
        {
            float arc = emitter->ShapeArcAngle;
            if (TimeGUI::DragFloat("Cone Arc Angle", &arc, 1.0f, 1.0f, 360.0f, "%.1f deg"))
                emitter->ShapeArcAngle = arc;
        }

        if (emitter->EmissionShape == EEmissionShape::Box2D || emitter->EmissionShape == EEmissionShape::Box3D)
        {
            float extents[3] = {emitter->ShapeBoxExtents.x, emitter->ShapeBoxExtents.y, emitter->ShapeBoxExtents.z};
            if (TimeGUI::DragFloat3("Box Extents", extents, 0.05f, 0.01f, 100.0f))
                emitter->ShapeBoxExtents = TEVector(extents[0], extents[1], extents[2]);
        }
    }

    if (ParticleWidgets::DrawModuleHeader("Particle Spawn - Initial Velocity", nullptr, true))
    {
        float dir[3] = {emitter->BaseDirection.x, emitter->BaseDirection.y, emitter->BaseDirection.z};
        if (TimeGUI::DragFloat3("Direction", dir, 0.05f))
            emitter->BaseDirection = TEVector(dir[0], dir[1], dir[2]);

        float speedMin = emitter->SpeedMin;
        float speedMax = emitter->SpeedMax;
        if (TimeGUI::DragFloat("Speed Min", &speedMin, 0.1f, 0.0f, 1000.0f, "%.2f"))
            emitter->SpeedMin = speedMin;
        if (TimeGUI::DragFloat("Speed Max", &speedMax, 0.1f, 0.0f, 1000.0f, "%.2f"))
            emitter->SpeedMax = speedMax;

        float spread = emitter->SpreadAngle;
        if (TimeGUI::DragFloat("Spread Angle", &spread, 1.0f, 0.0f, 360.0f, "%.1f deg"))
            emitter->SpreadAngle = spread;
    }

    if (ParticleWidgets::DrawModuleHeader("Particle Spawn - Lifetime & Initial Size", nullptr, true))
    {
        float lifeMin = emitter->LifetimeMin;
        float lifeMax = emitter->LifetimeMax;
        if (TimeGUI::DragFloat("Lifetime Min (s)", &lifeMin, 0.05f, 0.01f, 100.0f, "%.2f"))
            emitter->LifetimeMin = lifeMin;
        if (TimeGUI::DragFloat("Lifetime Max (s)", &lifeMax, 0.05f, 0.01f, 100.0f, "%.2f"))
            emitter->LifetimeMax = lifeMax;

        float sizeMin = emitter->SizeMin;
        float sizeMax = emitter->SizeMax;
        if (TimeGUI::DragFloat("Size Min", &sizeMin, 0.05f, 0.01f, 50.0f, "%.2f"))
            emitter->SizeMin = sizeMin;
        if (TimeGUI::DragFloat("Size Max", &sizeMax, 0.05f, 0.01f, 50.0f, "%.2f"))
            emitter->SizeMax = sizeMax;
    }
}

void ParticleStackEditorPanel::DrawParticleUpdateSection()
{
    ParticleEmitterComponent *emitter = m_TargetEmitter ? m_TargetEmitter : &m_StandaloneEmitter;

    if (ParticleWidgets::DrawModuleHeader("Particle Update - Gravity & Forces", nullptr, true))
    {
        float grav[3] = {emitter->Gravity.x, emitter->Gravity.y, emitter->Gravity.z};
        if (TimeGUI::DragFloat3("Gravity Vector", grav, 0.1f))
            emitter->Gravity = TEVector(grav[0], grav[1], grav[2]);

        float drag = emitter->LinearDrag;
        if (TimeGUI::DragFloat("Linear Drag", &drag, 0.01f, 0.0f, 10.0f, "%.3f"))
            emitter->LinearDrag = drag;
    }

    if (ParticleWidgets::DrawModuleHeader("Particle Update - Color Over Life", nullptr, true))
    {
        float startCol[4] = {emitter->StartColor.r, emitter->StartColor.g, emitter->StartColor.b, emitter->StartColor.a};
        if (TimeGUI::ColorEdit4("Start Color", startCol))
            emitter->StartColor = TEColor(startCol[0], startCol[1], startCol[2], startCol[3]);

        float endCol[4] = {emitter->EndColor.r, emitter->EndColor.g, emitter->EndColor.b, emitter->EndColor.a};
        if (TimeGUI::ColorEdit4("End Color", endCol))
            emitter->EndColor = TEColor(endCol[0], endCol[1], endCol[2], endCol[3]);
    }

    bool physicsColl = emitter->PhysicsSimulated;
    if (ParticleWidgets::DrawModuleHeader("Particle Update - Physics Raycast Collision", &physicsColl, false))
    {
        emitter->PhysicsSimulated = physicsColl;

        float bounce = emitter->Bounciness;
        if (TimeGUI::DragFloat("Bounciness", &bounce, 0.02f, 0.0f, 1.0f, "%.2f"))
            emitter->Bounciness = bounce;

        float friction = emitter->Friction;
        if (TimeGUI::DragFloat("Friction", &friction, 0.02f, 0.0f, 1.0f, "%.2f"))
            emitter->Friction = friction;
    }
}

void ParticleStackEditorPanel::DrawParticleRenderSection()
{
    ParticleEmitterComponent *emitter = m_TargetEmitter ? m_TargetEmitter : &m_StandaloneEmitter;

    if (ParticleWidgets::DrawModuleHeader("Particle Render", nullptr, true))
    {
        const char *blendModes[] = {"Alpha Blend", "Additive", "Multiply", "Opaque"};
        int currentBlend = (int)emitter->BlendMode;
        if (TimeGUI::Combo("Blend Mode", &currentBlend, blendModes, 4))
            emitter->BlendMode = (EParticleBlendMode)currentBlend;
    }
}

