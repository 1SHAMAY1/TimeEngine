#pragma once
#include <string>
#include <glm/glm.hpp>
#include "imgui.h"

namespace TE {

    namespace UIUtils {
        
        inline std::string CleanLabel(const std::string& label) {
            size_t pos = label.find("###");
            if (pos != std::string::npos) return label.substr(0, pos);
            return label;
        }

        inline bool DrawVec3Control(const std::string& label, glm::vec3& values, float resetValue = 0.0f, float columnWidth = 100.0f) {
            bool changed = false;
            ImGui::PushID(label.c_str());

            ImGui::Columns(2);
            ImGui::SetColumnWidth(0, columnWidth);
            ImGui::Text(CleanLabel(label).c_str());
            ImGui::NextColumn();

            float lineHeight = ImGui::GetFrameHeight();
            ImVec2 buttonSize = { lineHeight + 3.0f, lineHeight };
            float itemWidth = (ImGui::GetContentRegionAvail().x - buttonSize.x * 3.0f - ImGui::GetStyle().ItemSpacing.x * 2.0f) / 3.0f;

            ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{ 0, 0 });

            // X
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.8f, 0.1f, 0.15f, 1.0f });
            if (ImGui::Button("X", buttonSize)) { values.x = resetValue; changed = true; }
            ImGui::PopStyleColor();
            ImGui::SameLine();
            ImGui::SetNextItemWidth(itemWidth);
            if (ImGui::DragFloat("##X", &values.x, 0.1f, 0.0f, 0.0f, "%.2f")) changed = true;
            ImGui::SameLine();

            // Y
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.2f, 0.7f, 0.2f, 1.0f });
            if (ImGui::Button("Y", buttonSize)) { values.y = resetValue; changed = true; }
            ImGui::PopStyleColor();
            ImGui::SameLine();
            ImGui::SetNextItemWidth(itemWidth);
            if (ImGui::DragFloat("##Y", &values.y, 0.1f, 0.0f, 0.0f, "%.2f")) changed = true;
            ImGui::SameLine();

            // Z
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.1f, 0.25f, 0.8f, 1.0f });
            if (ImGui::Button("Z", buttonSize)) { values.z = resetValue; changed = true; }
            ImGui::PopStyleColor();
            ImGui::SameLine();
            ImGui::SetNextItemWidth(itemWidth);
            if (ImGui::DragFloat("##Z", &values.z, 0.1f, 0.0f, 0.0f, "%.2f")) changed = true;

            ImGui::PopStyleVar();
            ImGui::Columns(1);
            ImGui::PopID();

            return changed;
        }

        inline bool DrawVec2Control(const std::string& label, glm::vec2& values, float resetValue = 0.0f, float columnWidth = 100.0f) {
            bool changed = false;
            ImGui::PushID(label.c_str());

            ImGui::Columns(2);
            ImGui::SetColumnWidth(0, columnWidth);
            ImGui::Text(CleanLabel(label).c_str());
            ImGui::NextColumn();

            float lineHeight = ImGui::GetFrameHeight();
            ImVec2 buttonSize = { lineHeight + 3.0f, lineHeight };
            float itemWidth = (ImGui::GetContentRegionAvail().x - buttonSize.x * 2.0f - ImGui::GetStyle().ItemSpacing.x * 1.0f) / 2.0f;

            ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{ 0, 0 });

            // X
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.8f, 0.1f, 0.15f, 1.0f });
            if (ImGui::Button("X", buttonSize)) { values.x = resetValue; changed = true; }
            ImGui::PopStyleColor();
            ImGui::SameLine();
            ImGui::SetNextItemWidth(itemWidth);
            if (ImGui::DragFloat("##X", &values.x, 0.1f, 0.0f, 0.0f, "%.2f")) changed = true;
            ImGui::SameLine();

            // Y
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.2f, 0.7f, 0.2f, 1.0f });
            if (ImGui::Button("Y", buttonSize)) { values.y = resetValue; changed = true; }
            ImGui::PopStyleColor();
            ImGui::SameLine();
            ImGui::SetNextItemWidth(itemWidth);
            if (ImGui::DragFloat("##Y", &values.y, 0.1f, 0.0f, 0.0f, "%.2f")) changed = true;

            ImGui::PopStyleVar();
            ImGui::Columns(1);
            ImGui::PopID();
            return changed;
        }

        inline bool DrawVec4Control(const std::string& label, glm::vec4& values, float resetValue = 0.0f, float columnWidth = 100.0f) {
            bool changed = false;
            ImGui::PushID(label.c_str());

            ImGui::Columns(2);
            ImGui::SetColumnWidth(0, columnWidth);
            ImGui::Text(CleanLabel(label).c_str());
            ImGui::NextColumn();

            float lineHeight = ImGui::GetFrameHeight();
            ImVec2 buttonSize = { lineHeight + 3.0f, lineHeight };
            float itemWidth = (ImGui::GetContentRegionAvail().x - buttonSize.x * 4.0f - ImGui::GetStyle().ItemSpacing.x * 3.0f) / 4.0f;

            ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{ 0, 0 });

            static const char* labels[] = { "R", "G", "B", "A" };
            static ImVec4 colors[] = { {0.8f, 0.1f, 0.15f, 1.0f}, {0.2f, 0.7f, 0.2f, 1.0f}, {0.1f, 0.25f, 0.8f, 1.0f}, {0.4f, 0.4f, 0.4f, 1.0f} };

            for (int i = 0; i < 4; i++) {
                ImGui::PushStyleColor(ImGuiCol_Button, colors[i]);
                if (ImGui::Button(labels[i], buttonSize)) { values[i] = resetValue; changed = true; }
                ImGui::PopStyleColor();
                ImGui::SameLine();
                ImGui::SetNextItemWidth(itemWidth);
                if (ImGui::DragFloat((std::string("##") + labels[i]).c_str(), &values[i], 0.1f, 0.0f, 0.0f, "%.2f")) changed = true;
                if (i < 3) ImGui::SameLine();
            }

            ImGui::PopStyleVar();
            ImGui::Columns(1);
            ImGui::PopID();
            return changed;
        }

        inline bool DrawColorControl(const std::string& label, glm::vec4& values, float columnWidth = 100.0f) {
            bool changed = false;
            ImGui::PushID(label.c_str());

            ImGui::Columns(2);
            ImGui::SetColumnWidth(0, columnWidth);
            ImGui::Text(CleanLabel(label).c_str());
            ImGui::NextColumn();

            float lineHeight = ImGui::GetFrameHeight();
            ImVec2 pickerSize = { lineHeight * 1.5f, lineHeight };
            float itemWidth = (ImGui::GetContentRegionAvail().x - pickerSize.x - ImGui::GetStyle().ItemSpacing.x * 4.0f) / 4.0f;

            ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{ ImGui::GetStyle().ItemSpacing.x, ImGui::GetStyle().ItemSpacing.y });
            
            if (ImGui::ColorButton("##ColorPicker", *(ImVec4*)&values, ImGuiColorEditFlags_AlphaPreviewHalf, pickerSize))
                ImGui::OpenPopup("##ColorPickerPopup");

            if (ImGui::BeginPopup("##ColorPickerPopup")) {
                if (ImGui::ColorPicker4("##Picker", &values.x, ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_DisplayRGB | ImGuiColorEditFlags_InputRGB)) changed = true;
                ImGui::EndPopup();
            }

            ImGui::SameLine();
            
            // Draw individual components
            for (int i = 0; i < 4; i++) {
                ImGui::SetNextItemWidth(itemWidth);
                const char* ids[] = { "##R", "##G", "##B", "##A" };
                if (ImGui::DragFloat(ids[i], &values[i], 0.01f, 0.0f, 1.0f, "%.2f")) changed = true;
                if (i < 3) ImGui::SameLine();
            }

            ImGui::PopStyleVar();
            ImGui::Columns(1);
            ImGui::PopID();
            return changed;
        }

        inline bool DrawDeleteButton(const std::string& id, float size = 0.0f, float fontScale = 1.3f) {
            if (size == 0.0f) size = ImGui::GetFrameHeight() * 0.8f;
            ImVec2 pos = ImGui::GetCursorScreenPos();
            bool pressed = ImGui::InvisibleButton(id.c_str(), ImVec2{ size, size });
            
            ImVec4 color = { 0.7f, 0.1f, 0.1f, 0.6f };
            if (ImGui::IsItemActive()) color = { 0.5f, 0.1f, 0.1f, 0.7f };
            else if (ImGui::IsItemHovered()) color = { 0.9f, 0.2f, 0.2f, 0.8f };
            
            auto* drawList = ImGui::GetWindowDrawList();
            drawList->AddRectFilled(pos, { pos.x + size, pos.y + size }, ImGui::GetColorU32(color), 3.0f);
            
            const char* text = "x";
            ImGui::SetWindowFontScale(fontScale); 
            ImVec2 textSize = ImGui::CalcTextSize(text);
            drawList->AddText({ pos.x + (size - textSize.x) * 0.5f, pos.y + (size - textSize.y) * 0.5f + 1.0f }, 
                              ImGui::GetColorU32(ImGuiCol_Text), text);
            ImGui::SetWindowFontScale(1.0f); // Reset font scale
            
            return pressed;
        }

        inline bool DrawPlusButton(const std::string& id, float size = 0.0f, float fontScale = 1.3f) {
            if (size == 0.0f) size = ImGui::GetFrameHeight() * 0.8f;
            ImVec2 pos = ImGui::GetCursorScreenPos();
            bool pressed = ImGui::InvisibleButton(id.c_str(), ImVec2{ size, size });
            
            ImVec4 color = { 0.1f, 0.7f, 0.1f, 0.6f };
            if (ImGui::IsItemActive()) color = { 0.1f, 0.5f, 0.1f, 0.7f };
            else if (ImGui::IsItemHovered()) color = { 0.2f, 0.9f, 0.2f, 0.8f };
            
            auto* drawList = ImGui::GetWindowDrawList();
            drawList->AddRectFilled(pos, { pos.x + size, pos.y + size }, ImGui::GetColorU32(color), 3.0f);
            
            const char* text = "+";
            ImGui::SetWindowFontScale(fontScale); 
            ImVec2 textSize = ImGui::CalcTextSize(text);
            drawList->AddText({ pos.x + (size - textSize.x) * 0.5f, pos.y + (size - textSize.y) * 0.5f + 1.0f }, 
                              ImGui::GetColorU32(ImGuiCol_Text), text);
            ImGui::SetWindowFontScale(1.0f); // Reset font scale
            
            return pressed;
        }
    }
}
