#ifndef __WX_UTILITY_FACTORY__
#define __WX_UTILITY_FACTORY__

#include <wxutil/wx_include.h>

#include <vector>
#include <string>

namespace wx_utility
{
    template <typename Class, typename EventArg>
    wxComboBox* new_selction_box(
        wxWindow* parent, int id, std::vector<std::wstring>&& items, int initial_index, Class* target, void (Class::* event_method)(EventArg&))
    {
        wxArrayString strings;
        for (const auto& name : items)
            strings.Add(name.c_str());
        auto selection_box = new wxComboBox(
            parent, id, items[initial_index], wxDefaultPosition, wxDefaultSize, strings, wxCB_DROPDOWN | wxCB_READONLY);
        selection_box->Bind(wxEVT_COMBOBOX, event_method, target);
        
        return selection_box;
    }
}

#endif