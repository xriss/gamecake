/////////////////////////////////////////////////////////////////////////////
// Name:        src/gtk/dataview.cpp
// Purpose:     wxDataViewCtrl GTK+2 implementation
// Author:      Robert Roebling
// Id:          $Id$
// Copyright:   (c) 1998 Robert Roebling
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

// For compilers that support precompilation, includes "wx.h".
#include "wx/wxprec.h"

#if wxUSE_DATAVIEWCTRL

#include "wx/dataview.h"

#ifndef wxUSE_GENERICDATAVIEWCTRL

#ifndef WX_PRECOMP
    #include "wx/log.h"
    #include "wx/dcclient.h"
    #include "wx/sizer.h"
    #include "wx/settings.h"
    #include "wx/crt.h"
#endif

#include "wx/stockitem.h"
#include "wx/calctrl.h"
#include "wx/popupwin.h"
#include "wx/listimpl.cpp"

#include "wx/gtk/private.h"
#include "wx/gtk/dc.h"
#include "wx/gtk/dcclient.h"

#include "wx/gtk/private/gdkconv.h"
using namespace wxGTKImpl;

class wxGtkDataViewModelNotifier;

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

static wxDataViewCtrlInternal *gs_internal = NULL;

class wxGtkTreeModelNode;

extern "C" {
typedef struct _GtkWxTreeModel       GtkWxTreeModel;
}

// ----------------------------------------------------------------------------
// wxGtkTreePath: self-destroying GtkTreePath
// ----------------------------------------------------------------------------

// Usually this object is initialized with the associated GtkTreePath
// immediately when it's constructed but it can also be changed later either by
// using Assign() or by getting the pointer to the internally stored pointer
// value using ByRef(). The latter should be avoided but is very convenient
// when using GTK functions with GtkTreePath output parameters.
class wxGtkTreePath
{
public:
    // Ctor takes ownership of the given path and will free it if non-NULL.
    wxGtkTreePath(GtkTreePath *path = NULL) : m_path(path) { }

    // Creates a tree path for the given string path.
    wxGtkTreePath(const gchar *strpath)
        : m_path(gtk_tree_path_new_from_string(strpath))
    {
    }

    // Set the stored pointer if not done by ctor.
    void Assign(GtkTreePath *path)
    {
        wxASSERT_MSG( !m_path, "shouldn't be already initialized" );

        m_path = path;
    }

    // Return the pointer to the internally stored pointer. This should only be
    // used to initialize the object by passing it to some GTK function.
    GtkTreePath **ByRef()
    {
        wxASSERT_MSG( !m_path, "shouldn't be already initialized" );

        return &m_path;
    }


    operator GtkTreePath *() const { return m_path; }

    ~wxGtkTreePath() { if ( m_path ) gtk_tree_path_free(m_path); }

private:
    GtkTreePath *m_path;

    wxDECLARE_NO_COPY_CLASS(wxGtkTreePath);
};

// ----------------------------------------------------------------------------
// wxGtkTreeSelectionLock: prevent selection from changing during the
//                                 lifetime of this object
// ----------------------------------------------------------------------------

// Implementation note: it could be expected that setting the selection
// function in this class ctor and resetting it back to the old value in its
// dtor would work. However currently gtk_tree_selection_get_select_function()
// can't be passed NULL (see https://bugzilla.gnome.org/show_bug.cgi?id=626276)
// so we can't do this. Instead, we always use the selection function (which
// imposes extra overhead, albeit minimal one, on all selection operations) and
// just set/reset the flag telling it whether it should allow or forbid the
// selection.
//
// Also notice that currently only a single object of this class may exist at
// any given moment. It's just simpler like this and we don't need anything
// more for now.

extern "C"
gboolean wxdataview_selection_func(GtkTreeSelection * WXUNUSED(selection),
                                   GtkTreeModel * WXUNUSED(model),
                                   GtkTreePath * WXUNUSED(path),
                                   gboolean WXUNUSED(path_currently_selected),
                                   gpointer data)
{
    return data == NULL;
}

class wxGtkTreeSelectionLock
{
public:
    wxGtkTreeSelectionLock(GtkTreeSelection *selection)
        : m_selection(selection)
    {
        wxASSERT_MSG( !ms_instance, "this class is not reentrant currently" );

        ms_instance = this;

        CheckCurrentSelectionFunc(NULL);

        // Pass some non-NULL pointer as "data" for the callback, it doesn't
        // matter what it is as long as it's non-NULL.
        gtk_tree_selection_set_select_function(selection,
                                               wxdataview_selection_func,
                                               this,
                                               NULL);
    }

    ~wxGtkTreeSelectionLock()
    {
        CheckCurrentSelectionFunc(wxdataview_selection_func);

        gtk_tree_selection_set_select_function(m_selection,
                                               wxdataview_selection_func,
                                               NULL,
                                               NULL);

        ms_instance = NULL;
    }

private:
    void CheckCurrentSelectionFunc(GtkTreeSelectionFunc func)
    {
        // We can only use gtk_tree_selection_get_select_function() with 2.14+
        // so check for its availability both during compile- and run-time.
#if GTK_CHECK_VERSION(2, 14, 0)
        if ( gtk_check_version(2, 14, 0) != NULL )
            return;

        // If this assert is triggered, it means the code elsewhere has called
        // gtk_tree_selection_set_select_function() but currently doing this
        // breaks this class so the code here needs to be changed.
        wxASSERT_MSG
        (
            gtk_tree_selection_get_select_function(m_selection) == func,
            "selection function has changed unexpectedly, review this code!"
        );
#endif // GTK+ 2.14+

        wxUnusedVar(func);
    }

    static wxGtkTreeSelectionLock *ms_instance;

    GtkTreeSelection * const m_selection;

    wxDECLARE_NO_COPY_CLASS(wxGtkTreeSelectionLock);
};

wxGtkTreeSelectionLock *wxGtkTreeSelectionLock::ms_instance = NULL;

//-----------------------------------------------------------------------------
// wxDataViewCtrlInternal
//-----------------------------------------------------------------------------

WX_DECLARE_LIST(wxDataViewItem, ItemList);
WX_DEFINE_LIST(ItemList)

class wxDataViewCtrlInternal
{
public:
    wxDataViewCtrlInternal( wxDataViewCtrl *owner, wxDataViewModel *wx_model );
    ~wxDataViewCtrlInternal();

    // model iface
    GtkTreeModelFlags get_flags();
    gboolean get_iter( GtkTreeIter *iter, GtkTreePath *path );
    GtkTreePath *get_path( GtkTreeIter *iter);
    gboolean iter_next( GtkTreeIter *iter );
    gboolean iter_children( GtkTreeIter *iter, GtkTreeIter *parent);
    gboolean iter_has_child( GtkTreeIter *iter );
    gint iter_n_children( GtkTreeIter *iter );
    gboolean iter_nth_child( GtkTreeIter *iter, GtkTreeIter *parent, gint n );
    gboolean iter_parent( GtkTreeIter *iter, GtkTreeIter *child );

    // dnd iface

    bool EnableDragSource( const wxDataFormat &format );
    bool EnableDropTarget( const wxDataFormat &format );

    gboolean row_draggable( GtkTreeDragSource *drag_source, GtkTreePath *path );
    gboolean drag_data_delete( GtkTreeDragSource *drag_source, GtkTreePath* path );
    gboolean drag_data_get( GtkTreeDragSource *drag_source, GtkTreePath *path,
        GtkSelectionData *selection_data );
    gboolean drag_data_received( GtkTreeDragDest *drag_dest, GtkTreePath *dest,
        GtkSelectionData *selection_data );
    gboolean row_drop_possible( GtkTreeDragDest *drag_dest, GtkTreePath *dest_path,
        GtkSelectionData *selection_data );

    // notifactions from wxDataViewModel
    bool ItemAdded( const wxDataViewItem &parent, const wxDataViewItem &item );
    bool ItemDeleted( const wxDataViewItem &parent, const wxDataViewItem &item );
    bool ItemChanged( const wxDataViewItem &item );
    bool ValueChanged( const wxDataViewItem &item, unsigned int model_column );
    bool Cleared();
    bool BeforeReset();
    bool AfterReset();
    void Resort();

    // sorting interface
    void SetSortOrder( GtkSortType sort_order ) { m_sort_order = sort_order; }
    GtkSortType GetSortOrder() const            { return m_sort_order; }

    void SetSortColumn( int column )            { m_sort_column = column; }
    int GetSortColumn() const                   { return m_sort_column; }

    void SetDataViewSortColumn( wxDataViewColumn *column ) { m_dataview_sort_column = column; }
    wxDataViewColumn *GetDataViewSortColumn()   { return m_dataview_sort_column; }

    bool IsSorted()                             { return (m_sort_column >= 0); }

    // accessors
    wxDataViewModel* GetDataViewModel() { return m_wx_model; }
    const wxDataViewModel* GetDataViewModel() const { return m_wx_model; }
    wxDataViewCtrl* GetOwner()          { return m_owner; }
    GtkWxTreeModel* GetGtkModel()       { return m_gtk_model; }

    // item can be deleted already in the model
    int GetIndexOf( const wxDataViewItem &parent, const wxDataViewItem &item );

    virtual void OnInternalIdle();

protected:
    void InitTree();
    void ScheduleRefresh();

    wxGtkTreeModelNode *FindNode( const wxDataViewItem &item );
    wxGtkTreeModelNode *FindNode( GtkTreeIter *iter );
    wxGtkTreeModelNode *FindParentNode( const wxDataViewItem &item );
    wxGtkTreeModelNode *FindParentNode( GtkTreeIter *iter );
    void BuildBranch( wxGtkTreeModelNode *branch );

private:
    wxGtkTreeModelNode   *m_root;
    wxDataViewModel      *m_wx_model;
    GtkWxTreeModel       *m_gtk_model;
    wxDataViewCtrl       *m_owner;
    GtkSortType           m_sort_order;
    wxDataViewColumn     *m_dataview_sort_column;
    int                   m_sort_column;

    GtkTargetEntry        m_dragSourceTargetEntry;
    wxCharBuffer          m_dragSourceTargetEntryTarget;
    wxDataObject         *m_dragDataObject;

    GtkTargetEntry        m_dropTargetTargetEntry;
    wxCharBuffer          m_dropTargetTargetEntryTarget;
    wxDataObject         *m_dropDataObject;

    wxGtkDataViewModelNotifier *m_notifier;

    bool                  m_dirty;
};


//-----------------------------------------------------------------------------
// wxGtkTreeModelNode
//-----------------------------------------------------------------------------

static
int LINKAGEMODE wxGtkTreeModelChildCmp( void** id1, void** id2 )
{
    int ret = gs_internal->GetDataViewModel()->Compare( *id1, *id2,
        gs_internal->GetSortColumn(), (gs_internal->GetSortOrder() == GTK_SORT_ASCENDING) );

    return ret;
}

WX_DEFINE_ARRAY_PTR( wxGtkTreeModelNode*, wxGtkTreeModelNodes );
WX_DEFINE_ARRAY_PTR( void*, wxGtkTreeModelChildren );

class wxGtkTreeModelNode
{
public:
    wxGtkTreeModelNode( wxGtkTreeModelNode* parent, const wxDataViewItem &item,
                        wxDataViewCtrlInternal *internal )
    {
        m_parent = parent;
        m_item = item;
        m_internal = internal;
    }

    ~wxGtkTreeModelNode()
    {
        size_t count = m_nodes.GetCount();
        size_t i;
        for (i = 0; i < count; i++)
        {
            wxGtkTreeModelNode *child = m_nodes.Item( i );
            delete child;
        }
    }

    unsigned int AddNode( wxGtkTreeModelNode* child )
        {
            m_nodes.Add( child );

            void *id = child->GetItem().GetID();

            m_children.Add( id );

            if (m_internal->IsSorted() || m_internal->GetDataViewModel()->HasDefaultCompare())
            {
                gs_internal = m_internal;
                m_children.Sort( &wxGtkTreeModelChildCmp );
                return m_children.Index( id );
            }

            return m_children.GetCount()-1;
        }

    unsigned int AddLeave( void* id )
        {
            m_children.Add( id );

            if (m_internal->IsSorted() || m_internal->GetDataViewModel()->HasDefaultCompare())
            {
                gs_internal = m_internal;
                m_children.Sort( &wxGtkTreeModelChildCmp );
                return m_children.Index( id );
            }

            return m_children.GetCount()-1;
        }

    void DeleteChild( void* id )
        {
            m_children.Remove( id );

            unsigned int count = m_nodes.GetCount();
            unsigned int pos;
            for (pos = 0; pos < count; pos++)
            {
                wxGtkTreeModelNode *node = m_nodes.Item( pos );
                if (node->GetItem().GetID() == id)
                {
                    m_nodes.RemoveAt( pos );
                    delete node;
                    break;
                }
            }
        }

    wxGtkTreeModelNode* GetParent()
        { return m_parent; }
    wxGtkTreeModelNodes &GetNodes()
        { return m_nodes; }
    wxGtkTreeModelChildren &GetChildren()
        { return m_children; }

    unsigned int GetChildCount() const { return m_children.GetCount(); }
    unsigned int GetNodesCount() const { return m_nodes.GetCount(); }

    wxDataViewItem &GetItem() { return m_item; }
    wxDataViewCtrlInternal *GetInternal() { return m_internal; }

    void Resort();

private:
    wxGtkTreeModelNode         *m_parent;
    wxGtkTreeModelNodes         m_nodes;
    wxGtkTreeModelChildren      m_children;
    wxDataViewItem              m_item;
    wxDataViewCtrlInternal     *m_internal;
};


//-----------------------------------------------------------------------------
// data
//-----------------------------------------------------------------------------

extern bool   g_blockEventsOnDrag;

//-----------------------------------------------------------------------------
// define new GTK+ class wxGtkTreeModel
//-----------------------------------------------------------------------------

extern "C" {

#define GTK_TYPE_WX_TREE_MODEL               (gtk_wx_tree_model_get_type ())
#define GTK_WX_TREE_MODEL(obj)               (G_TYPE_CHECK_INSTANCE_CAST ((obj), GTK_TYPE_WX_TREE_MODEL, GtkWxTreeModel))
#define GTK_WX_TREE_MODEL_CLASS(klass)       (G_TYPE_CHECK_CLASS_CAST ((klass), GTK_TYPE_WX_TREE_MODEL, GtkWxTreeModelClass))
#define GTK_IS_WX_TREE_MODEL(obj)            (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GTK_TYPE_WX_TREE_MODEL))
#define GTK_IS_WX_TREE_MODEL_CLASS(klass)    (G_TYPE_CHECK_CLASS_TYPE ((klass), GTK_TYPE_WX_TREE_MODEL))
#define GTK_WX_TREE_MODEL_GET_CLASS(obj)     (G_TYPE_INSTANCE_GET_CLASS ((obj), GTK_TYPE_WX_TREE_MODEL, GtkWxTreeModelClass))

GType         gtk_wx_tree_model_get_type         (void);

typedef struct _GtkWxTreeModelClass  GtkWxTreeModelClass;

struct _GtkWxTreeModel
{
  GObject parent;

  /*< private >*/
  gint stamp;
  wxDataViewCtrlInternal *internal;
};

struct _GtkWxTreeModelClass
{
  GObjectClass list_parent_class;
};

static GtkWxTreeModel *wxgtk_tree_model_new          (void);
static void         wxgtk_tree_model_init            (GtkWxTreeModel       *tree_model);
static void         wxgtk_tree_model_class_init      (GtkWxTreeModelClass  *klass);

static void         wxgtk_tree_model_tree_model_init (GtkTreeModelIface       *iface);
static void         wxgtk_tree_model_sortable_init   (GtkTreeSortableIface    *iface);
static void         wxgtk_tree_model_drag_source_init(GtkTreeDragSourceIface  *iface);
static void         wxgtk_tree_model_drag_dest_init  (GtkTreeDragDestIface    *iface);

static void         wxgtk_tree_model_finalize        (GObject           *object);
static GtkTreeModelFlags wxgtk_tree_model_get_flags  (GtkTreeModel      *tree_model);
static gint         wxgtk_tree_model_get_n_columns   (GtkTreeModel      *tree_model);
static GType        wxgtk_tree_model_get_column_type (GtkTreeModel      *tree_model,
                                                      gint               index);
static gboolean     wxgtk_tree_model_get_iter        (GtkTreeModel      *tree_model,
                                                      GtkTreeIter       *iter,
                                                      GtkTreePath       *path);
static GtkTreePath *wxgtk_tree_model_get_path        (GtkTreeModel      *tree_model,
                                                      GtkTreeIter       *iter);
static void         wxgtk_tree_model_get_value       (GtkTreeModel      *tree_model,
                                                      GtkTreeIter       *iter,
                                                      gint               column,
                                                      GValue            *value);
static gboolean     wxgtk_tree_model_iter_next       (GtkTreeModel      *tree_model,
                                                      GtkTreeIter       *iter);
static gboolean     wxgtk_tree_model_iter_children   (GtkTreeModel      *tree_model,
                                                      GtkTreeIter       *iter,
                                                      GtkTreeIter       *parent);
static gboolean     wxgtk_tree_model_iter_has_child  (GtkTreeModel      *tree_model,
                                                      GtkTreeIter       *iter);
static gint         wxgtk_tree_model_iter_n_children (GtkTreeModel      *tree_model,
                                                      GtkTreeIter       *iter);
static gboolean     wxgtk_tree_model_iter_nth_child  (GtkTreeModel      *tree_model,
                                                      GtkTreeIter       *iter,
                                                      GtkTreeIter       *parent,
                                                      gint               n);
static gboolean     wxgtk_tree_model_iter_parent     (GtkTreeModel      *tree_model,
                                                      GtkTreeIter       *iter,
                                                      GtkTreeIter       *child);

/* sortable */
static gboolean wxgtk_tree_model_get_sort_column_id    (GtkTreeSortable       *sortable,
                                                        gint                  *sort_column_id,
                                                        GtkSortType           *order);
static void     wxgtk_tree_model_set_sort_column_id    (GtkTreeSortable       *sortable,
                                                        gint                   sort_column_id,
                                                        GtkSortType            order);
static void     wxgtk_tree_model_set_sort_func         (GtkTreeSortable       *sortable,
                                                        gint                   sort_column_id,
                                                        GtkTreeIterCompareFunc func,
                                                        gpointer               data,
                                                        GDestroyNotify       destroy);
static void     wxgtk_tree_model_set_default_sort_func (GtkTreeSortable       *sortable,
                                                        GtkTreeIterCompareFunc func,
                                                        gpointer               data,
                                                        GDestroyNotify       destroy);
static gboolean wxgtk_tree_model_has_default_sort_func (GtkTreeSortable       *sortable);

/* drag'n'drop */
static gboolean wxgtk_tree_model_row_draggable         (GtkTreeDragSource     *drag_source,
                                                        GtkTreePath           *path);
static gboolean wxgtk_tree_model_drag_data_delete      (GtkTreeDragSource     *drag_source,
                                                        GtkTreePath           *path);
static gboolean wxgtk_tree_model_drag_data_get         (GtkTreeDragSource     *drag_source,
                                                        GtkTreePath           *path,
                                                        GtkSelectionData      *selection_data);
static gboolean wxgtk_tree_model_drag_data_received    (GtkTreeDragDest       *drag_dest,
                                                        GtkTreePath           *dest,
                                                        GtkSelectionData      *selection_data);
static gboolean wxgtk_tree_model_row_drop_possible     (GtkTreeDragDest       *drag_dest,
                                                        GtkTreePath           *dest_path,
                                                        GtkSelectionData      *selection_data);


static GObjectClass *list_parent_class = NULL;

GType
gtk_wx_tree_model_get_type (void)
{
    static GType tree_model_type = 0;

    if (!tree_model_type)
    {
        const GTypeInfo tree_model_info =
        {
            sizeof (GtkWxTreeModelClass),
            NULL,   /* base_init */
            NULL,   /* base_finalize */
            (GClassInitFunc) wxgtk_tree_model_class_init,
            NULL,   /* class_finalize */
            NULL,   /* class_data */
            sizeof (GtkWxTreeModel),
            0,
            (GInstanceInitFunc) wxgtk_tree_model_init,
        };

        static const GInterfaceInfo tree_model_iface_info =
        {
            (GInterfaceInitFunc) wxgtk_tree_model_tree_model_init,
            NULL,
            NULL
        };

        static const GInterfaceInfo sortable_iface_info =
        {
            (GInterfaceInitFunc) wxgtk_tree_model_sortable_init,
            NULL,
            NULL
        };

        static const GInterfaceInfo drag_source_iface_info =
        {
            (GInterfaceInitFunc) wxgtk_tree_model_drag_source_init,
            NULL,
            NULL
        };

        static const GInterfaceInfo drag_dest_iface_info =
        {
            (GInterfaceInitFunc) wxgtk_tree_model_drag_dest_init,
            NULL,
            NULL
        };

        tree_model_type = g_type_register_static (G_TYPE_OBJECT, "GtkWxTreeModel",
                                                &tree_model_info, (GTypeFlags)0 );

        g_type_add_interface_static (tree_model_type,
                                     GTK_TYPE_TREE_MODEL,
                                     &tree_model_iface_info);
        g_type_add_interface_static (tree_model_type,
                                     GTK_TYPE_TREE_SORTABLE,
                                     &sortable_iface_info);
        g_type_add_interface_static (tree_model_type,
                                     GTK_TYPE_TREE_DRAG_DEST,
                                     &drag_dest_iface_info);
        g_type_add_interface_static (tree_model_type,
                                     GTK_TYPE_TREE_DRAG_SOURCE,
                                     &drag_source_iface_info);
    }

    return tree_model_type;
}

static GtkWxTreeModel *
wxgtk_tree_model_new(void)
{
    GtkWxTreeModel *retval = (GtkWxTreeModel *) g_object_new (GTK_TYPE_WX_TREE_MODEL, NULL);
    return retval;
}

static void
wxgtk_tree_model_class_init (GtkWxTreeModelClass *klass)
{
    list_parent_class = (GObjectClass*) g_type_class_peek_parent (klass);
    GObjectClass *object_class = (GObjectClass*) klass;
    object_class->finalize = wxgtk_tree_model_finalize;
}

static void
wxgtk_tree_model_tree_model_init (GtkTreeModelIface *iface)
{
    iface->get_flags = wxgtk_tree_model_get_flags;
    iface->get_n_columns = wxgtk_tree_model_get_n_columns;
    iface->get_column_type = wxgtk_tree_model_get_column_type;
    iface->get_iter = wxgtk_tree_model_get_iter;
    iface->get_path = wxgtk_tree_model_get_path;
    iface->get_value = wxgtk_tree_model_get_value;
    iface->iter_next = wxgtk_tree_model_iter_next;
    iface->iter_children = wxgtk_tree_model_iter_children;
    iface->iter_has_child = wxgtk_tree_model_iter_has_child;
    iface->iter_n_children = wxgtk_tree_model_iter_n_children;
    iface->iter_nth_child = wxgtk_tree_model_iter_nth_child;
    iface->iter_parent = wxgtk_tree_model_iter_parent;
}

static void
wxgtk_tree_model_sortable_init (GtkTreeSortableIface *iface)
{
    iface->get_sort_column_id = wxgtk_tree_model_get_sort_column_id;
    iface->set_sort_column_id = wxgtk_tree_model_set_sort_column_id;
    iface->set_sort_func = wxgtk_tree_model_set_sort_func;
    iface->set_default_sort_func = wxgtk_tree_model_set_default_sort_func;
    iface->has_default_sort_func = wxgtk_tree_model_has_default_sort_func;
}

static void
wxgtk_tree_model_drag_source_init(GtkTreeDragSourceIface  *iface)
{
    iface->row_draggable = wxgtk_tree_model_row_draggable;
    iface->drag_data_delete = wxgtk_tree_model_drag_data_delete;
    iface->drag_data_get = wxgtk_tree_model_drag_data_get;
}

static void
wxgtk_tree_model_drag_dest_init  (GtkTreeDragDestIface    *iface)
{
    iface->drag_data_received = wxgtk_tree_model_drag_data_received;
    iface->row_drop_possible = wxgtk_tree_model_row_drop_possible;
}

static void
wxgtk_tree_model_init (GtkWxTreeModel *tree_model)
{
    tree_model->internal = NULL;
    tree_model->stamp = g_random_int();
}

static void
wxgtk_tree_model_finalize (GObject *object)
{
    /* must chain up */
    (* list_parent_class->finalize) (object);
}

} // extern "C"

//-----------------------------------------------------------------------------
// implement callbacks from wxGtkTreeModel class by letting
// them call the methods of wxWidgets' wxDataViewModel
//-----------------------------------------------------------------------------

static GtkTreeModelFlags
wxgtk_tree_model_get_flags (GtkTreeModel *tree_model)
{
    GtkWxTreeModel *wxtree_model = (GtkWxTreeModel *) tree_model;
    g_return_val_if_fail (GTK_IS_WX_TREE_MODEL (wxtree_model), (GtkTreeModelFlags)0 );

    return wxtree_model->internal->get_flags();
}

static gint
wxgtk_tree_model_get_n_columns (GtkTreeModel *tree_model)
{
    GtkWxTreeModel *wxtree_model = (GtkWxTreeModel *) tree_model;
    g_return_val_if_fail (GTK_IS_WX_TREE_MODEL (wxtree_model), 0);

    return wxtree_model->internal->GetDataViewModel()->GetColumnCount();
}

static GType
wxgtk_tree_model_get_column_type (GtkTreeModel *tree_model,
                                  gint          index)
{
    GtkWxTreeModel *wxtree_model = (GtkWxTreeModel *) tree_model;
    g_return_val_if_fail (GTK_IS_WX_TREE_MODEL (wxtree_model), G_TYPE_INVALID);

    GType gtype = G_TYPE_INVALID;

    wxString wxtype = wxtree_model->internal->GetDataViewModel()->GetColumnType( (unsigned int) index );

    if (wxtype == wxT("string"))
        gtype = G_TYPE_STRING;
    else
    {
        gtype = G_TYPE_POINTER;
        // wxFAIL_MSG( wxT("non-string columns not supported for searching yet") );
    }

    return gtype;
}

static gboolean
wxgtk_tree_model_get_iter (GtkTreeModel *tree_model,
                           GtkTreeIter  *iter,
                           GtkTreePath  *path)
{
    GtkWxTreeModel *wxtree_model = (GtkWxTreeModel *) tree_model;
    g_return_val_if_fail (GTK_IS_WX_TREE_MODEL (wxtree_model), FALSE);
    g_return_val_if_fail (gtk_tree_path_get_depth (path) > 0, FALSE);

    return wxtree_model->internal->get_iter( iter, path );
}

static GtkTreePath *
wxgtk_tree_model_get_path (GtkTreeModel *tree_model,
                           GtkTreeIter  *iter)
{
    GtkWxTreeModel *wxtree_model = (GtkWxTreeModel *) tree_model;
    g_return_val_if_fail (GTK_IS_WX_TREE_MODEL (wxtree_model), NULL);
    g_return_val_if_fail (iter->stamp == GTK_WX_TREE_MODEL (wxtree_model)->stamp, NULL);

    return wxtree_model->internal->get_path( iter );
}

static void
wxgtk_tree_model_get_value (GtkTreeModel *tree_model,
                            GtkTreeIter  *iter,
                            gint          column,
                            GValue       *value)
{
    GtkWxTreeModel *wxtree_model = (GtkWxTreeModel *) tree_model;
    g_return_if_fail (GTK_IS_WX_TREE_MODEL (wxtree_model) );

    wxDataViewModel *model = wxtree_model->internal->GetDataViewModel();
    wxString mtype = model->GetColumnType( (unsigned int) column );
    if (mtype == wxT("string"))
    {
        wxVariant variant;
        g_value_init( value, G_TYPE_STRING );
        wxDataViewItem item( (void*) iter->user_data );
        model->GetValue( variant, item, (unsigned int) column );

        g_value_set_string( value, variant.GetString().utf8_str() );
    }
    else
    {
        wxFAIL_MSG( wxT("non-string columns not supported yet") );
    }
}

static gboolean
wxgtk_tree_model_iter_next (GtkTreeModel  *tree_model,
                            GtkTreeIter   *iter)
{
    GtkWxTreeModel *wxtree_model = (GtkWxTreeModel *) tree_model;

    // This happens when clearing the view by calling .._set_model( NULL );
    if (iter->stamp == 0) return FALSE;

    g_return_val_if_fail (GTK_IS_WX_TREE_MODEL (wxtree_model), FALSE);
    g_return_val_if_fail (wxtree_model->stamp == iter->stamp, FALSE);

    return wxtree_model->internal->iter_next( iter );
}

static gboolean
wxgtk_tree_model_iter_children (GtkTreeModel *tree_model,
                                GtkTreeIter  *iter,
                                GtkTreeIter  *parent)
{
    GtkWxTreeModel *wxtree_model = (GtkWxTreeModel *) tree_model;
    g_return_val_if_fail (GTK_IS_WX_TREE_MODEL (wxtree_model), FALSE);
    if (parent)
    {
        g_return_val_if_fail (wxtree_model->stamp == parent->stamp, FALSE);
    }

    return wxtree_model->internal->iter_children( iter, parent );
}

static gboolean
wxgtk_tree_model_iter_has_child (GtkTreeModel *tree_model,
                                 GtkTreeIter  *iter)
{
    GtkWxTreeModel *wxtree_model = (GtkWxTreeModel *) tree_model;
    g_return_val_if_fail (GTK_IS_WX_TREE_MODEL (wxtree_model), FALSE);
    g_return_val_if_fail (wxtree_model->stamp == iter->stamp, FALSE);

    return wxtree_model->internal->iter_has_child( iter );
}

static gint
wxgtk_tree_model_iter_n_children (GtkTreeModel *tree_model,
                                  GtkTreeIter  *iter)
{
    GtkWxTreeModel *wxtree_model = (GtkWxTreeModel *) tree_model;
    g_return_val_if_fail (GTK_IS_WX_TREE_MODEL (wxtree_model), 0);
    g_return_val_if_fail ( !iter || wxtree_model->stamp == iter->stamp, 0);

    return wxtree_model->internal->iter_n_children( iter );
}

static gboolean
wxgtk_tree_model_iter_nth_child (GtkTreeModel *tree_model,
                                 GtkTreeIter  *iter,
                                 GtkTreeIter  *parent,
                                 gint          n)
{
    GtkWxTreeModel *wxtree_model = (GtkWxTreeModel *) tree_model;
    g_return_val_if_fail (GTK_IS_WX_TREE_MODEL (wxtree_model), FALSE);

    return wxtree_model->internal->iter_nth_child( iter, parent, n );
}

static gboolean
wxgtk_tree_model_iter_parent (GtkTreeModel *tree_model,
                              GtkTreeIter  *iter,
                              GtkTreeIter  *child)
{
    GtkWxTreeModel *wxtree_model = (GtkWxTreeModel *) tree_model;
    g_return_val_if_fail (GTK_IS_WX_TREE_MODEL (wxtree_model), FALSE);
    g_return_val_if_fail (wxtree_model->stamp == child->stamp, FALSE);

    return wxtree_model->internal->iter_parent( iter, child );
}

/* drag'n'drop iface */
static gboolean
wxgtk_tree_model_row_draggable (GtkTreeDragSource *drag_source,
                                GtkTreePath       *path)
{
    GtkWxTreeModel *wxtree_model = (GtkWxTreeModel *) drag_source;
    g_return_val_if_fail (GTK_IS_WX_TREE_MODEL (wxtree_model), FALSE);

    return wxtree_model->internal->row_draggable( drag_source, path );
}

static gboolean
wxgtk_tree_model_drag_data_delete (GtkTreeDragSource *drag_source,
                                   GtkTreePath       *path)
{
    GtkWxTreeModel *wxtree_model = (GtkWxTreeModel *) drag_source;
    g_return_val_if_fail (GTK_IS_WX_TREE_MODEL (wxtree_model), FALSE);

    return wxtree_model->internal->drag_data_delete( drag_source, path );
}

static gboolean
wxgtk_tree_model_drag_data_get (GtkTreeDragSource *drag_source,
                                GtkTreePath       *path,
                                GtkSelectionData  *selection_data)
{
    GtkWxTreeModel *wxtree_model = (GtkWxTreeModel *) drag_source;
    g_return_val_if_fail (GTK_IS_WX_TREE_MODEL (wxtree_model), FALSE);

#if 0
    wxPrintf( "drag_get_data\n");

    wxGtkString atom_selection(gdk_atom_name(selection_data->selection));
    wxPrintf( "selection %s\n", wxString::FromAscii(atom_selection) );

    wxGtkString atom_target(gdk_atom_name(selection_data->target));
    wxPrintf( "target %s\n", wxString::FromAscii(atom_target) );

    wxGtkString atom_type(gdk_atom_name(selection_data->type));
    wxPrintf( "type %s\n", wxString::FromAscii(atom_type) );

    wxPrintf( "format %d\n", selection_data->format );
#endif

    return wxtree_model->internal->drag_data_get( drag_source, path, selection_data );
}

static gboolean
wxgtk_tree_model_drag_data_received (GtkTreeDragDest  *drag_dest,
                                     GtkTreePath      *dest,
                                     GtkSelectionData *selection_data)
{
    GtkWxTreeModel *wxtree_model = (GtkWxTreeModel *) drag_dest;
    g_return_val_if_fail (GTK_IS_WX_TREE_MODEL (wxtree_model), FALSE);

    return wxtree_model->internal->drag_data_received( drag_dest, dest, selection_data );
}

static gboolean
wxgtk_tree_model_row_drop_possible (GtkTreeDragDest  *drag_dest,
                                    GtkTreePath      *dest_path,
                                    GtkSelectionData *selection_data)
{
    GtkWxTreeModel *wxtree_model = (GtkWxTreeModel *) drag_dest;
    g_return_val_if_fail (GTK_IS_WX_TREE_MODEL (wxtree_model), FALSE);

    return wxtree_model->internal->row_drop_possible( drag_dest, dest_path, selection_data );
}

/* sortable iface */
static gboolean
wxgtk_tree_model_get_sort_column_id (GtkTreeSortable *sortable,
                                     gint            *sort_column_id,
                                     GtkSortType     *order)
{
    GtkWxTreeModel *wxtree_model = (GtkWxTreeModel *) sortable;

    g_return_val_if_fail (GTK_IS_WX_TREE_MODEL (sortable), FALSE);

    if (!wxtree_model->internal->IsSorted())
    {
        if (sort_column_id)
            *sort_column_id = -1;

        return TRUE;
    }


    if (sort_column_id)
        *sort_column_id = wxtree_model->internal->GetSortColumn();

    if (order)
        *order = wxtree_model->internal->GetSortOrder();

    return TRUE;
}

wxDataViewColumn *gs_lastLeftClickHeader = NULL;

static void
wxgtk_tree_model_set_sort_column_id (GtkTreeSortable *sortable,
                                     gint             sort_column_id,
                                     GtkSortType      order)
{
    GtkWxTreeModel *tree_model = (GtkWxTreeModel *) sortable;
    g_return_if_fail (GTK_IS_WX_TREE_MODEL (sortable) );

    tree_model->internal->SetDataViewSortColumn( gs_lastLeftClickHeader );

    if ((sort_column_id != (gint) tree_model->internal->GetSortColumn()) ||
        (order != tree_model->internal->GetSortOrder()))
    {
        tree_model->internal->SetSortColumn( sort_column_id );
        tree_model->internal->SetSortOrder( order );

        gtk_tree_sortable_sort_column_changed (sortable);

        tree_model->internal->GetDataViewModel()->Resort();
    }

    if (gs_lastLeftClickHeader)
    {
        wxDataViewCtrl *dv = tree_model->internal->GetOwner();
        wxDataViewEvent event( wxEVT_COMMAND_DATAVIEW_COLUMN_SORTED, dv->GetId() );
        event.SetDataViewColumn( gs_lastLeftClickHeader );
        event.SetModel( dv->GetModel() );
        dv->HandleWindowEvent( event );
    }

    gs_lastLeftClickHeader = NULL;
}

static void
wxgtk_tree_model_set_sort_func (GtkTreeSortable        *sortable,
                                gint                    WXUNUSED(sort_column_id),
                                GtkTreeIterCompareFunc  func,
                                gpointer                WXUNUSED(data),
                                GDestroyNotify        WXUNUSED(destroy))
{
    g_return_if_fail (GTK_IS_WX_TREE_MODEL (sortable) );
    g_return_if_fail (func != NULL);
}

static void
wxgtk_tree_model_set_default_sort_func (GtkTreeSortable          *sortable,
                                        GtkTreeIterCompareFunc    func,
                                        gpointer                  WXUNUSED(data),
                                        GDestroyNotify          WXUNUSED(destroy))
{
    g_return_if_fail (GTK_IS_WX_TREE_MODEL (sortable) );
    g_return_if_fail (func != NULL);

    //wxPrintf( "wxgtk_tree_model_set_default_sort_func\n" );
    // TODO: remove this code
}

static gboolean
wxgtk_tree_model_has_default_sort_func (GtkTreeSortable        *sortable)
{
    g_return_val_if_fail (GTK_IS_WX_TREE_MODEL (sortable), FALSE );

    return FALSE;
}

//-----------------------------------------------------------------------------
// define new GTK+ class GtkWxRendererText
//-----------------------------------------------------------------------------

extern "C" {

#define GTK_TYPE_WX_CELL_RENDERER_TEXT               (gtk_wx_cell_renderer_text_get_type ())
#define GTK_WX_CELL_RENDERER_TEXT(obj)               (G_TYPE_CHECK_INSTANCE_CAST ((obj), GTK_TYPE_WX_CELL_RENDERER_TEXT, GtkWxCellRendererText))
#define GTK_WX_CELL_RENDERER_TEXT_CLASS(klass)       (G_TYPE_CHECK_CLASS_CAST ((klass), GTK_TYPE_WX_CELL_RENDERER_TEXT, GtkWxCellRendererTextClass))
#define GTK_IS_WX_CELL_RENDERER_TEXT(obj)            (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GTK_TYPE_WX_CELL_RENDERER_TEXT))
#define GTK_IS_WX_CELL_RENDERER_TEXT_CLASS(klass)    (G_TYPE_CHECK_CLASS_TYPE ((klass), GTK_TYPE_WX_CELL_RENDERER_TEXT))
#define GTK_WX_CELL_RENDERER_TEXT_GET_CLASS(obj)     (G_TYPE_INSTANCE_GET_CLASS ((obj), GTK_TYPE_WX_CELL_RENDERER_TEXT, GtkWxCellRendererTextClass))

GType            gtk_wx_cell_renderer_text_get_type (void);

typedef struct _GtkWxCellRendererText GtkWxCellRendererText;
typedef struct _GtkWxCellRendererTextClass GtkWxCellRendererTextClass;

struct _GtkWxCellRendererText
{
  GtkCellRendererText parent;

  wxDataViewRenderer *wx_renderer;
};

struct _GtkWxCellRendererTextClass
{
  GtkCellRendererTextClass cell_parent_class;
};


static GtkWxCellRendererText *gtk_wx_cell_renderer_text_new   (void);
static void gtk_wx_cell_renderer_text_init (
                        GtkWxCellRendererText      *cell );
static void gtk_wx_cell_renderer_text_class_init(
                        GtkWxCellRendererTextClass *klass );
static void gtk_wx_cell_renderer_text_finalize (
                        GObject                *object );
static GtkCellEditable *gtk_wx_cell_renderer_text_start_editing(
                        GtkCellRenderer         *cell,
                        GdkEvent                *event,
                        GtkWidget               *widget,
                        const gchar             *path,
                        GdkRectangle            *background_area,
                        GdkRectangle            *cell_area,
                        GtkCellRendererState     flags );


static GObjectClass *text_cell_parent_class = NULL;

}  // extern "C"

GType
gtk_wx_cell_renderer_text_get_type (void)
{
    static GType cell_wx_type = 0;

    if (!cell_wx_type)
    {
        const GTypeInfo cell_wx_info =
        {
            sizeof (GtkWxCellRendererTextClass),
            NULL, /* base_init */
            NULL, /* base_finalize */
            (GClassInitFunc) gtk_wx_cell_renderer_text_class_init,
            NULL, /* class_finalize */
            NULL, /* class_data */
            sizeof (GtkWxCellRendererText),
            0,          /* n_preallocs */
            (GInstanceInitFunc) gtk_wx_cell_renderer_text_init,
        };

        cell_wx_type = g_type_register_static( GTK_TYPE_CELL_RENDERER_TEXT,
            "GtkWxCellRendererText", &cell_wx_info, (GTypeFlags)0 );
    }

    return cell_wx_type;
}

static void
gtk_wx_cell_renderer_text_init (GtkWxCellRendererText *cell)
{
    cell->wx_renderer = NULL;
}

static void
gtk_wx_cell_renderer_text_class_init (GtkWxCellRendererTextClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS (klass);
    GtkCellRendererClass *cell_class = GTK_CELL_RENDERER_CLASS (klass);

    text_cell_parent_class = (GObjectClass*) g_type_class_peek_parent (klass);

    object_class->finalize = gtk_wx_cell_renderer_text_finalize;

    cell_class->start_editing = gtk_wx_cell_renderer_text_start_editing;
}

static void
gtk_wx_cell_renderer_text_finalize (GObject *object)
{
    /* must chain up */
    (* G_OBJECT_CLASS (text_cell_parent_class)->finalize) (object);
}

GtkWxCellRendererText*
gtk_wx_cell_renderer_text_new (void)
{
    return (GtkWxCellRendererText*) g_object_new (GTK_TYPE_WX_CELL_RENDERER_TEXT, NULL);
}

static GtkCellEditable *gtk_wx_cell_renderer_text_start_editing(
                        GtkCellRenderer         *gtk_renderer,
                        GdkEvent                *gdk_event,
                        GtkWidget               *widget,
                        const gchar             *path,
                        GdkRectangle            *background_area,
                        GdkRectangle            *cell_area,
                        GtkCellRendererState     flags )
{
    GtkWxCellRendererText *wxgtk_renderer = (GtkWxCellRendererText *) gtk_renderer;
    wxDataViewRenderer *wx_renderer = wxgtk_renderer->wx_renderer;
    wxDataViewColumn *column = wx_renderer->GetOwner();

    wxDataViewItem
        item(column->GetOwner()->GTKPathToItem(wxGtkTreePath(path)));

    wxDataViewCtrl *dv = column->GetOwner();
    wxDataViewEvent event( wxEVT_COMMAND_DATAVIEW_ITEM_START_EDITING, dv->GetId() );
    event.SetDataViewColumn( column );
    event.SetModel( dv->GetModel() );
    event.SetColumn( column->GetModelColumn() );
    event.SetItem( item );
    dv->HandleWindowEvent( event );

    if (event.IsAllowed())
        return GTK_CELL_RENDERER_CLASS(text_cell_parent_class)->
           start_editing( gtk_renderer, gdk_event, widget, path, background_area, cell_area, flags );
    else
        return NULL;
}

//-----------------------------------------------------------------------------
// define new GTK+ class GtkWxCellRenderer
//-----------------------------------------------------------------------------

extern "C" {

#define GTK_TYPE_WX_CELL_RENDERER               (gtk_wx_cell_renderer_get_type ())
#define GTK_WX_CELL_RENDERER(obj)               (G_TYPE_CHECK_INSTANCE_CAST ((obj), GTK_TYPE_WX_CELL_RENDERER, GtkWxCellRenderer))
#define GTK_WX_CELL_RENDERER_CLASS(klass)       (G_TYPE_CHECK_CLASS_CAST ((klass), GTK_TYPE_WX_CELL_RENDERER, GtkWxCellRendererClass))
#define GTK_IS_WX_CELL_RENDERER(obj)            (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GTK_TYPE_WX_CELL_RENDERER))
#define GTK_IS_WX_CELL_RENDERER_CLASS(klass)    (G_TYPE_CHECK_CLASS_TYPE ((klass), GTK_TYPE_WX_CELL_RENDERER))
#define GTK_WX_CELL_RENDERER_GET_CLASS(obj)     (G_TYPE_INSTANCE_GET_CLASS ((obj), GTK_TYPE_WX_CELL_RENDERER, GtkWxCellRendererClass))

GType            gtk_wx_cell_renderer_get_type (void);

typedef struct _GtkWxCellRenderer GtkWxCellRenderer;
typedef struct _GtkWxCellRendererClass GtkWxCellRendererClass;

struct _GtkWxCellRenderer
{
  GtkCellRenderer parent;

  /*< private >*/
  wxDataViewCustomRenderer *cell;
  guint32 last_click;
};

struct _GtkWxCellRendererClass
{
  GtkCellRendererClass cell_parent_class;
};


static GtkCellRenderer *gtk_wx_cell_renderer_new   (void);
static void gtk_wx_cell_renderer_init (
                        GtkWxCellRenderer      *cell );
static void gtk_wx_cell_renderer_class_init(
                        GtkWxCellRendererClass *klass );
static void gtk_wx_cell_renderer_finalize (
                        GObject                *object );
static void gtk_wx_cell_renderer_get_size (
                        GtkCellRenderer         *cell,
                        GtkWidget               *widget,
                        GdkRectangle            *rectangle,
                        gint                    *x_offset,
                        gint                    *y_offset,
                        gint                    *width,
                        gint                    *height );
static void gtk_wx_cell_renderer_render (
                        GtkCellRenderer         *cell,
                        GdkWindow               *window,
                        GtkWidget               *widget,
                        GdkRectangle            *background_area,
                        GdkRectangle            *cell_area,
                        GdkRectangle            *expose_area,
                        GtkCellRendererState     flags );
static gboolean gtk_wx_cell_renderer_activate(
                        GtkCellRenderer         *cell,
                        GdkEvent                *event,
                        GtkWidget               *widget,
                        const gchar             *path,
                        GdkRectangle            *background_area,
                        GdkRectangle            *cell_area,
                        GtkCellRendererState     flags );
static GtkCellEditable *gtk_wx_cell_renderer_start_editing(
                        GtkCellRenderer         *cell,
                        GdkEvent                *event,
                        GtkWidget               *widget,
                        const gchar             *path,
                        GdkRectangle            *background_area,
                        GdkRectangle            *cell_area,
                        GtkCellRendererState     flags );


static GObjectClass *cell_parent_class = NULL;

}  // extern "C"

GType
gtk_wx_cell_renderer_get_type (void)
{
    static GType cell_wx_type = 0;

    if (!cell_wx_type)
    {
        const GTypeInfo cell_wx_info =
        {
            sizeof (GtkWxCellRendererClass),
            NULL, /* base_init */
            NULL, /* base_finalize */
            (GClassInitFunc) gtk_wx_cell_renderer_class_init,
            NULL, /* class_finalize */
            NULL, /* class_data */
            sizeof (GtkWxCellRenderer),
            0,          /* n_preallocs */
            (GInstanceInitFunc) gtk_wx_cell_renderer_init,
        };

        cell_wx_type = g_type_register_static( GTK_TYPE_CELL_RENDERER,
            "GtkWxCellRenderer", &cell_wx_info, (GTypeFlags)0 );
    }

    return cell_wx_type;
}

static void
gtk_wx_cell_renderer_init (GtkWxCellRenderer *cell)
{
    cell->cell = NULL;
    cell->last_click = 0;
}

static void
gtk_wx_cell_renderer_class_init (GtkWxCellRendererClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS (klass);
    GtkCellRendererClass *cell_class = GTK_CELL_RENDERER_CLASS (klass);

    cell_parent_class = (GObjectClass*) g_type_class_peek_parent (klass);

    object_class->finalize = gtk_wx_cell_renderer_finalize;

    cell_class->get_size = gtk_wx_cell_renderer_get_size;
    cell_class->render = gtk_wx_cell_renderer_render;
    cell_class->activate = gtk_wx_cell_renderer_activate;
    cell_class->start_editing = gtk_wx_cell_renderer_start_editing;
}

static void
gtk_wx_cell_renderer_finalize (GObject *object)
{
    /* must chain up */
    (* G_OBJECT_CLASS (cell_parent_class)->finalize) (object);
}

GtkCellRenderer*
gtk_wx_cell_renderer_new (void)
{
    return (GtkCellRenderer*) g_object_new (GTK_TYPE_WX_CELL_RENDERER, NULL);
}

static GtkCellEditable *gtk_wx_cell_renderer_start_editing(
                        GtkCellRenderer         *renderer,
                        GdkEvent                *WXUNUSED(event),
                        GtkWidget               *widget,
                        const gchar             *path,
                        GdkRectangle            *WXUNUSED(background_area),
                        GdkRectangle            *cell_area,
                        GtkCellRendererState     WXUNUSED(flags) )
{
    GtkWxCellRenderer *wxrenderer = (GtkWxCellRenderer *) renderer;
    wxDataViewCustomRenderer *cell = wxrenderer->cell;

    // Renderer doesn't support in-place editing
    if (!cell->HasEditorCtrl())
        return NULL;

    // An in-place editing control is still around
    if (cell->GetEditorCtrl())
        return NULL;

    GdkRectangle rect;
    gtk_wx_cell_renderer_get_size (renderer, widget, cell_area,
                                   &rect.x,
                                   &rect.y,
                                   &rect.width,
                                   &rect.height);

    rect.x += cell_area->x;
    rect.y += cell_area->y;
//    rect.width  -= renderer->xpad * 2;
//    rect.height -= renderer->ypad * 2;

//    wxRect renderrect(wxRectFromGDKRect(&rect));
    wxRect renderrect(wxRectFromGDKRect(cell_area));

    wxDataViewItem
        item(cell->GetOwner()->GetOwner()->GTKPathToItem(wxGtkTreePath(path)));

    cell->StartEditing( item, renderrect );

    return NULL;
}

static void
gtk_wx_cell_renderer_get_size (GtkCellRenderer *renderer,
                               GtkWidget       *WXUNUSED(widget),
                               GdkRectangle    *cell_area,
                               gint            *x_offset,
                               gint            *y_offset,
                               gint            *width,
                               gint            *height)
{
    GtkWxCellRenderer *wxrenderer = (GtkWxCellRenderer *) renderer;
    wxDataViewCustomRenderer *cell = wxrenderer->cell;

    wxSize size = cell->GetSize();

    wxDataViewCtrl * const ctrl = cell->GetOwner()->GetOwner();

    // Uniform row height, if specified, overrides the value returned by the
    // renderer.
    if ( !ctrl->HasFlag(wxDV_VARIABLE_LINE_HEIGHT) )
    {
        const int uniformHeight = ctrl->GTKGetUniformRowHeight();
        if ( uniformHeight > 0 )
            size.y = uniformHeight;
    }

    int xpad, ypad;
    gtk_cell_renderer_get_padding(renderer, &xpad, &ypad);
    int calc_width  = xpad * 2 + size.x;
    int calc_height = ypad * 2 + size.y;

    if (x_offset)
        *x_offset = 0;
    if (y_offset)
        *y_offset = 0;

    if (cell_area && size.x > 0 && size.y > 0)
    {
        float xalign, yalign;
        gtk_cell_renderer_get_alignment(renderer, &xalign, &yalign);
        if (x_offset)
        {
            *x_offset = int(xalign * (cell_area->width - calc_width - 2 * xpad));
            *x_offset = MAX(*x_offset, 0) + xpad;
        }
        if (y_offset)
        {
            *y_offset = int(yalign * (cell_area->height - calc_height - 2 * ypad));
            *y_offset = MAX(*y_offset, 0) + ypad;
        }
    }

    if (width)
        *width = calc_width;

    if (height)
        *height = calc_height;
}

static void
gtk_wx_cell_renderer_render (GtkCellRenderer      *renderer,
                             GdkWindow            *window,
                             GtkWidget            *widget,
                             GdkRectangle         *background_area,
                             GdkRectangle         *cell_area,
                             GdkRectangle         *expose_area,
                             GtkCellRendererState  flags)

{
    GtkWxCellRenderer *wxrenderer = (GtkWxCellRenderer *) renderer;
    wxDataViewCustomRenderer *cell = wxrenderer->cell;

    cell->GTKStashRenderParams(window, widget,
                               background_area, expose_area, flags);

    wxRect rect(wxRectFromGDKRect(cell_area));
    int xpad, ypad;
    gtk_cell_renderer_get_padding(renderer, &xpad, &ypad);
    rect = rect.Deflate(xpad, ypad);

    wxWindowDC* dc = (wxWindowDC*) cell->GetDC();
    wxWindowDCImpl *impl = (wxWindowDCImpl *) dc->GetImpl();

    // Reinitialize wxWindowDC's GDK window if drawing occurs into a different
    // window such as a DnD drop window.
    if (window != impl->m_gdkwindow)
    {
        impl->Destroy();
        impl->m_gdkwindow = window;
        impl->SetUpDC();
    }

    int state = 0;
    if (flags & GTK_CELL_RENDERER_SELECTED)
        state |= wxDATAVIEW_CELL_SELECTED;
    if (flags & GTK_CELL_RENDERER_PRELIT)
        state |= wxDATAVIEW_CELL_PRELIT;
    if (flags & GTK_CELL_RENDERER_INSENSITIVE)
        state |= wxDATAVIEW_CELL_INSENSITIVE;
    if (flags & GTK_CELL_RENDERER_INSENSITIVE)
        state |= wxDATAVIEW_CELL_INSENSITIVE;
    if (flags & GTK_CELL_RENDERER_FOCUSED)
        state |= wxDATAVIEW_CELL_FOCUSED;
    cell->WXCallRender( rect, dc, state );
}

static gboolean
gtk_wx_cell_renderer_activate(
                        GtkCellRenderer         *renderer,
                        GdkEvent                *event,
                        GtkWidget               *widget,
                        const gchar             *path,
                        GdkRectangle            *WXUNUSED(background_area),
                        GdkRectangle            *cell_area,
                        GtkCellRendererState     WXUNUSED(flags) )
{
    GtkWxCellRenderer *wxrenderer = (GtkWxCellRenderer *) renderer;
    wxDataViewCustomRenderer *cell = wxrenderer->cell;

    GdkRectangle rect;
    gtk_wx_cell_renderer_get_size (renderer, widget, cell_area,
                                   &rect.x,
                                   &rect.y,
                                   &rect.width,
                                   &rect.height);

    rect.x += cell_area->x;
    rect.y += cell_area->y;
    int xpad, ypad;
    gtk_cell_renderer_get_padding(renderer, &xpad, &ypad);
    rect.width  -= xpad * 2;
    rect.height -= ypad * 2;

    wxRect renderrect(wxRectFromGDKRect(&rect));

    wxDataViewCtrl * const ctrl = cell->GetOwner()->GetOwner();
    wxDataViewModel *model = ctrl->GetModel();

    wxDataViewItem item(ctrl->GTKPathToItem(wxGtkTreePath(path)));

    unsigned int model_col = cell->GetOwner()->GetModelColumn();

    if (!event)
    {
        bool ret = false;

        // activated by <ENTER>
        if (cell->Activate( renderrect, model, item, model_col ))
                    ret = true;

        return ret;
    }
    else if (event->type == GDK_BUTTON_PRESS)
    {
        GdkEventButton *button_event = (GdkEventButton*) event;
        wxPoint pt( ((int) button_event->x) - renderrect.x,
                    ((int) button_event->y) - renderrect.y );

        bool ret = false;
        if (button_event->button == 1)
        {
            if (cell->LeftClick( pt, renderrect, model, item, model_col ))
                ret = true;
            // TODO: query system double-click time
            if (button_event->time - wxrenderer->last_click < 400)
                if (cell->Activate( renderrect, model, item, model_col ))
                    ret = true;
        }
        wxrenderer->last_click = button_event->time;

        return ret;
    }

    return false;
}

// ---------------------------------------------------------
// wxGtkDataViewModelNotifier
// ---------------------------------------------------------

class wxGtkDataViewModelNotifier: public wxDataViewModelNotifier
{
public:
    wxGtkDataViewModelNotifier( wxDataViewModel *wx_model, wxDataViewCtrlInternal *internal );
    ~wxGtkDataViewModelNotifier();

    virtual bool ItemAdded( const wxDataViewItem &parent, const wxDataViewItem &item );
    virtual bool ItemDeleted( const wxDataViewItem &parent, const wxDataViewItem &item );
    virtual bool ItemChanged( const wxDataViewItem &item );
    virtual bool ValueChanged( const wxDataViewItem &item, unsigned int model_column );
    virtual bool Cleared();
    virtual void Resort();
    virtual bool BeforeReset();
    virtual bool AfterReset();

    void UpdateLastCount();

private:
    wxDataViewModel         *m_wx_model;
    wxDataViewCtrlInternal  *m_internal;
};

// ---------------------------------------------------------
// wxGtkDataViewListModelNotifier
// ---------------------------------------------------------

wxGtkDataViewModelNotifier::wxGtkDataViewModelNotifier(
    wxDataViewModel *wx_model, wxDataViewCtrlInternal *internal )
{
    m_wx_model = wx_model;
    m_internal = internal;
}

wxGtkDataViewModelNotifier::~wxGtkDataViewModelNotifier()
{
    m_wx_model = NULL;
    m_internal = NULL;
}

bool wxGtkDataViewModelNotifier::ItemAdded( const wxDataViewItem &parent, const wxDataViewItem &item )
{
    m_internal->ItemAdded( parent, item );
    GtkWxTreeModel *wxgtk_model = m_internal->GetGtkModel();

    GtkTreeIter iter;
    iter.stamp = wxgtk_model->stamp;
    iter.user_data = item.GetID();

    wxGtkTreePath path(wxgtk_tree_model_get_path(
        GTK_TREE_MODEL(wxgtk_model), &iter ));
    gtk_tree_model_row_inserted(
        GTK_TREE_MODEL(wxgtk_model), path, &iter);

    return true;
}

bool wxGtkDataViewModelNotifier::ItemDeleted( const wxDataViewItem &parent, const wxDataViewItem &item )
{
    GtkWxTreeModel *wxgtk_model = m_internal->GetGtkModel();
#if 0
    // using _get_path for a deleted item cannot be
    // a good idea
    GtkTreeIter iter;
    iter.stamp = wxgtk_model->stamp;
    iter.user_data = (gpointer) item.GetID();
    wxGtkTreePath path(wxgtk_tree_model_get_path(
        GTK_TREE_MODEL(wxgtk_model), &iter ));
#else
    // so get the path from the parent
    GtkTreeIter iter;
    iter.stamp = wxgtk_model->stamp;
    iter.user_data = (gpointer) parent.GetID();
    wxGtkTreePath path(wxgtk_tree_model_get_path(
        GTK_TREE_MODEL(wxgtk_model), &iter ));
    // and add the final index ourselves
    int index = m_internal->GetIndexOf( parent, item );
    gtk_tree_path_append_index( path, index );
#endif

    gtk_tree_model_row_deleted(
        GTK_TREE_MODEL(wxgtk_model), path );

    m_internal->ItemDeleted( parent, item );

    return true;
}

void wxGtkDataViewModelNotifier::Resort()
{
    m_internal->Resort();
}

bool wxGtkDataViewModelNotifier::ItemChanged( const wxDataViewItem &item )
{
    GtkWxTreeModel *wxgtk_model = m_internal->GetGtkModel();

    GtkTreeIter iter;
    iter.stamp = wxgtk_model->stamp;
    iter.user_data = (gpointer) item.GetID();

    wxGtkTreePath path(wxgtk_tree_model_get_path(
        GTK_TREE_MODEL(wxgtk_model), &iter ));
    gtk_tree_model_row_changed(
        GTK_TREE_MODEL(wxgtk_model), path, &iter );

    m_internal->ItemChanged( item );

    return true;
}

bool wxGtkDataViewModelNotifier::ValueChanged( const wxDataViewItem &item, unsigned int model_column )
{
    GtkWxTreeModel *wxgtk_model = m_internal->GetGtkModel();
    wxDataViewCtrl *ctrl = m_internal->GetOwner();

    // This adds GTK+'s missing MVC logic for ValueChanged
    unsigned int index;
    for (index = 0; index < ctrl->GetColumnCount(); index++)
    {
        wxDataViewColumn *column = ctrl->GetColumn( index );
        if (column->GetModelColumn() == model_column)
        {
            GtkTreeView *widget = GTK_TREE_VIEW(ctrl->GtkGetTreeView());
            GtkTreeViewColumn *gcolumn = GTK_TREE_VIEW_COLUMN(column->GetGtkHandle());

            // Get cell area
            GtkTreeIter iter;
            iter.stamp = wxgtk_model->stamp;
            iter.user_data = (gpointer) item.GetID();
            wxGtkTreePath path(wxgtk_tree_model_get_path(
                GTK_TREE_MODEL(wxgtk_model), &iter ));
            GdkRectangle cell_area;
            gtk_tree_view_get_cell_area( widget, path, gcolumn, &cell_area );

            GtkAdjustment* hadjust = gtk_tree_view_get_hadjustment( widget );
            double d = gtk_adjustment_get_value( hadjust );
            int xdiff = (int) d;

            int ydiff = gcolumn->button->allocation.height;
            // Redraw
            gtk_widget_queue_draw_area( GTK_WIDGET(widget),
                cell_area.x - xdiff, ydiff + cell_area.y, cell_area.width, cell_area.height );

            m_internal->ValueChanged( item, model_column );

            return true;
        }
    }

    return false;
}

bool wxGtkDataViewModelNotifier::BeforeReset()
{
    GtkWidget *treeview = m_internal->GetOwner()->GtkGetTreeView();
    gtk_tree_view_set_model( GTK_TREE_VIEW(treeview), NULL );

    return true;
}

bool wxGtkDataViewModelNotifier::AfterReset()
{
    GtkWidget *treeview = m_internal->GetOwner()->GtkGetTreeView();
    GtkWxTreeModel *wxgtk_model = m_internal->GetGtkModel();

    m_internal->Cleared();

    gtk_tree_view_set_model( GTK_TREE_VIEW(treeview), GTK_TREE_MODEL(wxgtk_model) );

    return true;
}

bool wxGtkDataViewModelNotifier::Cleared()
{
    GtkWxTreeModel *wxgtk_model = m_internal->GetGtkModel();

    // There is no call to tell the model that everything
    // has been deleted so call row_deleted() for every
    // child of root...

    int count = m_internal->iter_n_children( NULL ); // number of children of root

    GtkTreePath *path = gtk_tree_path_new_first();  // points to root

    int i;
    for (i = 0; i < count; i++)
        gtk_tree_model_row_deleted( GTK_TREE_MODEL(wxgtk_model), path );

    gtk_tree_path_free( path );

    m_internal->Cleared();

    return true;
}

// ---------------------------------------------------------
// wxDataViewRenderer
// ---------------------------------------------------------

static gpointer s_user_data = NULL;

static void
wxgtk_cell_editable_editing_done( GtkCellEditable *WXUNUSED(editable),
                                  wxDataViewRenderer *wxrenderer )
{
    wxDataViewColumn *column = wxrenderer->GetOwner();
    wxDataViewCtrl *dv = column->GetOwner();
    wxDataViewEvent event( wxEVT_COMMAND_DATAVIEW_ITEM_EDITING_DONE, dv->GetId() );
    event.SetDataViewColumn( column );
    event.SetModel( dv->GetModel() );
    wxDataViewItem item( s_user_data );
    event.SetItem( item );
    dv->HandleWindowEvent( event );
}

static void
wxgtk_renderer_editing_started( GtkCellRenderer *WXUNUSED(cell), GtkCellEditable *editable,
                                gchar *path, wxDataViewRenderer *wxrenderer )
{
    if (!editable)
       return;

    wxDataViewColumn *column = wxrenderer->GetOwner();
    wxDataViewCtrl *dv = column->GetOwner();
    wxDataViewEvent event( wxEVT_COMMAND_DATAVIEW_ITEM_EDITING_STARTED, dv->GetId() );
    event.SetDataViewColumn( column );
    event.SetModel( dv->GetModel() );
    wxDataViewItem item(dv->GTKPathToItem(wxGtkTreePath(path)));
    event.SetItem( item );
    dv->HandleWindowEvent( event );

    if (GTK_IS_CELL_EDITABLE(editable))
    {
        s_user_data = item.GetID();

        g_signal_connect (GTK_CELL_EDITABLE (editable), "editing_done",
            G_CALLBACK (wxgtk_cell_editable_editing_done),
            (gpointer) wxrenderer );

    }
}


IMPLEMENT_ABSTRACT_CLASS(wxDataViewRenderer, wxDataViewRendererBase)

wxDataViewRenderer::wxDataViewRenderer( const wxString &varianttype, wxDataViewCellMode mode,
                                        int align ) :
    wxDataViewRendererBase( varianttype, mode, align )
{
    m_renderer = NULL;
    m_mode = mode;

    // we haven't changed them yet
    m_usingDefaultAttrs = true;

    // NOTE: SetMode() and SetAlignment() needs to be called in the renderer's ctor,
    //       after the m_renderer pointer has been initialized
}

void wxDataViewRenderer::GtkPackIntoColumn(GtkTreeViewColumn *column)
{
    gtk_tree_view_column_pack_end( column, m_renderer, TRUE /* expand */);
}

void wxDataViewRenderer::GtkInitHandlers()
{
    if (!gtk_check_version(2,6,0))
    {
        g_signal_connect (GTK_CELL_RENDERER(m_renderer), "editing_started",
            G_CALLBACK (wxgtk_renderer_editing_started),
            this);
    }
}

void wxDataViewRenderer::SetMode( wxDataViewCellMode mode )
{
    GtkCellRendererMode gtkMode;
    switch (mode)
    {
        case wxDATAVIEW_CELL_INERT:
            gtkMode = GTK_CELL_RENDERER_MODE_INERT;
            break;

        case wxDATAVIEW_CELL_ACTIVATABLE:
            gtkMode = GTK_CELL_RENDERER_MODE_ACTIVATABLE;
            break;

        case wxDATAVIEW_CELL_EDITABLE:
            gtkMode = GTK_CELL_RENDERER_MODE_EDITABLE;
            break;

        default:
            wxFAIL_MSG( "unknown wxDataViewCellMode value" );
            return;
    }

    m_mode = mode;
    
    // This value is most often ignored in GtkTreeView
    GValue gvalue = { 0, };
    g_value_init( &gvalue, gtk_cell_renderer_mode_get_type() );
    g_value_set_enum( &gvalue, gtkMode );
    g_object_set_property( G_OBJECT(m_renderer), "mode", &gvalue );
    g_value_unset( &gvalue );
}

wxDataViewCellMode wxDataViewRenderer::GetMode() const
{
    wxDataViewCellMode ret;

    GValue gvalue;
    g_object_get( G_OBJECT(m_renderer), "mode", &gvalue, NULL);

    switch (g_value_get_enum(&gvalue))
    {
        default:
            wxFAIL_MSG( "unknown GtkCellRendererMode value" );
            // fall through (we have to return something)

        case GTK_CELL_RENDERER_MODE_INERT:
            ret = wxDATAVIEW_CELL_INERT;
            break;

        case GTK_CELL_RENDERER_MODE_ACTIVATABLE:
            ret = wxDATAVIEW_CELL_ACTIVATABLE;
            break;

        case GTK_CELL_RENDERER_MODE_EDITABLE:
            ret = wxDATAVIEW_CELL_EDITABLE;
            break;
    }

    g_value_unset( &gvalue );

    return ret;
}

void wxDataViewRenderer::GtkUpdateAlignment()
{
    int align = m_alignment;

    // query alignment from column ?
    if (align == -1)
    {
        // None there yet
        if (GetOwner() == NULL)
            return;

        align = GetOwner()->GetAlignment();
        align |= wxALIGN_CENTRE_VERTICAL;
    }

    // horizontal alignment:

    gfloat xalign = 0.0;
    if (align & wxALIGN_RIGHT)
        xalign = 1.0;
    else if (align & wxALIGN_CENTER_HORIZONTAL)
        xalign = 0.5;

    GValue gvalue = { 0, };
    g_value_init( &gvalue, G_TYPE_FLOAT );
    g_value_set_float( &gvalue, xalign );
    g_object_set_property( G_OBJECT(m_renderer), "xalign", &gvalue );
    g_value_unset( &gvalue );

    // vertical alignment:

    gfloat yalign = 0.0;
    if (align & wxALIGN_BOTTOM)
        yalign = 1.0;
    else if (align & wxALIGN_CENTER_VERTICAL)
        yalign = 0.5;

    GValue gvalue2 = { 0, };
    g_value_init( &gvalue2, G_TYPE_FLOAT );
    g_value_set_float( &gvalue2, yalign );
    g_object_set_property( G_OBJECT(m_renderer), "yalign", &gvalue2 );
    g_value_unset( &gvalue2 );
}

void wxDataViewRenderer::SetAlignment( int align )
{
    m_alignment = align;
    GtkUpdateAlignment();
}

int wxDataViewRenderer::GetAlignment() const
{
    return m_alignment;
}

void wxDataViewRenderer::EnableEllipsize(wxEllipsizeMode mode)
{
#ifdef __WXGTK26__
    if ( gtk_check_version(2, 6, 0) != NULL )
        return;

    GtkCellRendererText * const rend = GtkGetTextRenderer();
    if ( !rend )
        return;

    // we use the same values in wxEllipsizeMode as PangoEllipsizeMode so we
    // can just cast between them
    GValue gvalue = { 0, };
    g_value_init( &gvalue, PANGO_TYPE_ELLIPSIZE_MODE );
    g_value_set_enum( &gvalue, static_cast<PangoEllipsizeMode>(mode) );
    g_object_set_property( G_OBJECT(rend), "ellipsize", &gvalue );
    g_value_unset( &gvalue );
#else // GTK < 2.6
    wxUnusedVar(mode);
#endif // GTK 2.6/before
}

wxEllipsizeMode wxDataViewRenderer::GetEllipsizeMode() const
{
#ifdef __WXGTK26__
    if ( gtk_check_version(2, 6, 0) != NULL )
        return wxELLIPSIZE_NONE;

    GtkCellRendererText * const rend = GtkGetTextRenderer();
    if ( !rend )
        return wxELLIPSIZE_NONE;

    GValue gvalue = { 0, };
    g_value_init( &gvalue, PANGO_TYPE_ELLIPSIZE_MODE );
    g_object_get_property( G_OBJECT(rend), "ellipsize", &gvalue );
    wxEllipsizeMode
        mode = static_cast<wxEllipsizeMode>(g_value_get_enum( &gvalue ));
    g_value_unset( &gvalue );

    return mode;
#else // GTK < 2.6
    return wxELLIPSIZE_NONE;
#endif // GTK 2.6/before
}

void
wxDataViewRenderer::GtkOnTextEdited(const gchar *itempath, const wxString& str)
{
    wxVariant value(str);
    if (!Validate( value ))
        return;

    wxDataViewItem
        item(GetOwner()->GetOwner()->GTKPathToItem(wxGtkTreePath(itempath)));

    GtkOnCellChanged(value, item, GetOwner()->GetModelColumn());
}

void
wxDataViewRenderer::GtkOnCellChanged(const wxVariant& value,
                                     const wxDataViewItem& item,
                                     unsigned col)
{
    wxDataViewModel *model = GetOwner()->GetOwner()->GetModel();
    model->ChangeValue( value, item, col );
}

// ---------------------------------------------------------
// wxDataViewTextRenderer
// ---------------------------------------------------------

extern "C"
{

static void wxGtkTextRendererEditedCallback( GtkCellRendererText *WXUNUSED(renderer),
    gchar *arg1, gchar *arg2, gpointer user_data )
{
    wxDataViewRenderer *cell = (wxDataViewRenderer*) user_data;

    cell->GtkOnTextEdited(arg1, wxGTK_CONV_BACK_FONT(
                arg2, cell->GetOwner()->GetOwner()->GetFont()));
}

}

namespace
{

// helper function used by wxDataViewTextRenderer and
// wxDataViewCustomRenderer::RenderText(): it applies the attributes to the
// given text renderer and returns true if anything was done
bool GtkApplyAttr(GtkCellRendererText *renderer, const wxDataViewItemAttr& attr)
{
    bool usingDefaultAttrs = true;
    if (attr.HasColour())
    {
        const GdkColor * const gcol = attr.GetColour().GetColor();

        GValue gvalue = { 0, };
        g_value_init( &gvalue, GDK_TYPE_COLOR );
        g_value_set_boxed( &gvalue, gcol );
        g_object_set_property( G_OBJECT(renderer), "foreground_gdk", &gvalue );
        g_value_unset( &gvalue );

        usingDefaultAttrs = false;
    }
    else
    {
        GValue gvalue = { 0, };
        g_value_init( &gvalue, G_TYPE_BOOLEAN );
        g_value_set_boolean( &gvalue, FALSE );
        g_object_set_property( G_OBJECT(renderer), "foreground-set", &gvalue );
        g_value_unset( &gvalue );
    }

    if (attr.GetItalic())
    {
        GValue gvalue = { 0, };
        g_value_init( &gvalue, PANGO_TYPE_STYLE );
        g_value_set_enum( &gvalue, PANGO_STYLE_ITALIC );
        g_object_set_property( G_OBJECT(renderer), "style", &gvalue );
        g_value_unset( &gvalue );

        usingDefaultAttrs = false;
    }
    else
    {
        GValue gvalue = { 0, };
        g_value_init( &gvalue, G_TYPE_BOOLEAN );
        g_value_set_boolean( &gvalue, FALSE );
        g_object_set_property( G_OBJECT(renderer), "style-set", &gvalue );
        g_value_unset( &gvalue );
    }


    if (attr.GetBold())
    {
        GValue gvalue = { 0, };
        g_value_init( &gvalue, PANGO_TYPE_WEIGHT );
        g_value_set_enum( &gvalue, PANGO_WEIGHT_BOLD );
        g_object_set_property( G_OBJECT(renderer), "weight", &gvalue );
        g_value_unset( &gvalue );

        usingDefaultAttrs = false;
    }
    else
    {
        GValue gvalue = { 0, };
        g_value_init( &gvalue, G_TYPE_BOOLEAN );
        g_value_set_boolean( &gvalue, FALSE );
        g_object_set_property( G_OBJECT(renderer), "weight-set", &gvalue );
        g_value_unset( &gvalue );
    }

#if 0
    if (attr.HasBackgroundColour())
    {
        wxColour colour = attr.GetBackgroundColour();
        const GdkColor * const gcol = colour.GetColor();

        GValue gvalue = { 0, };
        g_value_init( &gvalue, GDK_TYPE_COLOR );
        g_value_set_boxed( &gvalue, gcol );
        g_object_set_property( G_OBJECT(renderer), "cell-background_gdk", &gvalue );
        g_value_unset( &gvalue );
    }
    else
    {
        GValue gvalue = { 0, };
        g_value_init( &gvalue, G_TYPE_BOOLEAN );
        g_value_set_boolean( &gvalue, FALSE );
        g_object_set_property( G_OBJECT(renderer), "cell-background-set", &gvalue );
        g_value_unset( &gvalue );
    }
#endif

    return !usingDefaultAttrs;
}

} // anonymous namespace

IMPLEMENT_CLASS(wxDataViewTextRenderer, wxDataViewRenderer)

wxDataViewTextRenderer::wxDataViewTextRenderer( const wxString &varianttype, wxDataViewCellMode mode,
                                                int align ) :
    wxDataViewRenderer( varianttype, mode, align )
{
    GtkWxCellRendererText *text_renderer = gtk_wx_cell_renderer_text_new();
    text_renderer->wx_renderer = this;
    m_renderer = (GtkCellRenderer*) text_renderer;

    if (mode & wxDATAVIEW_CELL_EDITABLE)
    {
        GValue gvalue = { 0, };
        g_value_init( &gvalue, G_TYPE_BOOLEAN );
        g_value_set_boolean( &gvalue, true );
        g_object_set_property( G_OBJECT(m_renderer), "editable", &gvalue );
        g_value_unset( &gvalue );

        g_signal_connect_after( m_renderer, "edited", G_CALLBACK(wxGtkTextRendererEditedCallback), this );

        GtkInitHandlers();
    }

    SetMode(mode);
    SetAlignment(align);
}

bool wxDataViewTextRenderer::SetTextValue(const wxString& str)
{
    GValue gvalue = { 0, };
    g_value_init( &gvalue, G_TYPE_STRING );
    g_value_set_string( &gvalue, wxGTK_CONV_FONT( str, GetOwner()->GetOwner()->GetFont() ) );
    g_object_set_property( G_OBJECT(m_renderer), "text", &gvalue );
    g_value_unset( &gvalue );

    return true;
}

bool wxDataViewTextRenderer::GetTextValue(wxString& str) const
{
    GValue gvalue = { 0, };
    g_value_init( &gvalue, G_TYPE_STRING );
    g_object_get_property( G_OBJECT(m_renderer), "text", &gvalue );
    str = wxGTK_CONV_BACK_FONT( g_value_get_string( &gvalue ), const_cast<wxDataViewTextRenderer*>(this)->GetOwner()->GetOwner()->GetFont() );
    g_value_unset( &gvalue );

    return true;
}

void wxDataViewTextRenderer::SetAlignment( int align )
{
    wxDataViewRenderer::SetAlignment(align);

    if (gtk_check_version(2,10,0))
        return;

    // horizontal alignment:
    PangoAlignment pangoAlign = PANGO_ALIGN_LEFT;
    if (align & wxALIGN_RIGHT)
        pangoAlign = PANGO_ALIGN_RIGHT;
    else if (align & wxALIGN_CENTER_HORIZONTAL)
        pangoAlign = PANGO_ALIGN_CENTER;

    GValue gvalue = { 0, };
    g_value_init( &gvalue, gtk_cell_renderer_mode_get_type() );
    g_value_set_enum( &gvalue, pangoAlign );
    g_object_set_property( G_OBJECT(m_renderer), "alignment", &gvalue );
    g_value_unset( &gvalue );
}

bool wxDataViewTextRenderer::GtkSetAttr(const wxDataViewItemAttr& attr)
{
    return GtkApplyAttr(GtkGetTextRenderer(), attr);
}

GtkCellRendererText *wxDataViewTextRenderer::GtkGetTextRenderer() const
{
    return GTK_CELL_RENDERER_TEXT(m_renderer);
}

// ---------------------------------------------------------
// wxDataViewBitmapRenderer
// ---------------------------------------------------------

namespace
{

// set "pixbuf" property on the given renderer
void SetPixbufProp(GtkCellRenderer *renderer, GdkPixbuf *pixbuf)
{
    GValue gvalue = { 0, };
    g_value_init( &gvalue, G_TYPE_OBJECT );
    g_value_set_object( &gvalue, pixbuf );
    g_object_set_property( G_OBJECT(renderer), "pixbuf", &gvalue );
    g_value_unset( &gvalue );
}

} // anonymous namespace

IMPLEMENT_CLASS(wxDataViewBitmapRenderer, wxDataViewRenderer)

wxDataViewBitmapRenderer::wxDataViewBitmapRenderer( const wxString &varianttype, wxDataViewCellMode mode,
                                                    int align ) :
    wxDataViewRenderer( varianttype, mode, align )
{
    m_renderer = gtk_cell_renderer_pixbuf_new();

    SetMode(mode);
    SetAlignment(align);
}

bool wxDataViewBitmapRenderer::SetValue( const wxVariant &value )
{
    if (value.GetType() == wxT("wxBitmap"))
    {
        wxBitmap bitmap;
        bitmap << value;

        // GetPixbuf() may create a Pixbuf representation in the wxBitmap
        // object (and it will stay there and remain owned by wxBitmap)
        SetPixbufProp(m_renderer, bitmap.GetPixbuf());
    }
    else if (value.GetType() == wxT("wxIcon"))
    {
        wxIcon icon;
        icon << value;

        SetPixbufProp(m_renderer, icon.GetPixbuf());
    }
    else
    {
        return false;
    }

    return true;
}

bool wxDataViewBitmapRenderer::GetValue( wxVariant &WXUNUSED(value) ) const
{
    return false;
}

// ---------------------------------------------------------
// wxDataViewToggleRenderer
// ---------------------------------------------------------

extern "C" {
static void wxGtkToggleRendererToggledCallback( GtkCellRendererToggle *renderer,
    gchar *path, gpointer user_data );
}

static void wxGtkToggleRendererToggledCallback( GtkCellRendererToggle *renderer,
    gchar *path, gpointer user_data )
{
    wxDataViewToggleRenderer *cell = (wxDataViewToggleRenderer*) user_data;

    // get old value
    GValue gvalue = { 0, };
    g_value_init( &gvalue, G_TYPE_BOOLEAN );
    g_object_get_property( G_OBJECT(renderer), "active", &gvalue );
    bool tmp = g_value_get_boolean( &gvalue );
    g_value_unset( &gvalue );
    // invert it
    tmp = !tmp;

    wxVariant value = tmp;
    if (!cell->Validate( value ))
        return;

    wxDataViewCtrl * const ctrl = cell->GetOwner()->GetOwner();
    wxDataViewModel *model = ctrl->GetModel();

    wxDataViewItem item(ctrl->GTKPathToItem(wxGtkTreePath(path)));

    unsigned int model_col = cell->GetOwner()->GetModelColumn();

    model->ChangeValue( value, item, model_col );
}

IMPLEMENT_CLASS(wxDataViewToggleRenderer, wxDataViewRenderer)

wxDataViewToggleRenderer::wxDataViewToggleRenderer( const wxString &varianttype,
                                                    wxDataViewCellMode mode, int align ) :
    wxDataViewRenderer( varianttype, mode, align )
{
    m_renderer = (GtkCellRenderer*) gtk_cell_renderer_toggle_new();

    if (mode & wxDATAVIEW_CELL_ACTIVATABLE)
    {
        g_signal_connect_after( m_renderer, "toggled",
                                G_CALLBACK(wxGtkToggleRendererToggledCallback), this );
    }
    else
    {
        GValue gvalue = { 0, };
        g_value_init( &gvalue, G_TYPE_BOOLEAN );
        g_value_set_boolean( &gvalue, false );
        g_object_set_property( G_OBJECT(m_renderer), "activatable", &gvalue );
        g_value_unset( &gvalue );
    }

    SetMode(mode);
    SetAlignment(align);
}

bool wxDataViewToggleRenderer::SetValue( const wxVariant &value )
{
    bool tmp = value;

    GValue gvalue = { 0, };
    g_value_init( &gvalue, G_TYPE_BOOLEAN );
    g_value_set_boolean( &gvalue, tmp );
    g_object_set_property( G_OBJECT(m_renderer), "active", &gvalue );
    g_value_unset( &gvalue );

    return true;
}

bool wxDataViewToggleRenderer::GetValue( wxVariant &value ) const
{
    GValue gvalue = { 0, };
    g_value_init( &gvalue, G_TYPE_BOOLEAN );
    g_object_get_property( G_OBJECT(m_renderer), "active", &gvalue );
    bool tmp = g_value_get_boolean( &gvalue );
    g_value_unset( &gvalue );

    value = tmp;

    return true;
}

// ---------------------------------------------------------
// wxDataViewCustomRenderer
// ---------------------------------------------------------

class wxDataViewCtrlDCImpl: public wxWindowDCImpl
{
public:
   wxDataViewCtrlDCImpl( wxDC *owner, wxDataViewCtrl *window ) :
       wxWindowDCImpl( owner )
   {
        GtkWidget *widget = window->m_treeview;
        // Set later
        m_gdkwindow = NULL;

        m_window = window;

        m_context = window->GTKGetPangoDefaultContext();
        m_layout = pango_layout_new( m_context );
        m_fontdesc = pango_font_description_copy(gtk_widget_get_style(widget)->font_desc);

        m_cmap = gtk_widget_get_colormap( widget ? widget : window->m_widget );

        // Set m_gdkwindow later
        // SetUpDC();
    }
};

class wxDataViewCtrlDC: public wxWindowDC
{
public:
    wxDataViewCtrlDC( wxDataViewCtrl *window ) :
        wxWindowDC( new wxDataViewCtrlDCImpl( this, window ) )
        { }
};


// ---------------------------------------------------------
// wxDataViewCustomRenderer
// ---------------------------------------------------------

IMPLEMENT_CLASS(wxDataViewCustomRenderer, wxDataViewRenderer)

wxDataViewCustomRenderer::wxDataViewCustomRenderer( const wxString &varianttype,
                                                    wxDataViewCellMode mode,
                                                    int align,
                                                    bool no_init )
    : wxDataViewCustomRendererBase( varianttype, mode, align )
{
    m_dc = NULL;
    m_text_renderer = NULL;

    if (no_init)
        m_renderer = NULL;
    else
        Init(mode, align);
}

GtkCellRendererText *wxDataViewCustomRenderer::GtkGetTextRenderer() const
{
    if ( !m_text_renderer )
    {
        // we create it on demand so need to do it even from a const function
        const_cast<wxDataViewCustomRenderer *>(this)->
        m_text_renderer = GTK_CELL_RENDERER_TEXT(gtk_cell_renderer_text_new());
        g_object_ref_sink(m_text_renderer);
    }

    return m_text_renderer;
}

void wxDataViewCustomRenderer::RenderText( const wxString &text,
                                           int xoffset,
                                           wxRect cell,
                                           wxDC *WXUNUSED(dc),
                                           int WXUNUSED(state) )
{

    GtkCellRendererText * const textRenderer = GtkGetTextRenderer();

    GValue gvalue = { 0, };
    g_value_init( &gvalue, G_TYPE_STRING );
    g_value_set_string( &gvalue, wxGTK_CONV_FONT( text, GetOwner()->GetOwner()->GetFont() ) );
    g_object_set_property( G_OBJECT(textRenderer), "text", &gvalue );
    g_value_unset( &gvalue );

    GtkApplyAttr(textRenderer, GetAttr());

    GdkRectangle cell_area;
    wxRectToGDKRect(cell, cell_area);
    cell_area.x += xoffset;
    cell_area.width -= xoffset;

    gtk_cell_renderer_render( GTK_CELL_RENDERER(textRenderer),
        m_renderParams.window,
        m_renderParams.widget,
        m_renderParams.background_area,
        &cell_area,
        m_renderParams.expose_area,
        (GtkCellRendererState) m_renderParams.flags );
}

bool wxDataViewCustomRenderer::Init(wxDataViewCellMode mode, int align)
{
    GtkWxCellRenderer *renderer = (GtkWxCellRenderer *) gtk_wx_cell_renderer_new();
    renderer->cell = this;

    m_renderer = (GtkCellRenderer*) renderer;

    SetMode(mode);
    SetAlignment(align);

    GtkInitHandlers();

    return true;
}

wxDataViewCustomRenderer::~wxDataViewCustomRenderer()
{
    if (m_dc)
        delete m_dc;

    if (m_text_renderer)
        g_object_unref(m_text_renderer);
}

wxDC *wxDataViewCustomRenderer::GetDC()
{
    if (m_dc == NULL)
    {
        if (GetOwner() == NULL)
            return NULL;
        if (GetOwner()->GetOwner() == NULL)
            return NULL;
        m_dc = new wxDataViewCtrlDC( GetOwner()->GetOwner() );
    }

    return m_dc;
}

// ---------------------------------------------------------
// wxDataViewProgressRenderer
// ---------------------------------------------------------

IMPLEMENT_CLASS(wxDataViewProgressRenderer, wxDataViewCustomRenderer)

wxDataViewProgressRenderer::wxDataViewProgressRenderer( const wxString &label,
    const wxString &varianttype, wxDataViewCellMode mode, int align ) :
    wxDataViewCustomRenderer( varianttype, mode, align, true )
{
    m_label = label;
    m_value = 0;

#ifdef __WXGTK26__
    if (!gtk_check_version(2,6,0))
    {
        m_renderer = (GtkCellRenderer*) gtk_cell_renderer_progress_new();

        SetMode(mode);
        SetAlignment(align);

#if !wxUSE_UNICODE
        // We can't initialize the renderer just yet because we don't have the
        // pointer to the column that uses this renderer yet and so attempt to
        // dereference GetOwner() to get the font that is used as a source of
        // encoding in multibyte-to-Unicode conversion in GTKSetLabel() in
        // non-Unicode builds would crash. So simply remember to do it later.
        if ( !m_label.empty() )
            m_needsToSetLabel = true;
        else
#endif // !wxUSE_UNICODE
            GTKSetLabel();
    }
    else
#endif
    {
        // Use custom cell code
        wxDataViewCustomRenderer::Init(mode, align);
    }
}

wxDataViewProgressRenderer::~wxDataViewProgressRenderer()
{
}

void wxDataViewProgressRenderer::GTKSetLabel()
{
    GValue gvalue = { 0, };
    g_value_init( &gvalue, G_TYPE_STRING );

    // Take care to not use GetOwner() here if the label is empty, we can be
    // called from ctor when GetOwner() is still NULL in this case.
    g_value_set_string( &gvalue,
                        m_label.empty() ? ""
                                        : wxGTK_CONV_FONT(m_label,
                                            GetOwner()->GetOwner()->GetFont())
                      );
    g_object_set_property( G_OBJECT(m_renderer), "text", &gvalue );
    g_value_unset( &gvalue );

#if !wxUSE_UNICODE
    m_needsToSetLabel = false;
#endif // !wxUSE_UNICODE
}

bool wxDataViewProgressRenderer::SetValue( const wxVariant &value )
{
#ifdef __WXGTK26__
    if (!gtk_check_version(2,6,0))
    {
#if !wxUSE_UNICODE
        if ( m_needsToSetLabel )
            GTKSetLabel();
#endif // !wxUSE_UNICODE

        gint tmp = (long) value;
        GValue gvalue = { 0, };
        g_value_init( &gvalue, G_TYPE_INT );
        g_value_set_int( &gvalue, tmp );
        g_object_set_property( G_OBJECT(m_renderer), "value", &gvalue );
        g_value_unset( &gvalue );
    }
    else
#endif
    {
        m_value = (long) value;

        if (m_value < 0) m_value = 0;
        if (m_value > 100) m_value = 100;
    }

    return true;
}

bool wxDataViewProgressRenderer::GetValue( wxVariant &WXUNUSED(value) ) const
{
    return false;
}

bool wxDataViewProgressRenderer::Render( wxRect cell, wxDC *dc, int WXUNUSED(state) )
{
    double pct = (double)m_value / 100.0;
    wxRect bar = cell;
    bar.width = (int)(cell.width * pct);
    dc->SetPen( *wxTRANSPARENT_PEN );
    dc->SetBrush( *wxBLUE_BRUSH );
    dc->DrawRectangle( bar );

    dc->SetBrush( *wxTRANSPARENT_BRUSH );
    dc->SetPen( *wxBLACK_PEN );
    dc->DrawRectangle( cell );

    return true;
}

wxSize wxDataViewProgressRenderer::GetSize() const
{
    return wxSize(40,12);
}

// -------------------------------------
// wxDataViewChoiceRenderer
// -------------------------------------

wxDataViewChoiceRenderer::wxDataViewChoiceRenderer( const wxArrayString &choices,
                            wxDataViewCellMode mode, int alignment  ) :
    wxDataViewCustomRenderer( "string", mode, alignment, true )
{
   m_choices = choices;

#ifdef __WXGTK26__
    if (!gtk_check_version(2,6,0))
    {
        m_renderer = (GtkCellRenderer*) gtk_cell_renderer_combo_new();

        GtkListStore *store = gtk_list_store_new( 1, G_TYPE_STRING );
        for (size_t n = 0; n < m_choices.GetCount(); n++)
        {
            gtk_list_store_insert_with_values(
                store, NULL, n, 0,
                static_cast<const char *>(m_choices[n].utf8_str()), -1 );
        }

        g_object_set (m_renderer,
                "model", store,
                "text-column", 0,
                "has-entry", FALSE,
                NULL);

        bool editable = (mode & wxDATAVIEW_CELL_EDITABLE);
        g_object_set (m_renderer, "editable", editable, NULL);

        SetAlignment(alignment);

        g_signal_connect_after( m_renderer, "edited", G_CALLBACK(wxGtkTextRendererEditedCallback), this );

        GtkInitHandlers();
    }
    else
#endif
    {
        // Use custom cell code
        wxDataViewCustomRenderer::Init(mode, alignment);
    }
}

bool wxDataViewChoiceRenderer::Render( wxRect rect, wxDC *dc, int state )
{
    RenderText( m_data, 0, rect, dc, state );
    return true;
}

wxSize wxDataViewChoiceRenderer::GetSize() const
{
    return wxSize(70,20);
}

bool wxDataViewChoiceRenderer::SetValue( const wxVariant &value )
{

#ifdef __WXGTK26__
    if (!gtk_check_version(2,6,0))
    {
        GValue gvalue = { 0, };
        g_value_init( &gvalue, G_TYPE_STRING );
        g_value_set_string(&gvalue,
                           wxGTK_CONV_FONT(value.GetString(),
                                           GetOwner()->GetOwner()->GetFont()));
        g_object_set_property( G_OBJECT(m_renderer), "text", &gvalue );
        g_value_unset( &gvalue );
    }
    else
#endif
        m_data = value.GetString();

    return true;
}

bool wxDataViewChoiceRenderer::GetValue( wxVariant &value ) const
{
#ifdef __WXGTK26__
    if (!gtk_check_version(2,6,0))
    {
        GValue gvalue = { 0, };
        g_value_init( &gvalue, G_TYPE_STRING );
        g_object_get_property( G_OBJECT(m_renderer), "text", &gvalue );
        wxString temp = wxGTK_CONV_BACK_FONT(g_value_get_string(&gvalue),
                                             GetOwner()->GetOwner()->GetFont());
        g_value_unset( &gvalue );
        value = temp;

        //wxPrintf( "temp %s\n", temp );
        // TODO: remove this code
    }
    else
#endif
        value = m_data;

    return true;
}

void wxDataViewChoiceRenderer::SetAlignment( int align )
{
    wxDataViewCustomRenderer::SetAlignment(align);

    if (gtk_check_version(2,10,0))
        return;

    // horizontal alignment:
    PangoAlignment pangoAlign = PANGO_ALIGN_LEFT;
    if (align & wxALIGN_RIGHT)
        pangoAlign = PANGO_ALIGN_RIGHT;
    else if (align & wxALIGN_CENTER_HORIZONTAL)
        pangoAlign = PANGO_ALIGN_CENTER;

    GValue gvalue = { 0, };
    g_value_init( &gvalue, gtk_cell_renderer_mode_get_type() );
    g_value_set_enum( &gvalue, pangoAlign );
    g_object_set_property( G_OBJECT(m_renderer), "alignment", &gvalue );
    g_value_unset( &gvalue );
}

// ----------------------------------------------------------------------------
// wxDataViewChoiceByIndexRenderer
// ----------------------------------------------------------------------------

wxDataViewChoiceByIndexRenderer::wxDataViewChoiceByIndexRenderer( const wxArrayString &choices,
                              wxDataViewCellMode mode, int alignment ) :
      wxDataViewChoiceRenderer( choices, mode, alignment )
{
}

void wxDataViewChoiceByIndexRenderer::GtkOnTextEdited(const gchar *itempath, const wxString& str)
{
    wxVariant value( (long) GetChoices().Index( str ) );

    if (!Validate( value ))
        return;

    wxDataViewItem
        item(GetOwner()->GetOwner()->GTKPathToItem(wxGtkTreePath(itempath)));

    GtkOnCellChanged(value, item, GetOwner()->GetModelColumn());
}

bool wxDataViewChoiceByIndexRenderer::SetValue( const wxVariant &value )
{
    wxVariant string_value = GetChoice( value.GetLong() );
    return wxDataViewChoiceRenderer::SetValue( string_value );
}

bool wxDataViewChoiceByIndexRenderer::GetValue( wxVariant &value ) const
{
    wxVariant string_value;
    if (!wxDataViewChoiceRenderer::GetValue( string_value ))
         return false;

    value = (long) GetChoices().Index( string_value.GetString() );
    return true;
}

// ---------------------------------------------------------
// wxDataViewDateRenderer
// ---------------------------------------------------------

class wxDataViewDateRendererPopupTransient: public wxPopupTransientWindow
{
public:
    wxDataViewDateRendererPopupTransient( wxWindow* parent, wxDateTime *value,
        wxDataViewModel *model, const wxDataViewItem &item, unsigned int col ) :
        wxPopupTransientWindow( parent, wxBORDER_SIMPLE )
    {
        m_model = model;
        m_item = item;
        m_col = col;
        m_cal = new wxCalendarCtrl( this, -1, *value );
        wxBoxSizer *sizer = new wxBoxSizer( wxHORIZONTAL );
        sizer->Add( m_cal, 1, wxGROW );
        SetSizer( sizer );
        sizer->Fit( this );
    }

    virtual void OnDismiss()
    {
    }

    void OnCalendar( wxCalendarEvent &event );

    wxCalendarCtrl   *m_cal;
    wxDataViewModel  *m_model;
    wxDataViewItem    m_item;
    unsigned int      m_col;

private:
    DECLARE_EVENT_TABLE()
};

BEGIN_EVENT_TABLE(wxDataViewDateRendererPopupTransient,wxPopupTransientWindow)
    EVT_CALENDAR( -1, wxDataViewDateRendererPopupTransient::OnCalendar )
END_EVENT_TABLE()

void wxDataViewDateRendererPopupTransient::OnCalendar( wxCalendarEvent &event )
{
    m_model->ChangeValue( event.GetDate(), m_item, m_col );
    DismissAndNotify();
}

IMPLEMENT_CLASS(wxDataViewDateRenderer, wxDataViewCustomRenderer)

wxDataViewDateRenderer::wxDataViewDateRenderer( const wxString &varianttype,
                        wxDataViewCellMode mode, int align ) :
    wxDataViewCustomRenderer( varianttype, mode, align )
{
    SetMode(mode);
    SetAlignment(align);
}

bool wxDataViewDateRenderer::SetValue( const wxVariant &value )
{
    m_date = value.GetDateTime();

    return true;
}

bool wxDataViewDateRenderer::GetValue( wxVariant &WXUNUSED(value) ) const
{
    return false;
}

bool wxDataViewDateRenderer::Render( wxRect cell, wxDC *dc, int state )
{
    dc->SetFont( GetOwner()->GetOwner()->GetFont() );
    wxString tmp = m_date.FormatDate();
    RenderText( tmp, 0, cell, dc, state );
    return true;
}

wxSize wxDataViewDateRenderer::GetSize() const
{
    wxString tmp = m_date.FormatDate();
    wxCoord x,y,d;
    GetView()->GetTextExtent( tmp, &x, &y, &d );
    return wxSize(x,y+d);
}

bool wxDataViewDateRenderer::Activate( const wxRect& WXUNUSED(cell), wxDataViewModel *model,
                                       const wxDataViewItem &item, unsigned int col )
{
    wxVariant variant;
    model->GetValue( variant, item, col );
    wxDateTime value = variant.GetDateTime();

    wxDataViewDateRendererPopupTransient *popup = new wxDataViewDateRendererPopupTransient(
        GetOwner()->GetOwner()->GetParent(), &value, model, item, col );
    wxPoint pos = wxGetMousePosition();
    popup->Move( pos );
    popup->Layout();
    popup->Popup( popup->m_cal );

    return true;
}


// ---------------------------------------------------------
// wxDataViewIconTextRenderer
// ---------------------------------------------------------

IMPLEMENT_CLASS(wxDataViewIconTextRenderer, wxDataViewCustomRenderer)

wxDataViewIconTextRenderer::wxDataViewIconTextRenderer
                            (
                             const wxString &varianttype,
                             wxDataViewCellMode mode,
                             int align
                            )
    : wxDataViewTextRenderer(varianttype, mode, align)
{
    m_rendererIcon = gtk_cell_renderer_pixbuf_new();
}

wxDataViewIconTextRenderer::~wxDataViewIconTextRenderer()
{
}

void wxDataViewIconTextRenderer::GtkPackIntoColumn(GtkTreeViewColumn *column)
{
    // add the icon renderer first
    gtk_tree_view_column_pack_start(column, m_rendererIcon, FALSE /* !expand */);

    // add the text renderer too
    wxDataViewRenderer::GtkPackIntoColumn(column);
}

bool wxDataViewIconTextRenderer::SetValue( const wxVariant &value )
{
    m_value << value;

    SetTextValue(m_value.GetText());
    SetPixbufProp(m_rendererIcon, m_value.GetIcon().GetPixbuf());

    return true;
}

bool wxDataViewIconTextRenderer::GetValue(wxVariant& value) const
{
    wxString str;
    if ( !GetTextValue(str) )
        return false;

    // user doesn't have any way to edit the icon so leave it unchanged
    value << wxDataViewIconText(str, m_value.GetIcon());

    return true;
}

void
wxDataViewIconTextRenderer::GtkOnCellChanged(const wxVariant& value,
                                             const wxDataViewItem& item,
                                             unsigned col)
{
    // we receive just the text part of our value as it's the only one which
    // can be edited, but we need the full wxDataViewIconText value for the
    // model
    wxVariant valueIconText;
    valueIconText << wxDataViewIconText(value.GetString(), m_value.GetIcon());
    wxDataViewTextRenderer::GtkOnCellChanged(valueIconText, item, col);
}

// ---------------------------------------------------------
// wxDataViewColumn
// ---------------------------------------------------------


static gboolean
gtk_dataview_header_button_press_callback( GtkWidget *WXUNUSED(widget),
                                           GdkEventButton *gdk_event,
                                           wxDataViewColumn *column )
{
    if (gdk_event->type != GDK_BUTTON_PRESS)
        return FALSE;

    if (gdk_event->button == 1)
    {
        gs_lastLeftClickHeader = column;

        wxDataViewCtrl *dv = column->GetOwner();
        wxDataViewEvent event( wxEVT_COMMAND_DATAVIEW_COLUMN_HEADER_CLICK, dv->GetId() );
        event.SetDataViewColumn( column );
        event.SetModel( dv->GetModel() );
        if (dv->HandleWindowEvent( event ))
            return FALSE;
    }

    if (gdk_event->button == 3)
    {
        wxDataViewCtrl *dv = column->GetOwner();
        wxDataViewEvent event( wxEVT_COMMAND_DATAVIEW_COLUMN_HEADER_RIGHT_CLICK, dv->GetId() );
        event.SetDataViewColumn( column );
        event.SetModel( dv->GetModel() );
        if (dv->HandleWindowEvent( event ))
            return FALSE;
    }

    return FALSE;
}

extern "C"
{

static void wxGtkTreeCellDataFunc( GtkTreeViewColumn *WXUNUSED(column),
                            GtkCellRenderer *renderer,
                            GtkTreeModel *model,
                            GtkTreeIter *iter,
                            gpointer data )
{
    g_return_if_fail (GTK_IS_WX_TREE_MODEL (model));
    GtkWxTreeModel *tree_model = (GtkWxTreeModel *) model;

    wxDataViewRenderer *cell = (wxDataViewRenderer*) data;

    wxDataViewItem item( (void*) iter->user_data );

    wxDataViewModel *wx_model = tree_model->internal->GetDataViewModel();

    if (!wx_model->IsVirtualListModel())
    {
        gboolean visible;
        if (wx_model->IsContainer( item ))
        {
            visible = wx_model->HasContainerColumns( item ) ||
                        (cell->GetOwner()->GetModelColumn() == 0);
        }
        else
        {
            visible = true;
        }

        GValue gvalue = { 0, };
        g_value_init( &gvalue, G_TYPE_BOOLEAN );
        g_value_set_boolean( &gvalue, visible );
        g_object_set_property( G_OBJECT(renderer), "visible", &gvalue );
        g_value_unset( &gvalue );

        if ( !visible )
            return;
    }

    wxVariant value;
    wx_model->GetValue( value, item, cell->GetOwner()->GetModelColumn() );

    if (value.GetType() != cell->GetVariantType())
    {
        wxLogError( wxT("Wrong type, required: %s but: %s"),
                    value.GetType().c_str(),
                    cell->GetVariantType().c_str() );
    }

    cell->SetValue( value );

    // deal with disabled items
    bool enabled = wx_model->IsEnabled( item, cell->GetOwner()->GetModelColumn() );

    // a) this sets the appearance to disabled grey    
    GValue gvalue = { 0, };
    g_value_init( &gvalue, G_TYPE_BOOLEAN );
    g_value_set_boolean( &gvalue, enabled );
    g_object_set_property( G_OBJECT(renderer), "sensitive", &gvalue );
    g_value_unset( &gvalue );

    // b) this actually disables the control/renderer
    if (enabled)
        cell->SetMode( cell->GtkGetMode() );
    else
        cell->SetMode( wxDATAVIEW_CELL_INERT );
        

    // deal with attributes: if the renderer doesn't support them at all, we
    // don't even need to query the model for them
    if ( !cell->GtkSupportsAttrs() )
        return;

    // it can support attributes so check if this item has any
    wxDataViewItemAttr attr;
    if ( wx_model->GetAttr( item, cell->GetOwner()->GetModelColumn(), attr )
            || !cell->GtkIsUsingDefaultAttrs() )
    {
        bool usingDefaultAttrs = !cell->GtkSetAttr(attr);
        cell->GtkSetUsingDefaultAttrs(usingDefaultAttrs);
    }
    // else: no custom attributes specified and we're already using the default
    //       ones -- nothing to do
    
}

} // extern "C"

#include <wx/listimpl.cpp>
WX_DEFINE_LIST(wxDataViewColumnList)

wxDataViewColumn::wxDataViewColumn( const wxString &title, wxDataViewRenderer *cell,
                                    unsigned int model_column, int width,
                                    wxAlignment align, int flags )
    : wxDataViewColumnBase( cell, model_column )
{
    Init( align, flags, width );

    SetTitle( title );
}

wxDataViewColumn::wxDataViewColumn( const wxBitmap &bitmap, wxDataViewRenderer *cell,
                                    unsigned int model_column, int width,
                                    wxAlignment align, int flags )
    : wxDataViewColumnBase( bitmap, cell, model_column )
{
    Init( align, flags, width );

    SetBitmap( bitmap );
}

void wxDataViewColumn::Init(wxAlignment align, int flags, int width)
{
    m_isConnected = false;

    GtkTreeViewColumn *column = gtk_tree_view_column_new();
    m_column = (GtkWidget*) column;

    SetFlags( flags );
    SetAlignment( align );

    SetWidth( width );

    // Create container for icon and label
    GtkWidget *box = gtk_hbox_new( FALSE, 1 );
    gtk_widget_show( box );
    // gtk_container_set_border_width((GtkContainer*)box, 2);
    m_image = gtk_image_new();
    gtk_box_pack_start(GTK_BOX(box), m_image, FALSE, FALSE, 1);
    m_label = gtk_label_new("");
    gtk_box_pack_end( GTK_BOX(box), GTK_WIDGET(m_label), FALSE, FALSE, 1 );
    gtk_tree_view_column_set_widget( column, box );

    wxDataViewRenderer * const colRenderer = GetRenderer();
    GtkCellRenderer * const cellRenderer = colRenderer->GetGtkHandle();

    colRenderer->GtkPackIntoColumn(column);

    gtk_tree_view_column_set_cell_data_func( column, cellRenderer,
        wxGtkTreeCellDataFunc, (gpointer) colRenderer, NULL );
}

void wxDataViewColumn::OnInternalIdle()
{
    if (m_isConnected)
        return;

    if (gtk_widget_get_realized(GetOwner()->m_treeview))
    {
        GtkTreeViewColumn *column = GTK_TREE_VIEW_COLUMN(m_column);
        if (column->button)
        {
            g_signal_connect(column->button, "button_press_event",
                      G_CALLBACK (gtk_dataview_header_button_press_callback), this);

            // otherwise the event will be blocked by GTK+
            gtk_tree_view_column_set_clickable( column, TRUE );

            m_isConnected = true;
        }
    }
}

void wxDataViewColumn::SetOwner( wxDataViewCtrl *owner )
{
    wxDataViewColumnBase::SetOwner( owner );

    GtkTreeViewColumn *column = GTK_TREE_VIEW_COLUMN(m_column);

    gtk_tree_view_column_set_title( column, wxGTK_CONV_FONT(GetTitle(), GetOwner()->GetFont() ) );
}

void wxDataViewColumn::SetTitle( const wxString &title )
{
    wxDataViewCtrl *ctrl = GetOwner();
    gtk_label_set_text( GTK_LABEL(m_label), ctrl ? wxGTK_CONV_FONT(title, ctrl->GetFont())
                                                 : wxGTK_CONV_SYS(title) );
    if (title.empty())
        gtk_widget_hide( m_label );
    else
        gtk_widget_show( m_label );
}

wxString wxDataViewColumn::GetTitle() const
{
    return wxGTK_CONV_BACK_FONT(
            gtk_label_get_text( GTK_LABEL(m_label) ),
            GetOwner()->GetFont()
           );
}

void wxDataViewColumn::SetBitmap( const wxBitmap &bitmap )
{
    wxDataViewColumnBase::SetBitmap( bitmap );

    if (bitmap.IsOk())
    {
        GtkImage *gtk_image = GTK_IMAGE(m_image);

        GdkBitmap *mask = NULL;
        if (bitmap.GetMask())
            mask = bitmap.GetMask()->GetBitmap();

        if (bitmap.HasPixbuf())
        {
            gtk_image_set_from_pixbuf(GTK_IMAGE(gtk_image),
                                      bitmap.GetPixbuf());
        }
        else
        {
            gtk_image_set_from_pixmap(GTK_IMAGE(gtk_image),
                                      bitmap.GetPixmap(), mask);
        }
        gtk_widget_show( m_image );
    }
    else
    {
        gtk_widget_hide( m_image );
    }
}

void wxDataViewColumn::SetHidden( bool hidden )
{
    gtk_tree_view_column_set_visible( GTK_TREE_VIEW_COLUMN(m_column), !hidden );
}

void wxDataViewColumn::SetResizeable( bool resizable )
{
    gtk_tree_view_column_set_resizable( GTK_TREE_VIEW_COLUMN(m_column), resizable );
}

void wxDataViewColumn::SetAlignment( wxAlignment align )
{
    GtkTreeViewColumn *column = GTK_TREE_VIEW_COLUMN(m_column);

    gfloat xalign = 0.0;
    if (align == wxALIGN_RIGHT)
        xalign = 1.0;
    if (align == wxALIGN_CENTER_HORIZONTAL ||
        align == wxALIGN_CENTER)
        xalign = 0.5;

    gtk_tree_view_column_set_alignment( column, xalign );

    if (m_renderer && m_renderer->GetAlignment() == -1)
        m_renderer->GtkUpdateAlignment();
}

wxAlignment wxDataViewColumn::GetAlignment() const
{
    gfloat xalign = gtk_tree_view_column_get_alignment( GTK_TREE_VIEW_COLUMN(m_column) );

    if (xalign == 1.0)
        return wxALIGN_RIGHT;
    if (xalign == 0.5)
        return wxALIGN_CENTER_HORIZONTAL;

    return wxALIGN_LEFT;
}

void wxDataViewColumn::SetSortable( bool sortable )
{
    GtkTreeViewColumn *column = GTK_TREE_VIEW_COLUMN(m_column);

    if ( sortable )
    {
        gtk_tree_view_column_set_sort_column_id( column, GetModelColumn() );
    }
    else
    {
        gtk_tree_view_column_set_sort_column_id( column, -1 );
        gtk_tree_view_column_set_sort_indicator( column, FALSE );
        gtk_tree_view_column_set_clickable( column, FALSE );
    }
}

bool wxDataViewColumn::IsSortable() const
{
    GtkTreeViewColumn *column = GTK_TREE_VIEW_COLUMN(m_column);
    return gtk_tree_view_column_get_clickable( column );
}

void wxDataViewColumn::SetAsSortKey( bool WXUNUSED(sort) )
{
    // it might not make sense to have this function in wxHeaderColumn at
    // all in fact, changing of the sort order should only be done using the
    // associated control API
    wxFAIL_MSG( "not implemented" );
}

bool wxDataViewColumn::IsSortKey() const
{
    GtkTreeViewColumn *column = GTK_TREE_VIEW_COLUMN(m_column);
    return gtk_tree_view_column_get_sort_indicator( column );
}

bool wxDataViewColumn::IsResizeable() const
{
    GtkTreeViewColumn *column = GTK_TREE_VIEW_COLUMN(m_column);
    return gtk_tree_view_column_get_resizable( column );
}

bool wxDataViewColumn::IsHidden() const
{
    GtkTreeViewColumn *column = GTK_TREE_VIEW_COLUMN(m_column);
    return !gtk_tree_view_column_get_visible( column );
}

void wxDataViewColumn::SetSortOrder( bool ascending )
{
    GtkTreeViewColumn *column = GTK_TREE_VIEW_COLUMN(m_column);

    if (ascending)
        gtk_tree_view_column_set_sort_order( column, GTK_SORT_ASCENDING );
    else
        gtk_tree_view_column_set_sort_order( column, GTK_SORT_DESCENDING );

    gtk_tree_view_column_set_sort_indicator( column, TRUE );
}

bool wxDataViewColumn::IsSortOrderAscending() const
{
    GtkTreeViewColumn *column = GTK_TREE_VIEW_COLUMN(m_column);

    return (gtk_tree_view_column_get_sort_order( column ) != GTK_SORT_DESCENDING);
}

void wxDataViewColumn::SetMinWidth( int width )
{
    gtk_tree_view_column_set_min_width( GTK_TREE_VIEW_COLUMN(m_column), width );
}

int wxDataViewColumn::GetMinWidth() const
{
    return gtk_tree_view_column_get_min_width( GTK_TREE_VIEW_COLUMN(m_column) );
}

int wxDataViewColumn::GetWidth() const
{
    return gtk_tree_view_column_get_width( GTK_TREE_VIEW_COLUMN(m_column) );
}

void wxDataViewColumn::SetWidth( int width )
{
    if ( width == wxCOL_WIDTH_AUTOSIZE )
    {
        // NB: this disables user resizing
        gtk_tree_view_column_set_sizing( GTK_TREE_VIEW_COLUMN(m_column), GTK_TREE_VIEW_COLUMN_AUTOSIZE );
    }
    else
    {
        if ( width == wxCOL_WIDTH_DEFAULT )
        {
            // TODO find a better calculation
            width = wxDVC_DEFAULT_WIDTH;
        }

        gtk_tree_view_column_set_sizing( GTK_TREE_VIEW_COLUMN(m_column), GTK_TREE_VIEW_COLUMN_FIXED );
        gtk_tree_view_column_set_fixed_width( GTK_TREE_VIEW_COLUMN(m_column), width );
    }
}

void wxDataViewColumn::SetReorderable( bool reorderable )
{
    gtk_tree_view_column_set_reorderable( GTK_TREE_VIEW_COLUMN(m_column), reorderable );
}

bool wxDataViewColumn::IsReorderable() const
{
    return gtk_tree_view_column_get_reorderable( GTK_TREE_VIEW_COLUMN(m_column) );
}

//-----------------------------------------------------------------------------
// wxGtkTreeModelNode
//-----------------------------------------------------------------------------

#if 0
class wxGtkTreeModelChildWithPos
{
public:
    unsigned int pos;
    void        *id;
};

static
int wxGtkTreeModelChildWithPosCmp( const void* data1, const void* data2, const void* user_data )
{
    const wxGtkTreeModelChildWithPos* child1 = (const wxGtkTreeModelChildWithPos*) data1;
    const wxGtkTreeModelChildWithPos* child2 = (const wxGtkTreeModelChildWithPos*) data2;
    const wxDataViewCtrlInternal *internal = (const wxDataViewCtrlInternal *) user_data;
    int ret = internal->GetDataViewModel()->Compare( child1->id, child2->id,
        internal->GetSortColumn(), (internal->GetSortOrder() == GTK_SORT_DESCENDING) );

    return ret;
}
#else
static
int LINKAGEMODE wxGtkTreeModelChildPtrCmp( void*** data1, void*** data2 )
{
    return gs_internal->GetDataViewModel()->Compare( **data1, **data2,
        gs_internal->GetSortColumn(), (gs_internal->GetSortOrder() == GTK_SORT_ASCENDING) );
}

WX_DEFINE_ARRAY_PTR( void**, wxGtkTreeModelChildrenPtr );
#endif

void wxGtkTreeModelNode::Resort()
{
    size_t child_count = GetChildCount();
    if (child_count == 0)
        return;

    size_t node_count = GetNodesCount();

    if (child_count == 1)
    {
        if (node_count == 1)
        {
            wxGtkTreeModelNode *node = m_nodes.Item( 0 );
            node->Resort();
        }
        return;
    }

    gint *new_order = new gint[child_count];

#if 1
    // m_children has the original *void
    // ptrs points to these
    wxGtkTreeModelChildrenPtr ptrs;
    size_t i;
    for (i = 0; i < child_count; i++)
       ptrs.Add( &(m_children[i]) );
    // Sort the ptrs
    gs_internal = m_internal;
    ptrs.Sort( &wxGtkTreeModelChildPtrCmp );

    wxGtkTreeModelChildren temp;
    void** base_ptr = &(m_children[0]);
    // Transfer positions to new_order array and
    // IDs to temp
    for (i = 0; i < child_count; i++)
    {
        new_order[i] = ptrs[i] - base_ptr;
        temp.Add( *ptrs[i] );
    }

    // Transfer IDs back to m_children
    m_children.Clear();
    WX_APPEND_ARRAY( temp, m_children );
#endif
#if 0
    // Too slow

    // Build up array with IDs and original positions
    wxGtkTreeModelChildWithPos* temp = new wxGtkTreeModelChildWithPos[child_count];
    size_t i;
    for (i = 0; i < child_count; i++)
    {
       temp[i].pos = i;
       temp[i].id = m_children[i];
    }
    // Sort array keeping original positions
    wxQsort( temp, child_count, sizeof(wxGtkTreeModelChildWithPos),
             &wxGtkTreeModelChildWithPosCmp, m_internal );
    // Transfer positions to new_order array and
    // IDs to m_children
    m_children.Clear();
    for (i = 0; i < child_count; i++)
    {
       new_order[i] = temp[i].pos;
       m_children.Add( temp[i].id );
    }
    // Delete array
    delete [] temp;
#endif

#if 0
    // Too slow

    wxGtkTreeModelChildren temp;
    WX_APPEND_ARRAY( temp, m_children );

    gs_internal = m_internal;
    m_children.Sort( &wxGtkTreeModelChildCmp );

    unsigned int pos;
    for (pos = 0; pos < child_count; pos++)
    {
        void *id = m_children.Item( pos );
        int old_pos = temp.Index( id );
        new_order[pos] = old_pos;
    }
#endif

    GtkTreeModel *gtk_tree_model = GTK_TREE_MODEL( m_internal->GetGtkModel() );

    GtkTreeIter iter;
    iter.user_data = GetItem().GetID();
    iter.stamp = m_internal->GetGtkModel()->stamp;

    gtk_tree_model_rows_reordered( gtk_tree_model,
            wxGtkTreePath(m_internal->get_path(&iter)), &iter, new_order );

    delete [] new_order;

    unsigned int pos;
    for (pos = 0; pos < node_count; pos++)
    {
        wxGtkTreeModelNode *node = m_nodes.Item( pos );
        node->Resort();
    }
}

//-----------------------------------------------------------------------------
// wxDataViewCtrlInternal
//-----------------------------------------------------------------------------

wxDataViewCtrlInternal::wxDataViewCtrlInternal( wxDataViewCtrl *owner, wxDataViewModel *wx_model )
{
    m_owner = owner;
    m_wx_model = wx_model;

    m_gtk_model = NULL;
    m_root = NULL;
    m_sort_order = GTK_SORT_ASCENDING;
    m_sort_column = -1;
    m_dataview_sort_column = NULL;

    m_dragDataObject = NULL;
    m_dropDataObject = NULL;

    m_dirty = false;

    m_gtk_model = wxgtk_tree_model_new();
    m_gtk_model->internal = this;

    m_notifier = new wxGtkDataViewModelNotifier( wx_model, this );

    wx_model->AddNotifier( m_notifier );

    // g_object_unref( gtk_model ); ???

    if (!m_wx_model->IsVirtualListModel())
        InitTree();

    gtk_tree_view_set_model( GTK_TREE_VIEW(m_owner->GtkGetTreeView()), GTK_TREE_MODEL(m_gtk_model) );
}

wxDataViewCtrlInternal::~wxDataViewCtrlInternal()
{
    m_wx_model->RemoveNotifier( m_notifier );

    // remove the model from the GtkTreeView before it gets destroyed
    gtk_tree_view_set_model( GTK_TREE_VIEW( m_owner->GtkGetTreeView() ), NULL );

    g_object_unref( m_gtk_model );

    delete m_dragDataObject;
    delete m_dropDataObject;
}

void wxDataViewCtrlInternal::ScheduleRefresh()
{
    m_dirty = true;
}

void wxDataViewCtrlInternal::OnInternalIdle()
{
    if (m_dirty)
    {
        GtkWidget *widget = m_owner->GtkGetTreeView();
        gtk_widget_queue_draw( widget );
        m_dirty = false;
    }
}

void wxDataViewCtrlInternal::InitTree()
{
    wxDataViewItem item;
    m_root = new wxGtkTreeModelNode( NULL, item, this );

    BuildBranch( m_root );
}

void wxDataViewCtrlInternal::BuildBranch( wxGtkTreeModelNode *node )
{
    if (node->GetChildCount() == 0)
    {
        wxDataViewItemArray children;
        unsigned int count = m_wx_model->GetChildren( node->GetItem(), children );

        unsigned int pos;
        for (pos = 0; pos < count; pos++)
        {
            wxDataViewItem child = children[pos];

            if (m_wx_model->IsContainer( child ))
                node->AddNode( new wxGtkTreeModelNode( node, child, this ) );
            else
                node->AddLeave( child.GetID() );

            // Don't send any events here
        }
    }
}

// GTK+ dnd iface

bool wxDataViewCtrlInternal::EnableDragSource( const wxDataFormat &format )
{
    wxGtkString atom_str( gdk_atom_name( format  ) );
    m_dragSourceTargetEntryTarget = wxCharBuffer( atom_str );

    m_dragSourceTargetEntry.target =  m_dragSourceTargetEntryTarget.data();
    m_dragSourceTargetEntry.flags = 0;
    m_dragSourceTargetEntry.info = static_cast<guint>(-1);

    gtk_tree_view_enable_model_drag_source( GTK_TREE_VIEW(m_owner->GtkGetTreeView() ),
       GDK_BUTTON1_MASK, &m_dragSourceTargetEntry, 1, (GdkDragAction) GDK_ACTION_COPY );

    return true;
}

bool wxDataViewCtrlInternal::EnableDropTarget( const wxDataFormat &format )
{
    wxGtkString atom_str( gdk_atom_name( format  ) );
    m_dropTargetTargetEntryTarget = wxCharBuffer( atom_str );

    m_dropTargetTargetEntry.target =  m_dropTargetTargetEntryTarget.data();
    m_dropTargetTargetEntry.flags = 0;
    m_dropTargetTargetEntry.info = static_cast<guint>(-1);

    gtk_tree_view_enable_model_drag_dest( GTK_TREE_VIEW(m_owner->GtkGetTreeView() ),
       &m_dropTargetTargetEntry, 1, (GdkDragAction) GDK_ACTION_COPY );

    return true;
}

gboolean wxDataViewCtrlInternal::row_draggable( GtkTreeDragSource *WXUNUSED(drag_source),
    GtkTreePath *path )
{
    delete m_dragDataObject;

    wxDataViewItem item(GetOwner()->GTKPathToItem(path));
    if ( !item )
        return FALSE;

    wxDataViewEvent event( wxEVT_COMMAND_DATAVIEW_ITEM_BEGIN_DRAG, m_owner->GetId() );
    event.SetEventObject( m_owner );
    event.SetItem( item );
    event.SetModel( m_wx_model );
    gint x, y;
    gtk_widget_get_pointer(m_owner->GtkGetTreeView(), &x, &y);
    event.SetPosition(x, y);
    if (!m_owner->HandleWindowEvent( event ))
        return FALSE;

    if (!event.IsAllowed())
        return FALSE;

    wxDataObject *obj = event.GetDataObject();
    if (!obj)
        return FALSE;

    m_dragDataObject = obj;

    return TRUE;
}

gboolean
wxDataViewCtrlInternal::drag_data_delete(GtkTreeDragSource *WXUNUSED(drag_source),
                                         GtkTreePath *WXUNUSED(path))
{
    return FALSE;
}

gboolean wxDataViewCtrlInternal::drag_data_get( GtkTreeDragSource *WXUNUSED(drag_source),
    GtkTreePath *path, GtkSelectionData *selection_data )
{
    wxDataViewItem item(GetOwner()->GTKPathToItem(path));
    if ( !item )
        return FALSE;

    GdkAtom target = gtk_selection_data_get_target(selection_data);
    if (!m_dragDataObject->IsSupported(target))
        return FALSE;

    size_t size = m_dragDataObject->GetDataSize(target);
    if (size == 0)
        return FALSE;

    void *buf = malloc( size );

    gboolean res = FALSE;
    if (m_dragDataObject->GetDataHere(target, buf))
    {
        res = TRUE;

        gtk_selection_data_set(selection_data, target,
            8, (const guchar*) buf, size );
    }

    free( buf );

    return res;
}

gboolean
wxDataViewCtrlInternal::drag_data_received(GtkTreeDragDest *WXUNUSED(drag_dest),
                                           GtkTreePath *path,
                                           GtkSelectionData *selection_data)
{
    wxDataViewItem item(GetOwner()->GTKPathToItem(path));
    if ( !item )
        return FALSE;

    wxDataViewEvent event( wxEVT_COMMAND_DATAVIEW_ITEM_DROP, m_owner->GetId() );
    event.SetEventObject( m_owner );
    event.SetItem( item );
    event.SetModel( m_wx_model );
    event.SetDataFormat(gtk_selection_data_get_target(selection_data));
    event.SetDataSize(gtk_selection_data_get_length(selection_data));
    event.SetDataBuffer(const_cast<guchar*>(gtk_selection_data_get_data(selection_data)));
    if (!m_owner->HandleWindowEvent( event ))
        return FALSE;

    if (!event.IsAllowed())
        return FALSE;

    return TRUE;
}

gboolean
wxDataViewCtrlInternal::row_drop_possible(GtkTreeDragDest *WXUNUSED(drag_dest),
                                          GtkTreePath *path,
                                          GtkSelectionData *selection_data)
{
    wxDataViewItem item(GetOwner()->GTKPathToItem(path));
    if ( !item )
        return FALSE;

    wxDataViewEvent event( wxEVT_COMMAND_DATAVIEW_ITEM_DROP_POSSIBLE, m_owner->GetId() );
    event.SetEventObject( m_owner );
    event.SetItem( item );
    event.SetModel( m_wx_model );
    event.SetDataFormat(gtk_selection_data_get_target(selection_data));
    if (!m_owner->HandleWindowEvent( event ))
        return FALSE;

    if (!event.IsAllowed())
        return FALSE;

    return TRUE;
}

// notifications from wxDataViewModel

bool wxDataViewCtrlInternal::Cleared()
{
    if (m_root)
    {
        delete m_root;
        m_root = NULL;
    }

    InitTree();

    ScheduleRefresh();

    return true;
}

void wxDataViewCtrlInternal::Resort()
{
    if (!m_wx_model->IsVirtualListModel())
        m_root->Resort();

    ScheduleRefresh();
}

bool wxDataViewCtrlInternal::ItemAdded( const wxDataViewItem &parent, const wxDataViewItem &item )
{
    if (!m_wx_model->IsVirtualListModel())
    {
        wxGtkTreeModelNode *parent_node = FindNode( parent );
        wxASSERT_MSG(parent_node,
            "Did you forget a call to ItemAdded()? The parent node is unknown to the wxGtkTreeModel");

        if (m_wx_model->IsContainer( item ))
            parent_node->AddNode( new wxGtkTreeModelNode( parent_node, item, this ) );
        else
            parent_node->AddLeave( item.GetID() );
    }

    ScheduleRefresh();

    return true;
}

bool wxDataViewCtrlInternal::ItemDeleted( const wxDataViewItem &parent, const wxDataViewItem &item )
{
    if (!m_wx_model->IsVirtualListModel())
    {
        wxGtkTreeModelNode *parent_node = FindNode( parent );
        wxASSERT_MSG(parent_node,
            "Did you forget a call to ItemAdded()? The parent node is unknown to the wxGtkTreeModel");

        parent_node->DeleteChild( item.GetID() );
    }

    ScheduleRefresh();

    return true;
}

bool wxDataViewCtrlInternal::ItemChanged( const wxDataViewItem &item )
{
    wxDataViewEvent event( wxEVT_COMMAND_DATAVIEW_ITEM_VALUE_CHANGED, m_owner->GetId() );
    event.SetEventObject( m_owner );
    event.SetModel( m_owner->GetModel() );
    event.SetItem( item );
    m_owner->HandleWindowEvent( event );

    return true;
}

bool wxDataViewCtrlInternal::ValueChanged( const wxDataViewItem &item, unsigned int view_column )
{
    wxDataViewEvent event( wxEVT_COMMAND_DATAVIEW_ITEM_VALUE_CHANGED, m_owner->GetId() );
    event.SetEventObject( m_owner );
    event.SetModel( m_owner->GetModel() );
    event.SetColumn( view_column );
    event.SetDataViewColumn( GetOwner()->GetColumn(view_column) );
    event.SetItem( item );
    m_owner->HandleWindowEvent( event );

    return true;
}

// GTK+ model iface

GtkTreeModelFlags wxDataViewCtrlInternal::get_flags()
{
    int flags = 0;

    if ( m_wx_model->IsListModel() )
        flags |= GTK_TREE_MODEL_LIST_ONLY;

    if ( !m_wx_model->IsVirtualListModel() )
        flags |= GTK_TREE_MODEL_ITERS_PERSIST;

    return GtkTreeModelFlags(flags);
}

gboolean wxDataViewCtrlInternal::get_iter( GtkTreeIter *iter, GtkTreePath *path )
{

    if (m_wx_model->IsVirtualListModel())
    {
        wxDataViewVirtualListModel *wx_model = (wxDataViewVirtualListModel*) m_wx_model;

        unsigned int i = (unsigned int)gtk_tree_path_get_indices (path)[0];

        if (i >= wx_model->GetCount())
            return FALSE;

        iter->stamp = m_gtk_model->stamp;
        // user_data is just the index +1
        iter->user_data = (gpointer) (i+1);

        return TRUE;
    }
    else
    {
        int depth = gtk_tree_path_get_depth( path );

        wxGtkTreeModelNode *node = m_root;

        int i;
        for (i = 0; i < depth; i++)
        {
            BuildBranch( node );

            gint pos = gtk_tree_path_get_indices (path)[i];
            if (pos < 0) return FALSE;
            if ((size_t)pos >= node->GetChildCount()) return FALSE;

            void* id = node->GetChildren().Item( (size_t) pos );

            if (i == depth-1)
            {
                iter->stamp = m_gtk_model->stamp;
                iter->user_data = id;
                return TRUE;
            }

            size_t count = node->GetNodes().GetCount();
            size_t pos2;
            for (pos2 = 0; pos2 < count; pos2++)
            {
                wxGtkTreeModelNode *child_node = node->GetNodes().Item( pos2 );
                if (child_node->GetItem().GetID() == id)
                {
                    node = child_node;
                    break;
                }
            }
        }
    }

    return FALSE;
}

GtkTreePath *wxDataViewCtrlInternal::get_path( GtkTreeIter *iter )
{
    // When this is called from ItemDeleted(), the item is already
    // deleted in the model.

    GtkTreePath *retval = gtk_tree_path_new ();

    if (m_wx_model->IsVirtualListModel())
    {
        // iter is root, add nothing
        if (!iter->user_data)
           return retval;

        // user_data is just the index +1
        int i = ( (wxUIntPtr) iter->user_data ) -1;
        gtk_tree_path_append_index (retval, i);
    }
    else
    {
        void *id = iter->user_data;

        wxGtkTreeModelNode *node = FindParentNode( iter );
        while (node)
        {
            int pos = node->GetChildren().Index( id );

            gtk_tree_path_prepend_index( retval, pos );

            id = node->GetItem().GetID();
            node = node->GetParent();
        }
    }

    return retval;
}

gboolean wxDataViewCtrlInternal::iter_next( GtkTreeIter *iter )
{
    if (m_wx_model->IsVirtualListModel())
    {
        wxDataViewVirtualListModel *wx_model = (wxDataViewVirtualListModel*) m_wx_model;

        // user_data is just the index +1
        int n = ( (wxUIntPtr) iter->user_data ) -1;

        if (n == -1)
        {
            iter->user_data = NULL;
            return FALSE;
        }

        if (n >= (int) wx_model->GetCount()-1)
        {
            iter->user_data = NULL;
            return FALSE;
        }

        // user_data is just the index +1 (+2 because we need the next)
        iter->user_data = (gpointer) (n+2);
    }
    else
    {
        wxGtkTreeModelNode *parent = FindParentNode( iter );
        if( parent == NULL )
        {
            iter->user_data = NULL;
            return FALSE;
        }

        int pos = parent->GetChildren().Index( iter->user_data );

        if (pos == (int) parent->GetChildCount()-1)
        {
            iter->user_data = NULL;
            return FALSE;
        }

        iter->user_data = parent->GetChildren().Item( pos+1 );
    }

    return TRUE;
}

gboolean wxDataViewCtrlInternal::iter_children( GtkTreeIter *iter, GtkTreeIter *parent )
{
    if (m_wx_model->IsVirtualListModel())
    {
        // this is a list, nodes have no children
        if (parent)
            return FALSE;

        iter->stamp = m_gtk_model->stamp;
        iter->user_data = (gpointer) 1;

        return TRUE;
    }
    else
    {
        if (iter == NULL)
        {
            if (m_root->GetChildCount() == 0) return FALSE;
            iter->stamp = m_gtk_model->stamp;
            iter->user_data = (gpointer) m_root->GetChildren().Item( 0 );
            return TRUE;
        }


        wxDataViewItem item;
        if (parent)
            item = wxDataViewItem( (void*) parent->user_data );

        if (!m_wx_model->IsContainer( item ))
            return FALSE;

        wxGtkTreeModelNode *parent_node = FindNode( parent );
        wxASSERT_MSG(parent_node,
            "Did you forget a call to ItemAdded()? The parent node is unknown to the wxGtkTreeModel");

        BuildBranch( parent_node );

        if (parent_node->GetChildCount() == 0)
            return FALSE;

        iter->stamp = m_gtk_model->stamp;
        iter->user_data = (gpointer) parent_node->GetChildren().Item( 0 );
    }

    return TRUE;
}

gboolean wxDataViewCtrlInternal::iter_has_child( GtkTreeIter *iter )
{
    if (m_wx_model->IsVirtualListModel())
    {
        wxDataViewVirtualListModel *wx_model = (wxDataViewVirtualListModel*) m_wx_model;

        if (iter == NULL)
            return (wx_model->GetCount() > 0);

        // this is a list, nodes have no children
        return FALSE;
    }
    else
    {
        if (iter == NULL)
            return (m_root->GetChildCount() > 0);

        wxDataViewItem item( (void*) iter->user_data );

        bool is_container = m_wx_model->IsContainer( item );

        if (!is_container)
            return FALSE;

        wxGtkTreeModelNode *node = FindNode( iter );
        wxASSERT_MSG(node,
            "Did you forget a call to ItemAdded()? The iterator is unknown to the wxGtkTreeModel");

        BuildBranch( node );

        return (node->GetChildCount() > 0);
    }
}

gint wxDataViewCtrlInternal::iter_n_children( GtkTreeIter *iter )
{
    if (m_wx_model->IsVirtualListModel())
    {
        wxDataViewVirtualListModel *wx_model = (wxDataViewVirtualListModel*) m_wx_model;

        if (iter == NULL)
            return (gint) wx_model->GetCount();

        return 0;
    }
    else
    {
        if (iter == NULL)
            return m_root->GetChildCount();

        wxDataViewItem item( (void*) iter->user_data );

        if (!m_wx_model->IsContainer( item ))
            return 0;

        wxGtkTreeModelNode *parent_node = FindNode( iter );
        wxASSERT_MSG(parent_node,
            "Did you forget a call to ItemAdded()? The parent node is unknown to the wxGtkTreeModel");

        BuildBranch( parent_node );

        return parent_node->GetChildCount();
    }
}

gboolean wxDataViewCtrlInternal::iter_nth_child( GtkTreeIter *iter, GtkTreeIter *parent, gint n )
{
    if (m_wx_model->IsVirtualListModel())
    {
        wxDataViewVirtualListModel *wx_model = (wxDataViewVirtualListModel*) m_wx_model;

        if (parent)
            return FALSE;

        if (n < 0)
            return FALSE;

        if (n >= (gint) wx_model->GetCount())
            return FALSE;

        iter->stamp = m_gtk_model->stamp;
        // user_data is just the index +1
        iter->user_data = (gpointer) (n+1);

        return TRUE;
    }
    else
    {
        void* id = NULL;
        if (parent) id = (void*) parent->user_data;
        wxDataViewItem item( id );

        if (!m_wx_model->IsContainer( item ))
            return FALSE;

        wxGtkTreeModelNode *parent_node = FindNode( parent );
        wxASSERT_MSG(parent_node,
            "Did you forget a call to ItemAdded()? The parent node is unknown to the wxGtkTreeModel");

        BuildBranch( parent_node );

        iter->stamp = m_gtk_model->stamp;
        iter->user_data = parent_node->GetChildren().Item( n );

        return TRUE;
    }
}

gboolean wxDataViewCtrlInternal::iter_parent( GtkTreeIter *iter, GtkTreeIter *child )
{
    if (m_wx_model->IsVirtualListModel())
    {
        return FALSE;
    }
    else
    {
        wxGtkTreeModelNode *node = FindParentNode( child );
        if (!node)
            return FALSE;

        iter->stamp = m_gtk_model->stamp;
        iter->user_data = (gpointer) node->GetItem().GetID();

        return TRUE;
    }
}

// item can be deleted already in the model
int wxDataViewCtrlInternal::GetIndexOf( const wxDataViewItem &parent, const wxDataViewItem &item )
{
    if (m_wx_model->IsVirtualListModel())
    {
        return wxPtrToUInt(item.GetID()) - 1;
    }
    else
    {
        wxGtkTreeModelNode *parent_node = FindNode( parent );
        wxGtkTreeModelChildren &children = parent_node->GetChildren();
        size_t j;
        for (j = 0; j < children.GetCount(); j++)
        {
            if (children[j] == item.GetID())
               return j;
        }
    }
    return -1;
}


static wxGtkTreeModelNode*
wxDataViewCtrlInternal_FindNode( wxDataViewModel * model, wxGtkTreeModelNode *treeNode, const wxDataViewItem &item )
{
    if( model == NULL )
        return NULL;

    ItemList list;
    list.DeleteContents( true );
    wxDataViewItem it( item );

    while( it.IsOk() )
    {
        wxDataViewItem * pItem = new wxDataViewItem( it );
        list.Insert( pItem );
        it = model->GetParent( it );
    }

    wxGtkTreeModelNode * node = treeNode;
    for( ItemList::compatibility_iterator n = list.GetFirst(); n; n = n->GetNext() )
    {
        if( node && node->GetNodes().GetCount() != 0 )
        {
            int len = node->GetNodes().GetCount();
            wxGtkTreeModelNodes &nodes = node->GetNodes();
            int j = 0;
            for( ; j < len; j ++)
            {
                if( nodes[j]->GetItem() == *(n->GetData()))
                {
                    node = nodes[j];
                    break;
                }
            }

            if( j == len )
            {
                return NULL;
            }
        }
        else
            return NULL;
    }
    return node;

}

wxGtkTreeModelNode *wxDataViewCtrlInternal::FindNode( GtkTreeIter *iter )
{
    if (!iter)
        return m_root;

    wxDataViewItem item( (void*) iter->user_data );
    if (!item.IsOk())
        return m_root;

    wxGtkTreeModelNode *result = wxDataViewCtrlInternal_FindNode( m_wx_model, m_root, item );

/*
    if (!result)
    {
        wxLogDebug( "Not found %p", iter->user_data );
        char *crash = NULL;
        *crash = 0;
    }
    // TODO: remove this code
*/

    return result;
}

wxGtkTreeModelNode *wxDataViewCtrlInternal::FindNode( const wxDataViewItem &item )
{
    if (!item.IsOk())
        return m_root;

    wxGtkTreeModelNode *result = wxDataViewCtrlInternal_FindNode( m_wx_model, m_root, item );

/*
    if (!result)
    {
        wxLogDebug( "Not found %p", item.GetID() );
        char *crash = NULL;
        *crash = 0;
    }
    // TODO: remove this code
*/

    return result;
}

static wxGtkTreeModelNode*
wxDataViewCtrlInternal_FindParentNode( wxDataViewModel * model, wxGtkTreeModelNode *treeNode, const wxDataViewItem &item )
{
    if( model == NULL )
        return NULL;

    ItemList list;
    list.DeleteContents( true );
    if( !item.IsOk() )
        return NULL;

    wxDataViewItem it( model->GetParent( item ) );
    while( it.IsOk() )
    {
        wxDataViewItem * pItem = new wxDataViewItem( it );
        list.Insert( pItem );
        it = model->GetParent( it );
    }

    wxGtkTreeModelNode * node = treeNode;
    for( ItemList::compatibility_iterator n = list.GetFirst(); n; n = n->GetNext() )
    {
        if( node && node->GetNodes().GetCount() != 0 )
        {
            int len = node->GetNodes().GetCount();
            wxGtkTreeModelNodes nodes = node->GetNodes();
            int j = 0;
            for( ; j < len; j ++)
            {
                if( nodes[j]->GetItem() == *(n->GetData()))
                {
                    node = nodes[j];
                    break;
                }
            }

            if( j == len )
            {
                return NULL;
            }
        }
        else
            return NULL;
    }
    //Examine whether the node is item's parent node
    int len = node->GetChildCount();
    for( int i = 0; i < len ; i ++ )
    {
        if( node->GetChildren().Item( i ) == item.GetID() )
            return node;
    }
    return NULL;
}

wxGtkTreeModelNode *wxDataViewCtrlInternal::FindParentNode( GtkTreeIter *iter )
{
    if (!iter)
        return NULL;

    wxDataViewItem item( (void*) iter->user_data );
    if (!item.IsOk())
        return NULL;

    return wxDataViewCtrlInternal_FindParentNode( m_wx_model, m_root, item );
}

wxGtkTreeModelNode *wxDataViewCtrlInternal::FindParentNode( const wxDataViewItem &item )
{
    if (!item.IsOk())
        return NULL;

    return wxDataViewCtrlInternal_FindParentNode( m_wx_model, m_root, item );
}

//-----------------------------------------------------------------------------
// wxDataViewCtrl signal callbacks
//-----------------------------------------------------------------------------

static void
wxdataview_selection_changed_callback( GtkTreeSelection* WXUNUSED(selection), wxDataViewCtrl *dv )
{
    if (!gtk_widget_get_realized(dv->m_widget))
        return;

    wxDataViewEvent event( wxEVT_COMMAND_DATAVIEW_SELECTION_CHANGED, dv->GetId() );
    event.SetItem( dv->GetSelection() );
    event.SetModel( dv->GetModel() );
    dv->HandleWindowEvent( event );
}

static void
wxdataview_row_activated_callback( GtkTreeView* WXUNUSED(treeview), GtkTreePath *path,
                                   GtkTreeViewColumn *WXUNUSED(column), wxDataViewCtrl *dv )
{
    wxDataViewEvent event( wxEVT_COMMAND_DATAVIEW_ITEM_ACTIVATED, dv->GetId() );

    wxDataViewItem item(dv->GTKPathToItem(path));
    event.SetItem( item );
    event.SetModel( dv->GetModel() );
    dv->HandleWindowEvent( event );
}

static gboolean
wxdataview_test_expand_row_callback( GtkTreeView* WXUNUSED(treeview), GtkTreeIter* iter,
                                     GtkTreePath *WXUNUSED(path), wxDataViewCtrl *dv )
{
    wxDataViewEvent event( wxEVT_COMMAND_DATAVIEW_ITEM_EXPANDING, dv->GetId() );

    wxDataViewItem item( (void*) iter->user_data );;
    event.SetItem( item );
    event.SetModel( dv->GetModel() );
    dv->HandleWindowEvent( event );

    return !event.IsAllowed();
}

static void
wxdataview_row_expanded_callback( GtkTreeView* WXUNUSED(treeview), GtkTreeIter* iter,
                                  GtkTreePath *WXUNUSED(path), wxDataViewCtrl *dv )
{
    wxDataViewEvent event( wxEVT_COMMAND_DATAVIEW_ITEM_EXPANDED, dv->GetId() );

    wxDataViewItem item( (void*) iter->user_data );;
    event.SetItem( item );
    event.SetModel( dv->GetModel() );
    dv->HandleWindowEvent( event );
}

static gboolean
wxdataview_test_collapse_row_callback( GtkTreeView* WXUNUSED(treeview), GtkTreeIter* iter,
                                       GtkTreePath *WXUNUSED(path), wxDataViewCtrl *dv )
{
    wxDataViewEvent event( wxEVT_COMMAND_DATAVIEW_ITEM_COLLAPSING, dv->GetId() );

    wxDataViewItem item( (void*) iter->user_data );;
    event.SetItem( item );
    event.SetModel( dv->GetModel() );
    dv->HandleWindowEvent( event );

    return !event.IsAllowed();
}

static void
wxdataview_row_collapsed_callback( GtkTreeView* WXUNUSED(treeview), GtkTreeIter* iter,
                                   GtkTreePath *WXUNUSED(path), wxDataViewCtrl *dv )
{
    wxDataViewEvent event( wxEVT_COMMAND_DATAVIEW_ITEM_COLLAPSED, dv->GetId() );

    wxDataViewItem item( (void*) iter->user_data );;
    event.SetItem( item );
    event.SetModel( dv->GetModel() );
    dv->HandleWindowEvent( event );
}

//-----------------------------------------------------------------------------
    // wxDataViewCtrl
//-----------------------------------------------------------------------------

void wxDataViewCtrl::AddChildGTK(wxWindowGTK* child)
{
    GtkWidget* treeview = GtkGetTreeView();

    // Insert widget in GtkTreeView
    if (gtk_widget_get_realized(treeview))
        gtk_widget_set_parent_window( child->m_widget,
          gtk_tree_view_get_bin_window( GTK_TREE_VIEW(treeview) ) );
    gtk_widget_set_parent( child->m_widget, treeview );
}

static
void gtk_dataviewctrl_size_callback( GtkWidget *WXUNUSED(widget),
                                     GtkAllocation *WXUNUSED(gtk_alloc),
                                     wxDataViewCtrl *win )
{
    wxWindowList::compatibility_iterator node = win->GetChildren().GetFirst();
    while (node)
    {
        wxWindow *child = node->GetData();

        GtkRequisition req;
        gtk_widget_size_request( child->m_widget, &req );

        GtkAllocation alloc;
        alloc.x = child->m_x;
        alloc.y = child->m_y;
        alloc.width = child->m_width;
        alloc.height = child->m_height;
        gtk_widget_size_allocate( child->m_widget, &alloc );

        node = node->GetNext();
    }
}


//-----------------------------------------------------------------------------
// "motion_notify_event"
//-----------------------------------------------------------------------------

static gboolean
gtk_dataview_motion_notify_callback( GtkWidget *WXUNUSED(widget),
                                     GdkEventMotion *gdk_event,
                                     wxDataViewCtrl *dv )
{
    if (gdk_event->is_hint)
    {
        int x = 0;
        int y = 0;
        GdkModifierType state;
        gdk_window_get_pointer(gdk_event->window, &x, &y, &state);
        gdk_event->x = x;
        gdk_event->y = y;
    }

    wxGtkTreePath path;
    GtkTreeViewColumn *column = NULL;
    gint cell_x = 0;
    gint cell_y = 0;
    if (gtk_tree_view_get_path_at_pos(
        GTK_TREE_VIEW(dv->GtkGetTreeView()),
        (int) gdk_event->x, (int) gdk_event->y,
        path.ByRef(),
        &column,
        &cell_x,
        &cell_y))
    {
        if (path)
        {
            GtkTreeIter iter;
            dv->GtkGetInternal()->get_iter( &iter, path );
        }
    }


    return FALSE;
}

//-----------------------------------------------------------------------------
// "button_press_event"
//-----------------------------------------------------------------------------

static gboolean
gtk_dataview_button_press_callback( GtkWidget *WXUNUSED(widget),
                                    GdkEventButton *gdk_event,
                                    wxDataViewCtrl *dv )
{
    if ((gdk_event->button == 3) && (gdk_event->type == GDK_BUTTON_PRESS))
    {
        wxGtkTreePath path;
        GtkTreeViewColumn *column = NULL;
        gint cell_x = 0;
        gint cell_y = 0;
        if (gtk_tree_view_get_path_at_pos(
            GTK_TREE_VIEW(dv->GtkGetTreeView()),
            (int) gdk_event->x, (int) gdk_event->y,
            path.ByRef(),
            &column,
            &cell_x,
            &cell_y))
        {
            if (path)
            {
                wxDataViewEvent event( wxEVT_COMMAND_DATAVIEW_ITEM_CONTEXT_MENU, dv->GetId() );
                event.SetItem(dv->GTKPathToItem(path));
                event.SetModel( dv->GetModel() );
                return dv->HandleWindowEvent( event );
             }
        }
    }

    return FALSE;
}

IMPLEMENT_DYNAMIC_CLASS(wxDataViewCtrl, wxDataViewCtrlBase)

wxDataViewCtrl::~wxDataViewCtrl()
{
    // Stop editing before destroying the control to remove any event handlers
    // which are added when editing started: if we didn't do this, the base
    // class dtor would assert as it checks for any leftover handlers.
    if ( m_treeview )
    {
        GtkTreeViewColumn *col;
        gtk_tree_view_get_cursor(GTK_TREE_VIEW(m_treeview), NULL, &col);

        wxDataViewColumn * const wxcol = FromGTKColumn(col);
        if ( wxcol )
        {
            // This won't do anything if we're not editing it
            wxcol->GetRenderer()->CancelEditing();
        }
    }

    m_cols.Clear();

    delete m_internal;
}

void wxDataViewCtrl::Init()
{
    m_internal = NULL;

    m_cols.DeleteContents( true );

    m_uniformRowHeight = -1;
}

bool wxDataViewCtrl::Create(wxWindow *parent,
                            wxWindowID id,
                            const wxPoint& pos,
                            const wxSize& size,
                            long style,
                            const wxValidator& validator,
                            const wxString& name)
{
    if (!PreCreation( parent, pos, size ) ||
        !CreateBase( parent, id, pos, size, style, validator, name ))
    {
        wxFAIL_MSG( wxT("wxDataViewCtrl creation failed") );
        return false;
    }

    m_widget = gtk_scrolled_window_new (NULL, NULL);
    g_object_ref(m_widget);

    GTKScrolledWindowSetBorder(m_widget, style);

    m_treeview = gtk_tree_view_new();
    gtk_container_add (GTK_CONTAINER (m_widget), m_treeview);

    g_signal_connect (m_treeview, "size_allocate",
                     G_CALLBACK (gtk_dataviewctrl_size_callback), this);

#ifdef __WXGTK26__
    if (!gtk_check_version(2,6,0))
    {
        bool fixed = (style & wxDV_VARIABLE_LINE_HEIGHT) == 0;
        gtk_tree_view_set_fixed_height_mode( GTK_TREE_VIEW(m_treeview), fixed );
    }
#endif

    if (style & wxDV_MULTIPLE)
    {
        GtkTreeSelection *selection = gtk_tree_view_get_selection( GTK_TREE_VIEW(m_treeview) );
        gtk_tree_selection_set_mode( selection, GTK_SELECTION_MULTIPLE );
    }

    gtk_tree_view_set_headers_visible( GTK_TREE_VIEW(m_treeview), (style & wxDV_NO_HEADER) == 0 );

#ifdef __WXGTK210__
    if (!gtk_check_version(2,10,0))
    {
        GtkTreeViewGridLines grid = GTK_TREE_VIEW_GRID_LINES_NONE;

        if ((style & wxDV_HORIZ_RULES) != 0 &&
            (style & wxDV_VERT_RULES) != 0)
            grid = GTK_TREE_VIEW_GRID_LINES_BOTH;
        else if (style & wxDV_VERT_RULES)
            grid = GTK_TREE_VIEW_GRID_LINES_VERTICAL;
        else if (style & wxDV_HORIZ_RULES)
            grid = GTK_TREE_VIEW_GRID_LINES_HORIZONTAL;

        if (grid != GTK_TREE_VIEW_GRID_LINES_NONE)
            gtk_tree_view_set_grid_lines( GTK_TREE_VIEW(m_treeview), grid );
    }
#endif

    gtk_tree_view_set_rules_hint( GTK_TREE_VIEW(m_treeview), (style & wxDV_ROW_LINES) != 0 );

    gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (m_widget),
        GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_widget_show (m_treeview);

    m_parent->DoAddChild( this );

    PostCreation(size);

    GtkEnableSelectionEvents();

    g_signal_connect_after (m_treeview, "row-activated",
                            G_CALLBACK (wxdataview_row_activated_callback), this);

    g_signal_connect (m_treeview, "test-collapse-row",
                            G_CALLBACK (wxdataview_test_collapse_row_callback), this);

    g_signal_connect_after (m_treeview, "row-collapsed",
                            G_CALLBACK (wxdataview_row_collapsed_callback), this);

    g_signal_connect (m_treeview, "test-expand-row",
                            G_CALLBACK (wxdataview_test_expand_row_callback), this);

    g_signal_connect_after (m_treeview, "row-expanded",
                            G_CALLBACK (wxdataview_row_expanded_callback), this);

    g_signal_connect (m_treeview, "motion_notify_event",
                      G_CALLBACK (gtk_dataview_motion_notify_callback), this);

    g_signal_connect (m_treeview, "button_press_event",
                      G_CALLBACK (gtk_dataview_button_press_callback), this);

    return true;
}

wxDataViewItem wxDataViewCtrl::GTKPathToItem(GtkTreePath *path) const
{
    GtkTreeIter iter;
    return wxDataViewItem(path && m_internal->get_iter(&iter, path)
                            ? iter.user_data
                            : NULL);
}

void wxDataViewCtrl::OnInternalIdle()
{
    wxWindow::OnInternalIdle();

    m_internal->OnInternalIdle();

    unsigned int cols = GetColumnCount();
    unsigned int i;
    for (i = 0; i < cols; i++)
    {
        wxDataViewColumn *col = GetColumn( i );
        col->OnInternalIdle();
    }

    if (m_ensureVisibleDefered.IsOk())
    {
        ExpandAncestors(m_ensureVisibleDefered);
        GtkTreeIter iter;
        iter.user_data = (gpointer) m_ensureVisibleDefered.GetID();
        wxGtkTreePath path(m_internal->get_path( &iter ));
        gtk_tree_view_scroll_to_cell( GTK_TREE_VIEW(m_treeview), path, NULL, false, 0.0, 0.0 );
        m_ensureVisibleDefered = wxDataViewItem(0);
    }
}

bool wxDataViewCtrl::AssociateModel( wxDataViewModel *model )
{
    wxDELETE(m_internal);

    if (!wxDataViewCtrlBase::AssociateModel( model ))
        return false;

#ifdef __WXGTK26__
    if (!gtk_check_version(2,6,0))
    {
        bool fixed = (((GetWindowStyle() & wxDV_VARIABLE_LINE_HEIGHT) == 0) || (model->IsVirtualListModel()));
        gtk_tree_view_set_fixed_height_mode( GTK_TREE_VIEW(m_treeview), fixed );
    }
#endif

    m_internal = new wxDataViewCtrlInternal( this, model );

    return true;
}

bool wxDataViewCtrl::EnableDragSource( const wxDataFormat &format )
{
    return m_internal->EnableDragSource( format );
}

bool wxDataViewCtrl::EnableDropTarget( const wxDataFormat &format )
{
    return m_internal->EnableDropTarget( format );
}

bool wxDataViewCtrl::AppendColumn( wxDataViewColumn *col )
{
    if (!wxDataViewCtrlBase::AppendColumn(col))
        return false;

    m_cols.Append( col );

#ifdef __WXGTK26__
    if (!gtk_check_version(2,6,0))
    {
        if (gtk_tree_view_column_get_sizing( GTK_TREE_VIEW_COLUMN(col->GetGtkHandle()) ) !=
               GTK_TREE_VIEW_COLUMN_FIXED)
           gtk_tree_view_set_fixed_height_mode( GTK_TREE_VIEW(m_treeview), FALSE );
    }
#endif

    gtk_tree_view_append_column( GTK_TREE_VIEW(m_treeview),
                                 GTK_TREE_VIEW_COLUMN(col->GetGtkHandle()) );

    return true;
}

bool wxDataViewCtrl::PrependColumn( wxDataViewColumn *col )
{
    if (!wxDataViewCtrlBase::PrependColumn(col))
        return false;

    m_cols.Insert( col );

#ifdef __WXGTK26__
    if (!gtk_check_version(2,6,0))
    {
        if (gtk_tree_view_column_get_sizing( GTK_TREE_VIEW_COLUMN(col->GetGtkHandle()) ) !=
               GTK_TREE_VIEW_COLUMN_FIXED)
           gtk_tree_view_set_fixed_height_mode( GTK_TREE_VIEW(m_treeview), FALSE );
    }
#endif

    gtk_tree_view_insert_column( GTK_TREE_VIEW(m_treeview),
                                 GTK_TREE_VIEW_COLUMN(col->GetGtkHandle()), 0 );

    return true;
}

bool wxDataViewCtrl::InsertColumn( unsigned int pos, wxDataViewColumn *col )
{
    if (!wxDataViewCtrlBase::InsertColumn(pos,col))
        return false;

    m_cols.Insert( pos, col );

#ifdef __WXGTK26__
    if (!gtk_check_version(2,6,0))
    {
        if (gtk_tree_view_column_get_sizing( GTK_TREE_VIEW_COLUMN(col->GetGtkHandle()) ) !=
               GTK_TREE_VIEW_COLUMN_FIXED)
           gtk_tree_view_set_fixed_height_mode( GTK_TREE_VIEW(m_treeview), FALSE );
    }
#endif

    gtk_tree_view_insert_column( GTK_TREE_VIEW(m_treeview),
                                 GTK_TREE_VIEW_COLUMN(col->GetGtkHandle()), pos );

    return true;
}

unsigned int wxDataViewCtrl::GetColumnCount() const
{
    return m_cols.GetCount();
}

wxDataViewColumn* wxDataViewCtrl::FromGTKColumn(GtkTreeViewColumn *gtk_col) const
{
    if ( !gtk_col )
        return NULL;

    wxDataViewColumnList::const_iterator iter;
    for (iter = m_cols.begin(); iter != m_cols.end(); ++iter)
    {
        wxDataViewColumn *col = *iter;
        if (GTK_TREE_VIEW_COLUMN(col->GetGtkHandle()) == gtk_col)
        {
            return col;
        }
    }

    wxFAIL_MSG( "No matching column?" );

    return NULL;
}

wxDataViewColumn* wxDataViewCtrl::GetColumn( unsigned int pos ) const
{
    GtkTreeViewColumn *gtk_col = gtk_tree_view_get_column( GTK_TREE_VIEW(m_treeview), pos );

    return FromGTKColumn(gtk_col);
}

bool wxDataViewCtrl::DeleteColumn( wxDataViewColumn *column )
{
    gtk_tree_view_remove_column( GTK_TREE_VIEW(m_treeview),
                                 GTK_TREE_VIEW_COLUMN(column->GetGtkHandle()) );

    m_cols.DeleteObject( column );

    return true;
}

bool wxDataViewCtrl::ClearColumns()
{
    wxDataViewColumnList::iterator iter;
    for (iter = m_cols.begin(); iter != m_cols.end(); ++iter)
    {
        wxDataViewColumn *col = *iter;
        gtk_tree_view_remove_column( GTK_TREE_VIEW(m_treeview),
                                     GTK_TREE_VIEW_COLUMN(col->GetGtkHandle()) );
    }

    m_cols.Clear();

    return true;
}

int wxDataViewCtrl::GetColumnPosition( const wxDataViewColumn *column ) const
{
    GtkTreeViewColumn *gtk_column = GTK_TREE_VIEW_COLUMN(column->GetGtkHandle());

    GList *list = gtk_tree_view_get_columns( GTK_TREE_VIEW(m_treeview) );

    gint pos = g_list_index( list, (gconstpointer)  gtk_column );

    g_list_free( list );

    return pos;
}

wxDataViewColumn *wxDataViewCtrl::GetSortingColumn() const
{
    return m_internal->GetDataViewSortColumn();
}

void wxDataViewCtrl::Expand( const wxDataViewItem & item )
{
    GtkTreeIter iter;
    iter.user_data = item.GetID();
    wxGtkTreePath path(m_internal->get_path( &iter ));
    gtk_tree_view_expand_row( GTK_TREE_VIEW(m_treeview), path, false );
}

void wxDataViewCtrl::Collapse( const wxDataViewItem & item )
{
    GtkTreeIter iter;
    iter.user_data = item.GetID();
    wxGtkTreePath path(m_internal->get_path( &iter ));
    gtk_tree_view_collapse_row( GTK_TREE_VIEW(m_treeview), path );
}

bool wxDataViewCtrl::IsExpanded( const wxDataViewItem & item ) const
{
    GtkTreeIter iter;
    iter.user_data = item.GetID();
    wxGtkTreePath path(m_internal->get_path( &iter ));
    return gtk_tree_view_row_expanded( GTK_TREE_VIEW(m_treeview), path );
}

wxDataViewItem wxDataViewCtrl::DoGetCurrentItem() const
{
    // The tree doesn't have any current item if it hadn't been created yet but
    // it's arguably not an error to call this function in this case so just
    // return an invalid item without asserting.
    if ( !m_treeview )
        return wxDataViewItem();

    wxGtkTreePath path;
    gtk_tree_view_get_cursor(GTK_TREE_VIEW(m_treeview), path.ByRef(), NULL);

    return GTKPathToItem(path);
}

void wxDataViewCtrl::DoSetCurrentItem(const wxDataViewItem& item)
{
    wxCHECK_RET( m_treeview,
                 "Current item can't be set before creating the control." );

    // We need to make sure the model knows about this item or the path would
    // be invalid and gtk_tree_view_set_cursor() would silently do nothing.
    ExpandAncestors(item);

    // We also need to preserve the existing selection from changing.
    // Unfortunately the only way to do it seems to use our own selection
    // function and forbid any selection changes during set cursor call.
    wxGtkTreeSelectionLock
        lock(gtk_tree_view_get_selection(GTK_TREE_VIEW(m_treeview)));

    // Do move the cursor now.
    GtkTreeIter iter;
    iter.user_data = item.GetID();
    wxGtkTreePath path(m_internal->get_path( &iter ));

    gtk_tree_view_set_cursor(GTK_TREE_VIEW(m_treeview), path, NULL, FALSE);
}

void wxDataViewCtrl::StartEditor(const wxDataViewItem& item, unsigned int column)
{
    wxCHECK_RET( m_treeview,
                 "Current item can't be set before creating the control." );

    // We need to make sure the model knows about this item or the path would
    // be invalid and gtk_tree_view_set_cursor() would silently do nothing.
    ExpandAncestors(item);
    
    wxDataViewColumn *dvcolumn = GetColumn(column);
    wxASSERT_MSG(dvcolumn, "Could not retrieve column");
    GtkTreeViewColumn *gcolumn = GTK_TREE_VIEW_COLUMN(dvcolumn->GetGtkHandle());

    // We also need to preserve the existing selection from changing.
    // Unfortunately the only way to do it seems to use our own selection
    // function and forbid any selection changes during set cursor call.
    wxGtkTreeSelectionLock
        lock(gtk_tree_view_get_selection(GTK_TREE_VIEW(m_treeview)));

    // Do move the cursor now.
    GtkTreeIter iter;
    iter.user_data = item.GetID();
    wxGtkTreePath path(m_internal->get_path( &iter ));

    gtk_tree_view_set_cursor(GTK_TREE_VIEW(m_treeview), path, gcolumn, TRUE);
}

wxDataViewItem wxDataViewCtrl::GetSelection() const
{
    GtkTreeSelection *selection = gtk_tree_view_get_selection( GTK_TREE_VIEW(m_treeview) );

    if (m_windowStyle & wxDV_MULTIPLE)
    {
        // Report the first one
        GtkTreeModel *model;
        GList *list = gtk_tree_selection_get_selected_rows( selection, &model );

        if (list)
        {
            GtkTreePath *path = (GtkTreePath*) list->data;
            wxDataViewItem item(GTKPathToItem(path));

            // delete list
            g_list_foreach( list, (GFunc) gtk_tree_path_free, NULL );
            g_list_free( list );

            return item;
        }
    }
    else
    {
        GtkTreeIter iter;
        if (gtk_tree_selection_get_selected( selection, NULL, &iter ))
        {
            wxDataViewItem item( iter.user_data );
            return item;
        }
    }

    return wxDataViewItem(0);
}

int wxDataViewCtrl::GetSelections( wxDataViewItemArray & sel ) const
{
    sel.Clear();

    GtkTreeSelection *selection = gtk_tree_view_get_selection( GTK_TREE_VIEW(m_treeview) );
    if (HasFlag(wxDV_MULTIPLE))
    {
        GtkTreeModel *model;
        GList *list = gtk_tree_selection_get_selected_rows( selection, &model );

        int count = 0;
        while (list)
        {
            GtkTreePath *path = (GtkTreePath*) list->data;

            sel.Add(GTKPathToItem(path));

            list = g_list_next( list );
            count++;
        }

        // delete list
        g_list_foreach( list, (GFunc) gtk_tree_path_free, NULL );
        g_list_free( list );

        return count;
    }
    else
    {
        GtkTreeModel *model;
        GtkTreeIter iter;
        gboolean has_selection = gtk_tree_selection_get_selected( selection, &model, &iter );
        if (has_selection)
        {
            sel.Add( wxDataViewItem( (void*) iter.user_data) );
            return 1;
        }
    }

    return 0;
}

void wxDataViewCtrl::SetSelections( const wxDataViewItemArray & sel )
{
    GtkDisableSelectionEvents();

    GtkTreeSelection *selection = gtk_tree_view_get_selection( GTK_TREE_VIEW(m_treeview) );

    gtk_tree_selection_unselect_all( selection );

    wxDataViewItem last_parent;

    size_t i;
    for (i = 0; i < sel.GetCount(); i++)
    {
        wxDataViewItem item = sel[i];
        wxDataViewItem parent = GetModel()->GetParent( item );
        if (parent)
        {
            if (parent != last_parent)
                ExpandAncestors(item);
        }
        last_parent = parent;

        GtkTreeIter iter;
        iter.stamp = m_internal->GetGtkModel()->stamp;
        iter.user_data = (gpointer) item.GetID();
        gtk_tree_selection_select_iter( selection, &iter );
    }

    GtkEnableSelectionEvents();
}

void wxDataViewCtrl::Select( const wxDataViewItem & item )
{
    ExpandAncestors(item);

    GtkDisableSelectionEvents();

    GtkTreeSelection *selection = gtk_tree_view_get_selection( GTK_TREE_VIEW(m_treeview) );

    GtkTreeIter iter;
    iter.stamp = m_internal->GetGtkModel()->stamp;
    iter.user_data = (gpointer) item.GetID();
    gtk_tree_selection_select_iter( selection, &iter );

    GtkEnableSelectionEvents();
}

void wxDataViewCtrl::Unselect( const wxDataViewItem & item )
{
    GtkDisableSelectionEvents();

    GtkTreeSelection *selection = gtk_tree_view_get_selection( GTK_TREE_VIEW(m_treeview) );

    GtkTreeIter iter;
    iter.stamp = m_internal->GetGtkModel()->stamp;
    iter.user_data = (gpointer) item.GetID();
    gtk_tree_selection_unselect_iter( selection, &iter );

    GtkEnableSelectionEvents();
}

bool wxDataViewCtrl::IsSelected( const wxDataViewItem & item ) const
{
    GtkTreeSelection *selection = gtk_tree_view_get_selection( GTK_TREE_VIEW(m_treeview) );

    GtkTreeIter iter;
    iter.stamp = m_internal->GetGtkModel()->stamp;
    iter.user_data = (gpointer) item.GetID();

    return gtk_tree_selection_iter_is_selected( selection, &iter );
}

void wxDataViewCtrl::SelectAll()
{
    GtkDisableSelectionEvents();

    GtkTreeSelection *selection = gtk_tree_view_get_selection( GTK_TREE_VIEW(m_treeview) );

    gtk_tree_selection_select_all( selection );

    GtkEnableSelectionEvents();
}

void wxDataViewCtrl::UnselectAll()
{
    GtkDisableSelectionEvents();

    GtkTreeSelection *selection = gtk_tree_view_get_selection( GTK_TREE_VIEW(m_treeview) );

    gtk_tree_selection_unselect_all( selection );

    GtkEnableSelectionEvents();
}

void wxDataViewCtrl::EnsureVisible(const wxDataViewItem& item,
                                   const wxDataViewColumn *WXUNUSED(column))
{
    m_ensureVisibleDefered = item;
    ExpandAncestors(item);

    GtkTreeIter iter;
    iter.user_data = (gpointer) item.GetID();
    wxGtkTreePath path(m_internal->get_path( &iter ));
    gtk_tree_view_scroll_to_cell( GTK_TREE_VIEW(m_treeview), path, NULL, false, 0.0, 0.0 );
}

void wxDataViewCtrl::HitTest(const wxPoint& point,
                             wxDataViewItem& item,
                             wxDataViewColumn *& column) const
{
    // gtk_tree_view_get_dest_row_at_pos() is the right one. But it does not tell the column.
    // gtk_tree_view_get_path_at_pos() is the wrong function. It doesn't mind the header but returns column.
    // See http://mail.gnome.org/archives/gtkmm-list/2005-January/msg00080.html
    // So we have to use both of them.
    // Friedrich Haase 2010-9-20
    wxGtkTreePath path, pathScratch;
    GtkTreeViewColumn* GtkColumn = NULL;
    GtkTreeViewDropPosition pos = GTK_TREE_VIEW_DROP_INTO_OR_AFTER;
    gint cell_x = 0;
    gint cell_y = 0;
    
    // cannot directly call GtkGetTreeView(), HitTest is const and so is this pointer
    wxDataViewCtrl* ctrl = (wxDataViewCtrl*)this; // ugly workaround, ctrl is NOT const
    GtkTreeView* treeView = GTK_TREE_VIEW(ctrl->GtkGetTreeView());
    
    // is there possibly a better suited function to get the column?
    gtk_tree_view_get_path_at_pos(                // and this is the wrong call but it delivers the column
      treeView,
      (int) point.x, (int) point.y,
      pathScratch.ByRef(),
      &GtkColumn,                                 // here we get the GtkColumn
      &cell_x,
      &cell_y );
      
    if ( GtkColumn != NULL )
    {                                             
        // we got GTK column
        // the right call now which takes the header into account
        gtk_tree_view_get_dest_row_at_pos( treeView, (int) point.x, (int) point.y, path.ByRef(), &pos);
          
        if (path)
            item = wxDataViewItem(GTKPathToItem(path));
        // else we got a GTK column but the position is not over an item, e.g. below last item
        for ( unsigned int i=0, cols=GetColumnCount(); i<cols; ++i )  // search the wx column
        {
            wxDataViewColumn* col = GetColumn(i);
            if ( GTK_TREE_VIEW_COLUMN(col->GetGtkHandle()) == GtkColumn )
            {
                column = col;                      // here we get the wx column
                break;
            }
        }
    }
    // else no column and thus no item, both null
}

wxRect
wxDataViewCtrl::GetItemRect(const wxDataViewItem& WXUNUSED(item),
                            const wxDataViewColumn *WXUNUSED(column)) const
{
    return wxRect();
}

bool wxDataViewCtrl::SetRowHeight(int rowHeight)
{
    m_uniformRowHeight = rowHeight;
    return true;
}

void wxDataViewCtrl::DoSetExpanderColumn()
{
    gtk_tree_view_set_expander_column( GTK_TREE_VIEW(m_treeview),
        GTK_TREE_VIEW_COLUMN( GetExpanderColumn()->GetGtkHandle() ) );
}

void wxDataViewCtrl::DoSetIndent()
{
}

void wxDataViewCtrl::GtkDisableSelectionEvents()
{
    GtkTreeSelection *selection = gtk_tree_view_get_selection( GTK_TREE_VIEW(m_treeview) );
    g_signal_handlers_disconnect_by_func( selection,
                            (gpointer) (wxdataview_selection_changed_callback), this);
}

void wxDataViewCtrl::GtkEnableSelectionEvents()
{
    GtkTreeSelection *selection = gtk_tree_view_get_selection( GTK_TREE_VIEW(m_treeview) );
    g_signal_connect_after (selection, "changed",
                            G_CALLBACK (wxdataview_selection_changed_callback), this);
}

// ----------------------------------------------------------------------------
// visual attributes stuff
// ----------------------------------------------------------------------------

// static
wxVisualAttributes
wxDataViewCtrl::GetClassDefaultAttributes(wxWindowVariant WXUNUSED(variant))
{
    return GetDefaultAttributesFromGTKWidget(gtk_tree_view_new);
}

void wxDataViewCtrl::DoApplyWidgetStyle(GtkRcStyle *style)
{
    wxDataViewCtrlBase::DoApplyWidgetStyle(style);
    gtk_widget_modify_style(m_treeview, style);
}

#endif // !wxUSE_GENERICDATAVIEWCTRL

#endif // wxUSE_DATAVIEWCTRL
