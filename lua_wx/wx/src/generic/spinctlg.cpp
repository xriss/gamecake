///////////////////////////////////////////////////////////////////////////////
// Name:        src/generic/spinctlg.cpp
// Purpose:     implements wxSpinCtrl as a composite control
// Author:      Vadim Zeitlin
// Modified by:
// Created:     29.01.01
// RCS-ID:      $Id$
// Copyright:   (c) 2001 Vadim Zeitlin <zeitlin@dptmaths.ens-cachan.fr>
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

#ifndef WX_PRECOMP
    #include "wx/textctrl.h"
#endif //WX_PRECOMP

#include "wx/spinctrl.h"
#include "wx/tooltip.h"

#if wxUSE_SPINCTRL

IMPLEMENT_DYNAMIC_CLASS(wxSpinDoubleEvent, wxNotifyEvent)

// There are port-specific versions for the wxSpinCtrl, so exclude the
// contents of this file in those cases
#if !defined(wxHAS_NATIVE_SPINCTRL) || !defined(wxHAS_NATIVE_SPINCTRLDOUBLE)

#include "wx/spinbutt.h"

#if wxUSE_SPINBTN

// ----------------------------------------------------------------------------
// constants
// ----------------------------------------------------------------------------

// The margin between the text control and the spin: the value here is the same
// as the margin between the spin button and its "buddy" text control in wxMSW
// so the generic control looks similarly to the native one there, we might
// need to use different value for the other platforms (and maybe even
// determine it dynamically?).
static const wxCoord MARGIN = 1;

#define SPINCTRLBUT_MAX 32000 // large to avoid wrap around trouble

// ----------------------------------------------------------------------------
// wxSpinCtrlTextGeneric: text control used by spin control
// ----------------------------------------------------------------------------

class wxSpinCtrlTextGeneric : public wxTextCtrl
{
public:
    wxSpinCtrlTextGeneric(wxSpinCtrlGenericBase *spin, const wxString& value, long style=0)
        : wxTextCtrl(spin->GetParent(), wxID_ANY, value, wxDefaultPosition, wxDefaultSize,
                     style & wxALIGN_MASK)
    {
        m_spin = spin;

        // remove the default minsize, the spinctrl will have one instead
        SetSizeHints(wxDefaultCoord, wxDefaultCoord);
    }

    virtual ~wxSpinCtrlTextGeneric()
    {
        // MSW sends extra kill focus event on destroy
        if (m_spin)
            m_spin->m_textCtrl = NULL;

        m_spin = NULL;
    }

    void OnChar( wxKeyEvent &event )
    {
        if (m_spin)
            m_spin->ProcessWindowEvent(event);
    }

    void OnKillFocus(wxFocusEvent& event)
    {
        if (m_spin)
            m_spin->ProcessWindowEvent(event);

        event.Skip();
    }

    wxSpinCtrlGenericBase *m_spin;

private:
    DECLARE_EVENT_TABLE()
};

BEGIN_EVENT_TABLE(wxSpinCtrlTextGeneric, wxTextCtrl)
    EVT_CHAR(wxSpinCtrlTextGeneric::OnChar)

    EVT_KILL_FOCUS(wxSpinCtrlTextGeneric::OnKillFocus)
END_EVENT_TABLE()

// ----------------------------------------------------------------------------
// wxSpinCtrlButtonGeneric: spin button used by spin control
// ----------------------------------------------------------------------------

class wxSpinCtrlButtonGeneric : public wxSpinButton
{
public:
    wxSpinCtrlButtonGeneric(wxSpinCtrlGenericBase *spin, int style)
        : wxSpinButton(spin->GetParent(), wxID_ANY, wxDefaultPosition,
                       wxDefaultSize, style | wxSP_VERTICAL)
    {
        m_spin = spin;

        SetRange(-SPINCTRLBUT_MAX, SPINCTRLBUT_MAX);

        // remove the default minsize, the spinctrl will have one instead
        SetSizeHints(wxDefaultCoord, wxDefaultCoord);
    }

    void OnSpinButton(wxSpinEvent& event)
    {
        if (m_spin)
            m_spin->OnSpinButton(event);
    }

    wxSpinCtrlGenericBase *m_spin;

private:
    DECLARE_EVENT_TABLE()
};

BEGIN_EVENT_TABLE(wxSpinCtrlButtonGeneric, wxSpinButton)
    EVT_SPIN_UP(  wxID_ANY, wxSpinCtrlButtonGeneric::OnSpinButton)
    EVT_SPIN_DOWN(wxID_ANY, wxSpinCtrlButtonGeneric::OnSpinButton)
END_EVENT_TABLE()

// ============================================================================
// wxSpinCtrlGenericBase
// ============================================================================

// ----------------------------------------------------------------------------
// wxSpinCtrlGenericBase creation
// ----------------------------------------------------------------------------

void wxSpinCtrlGenericBase::Init()
{
    m_value         = 0;
    m_min           = 0;
    m_max           = 100;
    m_increment     = 1;
    m_snap_to_ticks = false;
    m_format        = wxS("%g");

    m_spin_value    = 0;

    m_textCtrl = NULL;
    m_spinButton  = NULL;
}

bool wxSpinCtrlGenericBase::Create(wxWindow *parent,
                                   wxWindowID id,
                                   const wxString& value,
                                   const wxPoint& pos, const wxSize& size,
                                   long style,
                                   double min, double max, double initial,
                                   double increment,
                                   const wxString& name)
{
    // don't use borders for this control itself, it wouldn't look good with
    // the text control borders (but we might want to use style border bits to
    // select the text control style)
    if ( !wxControl::Create(parent, id, wxDefaultPosition, wxDefaultSize,
                            (style & ~wxBORDER_MASK) | wxBORDER_NONE,
                            wxDefaultValidator, name) )
    {
        return false;
    }

    m_value = initial;
    m_min   = min;
    m_max   = max;
    m_increment = increment;

    m_textCtrl   = new wxSpinCtrlTextGeneric(this, value, style);
    m_spinButton = new wxSpinCtrlButtonGeneric(this, style);
#if wxUSE_TOOLTIPS
    m_textCtrl->SetToolTip(GetToolTipText());
    m_spinButton->SetToolTip(GetToolTipText());
#endif // wxUSE_TOOLTIPS

    m_spin_value = m_spinButton->GetValue();

    // the string value overrides the numeric one (for backwards compatibility
    // reasons and also because it is simpler to satisfy the string value which
    // comes much sooner in the list of arguments and leave the initial
    // parameter unspecified)
    if ( !value.empty() )
    {
        double d;
        if ( value.ToDouble(&d) )
        {
            m_value = d;
            m_textCtrl->SetValue(wxString::Format(m_format, m_value));
        }
    }

    SetInitialSize(size);
    Move(pos);

    // have to disable this window to avoid interfering it with message
    // processing to the text and the button... but pretend it is enabled to
    // make IsEnabled() return true
    wxControl::Enable(false); // don't use non virtual Disable() here!
    m_isEnabled = true;

    // we don't even need to show this window itself - and not doing it avoids
    // that it overwrites the text control
    wxControl::Show(false);
    m_isShown = true;
    return true;
}

wxSpinCtrlGenericBase::~wxSpinCtrlGenericBase()
{
    // delete the controls now, don't leave them alive even though they would
    // still be eventually deleted by our parent - but it will be too late, the
    // user code expects them to be gone now

    if (m_textCtrl)
    {
        // null this since MSW sends KILL_FOCUS on deletion, see ~wxSpinCtrlTextGeneric
        wxDynamicCast(m_textCtrl, wxSpinCtrlTextGeneric)->m_spin = NULL;

        wxSpinCtrlTextGeneric *text = (wxSpinCtrlTextGeneric*)m_textCtrl;
        m_textCtrl = NULL;
        delete text;
    }

    wxDELETE(m_spinButton);
}

// ----------------------------------------------------------------------------
// geometry
// ----------------------------------------------------------------------------

wxSize wxSpinCtrlGenericBase::DoGetBestSize() const
{
    wxSize sizeBtn  = m_spinButton->GetBestSize(),
           sizeText = m_textCtrl->GetBestSize();

    return wxSize(sizeBtn.x + sizeText.x + MARGIN, sizeText.y);
}

void wxSpinCtrlGenericBase::DoMoveWindow(int x, int y, int width, int height)
{
    wxControl::DoMoveWindow(x, y, width, height);

    // position the subcontrols inside the client area
    wxSize sizeBtn = m_spinButton->GetSize();

    wxCoord wText = width - sizeBtn.x - MARGIN;
    m_textCtrl->SetSize(x, y, wText, height);
    m_spinButton->SetSize(x + wText + MARGIN, y, wxDefaultCoord, height);
}

// ----------------------------------------------------------------------------
// operations forwarded to the subcontrols
// ----------------------------------------------------------------------------

void wxSpinCtrlGenericBase::SetFocus()
{
    if ( m_textCtrl )
        m_textCtrl->SetFocus();
}

bool wxSpinCtrlGenericBase::Enable(bool enable)
{
    // Notice that we never enable this control itself, it must stay disabled
    // to avoid interfering with the siblings event handling (see e.g. #12045
    // for the kind of problems which arise otherwise).
    if ( enable == m_isEnabled )
        return false;

    m_isEnabled = enable;

    m_spinButton->Enable(enable);
    m_textCtrl->Enable(enable);

    return true;
}

bool wxSpinCtrlGenericBase::Show(bool show)
{
    if ( !wxControl::Show(show) )
        return false;

    // under GTK Show() is called the first time before we are fully
    // constructed
    if ( m_spinButton )
    {
        m_spinButton->Show(show);
        m_textCtrl->Show(show);
    }

    return true;
}

bool wxSpinCtrlGenericBase::Reparent(wxWindowBase *newParent)
{
    if ( m_spinButton )
    {
        m_spinButton->Reparent(newParent);
        m_textCtrl->Reparent(newParent);
    }

    return true;
}

#if wxUSE_TOOLTIPS
void wxSpinCtrlGenericBase::DoSetToolTip(wxToolTip *tip)
{
    // Notice that we must check for the subcontrols not being NULL (as they
    // could be if we were created with the default ctor and this is called
    // before Create() for some reason) and that we can't call SetToolTip(tip)
    // because this would take ownership of the wxToolTip object (twice).
    if ( m_textCtrl )
    {
        if ( tip )
            m_textCtrl->SetToolTip(tip->GetTip());
        else
            m_textCtrl->SetToolTip(NULL);
    }

    if ( m_spinButton )
    {
        if( tip )
            m_spinButton->SetToolTip(tip->GetTip());
        else
            m_spinButton->SetToolTip(NULL);
    }

    wxWindowBase::DoSetToolTip(tip);
}
#endif // wxUSE_TOOLTIPS

// ----------------------------------------------------------------------------
// Handle sub controls events
// ----------------------------------------------------------------------------

BEGIN_EVENT_TABLE(wxSpinCtrlGenericBase, wxSpinCtrlBase)
    EVT_CHAR(wxSpinCtrlGenericBase::OnTextChar)
    EVT_KILL_FOCUS(wxSpinCtrlGenericBase::OnTextLostFocus)
END_EVENT_TABLE()

void wxSpinCtrlGenericBase::OnSpinButton(wxSpinEvent& event)
{
    event.Skip();

    // Sync the textctrl since the user expects that the button will modify
    // what they see in the textctrl.
    SyncSpinToText();

    int spin_value = event.GetPosition();
    double step = (event.GetEventType() == wxEVT_SCROLL_LINEUP) ? 1 : -1;

    // Use the spinbutton's acceleration, if any, but not if wrapping around
    if (((spin_value >= 0) && (m_spin_value >= 0)) || ((spin_value <= 0) && (m_spin_value <= 0)))
        step *= abs(spin_value - m_spin_value);

    double value = AdjustToFitInRange(m_value + step*m_increment);

    // Ignore the edges when it wraps since the up/down event may be opposite
    // They are in GTK and Mac
    if (abs(spin_value - m_spin_value) > SPINCTRLBUT_MAX)
    {
        m_spin_value = spin_value;
        return;
    }

    m_spin_value = spin_value;

    if ( DoSetValue(value) )
        DoSendEvent();
}

void wxSpinCtrlGenericBase::OnTextLostFocus(wxFocusEvent& event)
{
    SyncSpinToText();
    DoSendEvent();

    event.Skip();
}

void wxSpinCtrlGenericBase::OnTextChar(wxKeyEvent& event)
{
    if ( !HasFlag(wxSP_ARROW_KEYS) )
    {
        event.Skip();
        return;
    }

    double value = m_value;
    switch ( event.GetKeyCode() )
    {
        case WXK_UP :
            value += m_increment;
            break;

        case WXK_DOWN :
            value -= m_increment;
            break;

        case WXK_PAGEUP :
            value += m_increment * 10.0;
            break;

        case WXK_PAGEDOWN :
            value -= m_increment * 10.0;
            break;

        default:
            event.Skip();
            return;
    }

    value = AdjustToFitInRange(value);

    SyncSpinToText();

    if ( DoSetValue(value) )
        DoSendEvent();
}

// ----------------------------------------------------------------------------
// Textctrl functions
// ----------------------------------------------------------------------------

bool wxSpinCtrlGenericBase::SyncSpinToText()
{
    if ( !m_textCtrl || !m_textCtrl->IsModified() )
        return false;

    double textValue;
    if ( m_textCtrl->GetValue().ToDouble(&textValue) )
    {
        if (textValue > m_max)
            textValue = m_max;
        else if (textValue < m_min)
            textValue = m_min;
    }
    else // text contents is not a valid number at all
    {
        // replace its contents with the last valid value
        textValue = m_value;
    }

    // we must always set the value here, even if it's equal to m_value, as
    // otherwise we could be left with an out of range value when leaving the
    // text control and the current value is already m_max for example
    return DoSetValue(textValue);
}

// ----------------------------------------------------------------------------
// changing value and range
// ----------------------------------------------------------------------------

void wxSpinCtrlGenericBase::SetValue(const wxString& text)
{
    wxCHECK_RET( m_textCtrl, wxT("invalid call to wxSpinCtrl::SetValue") );

    double val;
    if ( text.ToDouble(&val) && InRange(val) )
    {
        DoSetValue(val);
    }
    else // not a number at all or out of range
    {
        m_textCtrl->SetValue(text);
        m_textCtrl->SetSelection(0, -1);
        m_textCtrl->SetInsertionPointEnd();
    }
}

bool wxSpinCtrlGenericBase::DoSetValue(double val)
{
    wxCHECK_MSG( m_textCtrl, false, wxT("invalid call to wxSpinCtrl::SetValue") );

    if (!InRange(val))
        return false;

    if ( m_snap_to_ticks && (m_increment != 0) )
    {
        double snap_value = val / m_increment;

        if (wxFinite(snap_value)) // FIXME what to do about a failure?
        {
            if ((snap_value - floor(snap_value)) < (ceil(snap_value) - snap_value))
                val = floor(snap_value) * m_increment;
            else
                val = ceil(snap_value) * m_increment;
        }
    }

    wxString str(wxString::Format(m_format.c_str(), val));

    if ((val != m_value) || (str != m_textCtrl->GetValue()))
    {
        m_value = val;
        str.ToDouble( &m_value );    // wysiwyg for textctrl
        m_textCtrl->SetValue( str );
        m_textCtrl->SetInsertionPointEnd();
        m_textCtrl->DiscardEdits();
        return true;
    }

    return false;
}

double wxSpinCtrlGenericBase::AdjustToFitInRange(double value) const
{
    if (value < m_min)
        value = HasFlag(wxSP_WRAP) ? m_max : m_min;
    if (value > m_max)
        value = HasFlag(wxSP_WRAP) ? m_min : m_max;

    return value;
}

void wxSpinCtrlGenericBase::DoSetRange(double min, double max)
{
    m_min = min;
    m_max = max;
}

void wxSpinCtrlGenericBase::DoSetIncrement(double inc)
{
    m_increment = inc;
}

void wxSpinCtrlGenericBase::SetSnapToTicks(bool snap_to_ticks)
{
    m_snap_to_ticks = snap_to_ticks;
    DoSetValue(m_value);
}

void wxSpinCtrlGenericBase::SetSelection(long from, long to)
{
    wxCHECK_RET( m_textCtrl, wxT("invalid call to wxSpinCtrl::SetSelection") );

    m_textCtrl->SetSelection(from, to);
}

#ifndef wxHAS_NATIVE_SPINCTRL

//-----------------------------------------------------------------------------
// wxSpinCtrl
//-----------------------------------------------------------------------------

void wxSpinCtrl::DoSendEvent()
{
    wxSpinEvent event( wxEVT_COMMAND_SPINCTRL_UPDATED, GetId());
    event.SetEventObject( this );
    event.SetPosition((int)(m_value + 0.5)); // FIXME should be SetValue
    event.SetString(m_textCtrl->GetValue());
    GetEventHandler()->ProcessEvent( event );
}

#endif // !wxHAS_NATIVE_SPINCTRL

//-----------------------------------------------------------------------------
// wxSpinCtrlDouble
//-----------------------------------------------------------------------------

IMPLEMENT_DYNAMIC_CLASS(wxSpinCtrlDouble, wxSpinCtrlGenericBase)

void wxSpinCtrlDouble::DoSendEvent()
{
    wxSpinDoubleEvent event( wxEVT_COMMAND_SPINCTRLDOUBLE_UPDATED, GetId());
    event.SetEventObject( this );
    event.SetValue(m_value);
    event.SetString(m_textCtrl->GetValue());
    GetEventHandler()->ProcessEvent( event );
}

void wxSpinCtrlDouble::SetDigits(unsigned digits)
{
    wxCHECK_RET( digits <= 20, "too many digits for wxSpinCtrlDouble" );

    if ( digits == m_digits )
        return;

    m_digits = digits;

    m_format.Printf(wxT("%%0.%ulf"), digits);

    DoSetValue(m_value);
}

#endif // wxUSE_SPINBTN

#endif // !wxPort-with-native-spinctrl

#endif // wxUSE_SPINCTRL
