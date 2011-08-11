/*
	This file is part of lgob.

	lgob is free software: you can redistribute it and/or modify
	it under the terms of the GNU Lesser General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	lgob is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU Lesser General Public License for more details.

	You should have received a copy of the GNU Lesser General Public License
	along with lgob.  If not, see <http://www.gnu.org/licenses/>.

    Copyright (C) 2008 - 2010 Lucas Hermann Negri
*/

#define LGOB_TYPE_PROXY_LIST			(lgob_proxy_list_get_type())
#define LGOB_PROXY_LIST(obj)			(G_TYPE_CHECK_INSTANCE_CAST((obj), LGOB_TYPE_PROXY_LIST, LgobProxyList))
#define LGOB_PROXY_LIST_CLASS(klass)	(G_TYPE_CHECK_CLASS_CAST ((klass), LGOB_TYPE_PROXY_LIST, LgobProxyListClass))
#define LGOB_IS_PROXY_LIST(obj)			(G_TYPE_CHECK_INSTANCE_TYPE ((obj), LGOB_TYPE_PROXY_LIST))
#define LGOB_IS_PROXY_LIST_CLASS		(klass)(G_TYPE_CHECK_CLASS_TYPE((klass), LGOB_TYPE_PROXY_LIST))
#define LGOB_PROXY_LIST_GET_CLASS(obj)	(G_TYPE_INSTANCE_GET_CLASS ((obj), LGOB_TYPE_PROXY_LIST, LgobProxyListClass))

typedef struct _LgobProxyList       LgobProxyList;
typedef struct _LgobProxyListClass  LgobProxyListClass;

struct _LgobProxyList
{
	/* Std */
	GObject parent;
	gint stamp;
	
	/* Columns */
	gint n_columns;
	GType* column_types;
	
	/* Lua */
	lua_State *L;
	int table;
};

struct _LgobProxyListClass
{
	GObjectClass parent_class;
};

GType lgob_proxy_list_get_type();

static void lgob_proxy_list_init(LgobProxyList* model);

static void lgob_proxy_list_class_init(LgobProxyListClass* klass);

static void lgob_proxy_list_tree_model_init(GtkTreeModelIface* iface);

static void lgob_proxy_list_finalize(GObject* object);

static GtkTreeModelFlags lgob_proxy_list_get_flags(GtkTreeModel* model);

static gint lgob_proxy_list_get_n_columns(GtkTreeModel* model);

static GType lgob_proxy_list_get_column_type(GtkTreeModel* model, gint index);

static gboolean lgob_proxy_list_get_iter(GtkTreeModel* model, GtkTreeIter *iter,
	GtkTreePath* path);
	
static GtkTreePath* lgob_proxy_list_get_path(GtkTreeModel* model, GtkTreeIter* iter);

static void lgob_proxy_list_get_value(GtkTreeModel* model, GtkTreeIter* iter,
	gint column, GValue* value);
	
static gboolean lgob_proxy_list_iter_next(GtkTreeModel* model, GtkTreeIter* iter);

static gboolean lgob_proxy_list_iter_children(GtkTreeModel* model, GtkTreeIter* iter,
	GtkTreeIter* parent);
	
static gboolean lgob_proxy_list_iter_has_child(GtkTreeModel* model,	GtkTreeIter* iter);

static gint lgob_proxy_list_iter_n_children(GtkTreeModel* model, GtkTreeIter* iter);

static gboolean lgob_proxy_list_iter_nth_child(GtkTreeModel* model, GtkTreeIter* iter,
	GtkTreeIter* parent, gint n);
	
static gboolean lgob_proxy_list_iter_parent(GtkTreeModel* model, GtkTreeIter* iter,
	GtkTreeIter* child);

static GObjectClass* parent_class = NULL;

GType lgob_proxy_list_get_type()
{
	static GType lgob_proxy_list_type = 0;

	if(lgob_proxy_list_type == 0)
	{
    	static const GTypeInfo lgob_proxy_list_info =
		{
			sizeof(LgobProxyListClass),
			NULL,									/* base_init */
			NULL,									/* base_finalize */
			(GClassInitFunc)lgob_proxy_list_class_init,
			NULL,									/* class finalize */
			NULL,									/* class_data */
			sizeof(LgobProxyList),
			0,										/* n_preallocs */
			(GInstanceInitFunc)lgob_proxy_list_init
    	};
	
		static const GInterfaceInfo tree_model_info =
		{
			(GInterfaceInitFunc)lgob_proxy_list_tree_model_init,
			NULL,
			NULL
		};
		/* Gtk prefix needed to simplify class resolution */
		lgob_proxy_list_type = g_type_register_static(G_TYPE_OBJECT, "GtkProxyList",
			&lgob_proxy_list_info, (GTypeFlags)0);
		
		g_type_add_interface_static(lgob_proxy_list_type, GTK_TYPE_TREE_MODEL, &tree_model_info);
	}

	return lgob_proxy_list_type;
}

static void lgob_proxy_list_class_init(LgobProxyListClass* klass)
{
	GObjectClass* object_class;

	parent_class = (GObjectClass*)g_type_class_peek_parent(klass);
	object_class = (GObjectClass*)klass;

	object_class->finalize = lgob_proxy_list_finalize;
}

static void lgob_proxy_list_tree_model_init(GtkTreeModelIface* iface)
{
	iface->get_flags       = lgob_proxy_list_get_flags;
	iface->get_n_columns   = lgob_proxy_list_get_n_columns;
	iface->get_column_type = lgob_proxy_list_get_column_type;
	iface->get_iter        = lgob_proxy_list_get_iter;
	iface->get_path        = lgob_proxy_list_get_path;
	iface->get_value       = lgob_proxy_list_get_value;
	iface->iter_next       = lgob_proxy_list_iter_next;
	iface->iter_children   = lgob_proxy_list_iter_children;
	iface->iter_has_child  = lgob_proxy_list_iter_has_child;
	iface->iter_n_children = lgob_proxy_list_iter_n_children;
	iface->iter_nth_child  = lgob_proxy_list_iter_nth_child;
	iface->iter_parent     = lgob_proxy_list_iter_parent;
}

static void lgob_proxy_list_init(LgobProxyList* proxy_list)
{
	proxy_list->L = NULL;
	proxy_list->table = LUA_REFNIL;
	proxy_list->stamp = g_random_int();
}

static void lgob_proxy_list_finalize(GObject *object)
{
	LgobProxyList* list = LGOB_PROXY_LIST(object); 
	luaL_unref(list->L, LUA_REGISTRYINDEX, list->table);
  	list->table = LUA_REFNIL;
	g_free(list->column_types);

	(*parent_class->finalize)(object);
}

static GtkTreeModelFlags lgob_proxy_list_get_flags(GtkTreeModel* model)
{
	return (GTK_TREE_MODEL_LIST_ONLY | GTK_TREE_MODEL_ITERS_PERSIST);
}

void lgob_proxy_list_get_method(LgobProxyList* list, const char* name)
{
	#ifdef IDEBUG
	g_debug("Getting method %s!", name);
	#endif
	
	/* Get the function */
	lua_rawgeti(list->L, LUA_REGISTRYINDEX, list->table);
    lua_getfield(list->L, -1, name);
	
	if(!lua_isfunction(list->L, -1))
		luaL_error(list->L, "The method %s is required", name);
	
	/* Push the "self" */
	lua_pushvalue(list->L, -2);
}

static gint lgob_proxy_list_get_n_columns(GtkTreeModel* model)
{
	LgobProxyList* list = LGOB_PROXY_LIST(model);
	return list->n_columns;
}

static GType lgob_proxy_list_get_column_type(GtkTreeModel* model, gint index)
{
	return G_TYPE_STRING;
}

static gboolean lgob_proxy_list_get_iter(GtkTreeModel* model, GtkTreeIter* iter,
	GtkTreePath* path)
{
	LgobProxyList* proxy_list = LGOB_PROXY_LIST(model);
	
	gint* indices = gtk_tree_path_get_indices(path);
	iter->stamp = proxy_list->stamp;
	iter->user_data = GINT_TO_POINTER(indices[0]);
	iter->user_data2 = iter->user_data3 = NULL;
	
	return TRUE;
}

static GtkTreePath* lgob_proxy_list_get_path(GtkTreeModel* model, GtkTreeIter* iter)
{
	GtkTreePath* path = gtk_tree_path_new();
    gtk_tree_path_append_index(path, GPOINTER_TO_INT(iter->user_data));
		
	return path;
}

static void lgob_proxy_list_get_value(GtkTreeModel* model, GtkTreeIter* iter,
	gint column, GValue* value)
{
	LgobProxyList* proxy_list = LGOB_PROXY_LIST(model);
	lua_State *L = proxy_list->L;

	int top = lua_gettop(L);
    lgob_proxy_list_get_method(proxy_list, "get_value");
	lua_pushnumber(L, GPOINTER_TO_INT(iter->user_data));
    lua_pushnumber(L, column);
	lua_call(L, 3, 1);
	 
    /* Fill the GValue */
	lua_getfield(L, LUA_REGISTRYINDEX, "lgobValueSet");
	g_value_init(value, proxy_list->column_types[column]);
	lua_pushlightuserdata(L, value);
	lua_pushvalue(L, -3); /* push the returned value from getValue */
	lua_call(L, 2, 0);
	
    lua_settop(L, top);
}

static gboolean lgob_proxy_list_iter_next (GtkTreeModel* model, GtkTreeIter* iter)
{
	LgobProxyList* proxy_list = LGOB_PROXY_LIST(model);
	lua_State* L = proxy_list->L;
	gboolean ok;
	
	int top = lua_gettop(L);
	
    lgob_proxy_list_get_method(proxy_list, "iter_next");
	gint current = GPOINTER_TO_INT(iter->user_data);
  	lua_pushinteger(L, current);
	lua_call(L, 2, 1);
	 
    if(lua_isboolean(L, -1) && lua_toboolean(L, -1))
	{
		iter->stamp = proxy_list->stamp;
		iter->user_data = GINT_TO_POINTER(current + 1);
		ok = TRUE;
    }
	else
		ok = FALSE;
		
	lua_settop(L, top);
	
	return ok;
}

static gboolean lgob_proxy_list_iter_children (GtkTreeModel* model, GtkTreeIter* iter,
	GtkTreeIter* parent)
{
	if(parent)
		return FALSE;
	else
	{
		LgobProxyList* list = LGOB_PROXY_LIST(model);
		iter->stamp = list->stamp;
		iter->user_data = GINT_TO_POINTER(1);
		return TRUE;
	}
}

static gboolean lgob_proxy_list_iter_has_child (GtkTreeModel* model, GtkTreeIter* iter)
{
	return FALSE;
}

static gint lgob_proxy_list_iter_n_children(GtkTreeModel* model, GtkTreeIter* iter)
{
	if(!iter)
    	return lgob_proxy_list_get_n_columns(model);
	
	return 0;
}

static gboolean lgob_proxy_list_iter_nth_child (GtkTreeModel* model, GtkTreeIter* iter,
	GtkTreeIter* parent, gint n)
{
	if(parent)
		return FALSE;
	else
	{
		LgobProxyList* proxy_list = LGOB_PROXY_LIST(model);
		iter->stamp = proxy_list->stamp;
		iter->user_data = GINT_TO_POINTER(n);
		return TRUE;
	}
}

static gboolean lgob_proxy_list_iter_parent (GtkTreeModel* model, GtkTreeIter* iter,
	GtkTreeIter* child)
{
	return FALSE;
}

static int lgob_proxy_list_new(lua_State* L)
{
	luaL_checktype(L, 1, LUA_TTABLE);
	
	int n_columns = lua_gettop(L) - 1, i;
	GType* types = g_malloc(sizeof(GType) * n_columns);

	/* Fill the types array */
	for(i = 1; i <= n_columns; i++)
	{
		types[i - 1] = g_type_from_name(luaL_checkstring(L, i + 1));
	}

	LgobProxyList* ptr = g_object_new(LGOB_TYPE_PROXY_LIST, NULL);
	ptr->L = L;
	ptr->n_columns = n_columns;
	ptr->column_types = types;
	
	lua_pushvalue(L, 1);
	ptr->table = luaL_ref(L, LUA_REGISTRYINDEX);
	
	object_new(L, ptr, TRUE);
	return 1;
}

static int lgob_proxy_list_set_value(lua_State* L)
{
	luaL_checktype(L, 1, LUA_TUSERDATA);
	luaL_checktype(L, 2, LUA_TUSERDATA);
	luaL_checktype(L, 3, LUA_TNUMBER);

	Object* obj = lua_touserdata(L, 1);
	Object* stc = lua_touserdata(L, 2);
	
	lgob_proxy_list_get_method(LGOB_PROXY_LIST(obj->pointer), "set_value");
	lua_pushnumber(L, GPOINTER_TO_INT( ((GtkTreeIter*)stc->pointer)->user_data) );
    lua_pushnumber(L, lua_tointeger(L, 3));
    lua_pushvalue(L, 4);
	lua_call(L, 4, 0);
	
	return 0;
}

static const struct luaL_reg proxylist [] =
{
	{"new", lgob_proxy_list_new},
	{"set_value", lgob_proxy_list_set_value},
	{NULL, NULL}
};
