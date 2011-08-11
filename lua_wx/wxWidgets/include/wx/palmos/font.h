/////////////////////////////////////////////////////////////////////////////
// Name:        wx/palmos/font.h
// Purpose:     wxFont class
// Author:      William Osborne - minimal working wxPalmOS port
// Modified by: Yunhui Fu
// Created:     10/14/04
// RCS-ID:      $Id$
// Copyright:   (c) William Osborne
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_FONT_H_
#define _WX_FONT_H_

#include "wx/gdicmn.h"

// ----------------------------------------------------------------------------
// wxFont
// ----------------------------------------------------------------------------

class WXDLLIMPEXP_CORE wxFont : public wxFontBase
{
public:
    // ctors and such
    wxFont() { }

#if FUTURE_WXWIN_COMPATIBILITY_3_0
    wxFont(int size,
           int family,
           int style,
           int weight,
           bool underlined = false,
           const wxString& face = wxEmptyString,
           wxFontEncoding encoding = wxFONTENCODING_DEFAULT)
    {
        (void)Create(size, (wxFontFamily)family, (wxFontStyle)style, (wxFontWeight)weight, underlined, face, encoding);
    }
#endif

    wxFont(int size,
           wxFontFamily family,
           wxFontStyle style,
           wxFontWeight weight,
           bool underlined = false,
           const wxString& face = wxEmptyString,
           wxFontEncoding encoding = wxFONTENCODING_DEFAULT)
    {
        Create(size, family, style, weight, underlined, face, encoding);
    }

    bool Create(int size,
                wxFontFamily family,
                wxFontStyle style,
                wxFontWeight weight,
                bool underlined = false,
                const wxString& face = wxEmptyString,
                wxFontEncoding encoding = wxFONTENCODING_DEFAULT);

    wxFont(const wxSize& pixelSize,
           int family,
           int style,
           int weight,
           bool underlined = false,
           const wxString& face = wxEmptyString,
           wxFontEncoding encoding = wxFONTENCODING_DEFAULT)
    {
        (void)Create(pixelSize, family, style, weight,
                     underlined, face, encoding);
    }

    wxFont(const wxNativeFontInfo& info, WXHFONT hFont = 0)
    {
        Create(info, hFont);
    }

    wxFont(const wxString& fontDesc);

    bool Create(const wxSize& pixelSize,
                int family,
                int style,
                int weight,
                bool underlined = false,
                const wxString& face = wxEmptyString,
                wxFontEncoding encoding = wxFONTENCODING_DEFAULT)
    {
        return DoCreate(-1, pixelSize, true, family, style,
                        weight, underlined, face, encoding);
    }

    bool Create(const wxNativeFontInfo& info, WXHFONT hFont = 0);

    virtual ~wxFont();

    // wxFontBase overridden functions
    virtual wxString GetNativeFontInfoDesc() const;
    virtual wxString GetNativeFontInfoUserDesc() const;

    // implement base class pure virtuals
    virtual int GetPointSize() const;
    virtual wxSize GetPixelSize() const;
    virtual bool IsUsingSizeInPixels() const;
    virtual wxFontStyle GetStyle() const;
    virtual wxFontWeight GetWeight() const;
    virtual bool GetUnderlined() const;
    virtual wxString GetFaceName() const;
    virtual wxFontEncoding GetEncoding() const;
    virtual const wxNativeFontInfo *GetNativeFontInfo() const;

    virtual void SetPointSize(int pointSize);
    virtual void SetPixelSize(const wxSize& pixelSize);
    virtual void SetFamily(wxFontFamily family);
    virtual void SetStyle(wxFontStyle style);
    virtual void SetWeight(wxFontWeight weight);
    virtual bool SetFaceName(const wxString& faceName);
    virtual void SetUnderlined(bool underlined);
    virtual void SetEncoding(wxFontEncoding encoding);

    wxDECLARE_COMMON_FONT_METHODS();

    virtual bool IsFixedWidth() const;

    // implementation only from now on
    // -------------------------------

    virtual bool IsFree() const;
    virtual bool RealizeResource();
    virtual WXHANDLE GetResourceHandle() const;
    virtual bool FreeResource(bool force = false);

protected:
    // real font creation function, used in all cases
    bool DoCreate(int size,
                  const wxSize& pixelSize,
                  bool sizeUsingPixels,
                  int family,
                  int style,
                  int weight,
                  bool underlined = false,
                  const wxString& face = wxEmptyString,
                  wxFontEncoding encoding = wxFONTENCODING_DEFAULT);

    virtual void DoSetNativeFontInfo(const wxNativeFontInfo& info);
    virtual wxFontFamily DoGetFamily() const;

    // implement wxObject virtuals which are used by AllocExclusive()
    virtual wxGDIRefData *CreateGDIRefData() const;
    virtual wxGDIRefData *CloneGDIRefData(const wxGDIRefData *data) const;

private:
    DECLARE_DYNAMIC_CLASS(wxFont)
};

#endif // _WX_FONT_H_
