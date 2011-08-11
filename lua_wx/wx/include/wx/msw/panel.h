///////////////////////////////////////////////////////////////////////////////
// Name:        wx/msw/panel.h
// Purpose:     wxMSW-specific wxPanel class.
// Author:      Vadim Zeitlin
// Created:     2011-03-18
// RCS-ID:      $Id: wxhead.h,v 1.12 2010-04-22 12:44:51 zeitlin Exp $
// Copyright:   (c) 2011 Vadim Zeitlin <vadim@wxwidgets.org>
// Licence:     wxWindows licence
///////////////////////////////////////////////////////////////////////////////

#ifndef _WX_MSW_PANEL_H_
#define _WX_MSW_PANEL_H_

class WXDLLIMPEXP_FWD_CORE wxBrush;

// ----------------------------------------------------------------------------
// wxPanel
// ----------------------------------------------------------------------------

class WXDLLIMPEXP_CORE wxPanel : public wxPanelBase
{
public:
    wxPanel() { Init(); }

    wxPanel(wxWindow *parent,
            wxWindowID winid = wxID_ANY,
            const wxPoint& pos = wxDefaultPosition,
            const wxSize& size = wxDefaultSize,
            long style = wxTAB_TRAVERSAL | wxNO_BORDER,
            const wxString& name = wxPanelNameStr)
    {
        Init();

        Create(parent, winid, pos, size, style, name);
    }

    // This is overridden for MSW to return true for all panels that are child
    // of a window with themed background (such as wxNotebook) which should
    // show through the child panels.
    virtual bool HasTransparentBackground();


#ifdef WXWIN_COMPATIBILITY_2_8
    wxDEPRECATED_CONSTRUCTOR(
    wxPanel(wxWindow *parent,
            int x, int y, int width, int height,
            long style = wxTAB_TRAVERSAL | wxNO_BORDER,
            const wxString& name = wxPanelNameStr)
    {
        Create(parent, wxID_ANY, wxPoint(x, y), wxSize(width, height), style, name);
    }
    )
#endif // WXWIN_COMPATIBILITY_2_8

protected:
    void Init()
    {
        m_backgroundBrush = NULL;
    }

    virtual void DoSetBackgroundBitmap(const wxBitmap& bmp);
    virtual WXHBRUSH MSWGetCustomBgBrush();

private:
    wxBrush *m_backgroundBrush;

    wxDECLARE_DYNAMIC_CLASS_NO_COPY(wxPanel);
};

#endif // _WX_MSW_PANEL_H_
