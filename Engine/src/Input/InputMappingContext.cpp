#include "Core/PreRequisites.h"
#include "Input/InputMappingContext.hpp"
#include <fstream>

TE_REGISTER_ASSET(InputMappingContext)

bool InputMappingContext::SaveToFile(const TEString &filepath) const
{
    std::ofstream out(filepath, std::ios::binary);
    if (!out.is_open())
        return false;

    // Header / Context Name length & string
    uint32_t nameLen = static_cast<uint32_t>(ContextName.size());
    out.write(reinterpret_cast<const char *>(&nameLen), sizeof(nameLen));
    if (nameLen > 0)
        out.write(ContextName.data(), nameLen);

    // Mappings Count
    uint32_t count = static_cast<uint32_t>(Mappings.size());
    out.write(reinterpret_cast<const char *>(&count), sizeof(count));

    for (const auto &m : Mappings)
    {
        // Action Name
        uint32_t actNameLen = static_cast<uint32_t>(m.Action.Name.size());
        out.write(reinterpret_cast<const char *>(&actNameLen), sizeof(actNameLen));
        if (actNameLen > 0)
            out.write(m.Action.Name.data(), actNameLen);

        // Action ValueType
        uint8_t valType = static_cast<uint8_t>(m.Action.ValueType);
        out.write(reinterpret_cast<const char *>(&valType), sizeof(valType));

        // BindingType, Code, IsRemappable
        uint8_t bindType = static_cast<uint8_t>(m.BindingType);
        out.write(reinterpret_cast<const char *>(&bindType), sizeof(bindType));
        out.write(reinterpret_cast<const char *>(&m.Code), sizeof(m.Code));

        uint8_t remappable = m.IsRemappable ? 1 : 0;
        out.write(reinterpret_cast<const char *>(&remappable), sizeof(remappable));

        // Modifier
        uint8_t negate = m.Modifier.ScaleNegate ? 1 : 0;
        out.write(reinterpret_cast<const char *>(&negate), sizeof(negate));
        out.write(reinterpret_cast<const char *>(&m.Modifier.Multiplier), sizeof(m.Modifier.Multiplier));
        out.write(reinterpret_cast<const char *>(&m.Modifier.Deadzone), sizeof(m.Modifier.Deadzone));
    }

    out.close();
    return true;
}

bool InputMappingContext::LoadFromFile(const TEString &filepath)
{
    std::ifstream in(filepath, std::ios::binary);
    if (!in.is_open())
        return false;

    ContextName.Empty();
    Mappings.clear();

    uint32_t nameLen = 0;
    if (!in.read(reinterpret_cast<char *>(&nameLen), sizeof(nameLen)))
        return false;

    if (nameLen > 0)
    {
        TEArray<char> buf(nameLen + 1, 0);
        in.read(buf.data(), nameLen);
        ContextName = TEString(buf.data());
    }

    uint32_t count = 0;
    if (!in.read(reinterpret_cast<char *>(&count), sizeof(count)))
        return false;

    for (uint32_t i = 0; i < count; ++i)
    {
        ActionKeyMapping m;

        uint32_t actNameLen = 0;
        in.read(reinterpret_cast<char *>(&actNameLen), sizeof(actNameLen));
        if (actNameLen > 0)
        {
            TEArray<char> buf(actNameLen + 1, 0);
            in.read(buf.data(), actNameLen);
            m.Action.Name = TEString(buf.data());
        }

        uint8_t valType = 0;
        in.read(reinterpret_cast<char *>(&valType), sizeof(valType));
        m.Action.ValueType = static_cast<EInputActionValueType>(valType);

        uint8_t bindType = 0;
        in.read(reinterpret_cast<char *>(&bindType), sizeof(bindType));
        m.BindingType = static_cast<EInputBindingType>(bindType);

        in.read(reinterpret_cast<char *>(&m.Code), sizeof(m.Code));

        uint8_t remappable = 0;
        in.read(reinterpret_cast<char *>(&remappable), sizeof(remappable));
        m.IsRemappable = (remappable != 0);

        uint8_t negate = 0;
        in.read(reinterpret_cast<char *>(&negate), sizeof(negate));
        m.Modifier.ScaleNegate = (negate != 0);

        in.read(reinterpret_cast<char *>(&m.Modifier.Multiplier), sizeof(m.Modifier.Multiplier));
        in.read(reinterpret_cast<char *>(&m.Modifier.Deadzone), sizeof(m.Modifier.Deadzone));

        Mappings.push_back(m);
    }

    in.close();
    return true;
}
