#include "Core/PreRequisites.h"
#include "Input/InputMappingContext.hpp"
#include "Core/Log.h"
#include "Utils/TEFileSystem.hpp"

TE_REGISTER_ASSET(InputMappingContext)

bool InputMappingContext::SaveToFile(const TEString &filepath) const
{
    TEString content = "InputMappingContext: " + ContextName + "\n";
    content += "Mappings:\n";

    for (const auto &m : Mappings)
    {
        content += "  - Action: " + m.Action.Name + "\n";
        content += "    ValueType: " + TEString::FromInt(static_cast<int>(m.Action.ValueType)) + "\n";
        content += "    BindingType: " + TEString::FromInt(static_cast<int>(m.BindingType)) + "\n";
        content += "    Code: " + TEString::FromInt(static_cast<int>(m.Code)) + "\n";
        content += "    Remappable: " + TEString(m.IsRemappable ? "true" : "false") + "\n";
        content += "    ScaleNegate: " + TEString(m.Modifier.ScaleNegate ? "true" : "false") + "\n";
        content += "    Multiplier: " + TEString::FromFloat(m.Modifier.Multiplier) + "\n";
        content += "    Deadzone: " + TEString::FromFloat(m.Modifier.Deadzone) + "\n";
    }

    return TEFileSystem::WriteAllText(filepath, content);
}

bool InputMappingContext::LoadFromFile(const TEString &filepath)
{
    if (!TEFileSystem::Exists(filepath))
        return false;

    ContextName.Empty();
    Mappings.clear();

    ActionKeyMapping currentMapping;
    bool hasCurrentMapping = false;

    bool success = TEFileSystem::ForEachLine(
        filepath,
        [&](const TEString &line) -> bool
        {
            TEString trimmed = line;
            while (trimmed.StartsWith(" ") || trimmed.StartsWith("\t"))
                trimmed = trimmed.Substr(1);

            if (trimmed.StartsWith("InputMappingContext: "))
            {
                ContextName = trimmed.Substr(21);
            }
            else if (trimmed.StartsWith("- Action: "))
            {
                if (hasCurrentMapping)
                {
                    Mappings.push_back(currentMapping);
                }
                currentMapping = ActionKeyMapping();
                currentMapping.Action.Name = trimmed.Substr(10);
                hasCurrentMapping = true;
            }
            else if (trimmed.StartsWith("ValueType: ") && hasCurrentMapping)
            {
                currentMapping.Action.ValueType =
                    static_cast<EInputActionValueType>(std::stoi(trimmed.Substr(11).c_str()));
            }
            else if (trimmed.StartsWith("BindingType: ") && hasCurrentMapping)
            {
                currentMapping.BindingType = static_cast<EInputBindingType>(std::stoi(trimmed.Substr(13).c_str()));
            }
            else if (trimmed.StartsWith("Code: ") && hasCurrentMapping)
            {
                currentMapping.Code = static_cast<uint32_t>(std::stoul(trimmed.Substr(6).c_str()));
            }
            else if (trimmed.StartsWith("Remappable: ") && hasCurrentMapping)
            {
                currentMapping.IsRemappable = trimmed.Contains("true");
            }
            else if (trimmed.StartsWith("ScaleNegate: ") && hasCurrentMapping)
            {
                currentMapping.Modifier.ScaleNegate = trimmed.Contains("true");
            }
            else if (trimmed.StartsWith("Multiplier: ") && hasCurrentMapping)
            {
                currentMapping.Modifier.Multiplier = std::stof(trimmed.Substr(12).c_str());
            }
            else if (trimmed.StartsWith("Deadzone: ") && hasCurrentMapping)
            {
                currentMapping.Modifier.Deadzone = std::stof(trimmed.Substr(10).c_str());
            }
            return true;
        });

    if (hasCurrentMapping)
    {
        Mappings.push_back(currentMapping);
    }

    return success;
}
