/////////////////////////////////////////////////////////////////////////////
// Name:        wx/richtext/richtextformatdlg.h
// Purpose:     Formatting dialog for wxRichTextCtrl
// Author:      Julian Smart
// Modified by:
// Created:     2006-10-01
// RCS-ID:      $Id$
// Copyright:   (c) Julian Smart
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_RICHTEXTFORMATDLG_H_
#define _WX_RICHTEXTFORMATDLG_H_

/*!
 * Includes
 */

#include "wx/defs.h"

#if wxUSE_RICHTEXT

#include "wx/propdlg.h"
#include "wx/bookctrl.h"

#if wxUSE_HTML
#include "wx/htmllbox.h"
#endif

#include "wx/richtext/richtextbuffer.h"
#include "wx/richtext/richtextstyles.h"
#include "wx/richtext/richtextuicustomization.h"

class WXDLLIMPEXP_FWD_RICHTEXT wxRichTextFormattingDialog;
class WXDLLIMPEXP_FWD_CORE wxImageList;
class WXDLLIMPEXP_FWD_CORE wxComboBox;
class WXDLLIMPEXP_FWD_CORE wxCheckBox;

/*!
 * Flags determining the pages and buttons to be created in the dialog
 */

#define wxRICHTEXT_FORMAT_STYLE_EDITOR      0x0001
#define wxRICHTEXT_FORMAT_FONT              0x0002
#define wxRICHTEXT_FORMAT_TABS              0x0004
#define wxRICHTEXT_FORMAT_BULLETS           0x0008
#define wxRICHTEXT_FORMAT_INDENTS_SPACING   0x0010
#define wxRICHTEXT_FORMAT_LIST_STYLE        0x0020
#define wxRICHTEXT_FORMAT_MARGINS           0x0040
#define wxRICHTEXT_FORMAT_SIZE              0x0080
#define wxRICHTEXT_FORMAT_BORDERS           0x0100
#define wxRICHTEXT_FORMAT_BACKGROUND        0x0200

#define wxRICHTEXT_FORMAT_HELP_BUTTON       0x1000

/*!
 * Indices for bullet styles in list control
 */

enum {
    wxRICHTEXT_BULLETINDEX_NONE = 0,
    wxRICHTEXT_BULLETINDEX_ARABIC,
    wxRICHTEXT_BULLETINDEX_UPPER_CASE,
    wxRICHTEXT_BULLETINDEX_LOWER_CASE,
    wxRICHTEXT_BULLETINDEX_UPPER_CASE_ROMAN,
    wxRICHTEXT_BULLETINDEX_LOWER_CASE_ROMAN,
    wxRICHTEXT_BULLETINDEX_OUTLINE,
    wxRICHTEXT_BULLETINDEX_SYMBOL,
    wxRICHTEXT_BULLETINDEX_BITMAP,
    wxRICHTEXT_BULLETINDEX_STANDARD
};

/*!
 * Shorthand for common combinations of pages
 */

#define wxRICHTEXT_FORMAT_PARAGRAPH         (wxRICHTEXT_FORMAT_INDENTS_SPACING | wxRICHTEXT_FORMAT_BULLETS | wxRICHTEXT_FORMAT_TABS | wxRICHTEXT_FORMAT_FONT)
#define wxRICHTEXT_FORMAT_CHARACTER         (wxRICHTEXT_FORMAT_FONT)
#define wxRICHTEXT_FORMAT_STYLE             (wxRICHTEXT_FORMAT_PARAGRAPH | wxRICHTEXT_FORMAT_STYLE_EDITOR)

/*!
 * Factory for formatting dialog
 */

class WXDLLIMPEXP_RICHTEXT wxRichTextFormattingDialogFactory: public wxObject
{
public:
    wxRichTextFormattingDialogFactory() {}
    virtual ~wxRichTextFormattingDialogFactory() {}

// Overridables

    /// Create all pages, under the dialog's book control, also calling AddPage
    virtual bool CreatePages(long pages, wxRichTextFormattingDialog* dialog);

    /// Create a page, given a page identifier
    virtual wxPanel* CreatePage(int page, wxString& title, wxRichTextFormattingDialog* dialog);

    /// Enumerate all available page identifiers
    virtual int GetPageId(int i) const;

    /// Get the number of available page identifiers
    virtual int GetPageIdCount() const;

    /// Get the image index for the given page identifier
    virtual int GetPageImage(int WXUNUSED(id)) const { return -1; }

    /// Invoke help for the dialog
    virtual bool ShowHelp(int page, wxRichTextFormattingDialog* dialog);

    /// Set the sheet style, called at the start of wxRichTextFormattingDialog::Create
    virtual bool SetSheetStyle(wxRichTextFormattingDialog* dialog);

    /// Create the main dialog buttons
    virtual bool CreateButtons(wxRichTextFormattingDialog* dialog);
};

/*!
 * Formatting dialog for a wxRichTextCtrl
 */

class WXDLLIMPEXP_RICHTEXT wxRichTextFormattingDialog: public wxPropertySheetDialog
{
DECLARE_CLASS(wxRichTextFormattingDialog)
DECLARE_HELP_PROVISION()

public:
    wxRichTextFormattingDialog() { Init(); }

    wxRichTextFormattingDialog(long flags, wxWindow* parent, const wxString& title = wxGetTranslation(wxT("Formatting")), wxWindowID id = wxID_ANY,
        const wxPoint& pos = wxDefaultPosition, const wxSize& sz = wxDefaultSize,
        long style = wxDEFAULT_DIALOG_STYLE)
    {
        Init();
        Create(flags, parent, title, id, pos, sz, style);
    }

    ~wxRichTextFormattingDialog();

    void Init();

    bool Create(long flags, wxWindow* parent, const wxString& title = wxGetTranslation(wxT("Formatting")), wxWindowID id = wxID_ANY,
        const wxPoint& pos = wxDefaultPosition, const wxSize& sz = wxDefaultSize,
        long style = wxDEFAULT_DIALOG_STYLE);

    /// Get attributes from the given range
    virtual bool GetStyle(wxRichTextCtrl* ctrl, const wxRichTextRange& range);

    /// Set the attributes and optionally update the display
    virtual bool SetStyle(const wxRichTextAttr& style, bool update = true);

    /// Set the style definition and optionally update the display
    virtual bool SetStyleDefinition(const wxRichTextStyleDefinition& styleDef, wxRichTextStyleSheet* sheet, bool update = true);

    /// Get the style definition, if any
    virtual wxRichTextStyleDefinition* GetStyleDefinition() const { return m_styleDefinition; }

    /// Get the style sheet, if any
    virtual wxRichTextStyleSheet* GetStyleSheet() const { return m_styleSheet; }

    /// Update the display
    virtual bool UpdateDisplay();

    /// Apply attributes to the given range
    virtual bool ApplyStyle(wxRichTextCtrl* ctrl, const wxRichTextRange& range, int flags = wxRICHTEXT_SETSTYLE_WITH_UNDO|wxRICHTEXT_SETSTYLE_OPTIMIZE);
    
    /// Apply attributes to the object being edited, if any
    virtual bool ApplyStyle(wxRichTextCtrl* ctrl, int flags = wxRICHTEXT_SETSTYLE_WITH_UNDO);

    /// Gets and sets the attributes
    const wxRichTextAttr& GetAttributes() const { return m_attributes; }
    wxRichTextAttr& GetAttributes() { return m_attributes; }
    void SetAttributes(const wxRichTextAttr& attr) { m_attributes = attr; }
#if 0
    /// Gets and sets the attributes that the user wants to reset
    const wxRichTextAttr& GetResetAttributes() const { return m_resetAttributes; }
    wxRichTextAttr& GetResetAttributes() { return m_resetAttributes; }
    void SetResetAttributes(const wxRichTextAttr& attr) { m_resetAttributes = attr; }
#endif
    /// If editing the attributes for a particular object, such as an image,
    /// set the object so the code can initialize attributes such as size correctly.
    wxRichTextObject* GetObject() const { return m_object; }
    void SetObject(wxRichTextObject* obj) { m_object = obj; }

    /// Transfers the data and from to the window
    virtual bool TransferDataToWindow();
    virtual bool TransferDataFromWindow();

    /// Apply the styles when a different tab is selected, so the previews are
    /// up to date
    void OnTabChanged(wxBookCtrlEvent& event);

    /// Respond to help command
    void OnHelp(wxCommandEvent& event);
    void OnUpdateHelp(wxUpdateUIEvent& event);

    /// Set/get image list
    void SetImageList(wxImageList* imageList) { m_imageList = imageList; }
    wxImageList* GetImageList() const { return m_imageList; }

    /// Get/set formatting factory object
    static void SetFormattingDialogFactory(wxRichTextFormattingDialogFactory* factory);
    static wxRichTextFormattingDialogFactory* GetFormattingDialogFactory() { return ms_FormattingDialogFactory; }

    /// Helper for pages to get the top-level dialog
    static wxRichTextFormattingDialog* GetDialog(wxWindow* win);

    /// Helper for pages to get the attributes
    static wxRichTextAttr* GetDialogAttributes(wxWindow* win);

    /// Helper for pages to get the reset attributes
    static wxRichTextAttr* GetDialogResetAttributes(wxWindow* win);

    /// Helper for pages to get the style
    static wxRichTextStyleDefinition* GetDialogStyleDefinition(wxWindow* win);

    /// Should we show tooltips?
    static bool ShowToolTips() { return sm_showToolTips; }

    /// Determines whether tooltips will be shown
    static void SetShowToolTips(bool show) { sm_showToolTips = show; }

    /// Set the dimension into the value and units controls
    static void SetDimensionValue(wxTextAttrDimension& dim, wxTextCtrl* valueCtrl, wxComboBox* unitsCtrl, wxCheckBox* checkBox);

    /// Get the dimension from the value and units controls
    static void GetDimensionValue(wxTextAttrDimension& dim, wxTextCtrl* valueCtrl, wxComboBox* unitsCtrl, wxCheckBox* checkBox);
    
    /// Convert CM to MM
    static bool ConvertFromString(const wxString& string, int& ret, int scale);

    /// Map book control page index to our page id
    void AddPageId(int id) { m_pageIds.Add(id); }
    
    /// Find a page by class
    wxWindow* FindPage(wxClassInfo* info) const;

protected:

    wxImageList*                                m_imageList;
    wxRichTextAttr                              m_attributes;
    //wxRichTextAttr                              m_resetAttributes;
    wxRichTextStyleDefinition*                  m_styleDefinition;
    wxRichTextStyleSheet*                       m_styleSheet;
    wxRichTextObject*                           m_object;
    wxArrayInt                                  m_pageIds; // mapping of book control indexes to page ids

    static wxRichTextFormattingDialogFactory*   ms_FormattingDialogFactory;
    static bool                                 sm_showToolTips;

DECLARE_EVENT_TABLE()
};

//-----------------------------------------------------------------------------
// helper class - wxRichTextFontPreviewCtrl
//-----------------------------------------------------------------------------

class WXDLLIMPEXP_RICHTEXT wxRichTextFontPreviewCtrl : public wxWindow
{
public:
    wxRichTextFontPreviewCtrl(wxWindow *parent, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& sz = wxDefaultSize, long style = 0);

    void SetTextEffects(int effects) { m_textEffects = effects; }
    int GetTextEffects() const { return m_textEffects; }

private:
    int m_textEffects;

    void OnPaint(wxPaintEvent& event);
    DECLARE_EVENT_TABLE()
};

/*
 * A control for displaying a small preview of a colour or bitmap
 */

class WXDLLIMPEXP_RICHTEXT wxRichTextColourSwatchCtrl: public wxControl
{
    DECLARE_CLASS(wxRichTextColourSwatchCtrl)
public:
    wxRichTextColourSwatchCtrl(wxWindow* parent, wxWindowID id, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = 0);
    ~wxRichTextColourSwatchCtrl();

    void OnMouseEvent(wxMouseEvent& event);

    void SetColour(const wxColour& colour) { m_colour = colour; SetBackgroundColour(m_colour); }

    wxColour& GetColour() { return m_colour; }

    virtual wxSize DoGetBestSize() const { return GetSize(); }

protected:
    wxColour    m_colour;

DECLARE_EVENT_TABLE()
};

/*!
 * wxRichTextFontListBox class declaration
 * A listbox to display fonts.
 */

class WXDLLIMPEXP_RICHTEXT wxRichTextFontListBox: public wxHtmlListBox
{
    DECLARE_CLASS(wxRichTextFontListBox)
    DECLARE_EVENT_TABLE()

public:
    wxRichTextFontListBox()
    {
        Init();
    }
    wxRichTextFontListBox(wxWindow* parent, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition,
        const wxSize& size = wxDefaultSize, long style = 0);
    virtual ~wxRichTextFontListBox();

    void Init()
    {
    }

    bool Create(wxWindow* parent, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition,
        const wxSize& size = wxDefaultSize, long style = 0);

    /// Creates a suitable HTML fragment for a font
    wxString CreateHTML(const wxString& facename) const;

    /// Get font name for index
    wxString GetFaceName(size_t i) const ;

    /// Set selection for string, returning the index.
    int SetFaceNameSelection(const wxString& name);

    /// Updates the font list
    void UpdateFonts();

    /// Does this face name exist?
    bool HasFaceName(const wxString& faceName) const { return m_faceNames.Index(faceName) != wxNOT_FOUND; }

    /// Returns the array of face names
    const wxArrayString& GetFaceNames() const { return m_faceNames; }

protected:
    /// Returns the HTML for this item
    virtual wxString OnGetItem(size_t n) const;

private:

    wxArrayString           m_faceNames;
};

#endif
    // wxUSE_RICHTEXT

#endif
    // _WX_RICHTEXTFORMATDLG_H_
