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
    
    Copyright (C) 2009 - 2010 Lucas Hermann Negri
*/

#ifndef LGOB_GOBJECT_TYPES
#define LGOB_GOBJECT_TYPES

/**
 * GTypes for internal use. Helps table convertions.
 */
typedef enum
{
	TYPE_STRING, TYPE_DOUBLE, TYPE_BOOLEAN, TYPE_ENUM,
	TYPE_STRUCT, TYPE_OBJECT, TYPE_CUSTOM
} Type;

/**
 * Boxed value. Some GBoxed needs to be around until its used, and this
 * structure is used to store information to free then on GC.
 */
typedef struct
{
	gpointer pointer;
	GType type;
} Boxed;

/**
 * Generic info for the handler.
 */
typedef struct
{
	const GType* param_types;
	GType return_type;
	guint n_params;
} SignalInfo;

/**
 * Callback data.
 * Contains the lua_State to use and the reference to the function to call,
 * and all the info that the handler must use.
 */
typedef struct
{
	lua_State* L;
	SignalInfo signal_info;
	int function_ref;
	int ud_ref;
} Data;

#endif
