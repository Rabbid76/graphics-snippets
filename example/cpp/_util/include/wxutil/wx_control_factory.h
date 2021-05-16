#ifndef __WX_UTILITY_FACTORY__H__
#define __WX_UTILITY_FACTORY__H__

#include <wxutil/wx_include.h>

#include <vector>
#include <string>

namespace wx_utility
{
    wxStaticText* new_static_text(wxWindow* parent, int id, wxSizer* sizer, const std::wstring& text)
    {
        auto static_text = new wxStaticText(parent, id, wxString(text.c_str()), wxDefaultPosition);
        if (sizer != nullptr)
            sizer->Add(static_text);
        return static_text;
    }

    template <typename Class, typename EventArg>
    wxComboBox* new_selction_box(
        wxWindow* parent, int id, wxSizer *sizer, std::vector<std::wstring>&& items, int initial_index, Class* target, void (Class::* event_method)(EventArg&))
    {
        wxArrayString strings;
        for (const auto& name : items)
            strings.Add(name.c_str());
        auto selection_box = new wxComboBox(
            parent, id, items[initial_index], wxDefaultPosition, wxDefaultSize, strings, wxCB_DROPDOWN | wxCB_READONLY);
        selection_box->Bind(wxEVT_COMBOBOX, event_method, target);
        if (sizer != nullptr)
            sizer->Add(selection_box);
        
        return selection_box;
    }
}

#endif