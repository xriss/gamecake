///////////////////////////////////////////////////////////////////////////////
// Name:        src/palmos/menuitem.cpp
// Purpose:     wxMenuItem implementation
// Author:      William Osborne - minimal working wxPalmOS port
// Modified by:
// Created:     10/13/04
// RCS-ID:      $Id$
// Copyright:   (c) William Osborne
// Licence:     wxWindows licence
///////////////////////////////////////////////////////////////////////////////

// ===========================================================================
// declarations
// ===========================================================================

// ---------------------------------------------------------------------------
// headers
// ---------------------------------------------------------------------------

// For compilers that support precompilation, includes "wx.h".
#include "wx/wxprec.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#if wxUSE_MENUS

#include "wx/menuitem.h"

#ifndef WX_PRECOMP
    #include "wx/font.h"
    #include "wx/bitmap.h"
    #include "wx/settings.h"
    #include "wx/window.h"
    #include "wx/accel.h"
    #include "wx/string.h"
    #include "wx/log.h"
    #include "wx/menu.h"
#endif

#if wxUSE_ACCEL
    #include "wx/accel.h"
#endif // wxUSE_ACCEL

// ---------------------------------------------------------------------------
// macro
// ---------------------------------------------------------------------------

// conditional compilation
#if wxUSE_OWNER_DRAWN
    #define OWNER_DRAWN_ONLY( code ) if ( IsOwnerDrawn() ) code
#else // !wxUSE_OWNER_DRAWN
    #define OWNER_DRAWN_ONLY( code )
#endif // wxUSE_OWNER_DRAWN/!wxUSE_OWNER_DRAWN

// ============================================================================
// implementation
// ============================================================================

// ----------------------------------------------------------------------------
// wxMenuItem
// ----------------------------------------------------------------------------

// ctor & dtor
// -----------

wxMenuItem::wxMenuItem(wxMenu *pParentMenu,
                       int id,
                       const wxString& text,
                       const wxString& strHelp,
                       wxItemKind kind,
                       wxMenu *pSubMenu)
          : wxMenuItemBase(pParentMenu, id, text, strHelp, kind, pSubMenu)
#if wxUSE_OWNER_DRAWN
            , wxOwnerDrawn(text, kind == wxITEM_CHECK, true)
#endif // owner drawn
{
}

wxMenuItem::wxMenuItem(wxMenu *parentMenu,
                       int id,
                       const wxString& text,
                       const wxString& help,
                       bool isCheckable,
                       wxMenu *subMenu)
          : wxMenuItemBase(parentMenu, id, text, help,
                           isCheckable ? wxITEM_CHECK : wxITEM_NORMAL, subMenu)
#if wxUSE_OWNER_DRAWN
           , wxOwnerDrawn(text, isCheckable, true)
#endif // owner drawn
{
}

void wxMenuItem::Init()
{
}

wxMenuItem::~wxMenuItem()
{
}

// misc
// ----

// get item state
// --------------

bool wxMenuItem::IsChecked() const
{
    return false;
}

// radio group stuff
// -----------------

void wxMenuItem::SetAsRadioGroupStart()
{
}

void wxMenuItem::SetRadioGroupStart(int start)
{
}

void wxMenuItem::SetRadioGroupEnd(int end)
{
}

// change item state
// -----------------

void wxMenuItem::Enable(bool enable)
{
}

void wxMenuItem::Check(bool check)
{
}

void wxMenuItem::SetItemLabel(const wxString& text)
{
}

void wxMenuItem::SetCheckable(bool checkable)
{
}

// ----------------------------------------------------------------------------
// wxMenuItemBase
// ----------------------------------------------------------------------------

wxMenuItem *wxMenuItemBase::New(wxMenu *parentMenu,
                                int id,
                                const wxString& name,
                                const wxString& help,
                                wxItemKind kind,
                                wxMenu *subMenu)
{
    return new wxMenuItem(parentMenu, id, name, help, kind, subMenu);
}

#endif // wxUSE_MENUS
