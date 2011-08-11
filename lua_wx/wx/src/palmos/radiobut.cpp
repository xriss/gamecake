/////////////////////////////////////////////////////////////////////////////
// Name:        src/palmos/radiobut.cpp
// Purpose:     wxRadioButton
// Author:      William Osborne - minimal working wxPalmOS port
// Modified by: Wlodzimierz ABX Skiba - native wxRadioButton implementation
// Created:     10/13/04
// RCS-ID:      $Id$
// Copyright:   (c) William Osborne, Wlodzimierz Skiba
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

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

#if wxUSE_RADIOBTN

#include "wx/radiobut.h"

#ifndef WX_PRECOMP
    #include "wx/settings.h"
    #include "wx/dcscreen.h"
#endif

#include <Control.h>

// ============================================================================
// wxRadioButton implementation
// ============================================================================

// ----------------------------------------------------------------------------
// wxRadioButton creation
// ----------------------------------------------------------------------------

void wxRadioButton::Init()
{
    m_radioStyle = pushButtonCtl;
    m_groupID = 0;
}

bool wxRadioButton::Create(wxWindow *parent,
                           wxWindowID id,
                           const wxString& label,
                           const wxPoint& pos,
                           const wxSize& size,
                           long style,
                           const wxValidator& validator,
                           const wxString& name)
{
    // replace native push button with native checkbox
    if ( style & wxRB_USE_CHECKBOX )
        m_radioStyle = checkboxCtl;

    if(!wxControl::Create(parent, id, pos, size, style, validator, name))
        return false;

    return wxControl::PalmCreateControl(
                          // be sure only one of two possibilities was taken
                          m_radioStyle == checkboxCtl ? checkboxCtl : pushButtonCtl,
                          label,
                          pos,
                          size,
                          m_groupID
                      );
}

void wxRadioButton::SetGroup(uint8_t group)
{
    m_groupID = group;
}

// ----------------------------------------------------------------------------
// wxRadioButton functions
// ----------------------------------------------------------------------------

void wxRadioButton::SetValue(bool value)
{
    SetBoolValue(value);
}

bool wxRadioButton::GetValue() const
{
    return GetBoolValue();
}

// ----------------------------------------------------------------------------
// wxRadioButton event processing
// ----------------------------------------------------------------------------

bool wxRadioButton::SendClickEvent()
{
    wxCommandEvent event(wxEVT_COMMAND_RADIOBUTTON_SELECTED, GetId());
    event.SetInt(GetValue());
    event.SetEventObject(this);
    return ProcessCommand(event);
}

void wxRadioButton::Command (wxCommandEvent& event)
{
}

// ----------------------------------------------------------------------------
// wxRadioButton geometry
// ----------------------------------------------------------------------------

wxSize wxRadioButton::DoGetBestSize() const
{
    return wxSize(0,0);
}

#endif // wxUSE_RADIOBTN

