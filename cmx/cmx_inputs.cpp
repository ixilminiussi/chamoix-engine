#include "cmx_inputs.h"
#include "imgui.h"
#include <spdlog/spdlog.h>

namespace cmx
{

void Axis::editor(const std::string &label)
{
    auto findSelected = [&]() {
        int i = 0;

        for (auto &pair : CMX_AXIS_DICTIONARY)
        {
            if (*this == *pair.second)
            {
                return i;
            }
            i++;
        }

        return 0;
    };

    int currentItem = findSelected();

    ImGui::SetNextItemWidth(200);
    if (ImGui::BeginCombo(label.c_str(), CMX_AXIS_DICTIONARY[currentItem].first))
    {
        for (const std::pair<const char *, Axis *> &pair : CMX_AXIS_DICTIONARY)
        {
            bool isSelected = (*pair.second == *this);

            if (ImGui::Selectable(pair.first, isSelected))
            {
                code = pair.second->code;
                source = pair.second->source;
                id = pair.second->id;
            }

            if (isSelected)
            {
                ImGui::SetItemDefaultFocus();
            }
        }
        ImGui::EndCombo();
    }
}

void Button::editor(const std::string &label)
{
    auto findSelected = [&]() {
        int i = 0;

        for (const std::pair<const char *, Button *> &pair : CMX_BUTTON_DICTIONARY)
        {
            if (*this == *pair.second)
            {
                return i;
            }
            i++;
        }

        return 0;
    };

    ImGui::SetNextItemWidth(200);
    if (ImGui::BeginCombo(label.c_str(), CMX_BUTTON_DICTIONARY[findSelected()].first))
    {
        for (const std::pair<const char *, Button *> &pair : CMX_BUTTON_DICTIONARY)
        {
            bool isSelected = (*pair.second == *this);

            if (ImGui::Selectable(pair.first, isSelected))
            {
                code = pair.second->code;
                source = pair.second->source;
                id = pair.second->id;
            }

            if (isSelected)
            {
                ImGui::SetItemDefaultFocus();
            }
        }
        ImGui::EndCombo();
    }
}

} // namespace cmx
