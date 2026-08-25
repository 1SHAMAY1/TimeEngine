#pragma once

#include "Core/PreRequisites.h"
#include "Renderer/TEColor.hpp"
#include "Utils/TEString.hpp"
#include <cstdint>


enum class GraphPinKind : uint8_t
{
    Flow = 0,
    Bool,
    Int,
    Float,
    Float2,
    Float3,
    Float4,
    Texture,
    Slab,
    Custom,
    Any
};

enum class PinDirection : uint8_t
{
    Input = 0,
    Output
};

class TE_API GraphPinType
{
public:
    GraphPinType() : m_Kind(GraphPinKind::Float), m_TypeName("Float"), m_Color(0.4f, 0.8f, 0.3f, 1.0f) {}
    GraphPinType(GraphPinKind kind, const TEString &typeName, const TEColor &color = TEColor::White())
        : m_Kind(kind), m_TypeName(typeName), m_Color(color)
    {
    }

    GraphPinKind GetKind() const { return m_Kind; }
    const TEString &GetTypeName() const { return m_TypeName; }
    const TEColor &GetColor() const { return m_Color; }

    void SetKind(GraphPinKind kind) { m_Kind = kind; }
    void SetTypeName(const TEString &typeName) { m_TypeName = typeName; }
    void SetColor(const TEColor &color) { m_Color = color; }

    bool operator==(const GraphPinType &other) const
    {
        if (m_Kind == GraphPinKind::Custom && other.m_Kind == GraphPinKind::Custom)
            return m_TypeName == other.m_TypeName;
        return m_Kind == other.m_Kind;
    }

    bool operator!=(const GraphPinType &other) const { return !(*this == other); }

    bool CanConnectTo(const GraphPinType &other) const
    {
        if (m_Kind == GraphPinKind::Any || other.m_Kind == GraphPinKind::Any)
            return true;

        if (m_Kind == GraphPinKind::Flow || other.m_Kind == GraphPinKind::Flow)
            return m_Kind == other.m_Kind;

        if (m_Kind == GraphPinKind::Custom && other.m_Kind == GraphPinKind::Custom)
            return m_TypeName == other.m_TypeName;

        if (m_Kind == other.m_Kind)
            return true;

        // Auto-cast between numeric float vectors
        bool isThisFloatGroup = (m_Kind == GraphPinKind::Float || m_Kind == GraphPinKind::Float2 ||
                                 m_Kind == GraphPinKind::Float3 || m_Kind == GraphPinKind::Float4);
        bool isOtherFloatGroup = (other.m_Kind == GraphPinKind::Float || other.m_Kind == GraphPinKind::Float2 ||
                                  other.m_Kind == GraphPinKind::Float3 || other.m_Kind == GraphPinKind::Float4);

        if (isThisFloatGroup && isOtherFloatGroup)
            return true;

        return false;
    }

    // Built-in standard type generators
    static GraphPinType Flow() { return GraphPinType(GraphPinKind::Flow, "Flow", TEColor(0.95f, 0.95f, 0.95f, 1.0f)); }
    static GraphPinType Bool() { return GraphPinType(GraphPinKind::Bool, "Bool", TEColor(0.85f, 0.25f, 0.25f, 1.0f)); }
    static GraphPinType Int() { return GraphPinType(GraphPinKind::Int, "Int", TEColor(0.25f, 0.85f, 0.65f, 1.0f)); }
    static GraphPinType Float() { return GraphPinType(GraphPinKind::Float, "Float", TEColor(0.4f, 0.8f, 0.3f, 1.0f)); }
    static GraphPinType Float2() { return GraphPinType(GraphPinKind::Float2, "Float2", TEColor(0.95f, 0.85f, 0.25f, 1.0f)); }
    static GraphPinType Float3() { return GraphPinType(GraphPinKind::Float3, "Float3", TEColor(0.35f, 0.75f, 0.95f, 1.0f)); }
    static GraphPinType Float4() { return GraphPinType(GraphPinKind::Float4, "Float4", TEColor(0.85f, 0.45f, 0.95f, 1.0f)); }
    static GraphPinType Texture() { return GraphPinType(GraphPinKind::Texture, "Texture", TEColor(0.95f, 0.5f, 0.2f, 1.0f)); }
    static GraphPinType Slab() { return GraphPinType(GraphPinKind::Slab, "MaterialSlab", TEColor(0.2f, 0.85f, 0.9f, 1.0f)); }
    static GraphPinType Custom(const TEString &typeName, const TEColor &color = TEColor(0.7f, 0.7f, 0.75f, 1.0f))
    {
        return GraphPinType(GraphPinKind::Custom, typeName, color);
    }
    static GraphPinType Any() { return GraphPinType(GraphPinKind::Any, "Any", TEColor(0.8f, 0.8f, 0.8f, 1.0f)); }

private:
    GraphPinKind m_Kind = GraphPinKind::Float;
    TEString m_TypeName = "Float";
    TEColor m_Color = {0.4f, 0.8f, 0.3f, 1.0f};
};

struct TE_API GraphPin
{
    uint64_t ID = 0;
    uint64_t NodeID = 0;
    TEString Name;
    GraphPinType Type;
    PinDirection Direction = PinDirection::Input;
    TEString DefaultValue = "0.0";

    GraphPin() = default;
    GraphPin(uint64_t id, uint64_t nodeId, const TEString &name, const GraphPinType &type,
             PinDirection direction, const TEString &defaultValue = "")
        : ID(id), NodeID(nodeId), Name(name), Type(type),
          Direction(direction), DefaultValue(defaultValue)
    {
    }

    bool CanConnectTo(const GraphPin &other) const
    {
        if (NodeID == other.NodeID)
            return false;
        if (Direction == other.Direction)
            return false;
        return Type.CanConnectTo(other.Type);
    }
};
