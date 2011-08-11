/////////////////////////////////////////////////////////////////////////////
// Name:        src/gtk/dcclient.cpp
// Purpose:     wxWindowDCImpl implementation
// Author:      Robert Roebling
// RCS-ID:      $Id$
// Copyright:   (c) 1998 Robert Roebling, Chris Breeze
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

// For compilers that support precompilation, includes "wx.h".
#include "wx/wxprec.h"

#include "wx/gtk/dcclient.h"

#ifndef WX_PRECOMP
    #include "wx/window.h"
    #include "wx/log.h"
    #include "wx/dcmemory.h"
    #include "wx/math.h"
    #include "wx/image.h"
    #include "wx/module.h"
#endif

#include "wx/fontutil.h"

#include "wx/gtk/private.h"
#include "wx/gtk/private/object.h"

//-----------------------------------------------------------------------------
// local defines
//-----------------------------------------------------------------------------

#define XLOG2DEV(x)    LogicalToDeviceX(x)
#define XLOG2DEVREL(x) LogicalToDeviceXRel(x)
#define YLOG2DEV(y)    LogicalToDeviceY(y)
#define YLOG2DEVREL(y) LogicalToDeviceYRel(y)

#define USE_PAINT_REGION 1

//-----------------------------------------------------------------------------
// local data
//-----------------------------------------------------------------------------

#include "bdiag.xbm"
#include "fdiag.xbm"
#include "cdiag.xbm"
#include "horiz.xbm"
#include "verti.xbm"
#include "cross.xbm"

static GdkPixmap* hatches[wxBRUSHSTYLE_LAST_HATCH - wxBRUSHSTYLE_FIRST_HATCH + 1];

//-----------------------------------------------------------------------------
// constants
//-----------------------------------------------------------------------------

static const double RAD2DEG  = 180.0 / M_PI;

// ----------------------------------------------------------------------------
// private functions
// ----------------------------------------------------------------------------

static inline double dmax(double a, double b) { return a > b ? a : b; }
static inline double dmin(double a, double b) { return a < b ? a : b; }

static inline double DegToRad(double deg) { return (deg * M_PI) / 180.0; }

static GdkPixmap* GetHatch(int style)
{
    wxASSERT(style >= wxBRUSHSTYLE_FIRST_HATCH && style <= wxBRUSHSTYLE_LAST_HATCH);
    const int i = style - wxBRUSHSTYLE_FIRST_HATCH;
    if (hatches[i] == NULL)
    {
        // This macro creates a bitmap from an XBM file included above. Notice
        // the need for the cast because gdk_bitmap_create_from_data() doesn't
        // accept unsigned data but the arrays in XBM need to be unsigned to
        // avoid warnings (and even errors in C+0x mode) from g++.
#define CREATE_FROM_XBM_DATA(name) \
        gdk_bitmap_create_from_data \
        ( \
            NULL, \
            reinterpret_cast<gchar *>(name ## _bits), \
            name ## _width, \
            name ## _height \
        )

        switch (style)
        {
        case wxBRUSHSTYLE_BDIAGONAL_HATCH:
            hatches[i] = CREATE_FROM_XBM_DATA(bdiag);
            break;
        case wxBRUSHSTYLE_CROSSDIAG_HATCH:
            hatches[i] = CREATE_FROM_XBM_DATA(cdiag);
            break;
        case wxBRUSHSTYLE_CROSS_HATCH:
            hatches[i] = CREATE_FROM_XBM_DATA(cross);
            break;
        case wxBRUSHSTYLE_FDIAGONAL_HATCH:
            hatches[i] = CREATE_FROM_XBM_DATA(fdiag);
            break;
        case wxBRUSHSTYLE_HORIZONTAL_HATCH:
            hatches[i] = CREATE_FROM_XBM_DATA(horiz);
            break;
        case wxBRUSHSTYLE_VERTICAL_HATCH:
            hatches[i] = CREATE_FROM_XBM_DATA(verti);
            break;
        }

#undef CREATE_FROM_XBM_DATA
    }
    return hatches[i];
}

//-----------------------------------------------------------------------------
// Implement Pool of Graphic contexts. Creating them takes too much time.
//-----------------------------------------------------------------------------

enum wxPoolGCType
{
   wxGC_ERROR = 0,
   wxTEXT_MONO,
   wxBG_MONO,
   wxPEN_MONO,
   wxBRUSH_MONO,
   wxTEXT_COLOUR,
   wxBG_COLOUR,
   wxPEN_COLOUR,
   wxBRUSH_COLOUR,
   wxTEXT_SCREEN,
   wxBG_SCREEN,
   wxPEN_SCREEN,
   wxBRUSH_SCREEN
};

struct wxGC
{
    GdkGC        *m_gc;
    wxPoolGCType  m_type;
    bool          m_used;
};

#define GC_POOL_ALLOC_SIZE 100

static int wxGCPoolSize = 0;

static wxGC *wxGCPool = NULL;

static void wxInitGCPool()
{
    // This really could wait until the first call to
    // wxGetPoolGC, but we will make the first allocation
    // now when other initialization is being performed.

    // Set initial pool size.
    wxGCPoolSize = GC_POOL_ALLOC_SIZE;

    // Allocate initial pool.
    wxGCPool = (wxGC *)malloc(wxGCPoolSize * sizeof(wxGC));
    if (wxGCPool == NULL)
    {
        // If we cannot malloc, then fail with error
        // when debug is enabled.  If debug is not enabled,
        // the problem will eventually get caught
        // in wxGetPoolGC.
        wxFAIL_MSG( wxT("Cannot allocate GC pool") );
        return;
    }

    // Zero initial pool.
    memset(wxGCPool, 0, wxGCPoolSize * sizeof(wxGC));
}

static void wxCleanUpGCPool()
{
    for (int i = 0; i < wxGCPoolSize; i++)
    {
        if (wxGCPool[i].m_gc)
            g_object_unref (wxGCPool[i].m_gc);
    }

    free(wxGCPool);
    wxGCPool = NULL;
    wxGCPoolSize = 0;
}

static GdkGC* wxGetPoolGC( GdkWindow *window, wxPoolGCType type )
{
    wxGC *pptr;

    // Look for an available GC.
    for (int i = 0; i < wxGCPoolSize; i++)
    {
        if (!wxGCPool[i].m_gc)
        {
            wxGCPool[i].m_gc = gdk_gc_new( window );
            gdk_gc_set_exposures( wxGCPool[i].m_gc, FALSE );
            wxGCPool[i].m_type = type;
            wxGCPool[i].m_used = false;
        }
        if ((!wxGCPool[i].m_used) && (wxGCPool[i].m_type == type))
        {
            wxGCPool[i].m_used = true;
            return wxGCPool[i].m_gc;
        }
    }

    // We did not find an available GC.
    // We need to grow the GC pool.
    pptr = (wxGC *)realloc(wxGCPool,
        (wxGCPoolSize + GC_POOL_ALLOC_SIZE)*sizeof(wxGC));
    if (pptr != NULL)
    {
        // Initialize newly allocated pool.
        wxGCPool = pptr;
        memset(&wxGCPool[wxGCPoolSize], 0,
            GC_POOL_ALLOC_SIZE*sizeof(wxGC));

        // Initialize entry we will return.
        wxGCPool[wxGCPoolSize].m_gc = gdk_gc_new( window );
        gdk_gc_set_exposures( wxGCPool[wxGCPoolSize].m_gc, FALSE );
        wxGCPool[wxGCPoolSize].m_type = type;
        wxGCPool[wxGCPoolSize].m_used = true;

        // Set new value of pool size.
        wxGCPoolSize += GC_POOL_ALLOC_SIZE;

        // Return newly allocated entry.
        return wxGCPool[wxGCPoolSize-GC_POOL_ALLOC_SIZE].m_gc;
    }

    // The realloc failed.  Fall through to error.
    wxFAIL_MSG( wxT("No GC available") );

    return NULL;
}

static void wxFreePoolGC( GdkGC *gc )
{
    for (int i = 0; i < wxGCPoolSize; i++)
    {
        if (wxGCPool[i].m_gc == gc)
        {
            wxGCPool[i].m_used = false;
            return;
        }
    }

    wxFAIL_MSG( wxT("Wrong GC") );
}

//-----------------------------------------------------------------------------
// wxWindowDC
//-----------------------------------------------------------------------------

IMPLEMENT_ABSTRACT_CLASS(wxWindowDCImpl, wxGTKDCImpl)

wxWindowDCImpl::wxWindowDCImpl( wxDC *owner ) :
   wxGTKDCImpl( owner )
{
    m_gdkwindow = NULL;
    m_penGC = NULL;
    m_brushGC = NULL;
    m_textGC = NULL;
    m_bgGC = NULL;
    m_cmap = NULL;
    m_isScreenDC = false;
    m_context = NULL;
    m_layout = NULL;
    m_fontdesc = NULL;
}

wxWindowDCImpl::wxWindowDCImpl( wxDC *owner, wxWindow *window ) :
   wxGTKDCImpl( owner )
{
    wxASSERT_MSG( window, wxT("DC needs a window") );

    m_gdkwindow = NULL;
    m_penGC = NULL;
    m_brushGC = NULL;
    m_textGC = NULL;
    m_bgGC = NULL;
    m_cmap = NULL;
    m_isScreenDC = false;
    m_font = window->GetFont();

    GtkWidget *widget = window->m_wxwindow;
    m_gdkwindow = window->GTKGetDrawingWindow();

    // Some controls don't have m_wxwindow - like wxStaticBox, but the user
    // code should still be able to create wxClientDCs for them
    if ( !widget )
    {
        widget = window->m_widget;

        wxCHECK_RET(widget, "DC needs a widget");

        m_gdkwindow = widget->window;
        if (!gtk_widget_get_has_window(widget))
            SetDeviceLocalOrigin(widget->allocation.x, widget->allocation.y);
    }

    m_context = window->GTKGetPangoDefaultContext();
    m_layout = pango_layout_new( m_context );
    m_fontdesc = pango_font_description_copy( widget->style->font_desc );

    // Window not realized ?
    if (!m_gdkwindow)
    {
         // Don't report problems as per MSW.
         m_ok = true;

         return;
    }

    m_cmap = gtk_widget_get_colormap(widget);

    SetUpDC();

    /* this must be done after SetUpDC, bacause SetUpDC calls the
       repective SetBrush, SetPen, SetBackground etc functions
       to set up the DC. SetBackground call m_owner->SetBackground
       and this might not be desired as the standard dc background
       is white whereas a window might assume gray to be the
       standard (as e.g. wxStatusBar) */

    m_window = window;

    if (m_window && m_window->m_wxwindow &&
        (m_window->GetLayoutDirection() == wxLayout_RightToLeft))
    {
        // reverse sense
        m_signX = -1;

        // origin in the upper right corner
        m_deviceOriginX = m_window->GetClientSize().x;
    }
}

wxWindowDCImpl::~wxWindowDCImpl()
{
    Destroy();

    if (m_layout)
        g_object_unref (m_layout);
    if (m_fontdesc)
        pango_font_description_free( m_fontdesc );
}

void wxWindowDCImpl::SetUpDC( bool isMemDC )
{
    m_ok = true;

    wxASSERT_MSG( !m_penGC, wxT("GCs already created") );

    bool done = false;

    if ((isMemDC) && (GetSelectedBitmap().IsOk()))
    {
        if (GetSelectedBitmap().GetDepth() == 1)
        {
            m_penGC = wxGetPoolGC( m_gdkwindow, wxPEN_MONO );
            m_brushGC = wxGetPoolGC( m_gdkwindow, wxBRUSH_MONO );
            m_textGC = wxGetPoolGC( m_gdkwindow, wxTEXT_MONO );
            m_bgGC = wxGetPoolGC( m_gdkwindow, wxBG_MONO );
            done = true;
        }
    }

    if (!done)
    {
        if (m_isScreenDC)
        {
            m_penGC = wxGetPoolGC( m_gdkwindow, wxPEN_SCREEN );
            m_brushGC = wxGetPoolGC( m_gdkwindow, wxBRUSH_SCREEN );
            m_textGC = wxGetPoolGC( m_gdkwindow, wxTEXT_SCREEN );
            m_bgGC = wxGetPoolGC( m_gdkwindow, wxBG_SCREEN );
        }
        else
        {
            m_penGC = wxGetPoolGC( m_gdkwindow, wxPEN_COLOUR );
            m_brushGC = wxGetPoolGC( m_gdkwindow, wxBRUSH_COLOUR );
            m_textGC = wxGetPoolGC( m_gdkwindow, wxTEXT_COLOUR );
            m_bgGC = wxGetPoolGC( m_gdkwindow, wxBG_COLOUR );
        }
    }

    /* background colour */
    m_backgroundBrush = *wxWHITE_BRUSH;
    m_backgroundBrush.GetColour().CalcPixel( m_cmap );
    const GdkColor *bg_col = m_backgroundBrush.GetColour().GetColor();

    /* m_textGC */
    m_textForegroundColour.CalcPixel( m_cmap );
    gdk_gc_set_foreground( m_textGC, m_textForegroundColour.GetColor() );

    m_textBackgroundColour.CalcPixel( m_cmap );
    gdk_gc_set_background( m_textGC, m_textBackgroundColour.GetColor() );

    gdk_gc_set_fill( m_textGC, GDK_SOLID );

    gdk_gc_set_colormap( m_textGC, m_cmap );

    /* m_penGC */
    m_pen.GetColour().CalcPixel( m_cmap );
    gdk_gc_set_foreground( m_penGC, m_pen.GetColour().GetColor() );
    gdk_gc_set_background( m_penGC, bg_col );

    gdk_gc_set_line_attributes( m_penGC, 0, GDK_LINE_SOLID, GDK_CAP_NOT_LAST, GDK_JOIN_ROUND );

    /* m_brushGC */
    m_brush.GetColour().CalcPixel( m_cmap );
    gdk_gc_set_foreground( m_brushGC, m_brush.GetColour().GetColor() );
    gdk_gc_set_background( m_brushGC, bg_col );

    gdk_gc_set_fill( m_brushGC, GDK_SOLID );

    /* m_bgGC */
    gdk_gc_set_background( m_bgGC, bg_col );
    gdk_gc_set_foreground( m_bgGC, bg_col );

    gdk_gc_set_fill( m_bgGC, GDK_SOLID );

    /* ROPs */
    gdk_gc_set_function( m_textGC, GDK_COPY );
    gdk_gc_set_function( m_brushGC, GDK_COPY );
    gdk_gc_set_function( m_penGC, GDK_COPY );

    /* clipping */
    gdk_gc_set_clip_rectangle( m_penGC, NULL );
    gdk_gc_set_clip_rectangle( m_brushGC, NULL );
    gdk_gc_set_clip_rectangle( m_textGC, NULL );
    gdk_gc_set_clip_rectangle( m_bgGC, NULL );
}

void wxWindowDCImpl::DoGetSize( int* width, int* height ) const
{
    wxCHECK_RET( m_window, wxT("GetSize() doesn't work without window") );

    m_window->GetSize(width, height);
}

bool wxWindowDCImpl::DoFloodFill(wxCoord x, wxCoord y,
                                 const wxColour& col, wxFloodFillStyle style)
{
#if wxUSE_IMAGE
    extern bool wxDoFloodFill(wxDC *dc, wxCoord x, wxCoord y,
                              const wxColour & col, wxFloodFillStyle style);

    return wxDoFloodFill( GetOwner(), x, y, col, style);
#else
    wxUnusedVar(x);
    wxUnusedVar(y);
    wxUnusedVar(col);
    wxUnusedVar(style);

    return false;
#endif
}

bool wxWindowDCImpl::DoGetPixel( wxCoord x1, wxCoord y1, wxColour *col ) const
{
    GdkImage* image = NULL;
    if (m_gdkwindow)
    {
        const int x = LogicalToDeviceX(x1);
        const int y = LogicalToDeviceY(y1);
        wxRect rect;
        gdk_drawable_get_size(m_gdkwindow, &rect.width, &rect.height);
        if (rect.Contains(x, y))
            image = gdk_drawable_get_image(m_gdkwindow, x, y, 1, 1);
    }
    if (image == NULL)
    {
        *col = wxColour();
        return false;
    }
    GdkColormap* colormap = gdk_image_get_colormap(image);
    const unsigned pixel = gdk_image_get_pixel(image, 0, 0);
    if (colormap == NULL)
        *col = pixel ? m_textForegroundColour : m_textBackgroundColour;
    else
    {
        GdkColor c;
        gdk_colormap_query_color(colormap, pixel, &c);
        col->Set(c.red >> 8, c.green >> 8, c.blue >> 8);
    }
    g_object_unref(image);
    return true;
}

void wxWindowDCImpl::DoDrawLine( wxCoord x1, wxCoord y1, wxCoord x2, wxCoord y2 )
{
    wxCHECK_RET( IsOk(), wxT("invalid window dc") );

    if ( m_pen.IsNonTransparent() )
    {
        if (m_gdkwindow)
            gdk_draw_line( m_gdkwindow, m_penGC, XLOG2DEV(x1), YLOG2DEV(y1), XLOG2DEV(x2), YLOG2DEV(y2) );

        CalcBoundingBox(x1, y1);
        CalcBoundingBox(x2, y2);
    }
}

void wxWindowDCImpl::DoCrossHair( wxCoord x, wxCoord y )
{
    wxCHECK_RET( IsOk(), wxT("invalid window dc") );

    if ( m_pen.IsNonTransparent() )
    {
        int w = 0;
        int h = 0;
        GetOwner()->GetSize( &w, &h );
        wxCoord xx = XLOG2DEV(x);
        wxCoord yy = YLOG2DEV(y);
        if (m_gdkwindow)
        {
            gdk_draw_line( m_gdkwindow, m_penGC, 0, yy, XLOG2DEVREL(w), yy );
            gdk_draw_line( m_gdkwindow, m_penGC, xx, 0, xx, YLOG2DEVREL(h) );
        }
    }
}

void wxWindowDCImpl::DrawingSetup(GdkGC*& gc, bool& originChanged)
{
    gc = m_brushGC;
    GdkPixmap* pixmap = NULL;
    const int style = m_brush.GetStyle();

    if (style == wxBRUSHSTYLE_STIPPLE || style == wxBRUSHSTYLE_STIPPLE_MASK_OPAQUE)
    {
        const wxBitmap* stipple = m_brush.GetStipple();
        if (stipple->IsOk())
        {
            if (style == wxBRUSHSTYLE_STIPPLE)
                pixmap = stipple->GetPixmap();
            else if (stipple->GetMask())
            {
                pixmap = stipple->GetPixmap();
                gc = m_textGC;
            }
        }
    }
    else if (m_brush.IsHatch())
    {
        pixmap = GetHatch(style);
    }

    int origin_x = 0;
    int origin_y = 0;
    if (pixmap)
    {
        int w, h;
        gdk_drawable_get_size(pixmap, &w, &h);
        origin_x = m_deviceOriginX % w;
        origin_y = m_deviceOriginY % h;
    }

    originChanged = origin_x || origin_y;
    if (originChanged)
        gdk_gc_set_ts_origin(gc, origin_x, origin_y);
}

void wxWindowDCImpl::DoDrawArc( wxCoord x1, wxCoord y1, wxCoord x2, wxCoord y2,
                            wxCoord xc, wxCoord yc )
{
    wxCHECK_RET( IsOk(), wxT("invalid window dc") );

    wxCoord xx1 = XLOG2DEV(x1);
    wxCoord yy1 = YLOG2DEV(y1);
    wxCoord xx2 = XLOG2DEV(x2);
    wxCoord yy2 = YLOG2DEV(y2);
    wxCoord xxc = XLOG2DEV(xc);
    wxCoord yyc = YLOG2DEV(yc);
    double dx = xx1 - xxc;
    double dy = yy1 - yyc;
    double radius = sqrt((double)(dx*dx+dy*dy));
    wxCoord   r      = (wxCoord)radius;
    double radius1, radius2;

    if (xx1 == xx2 && yy1 == yy2)
    {
        radius1 = 0.0;
        radius2 = 360.0;
    }
    else if ( wxIsNullDouble(radius) )
    {
        radius1 =
        radius2 = 0.0;
    }
    else
    {
        radius1 = (xx1 - xxc == 0) ?
            (yy1 - yyc < 0) ? 90.0 : -90.0 :
            -atan2(double(yy1-yyc), double(xx1-xxc)) * RAD2DEG;
        radius2 = (xx2 - xxc == 0) ?
            (yy2 - yyc < 0) ? 90.0 : -90.0 :
            -atan2(double(yy2-yyc), double(xx2-xxc)) * RAD2DEG;
    }
    wxCoord alpha1 = wxCoord(radius1 * 64.0);
    wxCoord alpha2 = wxCoord((radius2 - radius1) * 64.0);
    while (alpha2 <= 0) alpha2 += 360*64;
    while (alpha1 > 360*64) alpha1 -= 360*64;

    if (m_gdkwindow)
    {
        if ( m_brush.IsNonTransparent() )
        {
            GdkGC* gc;
            bool originChanged;
            DrawingSetup(gc, originChanged);

            gdk_draw_arc(m_gdkwindow, gc, true, xxc-r, yyc-r, 2*r, 2*r, alpha1, alpha2);

            if (originChanged)
                gdk_gc_set_ts_origin(gc, 0, 0);
        }

        if ( m_pen.IsNonTransparent() )
        {
            gdk_draw_arc( m_gdkwindow, m_penGC, FALSE, xxc-r, yyc-r, 2*r,2*r, alpha1, alpha2 );

            if ( m_brush.IsNonTransparent() && (alpha2 - alpha1 != 360*64) )
            {
                gdk_draw_line( m_gdkwindow, m_penGC, xx1, yy1, xxc, yyc );
                gdk_draw_line( m_gdkwindow, m_penGC, xxc, yyc, xx2, yy2 );
            }
        }
    }

    CalcBoundingBox (x1, y1);
    CalcBoundingBox (x2, y2);
}

void wxWindowDCImpl::DoDrawEllipticArc( wxCoord x, wxCoord y, wxCoord width, wxCoord height, double sa, double ea )
{
    wxCHECK_RET( IsOk(), wxT("invalid window dc") );

    wxCoord xx = XLOG2DEV(x);
    wxCoord yy = YLOG2DEV(y);
    wxCoord ww = m_signX * XLOG2DEVREL(width);
    wxCoord hh = m_signY * YLOG2DEVREL(height);

    // CMB: handle -ve width and/or height
    if (ww < 0) { ww = -ww; xx = xx - ww; }
    if (hh < 0) { hh = -hh; yy = yy - hh; }

    if (m_gdkwindow)
    {
        wxCoord start = wxCoord(sa * 64.0);
        wxCoord end = wxCoord((ea-sa) * 64.0);

        if ( m_brush.IsNonTransparent() )
        {
            GdkGC* gc;
            bool originChanged;
            DrawingSetup(gc, originChanged);

            gdk_draw_arc(m_gdkwindow, gc, true, xx, yy, ww, hh, start, end);

            if (originChanged)
                gdk_gc_set_ts_origin(gc, 0, 0);
        }

        if ( m_pen.IsNonTransparent() )
            gdk_draw_arc( m_gdkwindow, m_penGC, FALSE, xx, yy, ww, hh, start, end );
    }

    CalcBoundingBox (x, y);
    CalcBoundingBox (x + width, y + height);
}

void wxWindowDCImpl::DoDrawPoint( wxCoord x, wxCoord y )
{
    wxCHECK_RET( IsOk(), wxT("invalid window dc") );

    if ( m_pen.IsNonTransparent() && m_gdkwindow )
        gdk_draw_point( m_gdkwindow, m_penGC, XLOG2DEV(x), YLOG2DEV(y) );

    CalcBoundingBox (x, y);
}

void wxWindowDCImpl::DoDrawLines( int n, wxPoint points[], wxCoord xoffset, wxCoord yoffset )
{
    wxCHECK_RET( IsOk(), wxT("invalid window dc") );

    if (n <= 0) return;

    if ( m_pen.IsTransparent() )
        return;

    //Check, if scaling is necessary
    const bool doScale =
        xoffset != 0 || yoffset != 0 || XLOG2DEV(10) != 10 || YLOG2DEV(10) != 10;

    // GdkPoint and wxPoint have the same memory layout, so we can cast one to the other
    GdkPoint* gpts = reinterpret_cast<GdkPoint*>(points);

    if (doScale)
        gpts = new GdkPoint[n];

    for (int i = 0; i < n; i++)
    {
        if (doScale)
        {
            gpts[i].x = XLOG2DEV(points[i].x + xoffset);
            gpts[i].y = YLOG2DEV(points[i].y + yoffset);
        }
        CalcBoundingBox(points[i].x + xoffset, points[i].y + yoffset);
    }

    if (m_gdkwindow)
        gdk_draw_lines( m_gdkwindow, m_penGC, gpts, n);

    if (doScale)
        delete[] gpts;
}

void wxWindowDCImpl::DoDrawPolygon( int n, wxPoint points[],
                                    wxCoord xoffset, wxCoord yoffset,
                                    wxPolygonFillMode WXUNUSED(fillStyle) )
{
    wxCHECK_RET( IsOk(), wxT("invalid window dc") );

    if (n <= 0) return;

    //Check, if scaling is necessary
    const bool doScale =
        xoffset != 0 || yoffset != 0 || XLOG2DEV(10) != 10 || YLOG2DEV(10) != 10;

    // GdkPoint and wxPoint have the same memory layout, so we can cast one to the other
    GdkPoint* gdkpoints = reinterpret_cast<GdkPoint*>(points);

    if (doScale)
        gdkpoints = new GdkPoint[n];

    int i;
    for (i = 0 ; i < n ; i++)
    {
        if (doScale)
        {
            gdkpoints[i].x = XLOG2DEV(points[i].x + xoffset);
            gdkpoints[i].y = YLOG2DEV(points[i].y + yoffset);
        }
        CalcBoundingBox(points[i].x + xoffset, points[i].y + yoffset);
    }

    if (m_gdkwindow)
    {
        if ( m_brush.IsNonTransparent() )
        {
            GdkGC* gc;
            bool originChanged;
            DrawingSetup(gc, originChanged);

            gdk_draw_polygon(m_gdkwindow, gc, true, gdkpoints, n);

            if (originChanged)
                gdk_gc_set_ts_origin(gc, 0, 0);
        }

        if ( m_pen.IsNonTransparent() )
        {
/*
            for (i = 0 ; i < n ; i++)
            {
                gdk_draw_line( m_gdkwindow, m_penGC,
                               gdkpoints[i%n].x,
                               gdkpoints[i%n].y,
                               gdkpoints[(i+1)%n].x,
                               gdkpoints[(i+1)%n].y);
            }
*/
            gdk_draw_polygon( m_gdkwindow, m_penGC, FALSE, gdkpoints, n );

        }
    }

    if (doScale)
        delete[] gdkpoints;
}

void wxWindowDCImpl::DoDrawRectangle( wxCoord x, wxCoord y, wxCoord width, wxCoord height )
{
    wxCHECK_RET( IsOk(), wxT("invalid window dc") );

    wxCoord xx = XLOG2DEV(x);
    wxCoord yy = YLOG2DEV(y);
    wxCoord ww = m_signX * XLOG2DEVREL(width);
    wxCoord hh = m_signY * YLOG2DEVREL(height);

    // CMB: draw nothing if transformed w or h is 0
    if (ww == 0 || hh == 0) return;

    // CMB: handle -ve width and/or height
    if (ww < 0) { ww = -ww; xx = xx - ww; }
    if (hh < 0) { hh = -hh; yy = yy - hh; }

    if (m_gdkwindow)
    {
        if ( m_brush.IsNonTransparent() )
        {
            GdkGC* gc;
            bool originChanged;
            DrawingSetup(gc, originChanged);

            gdk_draw_rectangle(m_gdkwindow, gc, true, xx, yy, ww, hh);

            if (originChanged)
                gdk_gc_set_ts_origin(gc, 0, 0);
        }

        if ( m_pen.IsNonTransparent() )
        {
            if ((m_pen.GetWidth() == 2) && (m_pen.GetCap() == wxCAP_ROUND) &&
                (m_pen.GetJoin() == wxJOIN_ROUND) && (m_pen.GetStyle() == wxPENSTYLE_SOLID))
            {
                // Use 2 1-line rects instead
                gdk_gc_set_line_attributes( m_penGC, 1, GDK_LINE_SOLID, GDK_CAP_ROUND, GDK_JOIN_ROUND );

                if (m_signX == -1)
                {
                    // Different for RTL
                    gdk_draw_rectangle( m_gdkwindow, m_penGC, FALSE, xx+1, yy, ww-2, hh-2 );
                    gdk_draw_rectangle( m_gdkwindow, m_penGC, FALSE, xx, yy-1, ww, hh );
                }
                else
                {
                    gdk_draw_rectangle( m_gdkwindow, m_penGC, FALSE, xx, yy, ww-2, hh-2 );
                    gdk_draw_rectangle( m_gdkwindow, m_penGC, FALSE, xx-1, yy-1, ww, hh );
                }

                // reset
                gdk_gc_set_line_attributes( m_penGC, 2, GDK_LINE_SOLID, GDK_CAP_ROUND, GDK_JOIN_ROUND );
            }
            else
            {
                // Just use X11 for other cases
                gdk_draw_rectangle( m_gdkwindow, m_penGC, FALSE, xx, yy, ww-1, hh-1 );
            }
        }
    }

    CalcBoundingBox( x, y );
    CalcBoundingBox( x + width, y + height );
}

void wxWindowDCImpl::DoDrawRoundedRectangle( wxCoord x, wxCoord y, wxCoord width, wxCoord height, double radius )
{
    wxCHECK_RET( IsOk(), wxT("invalid window dc") );

    if (radius < 0.0) radius = - radius * ((width < height) ? width : height);

    wxCoord xx = XLOG2DEV(x);
    wxCoord yy = YLOG2DEV(y);
    wxCoord ww = m_signX * XLOG2DEVREL(width);
    wxCoord hh = m_signY * YLOG2DEVREL(height);
    wxCoord rr = XLOG2DEVREL((wxCoord)radius);

    // CMB: handle -ve width and/or height
    if (ww < 0) { ww = -ww; xx = xx - ww; }
    if (hh < 0) { hh = -hh; yy = yy - hh; }

    // CMB: if radius is zero use DrawRectangle() instead to avoid
    // X drawing errors with small radii
    if (rr == 0)
    {
        DoDrawRectangle( x, y, width, height );
        return;
    }

    // CMB: draw nothing if transformed w or h is 0
    if (ww == 0 || hh == 0) return;

    // CMB: adjust size if outline is drawn otherwise the result is
    // 1 pixel too wide and high
    if ( m_pen.IsNonTransparent() )
    {
        ww--;
        hh--;
    }

    if (m_gdkwindow)
    {
        // CMB: ensure dd is not larger than rectangle otherwise we
        // get an hour glass shape
        wxCoord dd = 2 * rr;
        if (dd > ww) dd = ww;
        if (dd > hh) dd = hh;
        rr = dd / 2;

        if ( m_brush.IsNonTransparent() )
        {
            GdkGC* gc;
            bool originChanged;
            DrawingSetup(gc, originChanged);

            gdk_draw_rectangle(m_gdkwindow, gc, true, xx+rr, yy, ww-dd+1, hh);
            gdk_draw_rectangle(m_gdkwindow, gc, true, xx, yy+rr, ww, hh-dd+1);
            gdk_draw_arc(m_gdkwindow, gc, true, xx, yy, dd, dd, 90*64, 90*64);
            gdk_draw_arc(m_gdkwindow, gc, true, xx+ww-dd, yy, dd, dd, 0, 90*64);
            gdk_draw_arc(m_gdkwindow, gc, true, xx+ww-dd, yy+hh-dd, dd, dd, 270*64, 90*64);
            gdk_draw_arc(m_gdkwindow, gc, true, xx, yy+hh-dd, dd, dd, 180*64, 90*64);

            if (originChanged)
                gdk_gc_set_ts_origin(gc, 0, 0);
        }

        if ( m_pen.IsNonTransparent() )
        {
            gdk_draw_line( m_gdkwindow, m_penGC, xx+rr+1, yy, xx+ww-rr, yy );
            gdk_draw_line( m_gdkwindow, m_penGC, xx+rr+1, yy+hh, xx+ww-rr, yy+hh );
            gdk_draw_line( m_gdkwindow, m_penGC, xx, yy+rr+1, xx, yy+hh-rr );
            gdk_draw_line( m_gdkwindow, m_penGC, xx+ww, yy+rr+1, xx+ww, yy+hh-rr );
            gdk_draw_arc( m_gdkwindow, m_penGC, FALSE, xx, yy, dd, dd, 90*64, 90*64 );
            gdk_draw_arc( m_gdkwindow, m_penGC, FALSE, xx+ww-dd, yy, dd, dd, 0, 90*64 );
            gdk_draw_arc( m_gdkwindow, m_penGC, FALSE, xx+ww-dd, yy+hh-dd, dd, dd, 270*64, 90*64 );
            gdk_draw_arc( m_gdkwindow, m_penGC, FALSE, xx, yy+hh-dd, dd, dd, 180*64, 90*64 );
        }
    }

    // this ignores the radius
    CalcBoundingBox( x, y );
    CalcBoundingBox( x + width, y + height );
}

void wxWindowDCImpl::DoDrawEllipse( wxCoord x, wxCoord y, wxCoord width, wxCoord height )
{
    wxCHECK_RET( IsOk(), wxT("invalid window dc") );

    wxCoord xx = XLOG2DEV(x);
    wxCoord yy = YLOG2DEV(y);
    wxCoord ww = m_signX * XLOG2DEVREL(width);
    wxCoord hh = m_signY * YLOG2DEVREL(height);

    // CMB: handle -ve width and/or height
    if (ww < 0) { ww = -ww; xx = xx - ww; }
    if (hh < 0) { hh = -hh; yy = yy - hh; }

    if (m_gdkwindow)
    {
        if ( m_brush.IsNonTransparent() )
        {
            GdkGC* gc;
            bool originChanged;
            DrawingSetup(gc, originChanged);

            // If the pen is transparent pen we increase the size
            // for better compatibility with other platforms.
            if ( m_pen.IsNonTransparent() )
            {
                ++ww;
                ++hh;
            }

            gdk_draw_arc(m_gdkwindow, gc, true, xx, yy, ww, hh, 0, 360*64);

            if (originChanged)
                gdk_gc_set_ts_origin(gc, 0, 0);
        }

        if ( m_pen.IsNonTransparent() )
            gdk_draw_arc( m_gdkwindow, m_penGC, false, xx, yy, ww, hh, 0, 360*64 );
    }

    CalcBoundingBox( x, y );
    CalcBoundingBox( x + width, y + height );
}

void wxWindowDCImpl::DoDrawIcon( const wxIcon &icon, wxCoord x, wxCoord y )
{
    // VZ: egcs 1.0.3 refuses to compile this without cast, no idea why
    DoDrawBitmap( (const wxBitmap&)icon, x, y, true );
}

// scale a pixbuf, return new pixbuf, unref old one
static GdkPixbuf*
Scale(GdkPixbuf* pixbuf, int dst_w, int dst_h, double sx, double sy)
{
    GdkPixbuf* pixbuf_scaled = gdk_pixbuf_new(
        GDK_COLORSPACE_RGB, gdk_pixbuf_get_has_alpha(pixbuf), 8, dst_w, dst_h);
    gdk_pixbuf_scale(pixbuf, pixbuf_scaled,
        0, 0, dst_w, dst_h, 0, 0, sx, sy, GDK_INTERP_NEAREST);
    g_object_unref(pixbuf);
    return pixbuf_scaled;
}

// scale part of a pixmap using pixbuf scaling, return pixbuf
static GdkPixbuf*
Scale(GdkPixmap* pixmap, int x, int y, int w, int h, int dst_w, int dst_h, double sx, double sy)
{
    GdkPixbuf* pixbuf = gdk_pixbuf_get_from_drawable(
        NULL, pixmap, NULL, x, y, 0, 0, w, h);
    return Scale(pixbuf, dst_w, dst_h, sx, sy);
}

// scale part of a mask pixmap, return new mask, unref old one
static GdkPixmap*
ScaleMask(GdkPixmap* mask, int x, int y, int w, int h, int dst_w, int dst_h, double sx, double sy)
{
    GdkPixbuf* pixbuf = Scale(mask, x, y, w, h, dst_w, dst_h, sx, sy);

    // convert black and white pixbuf back to a mono pixmap
    const unsigned out_rowstride = (dst_w + 7) / 8;
    const size_t data_size = out_rowstride * size_t(dst_h);
    char* data = new char[data_size];
    char* out = data;
    const guchar* row = gdk_pixbuf_get_pixels(pixbuf);
    const int rowstride = gdk_pixbuf_get_rowstride(pixbuf);
    memset(data, 0, data_size);
    for (int j = 0; j < dst_h; j++, row += rowstride, out += out_rowstride)
    {
        const guchar* in = row;
        for (int i = 0; i < dst_w; i++, in += 3)
            if (*in)
                out[i >> 3] |= 1 << (i & 7);
    }
    g_object_unref(pixbuf);
    GdkPixmap* pixmap = gdk_bitmap_create_from_data(mask, data, dst_w, dst_h);
    delete[] data;
    g_object_unref(mask);
    return pixmap;
}

// Make a new mask from part of a mask and a clip region.
// Return new mask, unref old one.
static GdkPixmap*
ClipMask(GdkPixmap* mask, GdkRegion* clipRegion, int x, int y, int dst_x, int dst_y, int w, int h)
{
    GdkGCValues gcValues;
    gcValues.foreground.pixel = 0;
    GdkGC* gc = gdk_gc_new_with_values(mask, &gcValues, GDK_GC_FOREGROUND);
    GdkPixmap* pixmap = gdk_pixmap_new(mask, w, h, 1);
    // clear new mask, so clipped areas will be masked
    gdk_draw_rectangle(pixmap, gc, true, 0, 0, w, h);
    gdk_gc_set_clip_region(gc, clipRegion);
    gdk_gc_set_clip_origin(gc, -dst_x, -dst_y);
    // draw old mask onto new one, with clip
    gdk_draw_drawable(pixmap, gc, mask, x, y, 0, 0, w, h);
    g_object_unref(gc);
    g_object_unref(mask);
    return pixmap;
}

// make a color pixmap from part of a mono one, using text fg/bg colors
GdkPixmap*
wxWindowDCImpl::MonoToColor(GdkPixmap* monoPixmap, int x, int y, int w, int h) const
{
    GdkPixmap* pixmap = gdk_pixmap_new(m_gdkwindow, w, h, -1);
    GdkGCValues gcValues;
    gcValues.foreground.pixel = m_textForegroundColour.GetColor()->pixel;
    gcValues.background.pixel = m_textBackgroundColour.GetColor()->pixel;
    gcValues.stipple = monoPixmap;
    gcValues.fill = GDK_OPAQUE_STIPPLED;
    gcValues.ts_x_origin = -x;
    gcValues.ts_y_origin = -y;
    GdkGC* gc = gdk_gc_new_with_values(pixmap, &gcValues, GdkGCValuesMask(
        GDK_GC_FOREGROUND | GDK_GC_BACKGROUND | GDK_GC_STIPPLE | GDK_GC_FILL |
        GDK_GC_TS_X_ORIGIN | GDK_GC_TS_Y_ORIGIN));
    gdk_draw_rectangle(pixmap, gc, true, 0, 0, w, h);
    g_object_unref(gc);
    return pixmap;
}

void wxWindowDCImpl::DoDrawBitmap( const wxBitmap &bitmap,
                               wxCoord x, wxCoord y,
                               bool useMask )
{
    wxCHECK_RET( IsOk(), wxT("invalid window dc") );
    wxCHECK_RET( bitmap.IsOk(), wxT("invalid bitmap") );

    if (!m_gdkwindow) return;

    const int w = bitmap.GetWidth();
    const int h = bitmap.GetHeight();

    // notice that as the bitmap is not drawn upside down (or right to left)
    // even if the corresponding axis direction is inversed, we need to take it
    // into account when calculating its bounding box
    CalcBoundingBox(x, y);
    CalcBoundingBox(x + m_signX*w, y + m_signY*h);

    // device coords
    int xx = LogicalToDeviceX(x);
    const int yy = LogicalToDeviceY(y);
    const int ww = LogicalToDeviceXRel(w);
    const int hh = LogicalToDeviceYRel(h);

    if (m_window && m_window->GetLayoutDirection() == wxLayout_RightToLeft)
        xx -= ww;

    GdkRegion* const clipRegion = m_currentClippingRegion.GetRegion();
    // determine clip region overlap
    int overlap = wxInRegion;
    if (clipRegion)
    {
        overlap = m_currentClippingRegion.Contains(xx, yy, ww, hh);
        if (overlap == wxOutRegion)
            return;
    }

    const bool isScaled = ww != w || hh != h;
    const bool hasAlpha = bitmap.HasAlpha();
    GdkGC* const use_gc = m_penGC;

    GdkPixmap* mask = NULL;
    // mask does not work when drawing a pixbuf with alpha
    if (useMask && !hasAlpha)
    {
        wxMask* m = bitmap.GetMask();
        if (m)
            mask = m->GetBitmap();
    }
    if (mask)
    {
        g_object_ref(mask);
        if (isScaled)
            mask = ScaleMask(mask, 0, 0, w, h, ww, hh, m_scaleX, m_scaleY);
        if (overlap == wxPartRegion)
        {
            // need a new mask that also masks the clipped area,
            // because gc can't have both a mask and a clip region
            mask = ClipMask(mask, clipRegion, 0, 0, xx, yy, ww, hh);
        }
        gdk_gc_set_clip_mask(use_gc, mask);
        gdk_gc_set_clip_origin(use_gc, xx, yy);
    }

    // determine whether to use pixmap or pixbuf
    GdkPixmap* pixmap = NULL;
    GdkPixbuf* pixbuf = NULL;
    if (bitmap.HasPixmap())
        pixmap = bitmap.GetPixmap();
    if (pixmap && gdk_drawable_get_depth(pixmap) == 1)
    {
        // convert mono pixmap to color using text fg/bg colors
        pixmap = MonoToColor(pixmap, 0, 0, w, h);
    }
    else if (hasAlpha || pixmap == NULL)
    {
        pixmap = NULL;
        pixbuf = bitmap.GetPixbuf();
        g_object_ref(pixbuf);
    }
    else
    {
        g_object_ref(pixmap);
    }

    if (isScaled)
    {
        if (pixbuf)
            pixbuf = Scale(pixbuf, ww, hh, m_scaleX, m_scaleY);
        else
            pixbuf = Scale(pixmap, 0, 0, w, h, ww, hh, m_scaleX, m_scaleY);
    }

    if (pixbuf)
    {
        gdk_draw_pixbuf(m_gdkwindow, use_gc, pixbuf,
            0, 0, xx, yy, ww, hh, GDK_RGB_DITHER_NORMAL, 0, 0);
        g_object_unref(pixbuf);
    }
    else
    {
        gdk_draw_drawable(m_gdkwindow, use_gc, pixmap, 0, 0, xx, yy, ww, hh);
    }

    if (pixmap)
        g_object_unref(pixmap);
    if (mask)
    {
        g_object_unref(mask);
        gdk_gc_set_clip_region(use_gc, clipRegion);
    }
}

bool wxWindowDCImpl::DoBlit( wxCoord xdest, wxCoord ydest,
                         wxCoord width, wxCoord height,
                         wxDC *source,
                         wxCoord xsrc, wxCoord ysrc,
                         wxRasterOperationMode logical_func,
                         bool useMask,
                         wxCoord xsrcMask, wxCoord ysrcMask )
{
    wxCHECK_MSG( IsOk(), false, wxT("invalid window dc") );
    wxCHECK_MSG( source, false, wxT("invalid source dc") );

    if (!m_gdkwindow) return false;

    GdkDrawable* srcDrawable = NULL;
    GdkPixmap* mask = NULL;
    wxMemoryDC* memDC = wxDynamicCast(source, wxMemoryDC);
    if (memDC)
    {
        const wxBitmap& bitmap = memDC->GetSelectedBitmap();
        if (!bitmap.IsOk())
            return false;
        srcDrawable = bitmap.GetPixmap();
        if (useMask)
        {
            wxMask* m = bitmap.GetMask();
            if (m)
                mask = m->GetBitmap();
        }
    }
    else
    {
        wxDCImpl* impl = source->GetImpl();
        wxWindowDCImpl* gtk_impl = wxDynamicCast(impl, wxWindowDCImpl);
        if (gtk_impl)
            srcDrawable = gtk_impl->GetGDKWindow();
        if (srcDrawable == NULL)
            return false;
    }

    CalcBoundingBox(xdest, ydest);
    CalcBoundingBox(xdest + width, ydest + height);

    // source device coords
    int src_x = source->LogicalToDeviceX(xsrc);
    int src_y = source->LogicalToDeviceY(ysrc);
    int src_w = source->LogicalToDeviceXRel(width);
    int src_h = source->LogicalToDeviceYRel(height);

    // Clip source rect to source dc.
    // Only necessary when scaling, to avoid GDK errors when
    // converting to pixbuf, but no harm in always doing it.
    // If source rect changes, it also changes the dest rect.
    wxRect clip;
    gdk_drawable_get_size(srcDrawable, &clip.width, &clip.height);
    clip.Intersect(wxRect(src_x, src_y, src_w, src_h));
    if (src_w != clip.width || src_h != clip.height)
    {
        if (clip.width == 0)
            return true;

        src_w = clip.width;
        src_h = clip.height;
        width  = source->DeviceToLogicalXRel(src_w);
        height = source->DeviceToLogicalYRel(src_h);
        if (src_x != clip.x || src_y != clip.y)
        {
            xdest += source->DeviceToLogicalXRel(clip.x - src_x);
            ydest += source->DeviceToLogicalYRel(clip.y - src_y);
            src_x = clip.x;
            src_y = clip.y;
        }
    }

    // destination device coords
    const int dst_x = LogicalToDeviceX(xdest);
    const int dst_y = LogicalToDeviceY(ydest);
    const int dst_w = LogicalToDeviceXRel(width);
    const int dst_h = LogicalToDeviceYRel(height);

    GdkRegion* const clipRegion = m_currentClippingRegion.GetRegion();
    // determine dest clip region overlap
    int overlap = wxInRegion;
    if (clipRegion)
    {
        overlap = m_currentClippingRegion.Contains(dst_x, dst_y, dst_w, dst_h);
        if (overlap == wxOutRegion)
            return true;
    }

    const bool isScaled = src_w != dst_w || src_h != dst_h;
    double scale_x = 0;
    double scale_y = 0;
    if (isScaled)
    {
        // get source to dest scale
        double usx, usy, lsx, lsy;
        source->GetUserScale(&usx, &usy);
        source->GetLogicalScale(&lsx, &lsy);
        scale_x = m_scaleX / (usx * lsx);
        scale_y = m_scaleY / (usy * lsy);
    }

    GdkGC* const use_gc = m_penGC;

    if (mask)
    {
        g_object_ref(mask);
        int srcMask_x = src_x;
        int srcMask_y = src_y;
        if (xsrcMask != -1 || ysrcMask != -1)
        {
            srcMask_x = source->LogicalToDeviceX(xsrcMask);
            srcMask_y = source->LogicalToDeviceY(ysrcMask);
        }
        if (isScaled)
        {
            mask = ScaleMask(mask, srcMask_x, srcMask_y,
                src_w, src_h, dst_w, dst_h, scale_x, scale_y);
            srcMask_x = 0;
            srcMask_y = 0;
        }
        if (overlap == wxPartRegion)
        {
            // need a new mask that also masks the clipped area,
            // because gc can't have both a mask and a clip region
            mask = ClipMask(mask, clipRegion,
                srcMask_x, srcMask_y, dst_x, dst_y, dst_w, dst_h);
            srcMask_x = 0;
            srcMask_y = 0;
        }
        gdk_gc_set_clip_mask(use_gc, mask);
        gdk_gc_set_clip_origin(use_gc, dst_x - srcMask_x, dst_y - srcMask_y);
    }

    GdkPixmap* pixmap = NULL;
    if (gdk_drawable_get_depth(srcDrawable) == 1)
    {
        // Convert mono pixmap to color using text fg/bg colors.
        // Scaling/drawing is simpler if this is done first.
        pixmap = MonoToColor(srcDrawable, src_x, src_y, src_w, src_h);
        srcDrawable = pixmap;
        src_x = 0;
        src_y = 0;
    }

    const wxRasterOperationMode logical_func_save = m_logicalFunction;
    SetLogicalFunction(logical_func);
    if (memDC == NULL)
        gdk_gc_set_subwindow(use_gc, GDK_INCLUDE_INFERIORS);

    if (isScaled)
    {
        GdkPixbuf* pixbuf = Scale(srcDrawable,
            src_x, src_y, src_w, src_h, dst_w, dst_h, scale_x, scale_y);
        gdk_draw_pixbuf(m_gdkwindow, use_gc, pixbuf,
            0, 0, dst_x, dst_y, dst_w, dst_h, GDK_RGB_DITHER_NONE, 0, 0);
        g_object_unref(pixbuf);
    }
    else
    {
        gdk_draw_drawable(m_gdkwindow, use_gc, srcDrawable,
            src_x, src_y, dst_x, dst_y, dst_w, dst_h);
    }

    SetLogicalFunction(logical_func_save);
    if (memDC == NULL)
        gdk_gc_set_subwindow(use_gc, GDK_CLIP_BY_CHILDREN);

    if (pixmap)
        g_object_unref(pixmap);
    if (mask)
    {
        g_object_unref(mask);
        gdk_gc_set_clip_region(use_gc, clipRegion);
    }
    return true;
}

void wxWindowDCImpl::DoDrawText(const wxString& text,
                                wxCoord xLogical,
                                wxCoord yLogical)
{
    wxCHECK_RET( IsOk(), wxT("invalid window dc") );

    if (!m_gdkwindow) return;

    if (text.empty()) return;

    wxCoord x = XLOG2DEV(xLogical),
            y = YLOG2DEV(yLogical);

    wxCHECK_RET( m_context, wxT("no Pango context") );
    wxCHECK_RET( m_layout, wxT("no Pango layout") );
    wxCHECK_RET( m_fontdesc, wxT("no Pango font description") );

    gdk_pango_context_set_colormap( m_context, m_cmap );  // not needed in gtk+ >= 2.6

    bool underlined = m_font.IsOk() && m_font.GetUnderlined();

    wxCharBuffer data = wxGTK_CONV(text);
    if ( !data )
        return;
    size_t datalen = strlen(data);

    // in Pango >= 1.16 the "underline of leading/trailing spaces" bug
    // has been fixed and thus the hack implemented below should never be used
    static bool pangoOk = !wx_pango_version_check(1, 16, 0);

    bool needshack = underlined && !pangoOk;

    if (needshack)
    {
        // a PangoLayout which has leading/trailing spaces with underlined font
        // is not correctly drawn by this pango version: Pango won't underline the spaces.
        // This can be a problem; e.g. wxHTML rendering of underlined text relies on
        // this behaviour. To workaround this problem, we use a special hack here
        // suggested by pango maintainer Behdad Esfahbod: we prepend and append two
        // empty space characters and give them a dummy colour attribute.
        // This will force Pango to underline the leading/trailing spaces, too.

        wxCharBuffer data_tmp(datalen + 6);
        // copy the leading U+200C ZERO WIDTH NON-JOINER encoded in UTF8 format
        memcpy(data_tmp.data(), "\342\200\214", 3);
        // copy the user string
        memcpy(data_tmp.data() + 3, data, datalen);
        // copy the trailing U+200C ZERO WIDTH NON-JOINER encoded in UTF8 format
        memcpy(data_tmp.data() + 3 + datalen, "\342\200\214", 3);

        data = data_tmp;
        datalen += 6;
    }

    pango_layout_set_text(m_layout, data, datalen);

    if (underlined)
    {
        PangoAttrList *attrs = pango_attr_list_new();
        PangoAttribute *a = pango_attr_underline_new(PANGO_UNDERLINE_SINGLE);
        a->start_index = 0;
        a->end_index = datalen;
        pango_attr_list_insert(attrs, a);

        if (needshack)
        {
            // dummy colour for the leading space
            a = pango_attr_foreground_new (0x0057, 0x52A9, 0xD614);
            a->start_index = 0;
            a->end_index = 1;
            pango_attr_list_insert(attrs, a);

            // dummy colour for the trailing space
            a = pango_attr_foreground_new (0x0057, 0x52A9, 0xD614);
            a->start_index = datalen - 1;
            a->end_index = datalen;
            pango_attr_list_insert(attrs, a);
        }

        pango_layout_set_attributes(m_layout, attrs);
        pango_attr_list_unref(attrs);
    }

    int oldSize = 0;
    const bool isScaled = fabs(m_scaleY - 1.0) > 0.00001;
    if (isScaled)
    {
        // If there is a user or actually any scale applied to
        // the device context, scale the font.

        // scale font description
        oldSize = pango_font_description_get_size(m_fontdesc);
        pango_font_description_set_size(m_fontdesc, int(oldSize * m_scaleY));

        // actually apply scaled font
        pango_layout_set_font_description( m_layout, m_fontdesc );
    }

    int w, h;
    pango_layout_get_pixel_size(m_layout, &w, &h);

    // Draw layout.
    int x_rtl = x;
    if (m_window && m_window->GetLayoutDirection() == wxLayout_RightToLeft)
        x_rtl -= w;

    const GdkColor* bg_col = NULL;
    if (m_backgroundMode == wxBRUSHSTYLE_SOLID)
        bg_col = m_textBackgroundColour.GetColor();

    gdk_draw_layout_with_colors(m_gdkwindow, m_textGC, x_rtl, y, m_layout, NULL, bg_col);

    if (isScaled)
    {
         // reset unscaled size
         pango_font_description_set_size( m_fontdesc, oldSize );

         // actually apply unscaled font
         pango_layout_set_font_description( m_layout, m_fontdesc );
    }
    if (underlined)
    {
        // undo underline attributes setting:
        pango_layout_set_attributes(m_layout, NULL);
    }

    CalcBoundingBox(xLogical + int(w / m_scaleX), yLogical + int(h / m_scaleY));
    CalcBoundingBox(xLogical, yLogical);
}

// TODO: When GTK2.6 is required, merge DoDrawText and DoDrawRotatedText to
// avoid code duplication
void wxWindowDCImpl::DoDrawRotatedText( const wxString &text, wxCoord x, wxCoord y, double angle )
{
    if (!m_gdkwindow || text.empty())
        return;

    wxCHECK_RET( IsOk(), wxT("invalid window dc") );

#ifdef __WXGTK26__
    if (!gtk_check_version(2,6,0))
    {
        x = XLOG2DEV(x);
        y = YLOG2DEV(y);

        pango_layout_set_text(m_layout, wxGTK_CONV(text), -1);

        if (m_font.GetUnderlined())
        {
            PangoAttrList *attrs = pango_attr_list_new();
            PangoAttribute *a = pango_attr_underline_new(PANGO_UNDERLINE_SINGLE);
            pango_attr_list_insert(attrs, a);
            pango_layout_set_attributes(m_layout, attrs);
            pango_attr_list_unref(attrs);
        }

        int oldSize = 0;
        const bool isScaled = fabs(m_scaleY - 1.0) > 0.00001;
        if (isScaled)
        {
            //TODO: when Pango >= 1.6 is required, use pango_matrix_scale()
             // If there is a user or actually any scale applied to
             // the device context, scale the font.

             // scale font description
            oldSize = pango_font_description_get_size(m_fontdesc);
            pango_font_description_set_size(m_fontdesc, int(oldSize * m_scaleY));

             // actually apply scaled font
             pango_layout_set_font_description( m_layout, m_fontdesc );
        }

        int w, h;
        pango_layout_get_pixel_size(m_layout, &w, &h);

        const GdkColor* bg_col = NULL;
        if (m_backgroundMode == wxBRUSHSTYLE_SOLID)
            bg_col = m_textBackgroundColour.GetColor();

        // rotate the text
        PangoMatrix matrix = PANGO_MATRIX_INIT;
        pango_matrix_rotate (&matrix, angle);
        pango_context_set_matrix (m_context, &matrix);
        pango_layout_context_changed (m_layout);

        // To be compatible with MSW, the rotation axis must be in the old
        // top-left corner.
        // Calculate the vertices of the rotated rectangle containing the text,
        // relative to the old top-left vertex.
        // We could use the matrix for this, but it's simpler with trignonometry.
        double rad = DegToRad(angle);
        // the rectangle vertices are counted clockwise with the first one
        // being at (0, 0)
        double x2 = w * cos(rad);
        double y2 = -w * sin(rad);   // y axis points to the bottom, hence minus
        double x4 = h * sin(rad);
        double y4 = h * cos(rad);
        double x3 = x4 + x2;
        double y3 = y4 + y2;
        // Then we calculate max and min of the rotated rectangle.
        wxCoord maxX = (wxCoord)(dmax(dmax(0, x2), dmax(x3, x4)) + 0.5),
                maxY = (wxCoord)(dmax(dmax(0, y2), dmax(y3, y4)) + 0.5),
                minX = (wxCoord)(dmin(dmin(0, x2), dmin(x3, x4)) - 0.5),
                minY = (wxCoord)(dmin(dmin(0, y2), dmin(y3, y4)) - 0.5);

        gdk_draw_layout_with_colors(m_gdkwindow, m_textGC, x+minX, y+minY,
                                    m_layout, NULL, bg_col);

        if (m_font.GetUnderlined())
            pango_layout_set_attributes(m_layout, NULL);

        // clean up the transformation matrix
        pango_context_set_matrix(m_context, NULL);

        if (isScaled)
        {
             // reset unscaled size
             pango_font_description_set_size( m_fontdesc, oldSize );

             // actually apply unscaled font
             pango_layout_set_font_description( m_layout, m_fontdesc );
        }

        CalcBoundingBox(x+minX, y+minY);
        CalcBoundingBox(x+maxX, y+maxY);
    }
    else
#endif //__WXGTK26__
    {
#if wxUSE_IMAGE
    if ( wxIsNullDouble(angle) )
    {
        DoDrawText(text, x, y);
        return;
    }

    wxCoord w;
    wxCoord h;

    // TODO: implement later without GdkFont for GTK 2.0
    DoGetTextExtent(text, &w, &h, NULL,NULL, &m_font);

    // draw the string normally
    wxBitmap src(w, h);
    wxMemoryDC dc;
    dc.SelectObject(src);
    dc.SetFont(GetFont());
    dc.SetBackground(*wxBLACK_BRUSH);
    dc.SetBrush(*wxBLACK_BRUSH);
    dc.Clear();
    dc.SetTextForeground( *wxWHITE );
    dc.DrawText(text, 0, 0);
    dc.SelectObject(wxNullBitmap);

    // Calculate the size of the rotated bounding box.
    double rad = DegToRad(angle);
    double dx = cos(rad),
           dy = sin(rad);

    // the rectngle vertices are counted clockwise with the first one being at
    // (0, 0) (or, rather, at (x, y))
    double x2 = w*dx,
           y2 = -w*dy;      // y axis points to the bottom, hence minus
    double x4 = h*dy,
           y4 = h*dx;
    double x3 = x4 + x2,
           y3 = y4 + y2;

    // calc max and min
    wxCoord maxX = (wxCoord)(dmax(x2, dmax(x3, x4)) + 0.5),
            maxY = (wxCoord)(dmax(y2, dmax(y3, y4)) + 0.5),
            minX = (wxCoord)(dmin(x2, dmin(x3, x4)) - 0.5),
            minY = (wxCoord)(dmin(y2, dmin(y3, y4)) - 0.5);


    wxImage image = src.ConvertToImage();

    image.ConvertColourToAlpha( m_textForegroundColour.Red(),
                                m_textForegroundColour.Green(),
                                m_textForegroundColour.Blue() );
    image = image.Rotate( rad, wxPoint(0,0) );

    int i_angle = (int) angle;
    i_angle = i_angle % 360;
    if (i_angle < 0)
        i_angle += 360;
    int xoffset = 0;
    if ((i_angle >= 90.0) && (i_angle < 270.0))
        xoffset = image.GetWidth();
    int yoffset = 0;
    if ((i_angle >= 0.0) && (i_angle < 180.0))
        yoffset = image.GetHeight();

    if ((i_angle >= 0) && (i_angle < 90))
        yoffset -= (int)( cos(rad)*h );
    if ((i_angle >= 90) && (i_angle < 180))
        xoffset -= (int)( sin(rad)*h );
    if ((i_angle >= 180) && (i_angle < 270))
        yoffset -= (int)( cos(rad)*h );
    if ((i_angle >= 270) && (i_angle < 360))
        xoffset -= (int)( sin(rad)*h );

    int i_x = x - xoffset;
    int i_y = y - yoffset;

    src = image;
    DoDrawBitmap( src, i_x, i_y, true );


    // it would be better to draw with non underlined font and draw the line
    // manually here (it would be more straight...)
#if 0
    if ( m_font.GetUnderlined() )
    {
        gdk_draw_line( m_gdkwindow, m_textGC,
                       XLOG2DEV(x + x4), YLOG2DEV(y + y4 + font->descent),
                       XLOG2DEV(x + x3), YLOG2DEV(y + y3 + font->descent));
    }
#endif // 0

    // update the bounding box
    CalcBoundingBox(x + minX, y + minY);
    CalcBoundingBox(x + maxX, y + maxY);
#else // !wxUSE_IMAGE
    wxUnusedVar(text);
    wxUnusedVar(x);
    wxUnusedVar(y);
    wxUnusedVar(angle);
#endif // wxUSE_IMAGE/!wxUSE_IMAGE
    }
}

void wxWindowDCImpl::DoGetTextExtent(const wxString &string,
                                 wxCoord *width, wxCoord *height,
                                 wxCoord *descent, wxCoord *externalLeading,
                                 const wxFont *theFont) const
{
    if ( width )
        *width = 0;
    if ( height )
        *height = 0;
    if ( descent )
        *descent = 0;
    if ( externalLeading )
        *externalLeading = 0;

    if (string.empty())
        return;

    // ensure that theFont is always non-NULL
    if ( !theFont || !theFont->IsOk() )
        theFont = &m_font;

    // and use it if it's valid
    if ( theFont->IsOk() )
    {
        pango_layout_set_font_description
        (
            m_layout,
            theFont->GetNativeFontInfo()->description
        );
    }

    // Set layout's text
    const wxCharBuffer dataUTF8 = wxGTK_CONV_FONT(string, *theFont);
    if ( !dataUTF8 )
    {
        // hardly ideal, but what else can we do if conversion failed?
        return;
    }

    pango_layout_set_text(m_layout, dataUTF8, -1);

    int h;
    pango_layout_get_pixel_size(m_layout, width, &h);
    if (descent)
    {
        PangoLayoutIter *iter = pango_layout_get_iter(m_layout);
        int baseline = pango_layout_iter_get_baseline(iter);
        pango_layout_iter_free(iter);
        *descent = h - PANGO_PIXELS(baseline);
    }
    if (height)
        *height = h;

    // Reset old font description
    if (theFont->IsOk())
        pango_layout_set_font_description( m_layout, m_fontdesc );
}


bool wxWindowDCImpl::DoGetPartialTextExtents(const wxString& text,
                                         wxArrayInt& widths) const
{
    const size_t len = text.length();
    widths.Empty();
    widths.Add(0, len);

    if (text.empty())
        return true;

    // Set layout's text
    const wxCharBuffer dataUTF8 = wxGTK_CONV_FONT(text, m_font);
    if ( !dataUTF8 )
    {
        // hardly ideal, but what else can we do if conversion failed?
        wxLogLastError(wxT("DoGetPartialTextExtents"));
        return false;
    }

    pango_layout_set_text(m_layout, dataUTF8, -1);

    // Calculate the position of each character based on the widths of
    // the previous characters

    // Code borrowed from Scintilla's PlatGTK
    PangoLayoutIter *iter = pango_layout_get_iter(m_layout);
    PangoRectangle pos;
    pango_layout_iter_get_cluster_extents(iter, NULL, &pos);
    size_t i = 0;
    while (pango_layout_iter_next_cluster(iter))
    {
        pango_layout_iter_get_cluster_extents(iter, NULL, &pos);
        int position = PANGO_PIXELS(pos.x);
        widths[i++] = position;
    }
    while (i < len)
        widths[i++] = PANGO_PIXELS(pos.x + pos.width);
    pango_layout_iter_free(iter);

    return true;
}


wxCoord wxWindowDCImpl::GetCharWidth() const
{
    pango_layout_set_text( m_layout, "H", 1 );
    int w;
    pango_layout_get_pixel_size( m_layout, &w, NULL );
    return w;
}

wxCoord wxWindowDCImpl::GetCharHeight() const
{
    PangoFontMetrics *metrics = pango_context_get_metrics (m_context, m_fontdesc, pango_context_get_language(m_context));
    wxCHECK_MSG( metrics, -1, wxT("failed to get pango font metrics") );

    wxCoord h = PANGO_PIXELS (pango_font_metrics_get_descent (metrics) +
                              pango_font_metrics_get_ascent (metrics));
    pango_font_metrics_unref (metrics);
    return h;
}

void wxWindowDCImpl::Clear()
{
    wxCHECK_RET( IsOk(), wxT("invalid window dc") );

    if (!m_gdkwindow) return;

    int width,height;
    DoGetSize( &width, &height );
    gdk_draw_rectangle( m_gdkwindow, m_bgGC, TRUE, 0, 0, width, height );
}

void wxWindowDCImpl::SetFont( const wxFont &font )
{
    m_font = font;

    if (m_font.IsOk())
    {
        if (m_fontdesc)
            pango_font_description_free( m_fontdesc );

        m_fontdesc = pango_font_description_copy( m_font.GetNativeFontInfo()->description );


        if (m_window)
        {
            PangoContext *oldContext = m_context;

            m_context = m_window->GTKGetPangoDefaultContext();

            // If we switch back/forth between different contexts
            // we also have to create a new layout. I think so,
            // at least, and it doesn't hurt to do it.
            if (oldContext != m_context)
            {
                if (m_layout)
                    g_object_unref (m_layout);

                m_layout = pango_layout_new( m_context );
            }
        }

        pango_layout_set_font_description( m_layout, m_fontdesc );
    }
}

void wxWindowDCImpl::SetPen( const wxPen &pen )
{
    wxCHECK_RET( IsOk(), wxT("invalid window dc") );

    if (m_pen == pen) return;

    m_pen = pen;

    if (!m_pen.IsOk()) return;

    if (!m_gdkwindow) return;

    gint width = m_pen.GetWidth();
    if (width <= 0)
    {
        // CMB: if width is non-zero scale it with the dc
        width = 1;
    }
    else
    {
        // X doesn't allow different width in x and y and so we take
        // the average
        double w = 0.5 +
                   ( fabs((double) XLOG2DEVREL(width)) +
                     fabs((double) YLOG2DEVREL(width)) ) / 2.0;
        width = (int)w;
        if ( !width )
        {
            // width can't be 0 or an internal GTK error occurs inside
            // gdk_gc_set_dashes() below
            width = 1;
        }
    }

    static const wxGTKDash dotted[] = {1, 1};
    static const wxGTKDash short_dashed[] = {2, 2};
    static const wxGTKDash wxCoord_dashed[] = {2, 4};
    static const wxGTKDash dotted_dashed[] = {3, 3, 1, 3};

    // We express dash pattern in pen width unit, so we are
    // independent of zoom factor and so on...
    int req_nb_dash;
    const wxGTKDash *req_dash;

    GdkLineStyle lineStyle = GDK_LINE_ON_OFF_DASH;
    switch (m_pen.GetStyle())
    {
        case wxPENSTYLE_USER_DASH:
            req_nb_dash = m_pen.GetDashCount();
            req_dash = (wxGTKDash*)m_pen.GetDash();
            break;
        case wxPENSTYLE_DOT:
            req_nb_dash = 2;
            req_dash = dotted;
            break;
        case wxPENSTYLE_LONG_DASH:
            req_nb_dash = 2;
            req_dash = wxCoord_dashed;
            break;
        case wxPENSTYLE_SHORT_DASH:
            req_nb_dash = 2;
            req_dash = short_dashed;
            break;
        case wxPENSTYLE_DOT_DASH:
            req_nb_dash = 4;
            req_dash = dotted_dashed;
            break;

        case wxPENSTYLE_TRANSPARENT:
        case wxPENSTYLE_STIPPLE_MASK_OPAQUE:
        case wxPENSTYLE_STIPPLE:
        case wxPENSTYLE_SOLID:
        default:
            lineStyle = GDK_LINE_SOLID;
            req_dash = NULL;
            req_nb_dash = 0;
            break;
    }

    if (req_dash && req_nb_dash)
    {
        wxGTKDash *real_req_dash = new wxGTKDash[req_nb_dash];
        if (real_req_dash)
        {
            for (int i = 0; i < req_nb_dash; i++)
                real_req_dash[i] = req_dash[i] * width;
            gdk_gc_set_dashes( m_penGC, 0, real_req_dash, req_nb_dash );
            delete[] real_req_dash;
        }
        else
        {
            // No Memory. We use non-scaled dash pattern...
            gdk_gc_set_dashes( m_penGC, 0, (wxGTKDash*)req_dash, req_nb_dash );
        }
    }

    GdkCapStyle capStyle = GDK_CAP_ROUND;
    switch (m_pen.GetCap())
    {
        case wxCAP_PROJECTING: { capStyle = GDK_CAP_PROJECTING; break; }
        case wxCAP_BUTT:       { capStyle = GDK_CAP_BUTT;       break; }
        case wxCAP_ROUND:
        default:
            if (width <= 1)
            {
                width = 0;
                capStyle = GDK_CAP_NOT_LAST;
            }
            break;
    }

    GdkJoinStyle joinStyle = GDK_JOIN_ROUND;
    switch (m_pen.GetJoin())
    {
        case wxJOIN_BEVEL: { joinStyle = GDK_JOIN_BEVEL; break; }
        case wxJOIN_MITER: { joinStyle = GDK_JOIN_MITER; break; }
        case wxJOIN_ROUND:
        default:           { joinStyle = GDK_JOIN_ROUND; break; }
    }

    gdk_gc_set_line_attributes( m_penGC, width, lineStyle, capStyle, joinStyle );

    m_pen.GetColour().CalcPixel( m_cmap );
    gdk_gc_set_foreground( m_penGC, m_pen.GetColour().GetColor() );
}

void wxWindowDCImpl::SetBrush( const wxBrush &brush )
{
    wxCHECK_RET( IsOk(), wxT("invalid window dc") );

    if (m_brush == brush) return;

    m_brush = brush;

    if (!m_brush.IsOk()) return;

    if (!m_gdkwindow) return;

    m_brush.GetColour().CalcPixel( m_cmap );
    gdk_gc_set_foreground( m_brushGC, m_brush.GetColour().GetColor() );

    gdk_gc_set_fill( m_brushGC, GDK_SOLID );

    if ((m_brush.GetStyle() == wxBRUSHSTYLE_STIPPLE) && (m_brush.GetStipple()->IsOk()))
    {
        if (m_brush.GetStipple()->GetDepth() != 1)
        {
            gdk_gc_set_fill( m_brushGC, GDK_TILED );
            gdk_gc_set_tile( m_brushGC, m_brush.GetStipple()->GetPixmap() );
        }
        else
        {
            gdk_gc_set_fill( m_brushGC, GDK_STIPPLED );
            gdk_gc_set_stipple( m_brushGC, m_brush.GetStipple()->GetPixmap() );
        }
    }

    if ((m_brush.GetStyle() == wxBRUSHSTYLE_STIPPLE_MASK_OPAQUE) && (m_brush.GetStipple()->GetMask()))
    {
        gdk_gc_set_fill( m_textGC, GDK_OPAQUE_STIPPLED);
        gdk_gc_set_stipple( m_textGC, m_brush.GetStipple()->GetMask()->GetBitmap() );
    }

    if (m_brush.IsHatch())
    {
        gdk_gc_set_fill( m_brushGC, GDK_STIPPLED );
        gdk_gc_set_stipple(m_brushGC, GetHatch(m_brush.GetStyle()));
    }
}

void wxWindowDCImpl::SetBackground( const wxBrush &brush )
{
   /* CMB 21/7/98: Added SetBackground. Sets background brush
    * for Clear() and bg colour for shapes filled with cross-hatch brush */

    wxCHECK_RET( IsOk(), wxT("invalid window dc") );

    if (m_backgroundBrush == brush) return;

    m_backgroundBrush = brush;

    if (!m_backgroundBrush.IsOk()) return;

    if (!m_gdkwindow) return;

    wxColor color = m_backgroundBrush.GetColour();
    color.CalcPixel(m_cmap);
    const GdkColor* gdkColor = color.GetColor();
    gdk_gc_set_background(m_brushGC, gdkColor);
    gdk_gc_set_background(m_penGC,   gdkColor);
    gdk_gc_set_background(m_bgGC,    gdkColor);
    gdk_gc_set_foreground(m_bgGC,    gdkColor);


    gdk_gc_set_fill( m_bgGC, GDK_SOLID );

    if (m_backgroundBrush.GetStyle() == wxBRUSHSTYLE_STIPPLE)
    {
        const wxBitmap* stipple = m_backgroundBrush.GetStipple();
        if (stipple->IsOk())
        {
            if (stipple->GetDepth() != 1)
            {
                gdk_gc_set_fill(m_bgGC, GDK_TILED);
                gdk_gc_set_tile(m_bgGC, stipple->GetPixmap());
            }
            else
            {
                gdk_gc_set_fill(m_bgGC, GDK_STIPPLED);
                gdk_gc_set_stipple(m_bgGC, stipple->GetPixmap());
            }
        }
    }
    else if (m_backgroundBrush.IsHatch())
    {
        gdk_gc_set_fill( m_bgGC, GDK_STIPPLED );
        gdk_gc_set_stipple(m_bgGC, GetHatch(m_backgroundBrush.GetStyle()));
    }
}

void wxWindowDCImpl::SetLogicalFunction( wxRasterOperationMode function )
{
    wxCHECK_RET( IsOk(), wxT("invalid window dc") );

    if (m_logicalFunction == function)
        return;

    // VZ: shouldn't this be a CHECK?
    if (!m_gdkwindow)
        return;

    GdkFunction mode;
    switch (function)
    {
        case wxXOR:          mode = GDK_XOR;           break;
        case wxINVERT:       mode = GDK_INVERT;        break;
        case wxOR_REVERSE:   mode = GDK_OR_REVERSE;    break;
        case wxAND_REVERSE:  mode = GDK_AND_REVERSE;   break;
        case wxCLEAR:        mode = GDK_CLEAR;         break;
        case wxSET:          mode = GDK_SET;           break;
        case wxOR_INVERT:    mode = GDK_OR_INVERT;     break;
        case wxAND:          mode = GDK_AND;           break;
        case wxOR:           mode = GDK_OR;            break;
        case wxEQUIV:        mode = GDK_EQUIV;         break;
        case wxNAND:         mode = GDK_NAND;          break;
        case wxAND_INVERT:   mode = GDK_AND_INVERT;    break;
        case wxCOPY:         mode = GDK_COPY;          break;
        case wxNO_OP:        mode = GDK_NOOP;          break;
        case wxSRC_INVERT:   mode = GDK_COPY_INVERT;   break;
        case wxNOR:          mode = GDK_NOR;           break;
        default:
            wxFAIL_MSG("unknown mode");
            return;
    }

    m_logicalFunction = function;

    gdk_gc_set_function( m_penGC, mode );
    gdk_gc_set_function( m_brushGC, mode );

    // to stay compatible with wxMSW, we don't apply ROPs to the text
    // operations (i.e. DrawText/DrawRotatedText).
    // True, but mono-bitmaps use the m_textGC and they use ROPs as well.
    gdk_gc_set_function( m_textGC, mode );
}

void wxWindowDCImpl::SetTextForeground( const wxColour &col )
{
    wxCHECK_RET( IsOk(), wxT("invalid window dc") );

    // don't set m_textForegroundColour to an invalid colour as we'd crash
    // later then (we use m_textForegroundColour.GetColor() without checking
    // in a few places)
    if ( !col.IsOk() || (m_textForegroundColour == col) )
        return;

    m_textForegroundColour = col;

    if ( m_gdkwindow )
    {
        m_textForegroundColour.CalcPixel( m_cmap );
        gdk_gc_set_foreground( m_textGC, m_textForegroundColour.GetColor() );
    }
}

void wxWindowDCImpl::SetTextBackground( const wxColour &col )
{
    wxCHECK_RET( IsOk(), wxT("invalid window dc") );

    // same as above
    if ( !col.IsOk() || (m_textBackgroundColour == col) )
        return;

    m_textBackgroundColour = col;

    if ( m_gdkwindow )
    {
        m_textBackgroundColour.CalcPixel( m_cmap );
        gdk_gc_set_background( m_textGC, m_textBackgroundColour.GetColor() );
    }
}

void wxWindowDCImpl::SetBackgroundMode( int mode )
{
    wxCHECK_RET( IsOk(), wxT("invalid window dc") );

    m_backgroundMode = mode;
}

void wxWindowDCImpl::SetPalette( const wxPalette& WXUNUSED(palette) )
{
    wxFAIL_MSG( wxT("wxWindowDCImpl::SetPalette not implemented") );
}

void wxWindowDCImpl::DoSetClippingRegion( wxCoord x, wxCoord y, wxCoord width, wxCoord height )
{
    wxCHECK_RET( IsOk(), wxT("invalid window dc") );

    if (!m_gdkwindow) return;

    wxRect rect;
    rect.x = XLOG2DEV(x);
    rect.y = YLOG2DEV(y);
    rect.width = XLOG2DEVREL(width);
    rect.height = YLOG2DEVREL(height);

    if (m_window && m_window->m_wxwindow &&
        (m_window->GetLayoutDirection() == wxLayout_RightToLeft))
    {
        rect.x -= rect.width;
    }

    DoSetDeviceClippingRegion(wxRegion(rect));
}

void wxWindowDCImpl::DoSetDeviceClippingRegion( const wxRegion &region  )
{
    wxCHECK_RET( IsOk(), wxT("invalid window dc") );

    if (region.Empty())
    {
        DestroyClippingRegion();
        return;
    }

    if (!m_gdkwindow) return;

    if (!m_currentClippingRegion.IsNull())
        m_currentClippingRegion.Intersect( region );
    else
        m_currentClippingRegion.Union( region );

#if USE_PAINT_REGION
    if (!m_paintClippingRegion.IsNull())
        m_currentClippingRegion.Intersect( m_paintClippingRegion );
#endif

    wxCoord xx, yy, ww, hh;
    m_currentClippingRegion.GetBox( xx, yy, ww, hh );
    wxGTKDCImpl::DoSetClippingRegion( xx, yy, ww, hh );

    GdkRegion* gdkRegion = m_currentClippingRegion.GetRegion();
    gdk_gc_set_clip_region(m_penGC,   gdkRegion);
    gdk_gc_set_clip_region(m_brushGC, gdkRegion);
    gdk_gc_set_clip_region(m_textGC,  gdkRegion);
    gdk_gc_set_clip_region(m_bgGC,    gdkRegion);
}

void wxWindowDCImpl::DestroyClippingRegion()
{
    wxCHECK_RET( IsOk(), wxT("invalid window dc") );

    wxDCImpl::DestroyClippingRegion();

    m_currentClippingRegion.Clear();

#if USE_PAINT_REGION
    if (!m_paintClippingRegion.IsEmpty())
        m_currentClippingRegion.Union( m_paintClippingRegion );
#endif

    if (!m_gdkwindow) return;

    GdkRegion* gdkRegion = NULL;
    if (!m_currentClippingRegion.IsEmpty())
        gdkRegion = m_currentClippingRegion.GetRegion();

    gdk_gc_set_clip_region(m_penGC,   gdkRegion);
    gdk_gc_set_clip_region(m_brushGC, gdkRegion);
    gdk_gc_set_clip_region(m_textGC,  gdkRegion);
    gdk_gc_set_clip_region(m_bgGC,    gdkRegion);
}

void wxWindowDCImpl::Destroy()
{
    if (m_penGC) wxFreePoolGC( m_penGC );
    m_penGC = NULL;
    if (m_brushGC) wxFreePoolGC( m_brushGC );
    m_brushGC = NULL;
    if (m_textGC) wxFreePoolGC( m_textGC );
    m_textGC = NULL;
    if (m_bgGC) wxFreePoolGC( m_bgGC );
    m_bgGC = NULL;
}

void wxWindowDCImpl::SetDeviceOrigin( wxCoord x, wxCoord y )
{
    m_deviceOriginX = x;
    m_deviceOriginY = y;

    ComputeScaleAndOrigin();
}

void wxWindowDCImpl::SetAxisOrientation( bool xLeftRight, bool yBottomUp )
{
    m_signX = (xLeftRight ?  1 : -1);
    m_signY = (yBottomUp  ? -1 :  1);

    if (m_window && m_window->m_wxwindow &&
        (m_window->GetLayoutDirection() == wxLayout_RightToLeft))
        m_signX = -m_signX;

    ComputeScaleAndOrigin();
}

void wxWindowDCImpl::ComputeScaleAndOrigin()
{
    const wxRealPoint origScale(m_scaleX, m_scaleY);

    wxDCImpl::ComputeScaleAndOrigin();

    // if scale has changed call SetPen to recalulate the line width
    if ( wxRealPoint(m_scaleX, m_scaleY) != origScale && m_pen.IsOk() )
    {
        // this is a bit artificial, but we need to force wxDC to think the pen
        // has changed
        wxPen pen = m_pen;
        m_pen = wxNullPen;
        SetPen( pen );
    }
}

// Resolution in pixels per logical inch
wxSize wxWindowDCImpl::GetPPI() const
{
    return wxSize( (int) (m_mm_to_pix_x * 25.4 + 0.5), (int) (m_mm_to_pix_y * 25.4 + 0.5));
}

int wxWindowDCImpl::GetDepth() const
{
    return gdk_drawable_get_depth(m_gdkwindow);
}


//-----------------------------------------------------------------------------
// wxClientDCImpl
//-----------------------------------------------------------------------------

IMPLEMENT_ABSTRACT_CLASS(wxClientDCImpl, wxWindowDCImpl)

wxClientDCImpl::wxClientDCImpl( wxDC *owner )
          : wxWindowDCImpl( owner )
{
}

wxClientDCImpl::wxClientDCImpl( wxDC *owner, wxWindow *win )
          : wxWindowDCImpl( owner, win )
{
    wxCHECK_RET( win, wxT("NULL window in wxClientDCImpl::wxClientDC") );

#ifdef __WXUNIVERSAL__
    wxPoint ptOrigin = win->GetClientAreaOrigin();
    SetDeviceOrigin(ptOrigin.x, ptOrigin.y);
    wxSize size = win->GetClientSize();
    DoSetClippingRegion(0, 0, size.x, size.y);
#endif
    // __WXUNIVERSAL__
}

void wxClientDCImpl::DoGetSize(int *width, int *height) const
{
    wxCHECK_RET( m_window, wxT("GetSize() doesn't work without window") );

    m_window->GetClientSize( width, height );
}

//-----------------------------------------------------------------------------
// wxPaintDCImpl
//-----------------------------------------------------------------------------

IMPLEMENT_ABSTRACT_CLASS(wxPaintDCImpl, wxClientDCImpl)

// Limit the paint region to the window size. Sometimes
// the paint region is too big, and this risks X11 errors
static void wxLimitRegionToSize(wxRegion& region, const wxSize& sz)
{
    wxRect originalRect = region.GetBox();
    wxRect rect(originalRect);
    if (rect.width + rect.x > sz.x)
        rect.width = sz.x - rect.x;
    if (rect.height + rect.y > sz.y)
        rect.height = sz.y - rect.y;
    if (rect != originalRect)
    {
        region = wxRegion(rect);
        wxLogTrace(wxT("painting"), wxT("Limiting region from %d, %d, %d, %d to %d, %d, %d, %d\n"),
                   originalRect.x, originalRect.y, originalRect.width, originalRect.height,
                   rect.x, rect.y, rect.width, rect.height);
    }
}

wxPaintDCImpl::wxPaintDCImpl( wxDC *owner )
         : wxClientDCImpl( owner )
{
}

wxPaintDCImpl::wxPaintDCImpl( wxDC *owner, wxWindow *win )
         : wxClientDCImpl( owner, win )
{
#if USE_PAINT_REGION
    if (!win->m_clipPaintRegion)
        return;

    wxSize sz = win->GetSize();
    m_paintClippingRegion = win->m_nativeUpdateRegion;
    wxLimitRegionToSize(m_paintClippingRegion, sz);

    GdkRegion *region = m_paintClippingRegion.GetRegion();
    if ( region )
    {
        m_currentClippingRegion.Union( m_paintClippingRegion );
        wxLimitRegionToSize(m_currentClippingRegion, sz);

        if (sz.x <= 0 || sz.y <= 0)
            return ;

        gdk_gc_set_clip_region( m_penGC, region );
        gdk_gc_set_clip_region( m_brushGC, region );
        gdk_gc_set_clip_region( m_textGC, region );
        gdk_gc_set_clip_region( m_bgGC, region );
    }
#endif
}

// ----------------------------------------------------------------------------
// wxDCModule
// ----------------------------------------------------------------------------

class wxDCModule : public wxModule
{
public:
    bool OnInit();
    void OnExit();

private:
    DECLARE_DYNAMIC_CLASS(wxDCModule)
};

IMPLEMENT_DYNAMIC_CLASS(wxDCModule, wxModule)

bool wxDCModule::OnInit()
{
    wxInitGCPool();
    return true;
}

void wxDCModule::OnExit()
{
    wxCleanUpGCPool();

    for (int i = wxBRUSHSTYLE_LAST_HATCH - wxBRUSHSTYLE_FIRST_HATCH; i--; )
    {
        if (hatches[i])
            g_object_unref(hatches[i]);
    }
}
