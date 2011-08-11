///////////////////////////////////////////////////////////////////////////////
// Name:        wx/gtk/dvrenderers.h
// Purpose:     All GTK wxDataViewCtrl renderer classes
// Author:      Robert Roebling, Vadim Zeitlin
// Created:     2009-11-07 (extracted from wx/gtk/dataview.h)
// RCS-ID:      $Id$
// Copyright:   (c) 2006 Robert Roebling
//              (c) 2009 Vadim Zeitlin <vadim@wxwidgets.org>
// Licence:     wxWindows licence
///////////////////////////////////////////////////////////////////////////////

#ifndef _WX_GTK_DVRENDERERS_H_
#define _WX_GTK_DVRENDERERS_H_

typedef struct _GdkRectangle GdkRectangle;

// ---------------------------------------------------------
// wxDataViewTextRenderer
// ---------------------------------------------------------

class WXDLLIMPEXP_ADV wxDataViewTextRenderer: public wxDataViewRenderer
{
public:
    wxDataViewTextRenderer( const wxString &varianttype = "string",
                            wxDataViewCellMode mode = wxDATAVIEW_CELL_INERT,
                            int align = wxDVR_DEFAULT_ALIGNMENT );

    virtual bool SetValue( const wxVariant &value )
    {
        return SetTextValue(value);
    }

    virtual bool GetValue( wxVariant &value ) const
    {
        wxString str;
        if ( !GetTextValue(str) )
            return false;

        value = str;

        return true;
    }

    virtual void SetAlignment( int align );

    virtual bool GtkSupportsAttrs() const { return true; }
    virtual bool GtkSetAttr(const wxDataViewItemAttr& attr);

    virtual GtkCellRendererText *GtkGetTextRenderer() const;

protected:
    // implementation of Set/GetValue()
    bool SetTextValue(const wxString& str);
    bool GetTextValue(wxString& str) const;


    DECLARE_DYNAMIC_CLASS_NO_COPY(wxDataViewTextRenderer)
};

// ---------------------------------------------------------
// wxDataViewBitmapRenderer
// ---------------------------------------------------------

class WXDLLIMPEXP_ADV wxDataViewBitmapRenderer: public wxDataViewRenderer
{
public:
    wxDataViewBitmapRenderer( const wxString &varianttype = "wxBitmap",
                              wxDataViewCellMode mode = wxDATAVIEW_CELL_INERT,
                              int align = wxDVR_DEFAULT_ALIGNMENT );

    bool SetValue( const wxVariant &value );
    bool GetValue( wxVariant &value ) const;

protected:
    DECLARE_DYNAMIC_CLASS_NO_COPY(wxDataViewBitmapRenderer)
};

// ---------------------------------------------------------
// wxDataViewToggleRenderer
// ---------------------------------------------------------

class WXDLLIMPEXP_ADV wxDataViewToggleRenderer: public wxDataViewRenderer
{
public:
    wxDataViewToggleRenderer( const wxString &varianttype = "bool",
                              wxDataViewCellMode mode = wxDATAVIEW_CELL_INERT,
                              int align = wxDVR_DEFAULT_ALIGNMENT );

    bool SetValue( const wxVariant &value );
    bool GetValue( wxVariant &value ) const;

protected:
    DECLARE_DYNAMIC_CLASS_NO_COPY(wxDataViewToggleRenderer)
};

// ---------------------------------------------------------
// wxDataViewCustomRenderer
// ---------------------------------------------------------

class WXDLLIMPEXP_ADV wxDataViewCustomRenderer: public wxDataViewCustomRendererBase
{
public:
    wxDataViewCustomRenderer( const wxString &varianttype = "string",
                              wxDataViewCellMode mode = wxDATAVIEW_CELL_INERT,
                              int align = wxDVR_DEFAULT_ALIGNMENT,
                              bool no_init = false );
    virtual ~wxDataViewCustomRenderer();


    // Create DC on request
    virtual wxDC *GetDC();

    // override the base class function to use GTK text cell renderer
    virtual void RenderText(const wxString& text,
                            int xoffset,
                            wxRect cell,
                            wxDC *dc,
                            int state);

    // store GTK render call parameters for possible later use
    void GTKStashRenderParams(GdkWindow *window,
                              GtkWidget *widget,
                              GdkRectangle *background_area,
                              GdkRectangle *expose_area,
                              int flags)
    {
        m_renderParams.window = window;
        m_renderParams.widget = widget;
        m_renderParams.background_area = background_area;
        m_renderParams.expose_area = expose_area;
        m_renderParams.flags = flags;
    }

    // we may or not support attributes, as we don't know it, return true to
    // make it possible to use them
    virtual bool GtkSupportsAttrs() const { return true; }

    virtual bool GtkSetAttr(const wxDataViewItemAttr& attr)
    {
        SetAttr(attr);
        return !attr.IsDefault();
    }

    virtual GtkCellRendererText *GtkGetTextRenderer() const;

protected:
    bool Init(wxDataViewCellMode mode, int align);

private:
    wxDC        *m_dc;

    GtkCellRendererText      *m_text_renderer;

    // parameters of the original render() call stored so that we could pass
    // them forward to m_text_renderer if our RenderText() is called
    struct GTKRenderParams
    {
        GdkWindow            *window;
        GtkWidget            *widget;
        GdkRectangle         *background_area;
        GdkRectangle         *expose_area;
        int                   flags;
    } m_renderParams;

    DECLARE_DYNAMIC_CLASS_NO_COPY(wxDataViewCustomRenderer)
};

// ---------------------------------------------------------
// wxDataViewProgressRenderer
// ---------------------------------------------------------

class WXDLLIMPEXP_ADV wxDataViewProgressRenderer: public wxDataViewCustomRenderer
{
public:
    wxDataViewProgressRenderer( const wxString &label = wxEmptyString,
                                const wxString &varianttype = "long",
                                wxDataViewCellMode mode = wxDATAVIEW_CELL_INERT,
                                int align = wxDVR_DEFAULT_ALIGNMENT );
    virtual ~wxDataViewProgressRenderer();

    bool SetValue( const wxVariant &value );
    bool GetValue( wxVariant &value ) const;

    virtual bool Render( wxRect cell, wxDC *dc, int state );
    virtual wxSize GetSize() const;

private:
    void GTKSetLabel();

    wxString    m_label;
    int         m_value;

#if !wxUSE_UNICODE
    // Flag used to indicate that we need to set the label because we were
    // unable to do it in the ctor (see comments there).
    bool m_needsToSetLabel;
#endif // !wxUSE_UNICODE

protected:
    DECLARE_DYNAMIC_CLASS_NO_COPY(wxDataViewProgressRenderer)
};

// ---------------------------------------------------------
// wxDataViewIconTextRenderer
// ---------------------------------------------------------

class WXDLLIMPEXP_ADV wxDataViewIconTextRenderer: public wxDataViewTextRenderer
{
public:
    wxDataViewIconTextRenderer( const wxString &varianttype = "wxDataViewIconText",
                                wxDataViewCellMode mode = wxDATAVIEW_CELL_INERT,
                                int align = wxDVR_DEFAULT_ALIGNMENT );
    virtual ~wxDataViewIconTextRenderer();

    bool SetValue( const wxVariant &value );
    bool GetValue( wxVariant &value ) const;

    virtual void GtkPackIntoColumn(GtkTreeViewColumn *column);

protected:
    virtual void GtkOnCellChanged(const wxVariant& value,
                                  const wxDataViewItem& item,
                                  unsigned col);

private:
    wxDataViewIconText   m_value;

    // we use the base class m_renderer for the text and this one for the icon
    GtkCellRenderer *m_rendererIcon;

    DECLARE_DYNAMIC_CLASS_NO_COPY(wxDataViewIconTextRenderer)
};

// ---------------------------------------------------------
// wxDataViewDateRenderer
// ---------------------------------------------------------

class WXDLLIMPEXP_ADV wxDataViewDateRenderer: public wxDataViewCustomRenderer
{
public:
    wxDataViewDateRenderer( const wxString &varianttype = "datetime",
                            wxDataViewCellMode mode = wxDATAVIEW_CELL_ACTIVATABLE,
                            int align = wxDVR_DEFAULT_ALIGNMENT );

    bool SetValue( const wxVariant &value );
    bool GetValue( wxVariant &value ) const;

    virtual bool Render( wxRect cell, wxDC *dc, int state );
    virtual wxSize GetSize() const;
    virtual bool Activate( const wxRect& cell,
                           wxDataViewModel *model,
                           const wxDataViewItem &item,
                           unsigned int col );

private:
    wxDateTime    m_date;

protected:
    DECLARE_DYNAMIC_CLASS_NO_COPY(wxDataViewDateRenderer)
};

// -------------------------------------
// wxDataViewChoiceRenderer
// -------------------------------------

class WXDLLIMPEXP_ADV wxDataViewChoiceRenderer: public wxDataViewCustomRenderer
{
public:
    wxDataViewChoiceRenderer(const wxArrayString &choices,
                             wxDataViewCellMode mode = wxDATAVIEW_CELL_EDITABLE,
                             int alignment = wxDVR_DEFAULT_ALIGNMENT );
    virtual bool Render( wxRect rect, wxDC *dc, int state );
    virtual wxSize GetSize() const;
    virtual bool SetValue( const wxVariant &value );
    virtual bool GetValue( wxVariant &value ) const;

    void SetAlignment( int align );

    wxString GetChoice(size_t index) const { return m_choices[index]; }
    const wxArrayString& GetChoices() const { return m_choices; }

private:
    wxArrayString m_choices;
    wxString      m_data;
};

// ----------------------------------------------------------------------------
// wxDataViewChoiceByIndexRenderer
// ----------------------------------------------------------------------------

class WXDLLIMPEXP_ADV wxDataViewChoiceByIndexRenderer: public wxDataViewChoiceRenderer
{
public:
    wxDataViewChoiceByIndexRenderer( const wxArrayString &choices,
                              wxDataViewCellMode mode = wxDATAVIEW_CELL_EDITABLE,
                              int alignment = wxDVR_DEFAULT_ALIGNMENT );

    virtual bool SetValue( const wxVariant &value );
    virtual bool GetValue( wxVariant &value ) const;

private:
    virtual void GtkOnTextEdited(const gchar *itempath, const wxString& str);
};



#endif // _WX_GTK_DVRENDERERS_H_

