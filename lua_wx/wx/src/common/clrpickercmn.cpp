///////////////////////////////////////////////////////////////////////////////
// Name:        src/common/clrpickercmn.cpp
// Purpose:     wxColourPickerCtrl class implementation
// Author:      Francesco Montorsi (readapted code written by Vadim Zeitlin)
// Modified by:
// Created:     15/04/2006
// RCS-ID:      $Id$
// Copyright:   (c) Vadim Zeitlin, Francesco Montorsi
// Licence:     wxWindows licence
///////////////////////////////////////////////////////////////////////////////

// ============================================================================
// declarations
// ============================================================================

// ----------------------------------------------------------------------------
// headers
// ----------------------------------------------------------------------------

// For compilers that support precompilation, includes "wx.h".
#include "wx/wxprec.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#if wxUSE_COLOURPICKERCTRL

#include "wx/clrpicker.h"

#ifndef WX_PRECOMP
    #include "wx/textctrl.h"
#endif

const char wxColourPickerCtrlNameStr[] = "colourpicker";
const char wxColourPickerWidgetNameStr[] = "colourpickerwidget";

// ============================================================================
// implementation
// ============================================================================

wxDEFINE_EVENT(wxEVT_COMMAND_COLOURPICKER_CHANGED, wxColourPickerEvent);
IMPLEMENT_DYNAMIC_CLASS(wxColourPickerCtrl, wxPickerBase)
IMPLEMENT_DYNAMIC_CLASS(wxColourPickerEvent, wxEvent)

// ----------------------------------------------------------------------------
// wxColourPickerCtrl
// ----------------------------------------------------------------------------

#define M_PICKER     ((wxColourPickerWidget*)m_picker)

bool wxColourPickerCtrl::Create( wxWindow *parent, wxWindowID id,
                        const wxColour &col,
                        const wxPoint &pos, const wxSize &size,
                        long style, const wxValidator& validator,
                        const wxString &name )
{
    if (!wxPickerBase::CreateBase(parent, id, col.GetAsString(), pos, size,
                                  style, validator, name))
        return false;

    // we are not interested to the ID of our picker as we connect
    // to its "changed" event dynamically...
    m_picker = new wxColourPickerWidget(this, wxID_ANY, col,
                                        wxDefaultPosition, wxDefaultSize,
                                        GetPickerStyle(style));

    // complete sizer creation
    wxPickerBase::PostCreation();

    m_picker->Connect(wxEVT_COMMAND_COLOURPICKER_CHANGED,
            wxColourPickerEventHandler(wxColourPickerCtrl::OnColourChange),
            NULL, this);

    return true;
}

void wxColourPickerCtrl::SetColour(const wxColour &col)
{
    M_PICKER->SetColour(col);
    UpdateTextCtrlFromPicker();
}

bool wxColourPickerCtrl::SetColour(const wxString &text)
{
    wxColour col(text);     // smart wxString->wxColour conversion
    if ( !col.IsOk() )
        return false;
    M_PICKER->SetColour(col);
    UpdateTextCtrlFromPicker();

    return true;
}

void wxColourPickerCtrl::UpdatePickerFromTextCtrl()
{
    wxASSERT(m_text);

    if (m_bIgnoreNextTextCtrlUpdate)
    {
        // ignore this update
        m_bIgnoreNextTextCtrlUpdate = false;
        return;
    }

    // wxString -> wxColour conversion
    wxColour col(m_text->GetValue());
    if ( !col.IsOk() )
        return;     // invalid user input

    if (M_PICKER->GetColour() != col)
    {
        M_PICKER->SetColour(col);

        // fire an event
        wxColourPickerEvent event(this, GetId(), col);
        GetEventHandler()->ProcessEvent(event);
    }
}

void wxColourPickerCtrl::UpdateTextCtrlFromPicker()
{
    if (!m_text)
        return;     // no textctrl to update

    // NOTE: this SetValue() will generate an unwanted wxEVT_COMMAND_TEXT_UPDATED
    //       which will trigger a unneeded UpdateFromTextCtrl(); thus before using
    //       SetValue() we set the m_bIgnoreNextTextCtrlUpdate flag...
    m_bIgnoreNextTextCtrlUpdate = true;
    m_text->SetValue(M_PICKER->GetColour().GetAsString());
}



// ----------------------------------------------------------------------------
// wxColourPickerCtrl - event handlers
// ----------------------------------------------------------------------------

void wxColourPickerCtrl::OnColourChange(wxColourPickerEvent &ev)
{
    UpdateTextCtrlFromPicker();

    // the wxColourPickerWidget sent us a colour-change notification.
    // forward this event to our parent
    wxColourPickerEvent event(this, GetId(), ev.GetColour());
    GetEventHandler()->ProcessEvent(event);
}

#endif  // wxUSE_COLOURPICKERCTRL
