#include "IKMCPTools.hpp"
#include "../Solvers/FABRIKSolver2D.hpp"
#include "../Solvers/TwoBoneIKSolver2D.hpp"
#include "GameFrameWork/GameplayUtils.hpp"
#include "Utils/TEString.hpp"

#if defined(TE_HAS_PLUGIN_MCPPLUGIN) || defined(TE_PLUGIN_MCPPLUGIN) || 1
#include "../../../MCPPlugin/src/MCPToolRegistry.hpp"

static TEString IKSolveTwoBone2DHandler(const TEString &paramsJson)
{
    TEString jsonStr = paramsJson;

    float rx = 0.0f;
    float ry = 0.0f;
    float la = 50.0f;
    float lb = 50.0f;
    float tx = 100.0f;
    float ty = 0.0f;
    bool bendPos = true;

    auto extractFloat = [&](const TEString &key, float defaultVal) -> float
    {
        size_t pos = jsonStr.Find("\"" + key + "\"");
        if (pos != TEString::npos)
        {
            int colon = jsonStr.Find(":", ESearchCase::IgnoreCase, ESearchDir::FromStart, (int)pos);
            if (colon != -1)
                return std::stof(jsonStr.Substr(colon + 1).c_str());
        }
        return defaultVal;
    };

    rx = extractFloat("root_x", 0.0f);
    ry = extractFloat("root_y", 0.0f);
    la = extractFloat("length_a", 50.0f);
    lb = extractFloat("length_b", 50.0f);
    tx = extractFloat("target_x", 100.0f);
    ty = extractFloat("target_y", 0.0f);
    bendPos = (jsonStr.Find("\"bend_positive\":false") == TEString::npos);

    TEVector2 root(rx, ry);
    TEVector2 target(tx, ty);
    TEVector2 midPos;
    float angleA = 0.0f;
    float angleB = 0.0f;

    bool success = IK::TwoBoneIKSolver2D::Solve(root, la, lb, target, bendPos, midPos, angleA, angleB);
    if (success)
    {
        TEString res = "{";
        res += "\"success\":true,";
        res += "\"mid_x\":" + TEString::FromFloat(midPos.x) + ",";
        res += "\"mid_y\":" + TEString::FromFloat(midPos.y) + ",";
        res += "\"angle_a\":" + TEString::FromFloat(angleA) + ",";
        res += "\"angle_b\":" + TEString::FromFloat(angleB);
        res += "}";
        return res;
    }

    return "{\"success\":false,\"error\":\"Target position unreachable or invalid\"}";
}

// 1. ik_solve_twobone2d
TE_REGISTER_MCP_TOOL(ik_solve_twobone2d,
                     "Solve analytical 2-Bone Inverse Kinematics in 2D given root position, bone lengths, target "
                     "position, and bend direction.",
                     "{\"type\":\"object\",\"properties\":{\"root_x\":{\"type\":\"number\"},\"root_y\":{\"type\":"
                     "\"number\"},\"length_a\":{\"type\":\"number\"},\"length_b\":{\"type\":\"number\"},\"target_x\":{"
                     "\"type\":\"number\"},\"target_y\":{\"type\":\"number\"},\"bend_positive\":{\"type\":\"boolean\"}}"
                     ",\"required\":[\"root_x\",\"root_y\",\"length_a\",\"length_b\",\"target_x\",\"target_y\"]}",
                     IKSolveTwoBone2DHandler);

namespace IK
{

void IKMCPTools::RegisterTools()
{
    // Registration handled through static macros
}

} // namespace IK
#endif // TE_HAS_PLUGIN_MCPPLUGIN
