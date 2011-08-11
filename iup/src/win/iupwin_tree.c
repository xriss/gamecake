/** \file
 * \brief Tree Control
 *
 * See Copyright Notice in iup.h
 */
#undef NOTREEVIEW
#include <windows.h>
#include <commctrl.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <memory.h>
#include <stdarg.h>

#include "iup.h"
#include "iupcbs.h"

#include "iup_object.h"
#include "iup_attrib.h"
#include "iup_str.h"
#include "iup_drv.h"
#include "iup_drvinfo.h"
#include "iup_stdcontrols.h"
#include "iup_tree.h"
#include "iup_image.h"
#include "iup_array.h"

#include "iupwin_drv.h"
#include "iupwin_handle.h"
#include "iupwin_draw.h"
#include "iupwin_info.h"



/* Not defined for Cygwin and MingW */
#ifndef TVN_ITEMCHANGING                          
typedef struct tagNMTVITEMCHANGE {
    NMHDR hdr;
    UINT uChanged;
    HTREEITEM hItem;
    UINT uStateNew;
    UINT uStateOld;
    LPARAM lParam;
} NMTVITEMCHANGE;
#define TVN_ITEMCHANGINGA        (TVN_FIRST-16)    
#define TVN_ITEMCHANGINGW        (TVN_FIRST-17)    
#endif

#ifndef TVS_EX_DOUBLEBUFFER
#define TVS_EX_DOUBLEBUFFER         0x0004
#endif

#ifndef TTM_POPUP
#define TTM_POPUP               (WM_USER + 34)
#endif

#ifndef TVM_SETEXTENDEDSTYLE
#define TVM_SETEXTENDEDSTYLE      (TV_FIRST + 44)
#endif
/* End Cygwin/MingW */


typedef struct _winTreeItemData
{
  COLORREF color;
  unsigned char kind;
  HFONT hFont;
  short image;
  short image_expanded;
} winTreeItemData;


static void winTreeSetFocusNode(Ihandle* ih, HTREEITEM hItem);


/*****************************************************************************/
/* FINDING ITEMS                                                             */
/*****************************************************************************/

InodeHandle* iupdrvTreeGetFocusNode(Ihandle* ih)
{
  return (InodeHandle*)SendMessage(ih->handle, TVM_GETNEXTITEM, TVGN_CARET, 0);
}

static HTREEITEM winTreeFindNodeXY(Ihandle* ih, int x, int y)
{
  TVHITTESTINFO info;
  info.pt.x = x;
  info.pt.y = y;
  return (HTREEITEM)SendMessage(ih->handle, TVM_HITTEST, 0, (LPARAM)(LPTVHITTESTINFO)&info);
}

int iupwinGetColor(const char* value, COLORREF *color)
{
  unsigned char r, g, b;
  if (iupStrToRGB(value, &r, &g, &b))
  {
    *color = RGB(r,g,b);
    return 1;
  }
  return 0;
}

static void winTreeChildCountRec(Ihandle* ih, HTREEITEM hItem, int *count)
{
  hItem = (HTREEITEM)SendMessage(ih->handle, TVM_GETNEXTITEM, TVGN_CHILD, (LPARAM)hItem);
  while(hItem != NULL)
  {
    (*count)++;

    /* go recursive to children */
    winTreeChildCountRec(ih, hItem, count);

    /* Go to next sibling item */
    hItem = (HTREEITEM)SendMessage(ih->handle, TVM_GETNEXTITEM, TVGN_NEXT, (LPARAM)hItem);
  }
}

int iupdrvTreeTotalChildCount(Ihandle* ih, HTREEITEM hItem)
{
  int count = 0;
  winTreeChildCountRec(ih, hItem, &count);
  return count;
}

static void winTreeChildRebuildCacheRec(Ihandle* ih, HTREEITEM hItem, int *id)
{
  hItem = (HTREEITEM)SendMessage(ih->handle, TVM_GETNEXTITEM, TVGN_CHILD, (LPARAM)hItem);
  while(hItem != NULL)
  {
    (*id)++;
    ih->data->node_cache[*id].node_handle = hItem;

    /* go recursive to children */
    winTreeChildRebuildCacheRec(ih, hItem, id);

    /* Go to next sibling item */
    hItem = (HTREEITEM)SendMessage(ih->handle, TVM_GETNEXTITEM, TVGN_NEXT, (LPARAM)hItem);
  }
}

static void winTreeRebuildNodeCache(Ihandle* ih, int id, HTREEITEM hItem)
{
  ih->data->node_cache[id].node_handle = hItem;
  winTreeChildRebuildCacheRec(ih, hItem, &id);
}


/*****************************************************************************/
/* ADDING ITEMS                                                              */
/*****************************************************************************/

static void winTreeExpandItem(Ihandle* ih, HTREEITEM hItem, int expand);

void iupdrvTreeAddNode(Ihandle* ih, int id, int kind, const char* title, int add)
{
  TVITEM item;
  TVINSERTSTRUCT tvins;
  HTREEITEM hPrevItem = iupTreeGetNode(ih, id);
  HTREEITEM hItemNew;
  winTreeItemData* itemData;

  /* the previous node is not necessary only
     if adding the root in an empty tree. */
  if (!hPrevItem && ih->data->node_count!=0)
      return;

  if (!title)
    title = "";

  itemData = calloc(1, sizeof(winTreeItemData));
  itemData->image = -1;
  itemData->image_expanded = -1;
  itemData->kind = (unsigned char)kind;

  ZeroMemory(&item, sizeof(TVITEM));
  item.mask = TVIF_PARAM | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_TEXT; 
  item.pszText = (char*)title;
  item.lParam = (LPARAM)itemData;

  iupwinGetColor(iupAttribGetStr(ih, "FGCOLOR"), &itemData->color);

  if (kind == ITREE_BRANCH)
    item.iSelectedImage = item.iImage = (int)ih->data->def_image_collapsed;
  else
    item.iSelectedImage = item.iImage = (int)ih->data->def_image_leaf;

  /* Save the heading level in the node's application-defined data area */
  tvins.item = item;

  if (hPrevItem)
  {
    int kindPrev;
    TVITEM tviPrevItem;

    /* get the KIND attribute of reference node */ 
    tviPrevItem.hItem = hPrevItem;
    tviPrevItem.mask = TVIF_PARAM|TVIF_CHILDREN; 
    SendMessage(ih->handle, TVM_GETITEM, 0, (LPARAM)(LPTVITEM)&tviPrevItem);
    kindPrev = ((winTreeItemData*)tviPrevItem.lParam)->kind;

    /* Define the parent and the position to the new node inside
       the list, using the KIND attribute of reference node */
    if (kindPrev == ITREE_BRANCH && add)
    {
      /* depth+1 */
      tvins.hParent = hPrevItem;
      tvins.hInsertAfter = TVI_FIRST;   /* insert the new node after the reference node, as first child */
    }
    else
    {
      /* same depth */
      tvins.hParent = (HTREEITEM)SendMessage(ih->handle, TVM_GETNEXTITEM, TVGN_PARENT, (LPARAM)hPrevItem);
      tvins.hInsertAfter = hPrevItem;   /* insert the new node after reference node */
    }

    /* Add the new node */
    hItemNew = (HTREEITEM)SendMessage(ih->handle, TVM_INSERTITEM, 0, (LPARAM)(LPTVINSERTSTRUCT)&tvins);
    iupTreeAddToCache(ih, add, kindPrev, hPrevItem, hItemNew);

    if (kindPrev == ITREE_BRANCH && tviPrevItem.cChildren==0)  /* was 0, now is 1 */
    {
      /* this is the first child, redraw to update the '+'/'-' buttons */
      if (ih->data->add_expanded)
        winTreeExpandItem(ih, hPrevItem, 1);
      else
        winTreeExpandItem(ih, hPrevItem, 0);
    }

  }
  else
  {
    tvins.hInsertAfter = TVI_FIRST;
    tvins.hParent = TVI_ROOT;

    /* Add the new node */
    hItemNew = (HTREEITEM)SendMessage(ih->handle, TVM_INSERTITEM, 0, (LPARAM)(LPTVINSERTSTRUCT)&tvins);
    iupTreeAddToCache(ih, 0, 0, NULL, hItemNew);

    if (ih->data->node_count == 1)
    {
      /* MarkStart node */
      iupAttribSetStr(ih, "_IUPTREE_MARKSTART_NODE", (char*)hItemNew);

      /* Set the default VALUE */
      winTreeSetFocusNode(ih, hItemNew);
    }
  }
}

static int winTreeIsItemExpanded(Ihandle* ih, HTREEITEM hItem)
{
  TVITEM item;
  item.hItem = hItem;
  item.mask = TVIF_HANDLE | TVIF_STATE;
  SendMessage(ih->handle, TVM_GETITEM, 0, (LPARAM)(LPTVITEM)&item);
  return (item.state & TVIS_EXPANDED) != 0;
}

static void winTreeExpandItem(Ihandle* ih, HTREEITEM hItem, int expand)
{
  TVITEM item;
  winTreeItemData* itemData;

  iupAttribSetStr(ih, "_IUPTREE_IGNORE_BRANCH_CB", "1");
  /* it only works if the branch has children */
  SendMessage(ih->handle, TVM_EXPAND, expand? TVE_EXPAND: TVE_COLLAPSE, (LPARAM)hItem);
  iupAttribSetStr(ih, "_IUPTREE_IGNORE_BRANCH_CB", NULL);

  /* update image */
  item.hItem = hItem;
  item.mask = TVIF_HANDLE|TVIF_PARAM;
  SendMessage(ih->handle, TVM_GETITEM, 0, (LPARAM)(LPTVITEM)&item);
  itemData = (winTreeItemData*)item.lParam;

  if (expand)
    item.iSelectedImage = item.iImage = (itemData->image_expanded!=-1)? itemData->image_expanded: (int)ih->data->def_image_expanded;
  else
    item.iSelectedImage = item.iImage = (itemData->image!=-1)? itemData->image: (int)ih->data->def_image_collapsed;

  item.hItem = hItem;
  item.mask = TVIF_HANDLE | TVIF_IMAGE | TVIF_SELECTEDIMAGE;
  SendMessage(ih->handle, TVM_SETITEM, 0, (LPARAM)(LPTVITEM)&item);
}

/*****************************************************************************/
/* EXPANDING AND STORING ITEMS                                               */
/*****************************************************************************/
static void winTreeExpandTree(Ihandle* ih, HTREEITEM hItem, int expand)
{
  HTREEITEM hItemChild;
  while(hItem != NULL)
  {
    hItemChild = (HTREEITEM)SendMessage(ih->handle, TVM_GETNEXTITEM, TVGN_CHILD, (LPARAM)hItem);

    /* Check whether we have child items */
    if (hItemChild)
    {
      winTreeExpandItem(ih, hItem, expand);

      /* Recursively traverse child items */
      winTreeExpandTree(ih, hItemChild, expand);
    }

    /* Go to next sibling item */
    hItem = (HTREEITEM)SendMessage(ih->handle, TVM_GETNEXTITEM, TVGN_NEXT, (LPARAM)hItem);
  }
}

/*****************************************************************************/
/* SELECTING ITEMS                                                           */
/*****************************************************************************/

static int winTreeIsNodeSelected(Ihandle* ih, HTREEITEM hItem)
{
  return ((SendMessage(ih->handle, TVM_GETITEMSTATE, (WPARAM)hItem, TVIS_SELECTED)) & TVIS_SELECTED)!=0;
}

static void winTreeSelectNode(Ihandle* ih, HTREEITEM hItem, int select)
{
  TV_ITEM item;
  item.mask = TVIF_STATE | TVIF_HANDLE;
  item.stateMask = TVIS_SELECTED;
  item.hItem = hItem;

  if (select == -1)
    select = !winTreeIsNodeSelected(ih, hItem);  /* toggle */

  item.state = select ? TVIS_SELECTED : 0;

  iupAttribSetStr(ih, "_IUPTREE_IGNORE_SELECTION_CB", "1");
  SendMessage(ih->handle, TVM_SETITEM, 0, (LPARAM)&item);
  iupAttribSetStr(ih, "_IUPTREE_IGNORE_SELECTION_CB", NULL);
}

/* ------------Comment from wxWidgets--------------------
   Helper function which tricks the standard control into changing the focused
   item without changing anything else. */
static void winTreeSetFocusNode(Ihandle* ih, HTREEITEM hItem)
{
  HTREEITEM hItemFocus = iupdrvTreeGetFocusNode(ih);
  if (hItem != hItemFocus)
  {
    /* remember the selection state of the item */
    int wasSelected = winTreeIsNodeSelected(ih, hItem);
    int wasFocusSelected = 0;

    if (iupwinIsVistaOrNew() && iupwin_comctl32ver6)
      iupAttribSetStr(ih, "_IUPTREE_ALLOW_CHANGE", (char*)hItem);  /* Vista Only */
    else
      wasFocusSelected = hItemFocus && winTreeIsNodeSelected(ih, hItemFocus);

    iupAttribSetStr(ih, "_IUPTREE_IGNORE_SELECTION_CB", "1");

    if (wasFocusSelected)
    {
      /* prevent the tree from unselecting the old focus which it would do by default */
      SendMessage(ih->handle, TVM_SELECTITEM, TVGN_CARET, (LPARAM)NULL); /* remove the focus */
      winTreeSelectNode(ih, hItemFocus, 1); /* select again */
    }

    SendMessage(ih->handle, TVM_SELECTITEM, TVGN_CARET, (LPARAM)hItem); /* set focus, selection, and unselect the previous focus */

    if (!wasSelected)
      winTreeSelectNode(ih, hItem, 0); /* need to clear the selection if was not selected */

    iupAttribSetStr(ih, "_IUPTREE_IGNORE_SELECTION_CB", NULL);
    iupAttribSetStr(ih, "_IUPTREE_ALLOW_CHANGE", NULL);
  }
}

static void winTreeSelectRange(Ihandle* ih, HTREEITEM hItem1, HTREEITEM hItem2, int clear)
{
  int i;
  int id1 = iupTreeFindNodeId(ih, hItem1);
  int id2 = iupTreeFindNodeId(ih, hItem2);
  
  if (id1 > id2)
  {
    int tmp = id1;
    id1 = id2;
    id2 = tmp;
  }

  for (i = 0; i < ih->data->node_count; i++)
  {
    if (i < id1 || i > id2)
    {
      if (clear)
        winTreeSelectNode(ih, ih->data->node_cache[i].node_handle, 0);
    }
    else
      winTreeSelectNode(ih, ih->data->node_cache[i].node_handle, 1);
  }
}

static void winTreeSelectAll(Ihandle* ih)
{
  int i;
  for (i = 0; i < ih->data->node_count; i++)
    winTreeSelectNode(ih, ih->data->node_cache[i].node_handle, 1);
}

static void winTreeClearSelection(Ihandle* ih, HTREEITEM hItemExcept)
{
  int i;
  for (i = 0; i < ih->data->node_count; i++)
  {
    if (ih->data->node_cache[i].node_handle != hItemExcept)
      winTreeSelectNode(ih, ih->data->node_cache[i].node_handle, 0);
  }
}

static int winTreeInvertSelectFunc(Ihandle* ih, HTREEITEM hItem, int id, void* userdata)
{
  (void)id;
  winTreeSelectNode(ih, hItem, -1);
  (void)userdata;
  return 1;
}

typedef struct _winTreeSelArray{
  Iarray* markedArray;
  char is_handle;
}winTreeSelArray;

static int winTreeSelectedArrayFunc(Ihandle* ih, HTREEITEM hItem, int id, winTreeSelArray* selarray)
{                                                                                  
  if (winTreeIsNodeSelected(ih, hItem))
  {
    if (selarray->is_handle)
    {
      HTREEITEM* hItemArrayData = (HTREEITEM*)iupArrayInc(selarray->markedArray);
      int i = iupArrayCount(selarray->markedArray);
      hItemArrayData[i-1] = hItem;
    }
    else
    {
      int* intArrayData = (int*)iupArrayInc(selarray->markedArray);
      int i = iupArrayCount(selarray->markedArray);
      intArrayData[i-1] = id;
    }
  }

  return 1;
}

static Iarray* winTreeGetSelectedArrayId(Ihandle* ih)
{
  Iarray* markedArray = iupArrayCreate(1, sizeof(int));
  winTreeSelArray selarray;
  selarray.markedArray = markedArray;
  selarray.is_handle = 0;

  iupTreeForEach(ih, (iupTreeNodeFunc)winTreeSelectedArrayFunc, &selarray);

  return markedArray;
}


/*****************************************************************************/
/* COPYING ITEMS (Branches and its children)                                 */
/*****************************************************************************/
/* Insert the copied item in a new location. Returns the new item. */
static HTREEITEM winTreeCopyItem(Ihandle* ih, HTREEITEM hItem, HTREEITEM hParent, HTREEITEM hPosition, int is_copy)
{
  TVITEM item; 
  TVINSERTSTRUCT tvins;
  char* title = iupStrGetMemory(255);

  item.hItem = hItem;
  item.mask = TVIF_HANDLE | TVIF_PARAM | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_TEXT | TVIF_STATE | TVIF_PARAM;
  item.pszText = title;
  item.cchTextMax = 255;
  SendMessage(ih->handle, TVM_GETITEM, 0, (LPARAM)(LPTVITEM)&item);

  if (is_copy) /* during a copy the itemdata reference is not reused */
  {
    /* create a new one */
    winTreeItemData* itemDataNew = malloc(sizeof(winTreeItemData));
    memcpy(itemDataNew, (void*)item.lParam, sizeof(winTreeItemData));
    item.lParam = (LPARAM)itemDataNew;
  }

  /* Copy everything including itemdata reference */
  tvins.item = item; 
  tvins.hInsertAfter = hPosition;
  tvins.hParent = hParent;

  /* Add the new node */
  ih->data->node_count++;
  return (HTREEITEM)SendMessage(ih->handle, TVM_INSERTITEM, 0, (LPARAM)(LPTVINSERTSTRUCT)&tvins);
}

static void winTreeCopyChildren(Ihandle* ih, HTREEITEM hItemSrc, HTREEITEM hItemDst, int is_copy)
{
  HTREEITEM hChildSrc = (HTREEITEM)SendMessage(ih->handle, TVM_GETNEXTITEM, TVGN_CHILD, (LPARAM)hItemSrc);
  HTREEITEM hItemNew = TVI_FIRST;
  while (hChildSrc != NULL)
  {
    hItemNew = winTreeCopyItem(ih, hChildSrc, hItemDst, hItemNew, is_copy); /* Use the same order they where enumerated */

    /* Recursively transfer all the items */
    winTreeCopyChildren(ih, hChildSrc, hItemNew, is_copy);  

    /* Go to next sibling item */
    hChildSrc = (HTREEITEM)SendMessage(ih->handle, TVM_GETNEXTITEM, TVGN_NEXT, (LPARAM)hChildSrc);
  }
}

/* Copies all items in a branch to a new location. Returns the new branch node. */
static HTREEITEM winTreeCopyMoveNode(Ihandle* ih, HTREEITEM hItemSrc, HTREEITEM hItemDst, int is_copy)
{
  HTREEITEM hItemNew, hParent;
  TVITEM item;
  winTreeItemData* itemDataDst;
  int id_new, count, id_src, id_dst;

  int old_count = ih->data->node_count;

  id_src = iupTreeFindNodeId(ih, hItemSrc);
  id_dst = iupTreeFindNodeId(ih, hItemDst);
  id_new = id_dst+1; /* contains the position for a copy operation */

  /* Get DST node attributes */
  item.hItem = hItemDst;
  item.mask = TVIF_HANDLE | TVIF_PARAM | TVIF_STATE;
  SendMessage(ih->handle, TVM_GETITEM, 0, (LPARAM)(LPTVITEM)&item);
  itemDataDst = (winTreeItemData*)item.lParam;

  if (itemDataDst->kind == ITREE_BRANCH && (item.state & TVIS_EXPANDED))
  {
    /* copy as first child of expanded branch */
    hParent = hItemDst;
    hItemDst = TVI_FIRST;
  }
  else
  {                                    
    if (itemDataDst->kind == ITREE_BRANCH)
    {
      int child_count = iupdrvTreeTotalChildCount(ih, hItemDst);
      id_new += child_count;
    }

    /* copy as next brother of item or collapsed branch */
    hParent = (HTREEITEM)SendMessage(ih->handle, TVM_GETNEXTITEM, TVGN_PARENT, (LPARAM)hItemDst);
  }

  /* move to the same place does nothing */
  if (!is_copy && id_new == id_src)
    return NULL;

  hItemNew = winTreeCopyItem(ih, hItemSrc, hParent, hItemDst, is_copy);

  winTreeCopyChildren(ih, hItemSrc, hItemNew, is_copy);

  count = ih->data->node_count - old_count;
  iupTreeCopyMoveCache(ih, id_src, id_new, count, is_copy);

  if (!is_copy)
  {
    /* Deleting the node (and its children) from the old position */
    /* do not delete the itemdata, we reuse the references in CopyNode */
    iupAttribSetStr(ih, "_IUPTREE_IGNORE_SELECTION_CB", "1");
    SendMessage(ih->handle, TVM_DELETEITEM, 0, (LPARAM)hItemSrc);
    iupAttribSetStr(ih, "_IUPTREE_IGNORE_SELECTION_CB", NULL);

    /* restore count, because we remove src */
    ih->data->node_count = old_count;

    /* compensate position for a move */
    if (id_new > id_src)
      id_new -= count;
  }

  winTreeRebuildNodeCache(ih, id_new, hItemNew);

  return hItemNew;
}

/*****************************************************************************/
/* MANIPULATING IMAGES                                                       */
/*****************************************************************************/
static void winTreeUpdateImages(Ihandle* ih, int mode)
{
  HTREEITEM hItem;
  TVITEM item;
  winTreeItemData* itemData;
  int i;

  /* called when one of the default images is changed */
  for (i = 0; i < ih->data->node_count; i++)
  {
    hItem = ih->data->node_cache[i].node_handle;

    /* Get node attributes */
    item.hItem = hItem;
    item.mask = TVIF_HANDLE | TVIF_PARAM | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_STATE;
    SendMessage(ih->handle, TVM_GETITEM, 0, (LPARAM)(LPTVITEM)&item);
    itemData = (winTreeItemData*)item.lParam;

    if (itemData->kind == ITREE_BRANCH)
    {
      if (item.state & TVIS_EXPANDED)
      {
        if (mode == ITREE_UPDATEIMAGE_EXPANDED)
        {
          item.mask = TVIF_HANDLE | TVIF_IMAGE | TVIF_SELECTEDIMAGE;
          item.iSelectedImage = item.iImage = (itemData->image_expanded!=-1)? itemData->image_expanded: (int)ih->data->def_image_expanded;
          SendMessage(ih->handle, TVM_SETITEM, 0, (LPARAM)(LPTVITEM)&item);
        }
      }
      else
      {
        if (mode == ITREE_UPDATEIMAGE_COLLAPSED)
        {
          item.mask = TVIF_HANDLE | TVIF_IMAGE | TVIF_SELECTEDIMAGE;
          item.iSelectedImage = item.iImage = (itemData->image!=-1)? itemData->image: (int)ih->data->def_image_collapsed;
          SendMessage(ih->handle, TVM_SETITEM, 0, (LPARAM)(LPTVITEM)&item);
        }
      }
    }
    else
    {
      if (mode == ITREE_UPDATEIMAGE_LEAF)
      {
        item.mask = TVIF_HANDLE | TVIF_IMAGE | TVIF_SELECTEDIMAGE;
        item.iSelectedImage = item.iImage = (itemData->image!=-1)? itemData->image: (int)ih->data->def_image_leaf;
        SendMessage(ih->handle, TVM_SETITEM, 0, (LPARAM)(LPTVITEM)&item);
      }
    }
  }
}

static int winTreeGetImageIndex(Ihandle* ih, const char* name)
{
  HIMAGELIST image_list;
  int count, i;
  Iarray* bmpArray;
  HBITMAP *bmpArrayData;
  HBITMAP bmp = iupImageGetImage(name, ih, 0);
  if (!bmp)
    return -1;

  /* the array is used to avoi adding the same bitmap twice */
  bmpArray = (Iarray*)iupAttribGet(ih, "_IUPWIN_BMPARRAY");
  if (!bmpArray)
  {
    /* create the array if does not exist */
    bmpArray = iupArrayCreate(50, sizeof(HBITMAP));
    iupAttribSetStr(ih, "_IUPWIN_BMPARRAY", (char*)bmpArray);
  }

  bmpArrayData = iupArrayGetData(bmpArray);

  image_list = (HIMAGELIST)SendMessage(ih->handle, TVM_GETIMAGELIST, TVSIL_NORMAL, 0);
  if (!image_list)
  {
    int width, height;

    /* must use this info, since image can be a driver image loaded from resources */
    iupdrvImageGetInfo(bmp, &width, &height, NULL);

    /* create the image list if does not exist */
    image_list = ImageList_Create(width, height, ILC_COLOR32, 0, 50);
    SendMessage(ih->handle, TVM_SETIMAGELIST, 0, (LPARAM)image_list);
  }

  /* check if that bitmap is already added to the list,
     but we can not compare with the actual bitmap at the list since it is a copy */
  count = ImageList_GetImageCount(image_list);
  for (i = 0; i < count; i++)
  {
    if (bmpArrayData[i] == bmp)
      return i;
  }

  bmpArrayData = iupArrayInc(bmpArray);
  bmpArrayData[i] = bmp;
  return ImageList_Add(image_list, bmp, NULL);  /* the bmp is duplicated at the list */
}


/*****************************************************************************/
/* CALLBACKS                                                                 */
/*****************************************************************************/

static int winTreeCallBranchLeafCb(Ihandle* ih, HTREEITEM hItem)
{
  TVITEM item;
  winTreeItemData* itemData;

  /* Get Children: branch or leaf */
  item.mask = TVIF_HANDLE|TVIF_PARAM|TVIF_STATE; 
  item.hItem = hItem;
  SendMessage(ih->handle, TVM_GETITEM, 0, (LPARAM)(LPTVITEM)&item);
  itemData = (winTreeItemData*)item.lParam;

  if (itemData->kind == ITREE_BRANCH)
  {
    if (iupAttribGet(ih, "_IUPTREE_IGNORE_BRANCH_CB"))
      return IUP_DEFAULT;

    if (item.state & TVIS_EXPANDED)
    {
      IFni cbBranchClose = (IFni)IupGetCallback(ih, "BRANCHCLOSE_CB");
      if (cbBranchClose)
        return cbBranchClose(ih, iupTreeFindNodeId(ih, hItem));
    }
    else
    {
      IFni cbBranchOpen  = (IFni)IupGetCallback(ih, "BRANCHOPEN_CB");
      if (cbBranchOpen)
        return cbBranchOpen(ih, iupTreeFindNodeId(ih, hItem));
    }
  }
  else
  {
    IFni cbExecuteLeaf = (IFni)IupGetCallback(ih, "EXECUTELEAF_CB");
    if (cbExecuteLeaf)
      return cbExecuteLeaf(ih, iupTreeFindNodeId(ih, hItem));
  }

  return IUP_DEFAULT;
}

static void winTreeCallMultiUnSelectionCb(Ihandle* ih)
{
  IFnIi cbMulti = (IFnIi)IupGetCallback(ih, "MULTIUNSELECTION_CB");
  IFnii cbSelec = (IFnii)IupGetCallback(ih, "SELECTION_CB");
  if (cbSelec || cbMulti)
  {
    Iarray* markedArray = winTreeGetSelectedArrayId(ih);
    int* id_hitem = (int*)iupArrayGetData(markedArray);
    int i, count = iupArrayCount(markedArray);

    if (count > 1)
    {
      if (cbMulti)
        cbMulti(ih, id_hitem, iupArrayCount(markedArray));
      else
      {
        for (i=0; i<count; i++)
          cbSelec(ih, id_hitem[i], 0);
      }
    }

    iupArrayDestroy(markedArray);
  }
}

static void winTreeCallMultiSelectionCb(Ihandle* ih)
{
  IFnIi cbMulti = (IFnIi)IupGetCallback(ih, "MULTISELECTION_CB");
  if(cbMulti)
  {
    Iarray* markedArray = winTreeGetSelectedArrayId(ih);
    int* id_hitem = (int*)iupArrayGetData(markedArray);

    cbMulti(ih, id_hitem, iupArrayCount(markedArray));

    iupArrayDestroy(markedArray);
  }
  else
  {
    IFnii cbSelec = (IFnii)IupGetCallback(ih, "SELECTION_CB");
    if (cbSelec)
    {
      Iarray* markedArray = winTreeGetSelectedArrayId(ih);
      int* id_hitem = (int*)iupArrayGetData(markedArray);
      int i, count = iupArrayCount(markedArray);

      for (i=0; i<count; i++)
        cbSelec(ih, id_hitem[i], 1);

      iupArrayDestroy(markedArray);
    }
  }
}

static void winTreeCallSelectionCb(Ihandle* ih, int status, HTREEITEM hItem)
{
  IFnii cbSelec = (IFnii)IupGetCallback(ih, "SELECTION_CB");
  if (cbSelec)
  {
    int id;

    if (ih->data->mark_mode == ITREE_MARK_MULTIPLE && IupGetCallback(ih,"MULTISELECTION_CB"))
    {
      if ((GetKeyState(VK_SHIFT) & 0x8000))
        return;
    }

    if (iupAttribGet(ih, "_IUPTREE_IGNORE_SELECTION_CB"))
      return;
    
    id = iupTreeFindNodeId(ih, hItem);
    if (id != -1)
      cbSelec(ih, id, status);
  }
}

static int winTreeCallDragDropCb(Ihandle* ih, HTREEITEM	hItemDrag, HTREEITEM hItemDrop, int *is_ctrl)
{
  IFniiii cbDragDrop = (IFniiii)IupGetCallback(ih, "DRAGDROP_CB");
  int is_shift = 0;
  if ((GetKeyState(VK_SHIFT) & 0x8000))
    is_shift = 1;
  if ((GetKeyState(VK_CONTROL) & 0x8000))
    *is_ctrl = 1;
  else
    *is_ctrl = 0;

  if (cbDragDrop)
  {
    int drag_id = iupTreeFindNodeId(ih, hItemDrag);
    int drop_id = iupTreeFindNodeId(ih, hItemDrop);
    return cbDragDrop(ih, drag_id, drop_id, is_shift, *is_ctrl);
  }

  return IUP_CONTINUE; /* allow to move by default if callback not defined */
}


/*****************************************************************************/
/* GET AND SET ATTRIBUTES                                                    */
/*****************************************************************************/


static int winTreeSetImageBranchExpandedAttrib(Ihandle* ih, const char* value)
{
  ih->data->def_image_expanded = (void*)winTreeGetImageIndex(ih, value);

  /* Update all images */
  winTreeUpdateImages(ih, ITREE_UPDATEIMAGE_EXPANDED);

  return 1;
}

static int winTreeSetImageBranchCollapsedAttrib(Ihandle* ih, const char* value)
{
  ih->data->def_image_collapsed = (void*)winTreeGetImageIndex(ih, value);

  /* Update all images */
  winTreeUpdateImages(ih, ITREE_UPDATEIMAGE_COLLAPSED);

  return 1;
}

static int winTreeSetImageLeafAttrib(Ihandle* ih, const char* value)
{
  ih->data->def_image_leaf = (void*)winTreeGetImageIndex(ih, value);

  /* Update all images */
  winTreeUpdateImages(ih, ITREE_UPDATEIMAGE_LEAF);

  return 1;
}

static int winTreeSetImageExpandedAttrib(Ihandle* ih, int id, const char* value)
{
  TVITEM item;
  winTreeItemData* itemData;
  HTREEITEM hItem = iupTreeGetNode(ih, id);
  if (!hItem)
    return 0;

  item.hItem = hItem;
  item.mask = TVIF_HANDLE | TVIF_PARAM | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_STATE;
  SendMessage(ih->handle, TVM_GETITEM, 0, (LPARAM)(LPTVITEM)&item);
  itemData = (winTreeItemData*)item.lParam;
  itemData->image_expanded = (short)winTreeGetImageIndex(ih, value);

  if (itemData->kind == ITREE_BRANCH && item.state & TVIS_EXPANDED)
  {
    if (itemData->image_expanded == -1)
      item.iSelectedImage = item.iImage = (int)ih->data->def_image_expanded;
    else
      item.iSelectedImage = item.iImage = itemData->image_expanded;
    item.mask = TVIF_HANDLE | TVIF_IMAGE | TVIF_SELECTEDIMAGE;
    SendMessage(ih->handle, TVM_SETITEM, 0, (LPARAM)(const LPTVITEM)&item);
  }

  return 1;
}

static int winTreeSetImageAttrib(Ihandle* ih, int id, const char* value)
{
  TVITEM item;
  winTreeItemData* itemData;
  HTREEITEM hItem = iupTreeGetNode(ih, id);
  if (!hItem)
    return 0;

  item.hItem = hItem;
  item.mask = TVIF_HANDLE | TVIF_PARAM | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_STATE;
  SendMessage(ih->handle, TVM_GETITEM, 0, (LPARAM)(LPTVITEM)&item);
  itemData = (winTreeItemData*)item.lParam;
  itemData->image = (short)winTreeGetImageIndex(ih, value);

  if (itemData->kind == ITREE_BRANCH)
  {
    if (!(item.state & TVIS_EXPANDED))
    {
      if (itemData->image == -1)
        item.iSelectedImage = item.iImage = (int)ih->data->def_image_collapsed;
      else
        item.iSelectedImage = item.iImage = itemData->image;

      item.mask = TVIF_HANDLE | TVIF_IMAGE | TVIF_SELECTEDIMAGE;
      SendMessage(ih->handle, TVM_SETITEM, 0, (LPARAM)(const LPTVITEM)&item);
    }
  }
  else
  {
    if (itemData->image == -1)
      item.iSelectedImage = item.iImage = (int)ih->data->def_image_leaf;
    else
      item.iSelectedImage = item.iImage = itemData->image;

    item.mask = TVIF_HANDLE | TVIF_IMAGE | TVIF_SELECTEDIMAGE;
    SendMessage(ih->handle, TVM_SETITEM, 0, (LPARAM)(const LPTVITEM)&item);
  }

  return 1;
}

static int winTreeSetTopItemAttrib(Ihandle* ih, const char* value)
{
  HTREEITEM hItem = iupTreeGetNodeFromString(ih, value);
  if (hItem)
    SendMessage(ih->handle, TVM_ENSUREVISIBLE, 0, (LPARAM)hItem);
  return 0;
}

static int winTreeSetSpacingAttrib(Ihandle* ih, const char* value)
{
  if (!iupStrToInt(value, &ih->data->spacing))
    ih->data->spacing = 1;

  if(ih->data->spacing < 1)
    ih->data->spacing = 1;

  if (ih->handle)
  {
    int old_spacing = iupAttribGetInt(ih, "_IUPWIN_OLDSPACING");
    int height = SendMessage(ih->handle, TVM_GETITEMHEIGHT, 0, 0);
    height -= 2*old_spacing;
    height += 2*ih->data->spacing;
    SendMessage(ih->handle, TVM_SETITEMHEIGHT, height, 0);
    iupAttribSetInt(ih, "_IUPWIN_OLDSPACING", ih->data->spacing);
    return 0;
  }
  else
    return 1; /* store until not mapped, when mapped will be set again */
}

static int winTreeSetExpandAllAttrib(Ihandle* ih, const char* value)
{
  HTREEITEM hItemRoot = (HTREEITEM)SendMessage(ih->handle, TVM_GETNEXTITEM, TVGN_ROOT, 0);
  int expand = iupStrBoolean(value);
  winTreeExpandTree(ih, hItemRoot, expand);
  return 0;
}

static int winTreeSetBgColorAttrib(Ihandle* ih, const char* value)
{
  unsigned char r, g, b;
  if (iupStrToRGB(value, &r, &g, &b))
  {
    COLORREF cr = RGB(r,g,b);
    SendMessage(ih->handle, TVM_SETBKCOLOR, 0, (LPARAM)cr);

    /* update internal image cache */
    iupTreeUpdateImages(ih);
  }
  return 0;
}

static char* winTreeGetBgColorAttrib(Ihandle* ih)
{
  COLORREF cr = (COLORREF)SendMessage(ih->handle, TVM_GETBKCOLOR, 0, 0);
  if (cr == (COLORREF)-1)
    return IupGetGlobal("TXTBGCOLOR");
  else
  {
    char* str = iupStrGetMemory(20);
    sprintf(str, "%d %d %d", (int)GetRValue(cr), (int)GetGValue(cr), (int)GetBValue(cr));
    return str;
  }
}

static void winTreeSetRenameCaretPos(HWND hEdit, const char* value)
{
  int pos = 1;

  if (iupStrToInt(value, &pos))
  {
    if (pos < 1) pos = 1;
    pos--; /* IUP starts at 1 */

    SendMessage(hEdit, EM_SETSEL, (WPARAM)pos, (LPARAM)pos);
  }
}

static void winTreeSetRenameSelectionPos(HWND hEdit, const char* value)
{
  int start = 1, end = 1;

  if (iupStrToIntInt(value, &start, &end, ':') != 2) 
    return;

  if(start < 1 || end < 1) 
    return;

  start--; /* IUP starts at 1 */
  end--;

  SendMessage(hEdit, EM_SETSEL, (WPARAM)start, (LPARAM)end);
}

static char* winTreeGetTitle(Ihandle* ih, HTREEITEM hItem)
{
  TVITEM item;
  item.hItem = hItem;
  item.mask = TVIF_HANDLE | TVIF_TEXT; 
  item.pszText = iupStrGetMemory(255);
  item.cchTextMax = 255;
  SendMessage(ih->handle, TVM_GETITEM, 0, (LPARAM)(LPTVITEM)&item);
  return item.pszText;
}

static char* winTreeGetTitleAttrib(Ihandle* ih, int id)
{
  HTREEITEM hItem = iupTreeGetNode(ih, id);
  if (!hItem)
    return NULL;
  return winTreeGetTitle(ih, hItem);
}

static int winTreeSetTitleAttrib(Ihandle* ih, int id, const char* value)
{
  TVITEM item; 
  HTREEITEM hItem = iupTreeGetNode(ih, id);
  if (!hItem)
    return 0;

  if (!value)
    value = "";

  item.hItem = hItem;
  item.mask = TVIF_HANDLE | TVIF_TEXT; 
  item.pszText = (char*)value;
  SendMessage(ih->handle, TVM_SETITEM, 0, (LPARAM)(const LPTVITEM)&item);
  return 0;
}

static char* winTreeGetTitleFontAttrib(Ihandle* ih, int id)
{
  TVITEM item;
  winTreeItemData* itemData;
  HTREEITEM hItem = iupTreeGetNode(ih, id);
  if (!hItem)
    return NULL;

  item.hItem = hItem;
  item.mask = TVIF_HANDLE | TVIF_PARAM;
  SendMessage(ih->handle, TVM_GETITEM, 0, (LPARAM)(LPTVITEM)&item);
  itemData = (winTreeItemData*)item.lParam;

  return iupwinFindHFont(itemData->hFont);
}

static int winTreeSetTitleFontAttrib(Ihandle* ih, int id, const char* value)
{
  TVITEM item; 
  winTreeItemData* itemData;
  HTREEITEM hItem = iupTreeGetNode(ih, id);
  if (!hItem)
    return 0;

  item.hItem = hItem;
  item.mask = TVIF_HANDLE | TVIF_PARAM;
  SendMessage(ih->handle, TVM_GETITEM, 0, (LPARAM)(LPTVITEM)&item);
  itemData = (winTreeItemData*)item.lParam;

  if (value)
  {
    itemData->hFont = iupwinGetHFont(value);
    if (itemData->hFont)
    {
      TV_ITEM item;
      item.mask = TVIF_STATE | TVIF_HANDLE | TVIF_TEXT;
      item.stateMask = TVIS_BOLD;
      item.hItem = hItem;
      item.pszText = winTreeGetTitle(ih, hItem);   /* reset text to resize item */
      item.state = (strstr(value, "Bold")||strstr(value, "BOLD"))? TVIS_BOLD: 0;
      SendMessage(ih->handle, TVM_SETITEM, 0, (LPARAM)&item);
    }
  }
  else
    itemData->hFont = NULL;

  iupdrvPostRedraw(ih);

  return 0;
}

static char* winTreeGetIndentationAttrib(Ihandle* ih)
{
  char* str = iupStrGetMemory(255);
  int indent = (int)SendMessage(ih->handle, TVM_GETINDENT, 0, 0);
  sprintf(str, "%d", indent);
  return str;
}

static int winTreeSetIndentationAttrib(Ihandle* ih, const char* value)
{
  int indent;
  if (iupStrToInt(value, &indent))
    SendMessage(ih->handle, TVM_SETINDENT, (WPARAM)indent, 0);
  return 0;
}

static char* winTreeGetStateAttrib(Ihandle* ih, int id)
{
  TVITEM item;
  winTreeItemData* itemData;
  HTREEITEM hItem = iupTreeGetNode(ih, id);
  if (!hItem)
    return NULL;

  item.hItem = hItem;
  item.mask = TVIF_HANDLE|TVIF_PARAM; 
  SendMessage(ih->handle, TVM_GETITEM, 0, (LPARAM)(LPTVITEM)&item);
  itemData = (winTreeItemData*)item.lParam;

  if (itemData->kind == ITREE_BRANCH)
  {
    if (winTreeIsItemExpanded(ih, hItem))
      return "EXPANDED";
    else
      return "COLLAPSED";
  }

  return NULL;
}

static int winTreeSetStateAttrib(Ihandle* ih, int id, const char* value)
{
  TVITEM item;
  winTreeItemData* itemData;
  HTREEITEM hItem = iupTreeGetNode(ih, id);
  if (!hItem)
    return 0;

  /* Get Children: branch or leaf */
  item.mask = TVIF_HANDLE|TVIF_PARAM; 
  item.hItem = hItem;
  SendMessage(ih->handle, TVM_GETITEM, 0, (LPARAM)(LPTVITEM)&item);
  itemData = (winTreeItemData*)item.lParam;

  if (itemData->kind == ITREE_BRANCH)
    winTreeExpandItem(ih, hItem, iupStrEqualNoCase(value, "EXPANDED"));

  return 0;
}

static char* winTreeGetDepthAttrib(Ihandle* ih, int id)
{
  HTREEITEM hItem = iupTreeGetNode(ih, id);
  int depth = -1;
  char* str;

  if (!hItem)  
    return NULL;

  while(hItem != NULL)
  {
    hItem = (HTREEITEM)SendMessage(ih->handle, TVM_GETNEXTITEM, TVGN_PARENT, (LPARAM)hItem);
    depth++;
  }

  str = iupStrGetMemory(10);
  sprintf(str, "%d", depth);
  return str;
}

static int winTreeSetMoveNodeAttrib(Ihandle* ih, int id, const char* value)
{
  HTREEITEM hItemDst, hParent, hItemSrc;

  if (!ih->handle)  /* do not do the action before map */
    return 0;

  hItemSrc = iupTreeGetNode(ih, id);
  if (!hItemSrc)
    return 0;
  hItemDst = iupTreeGetNodeFromString(ih, value);
  if (!hItemDst)
    return 0;

  /* If Drag item is an ancestor of Drop item then return */
  hParent = hItemDst;
  while(hParent)
  {
    hParent = (HTREEITEM)SendMessage(ih->handle, TVM_GETNEXTITEM, TVGN_PARENT, (LPARAM)hParent);
    if (hParent == hItemSrc)
      return 0;
  }

  /* Move the node and its children to the new position */
  winTreeCopyMoveNode(ih, hItemSrc, hItemDst, 0);

  return 0;
}

static int winTreeSetCopyNodeAttrib(Ihandle* ih, int id, const char* value)
{
  HTREEITEM hItemDst, hParent, hItemSrc;

  if (!ih->handle)  /* do not do the action before map */
    return 0;

  hItemSrc = iupTreeGetNode(ih, id);
  if (!hItemSrc)
    return 0;
  hItemDst = iupTreeGetNodeFromString(ih, value);
  if (!hItemDst)
    return 0;

  /* If Drag item is an ancestor of Drop item then return */
  hParent = hItemDst;
  while(hParent)
  {
    hParent = (HTREEITEM)SendMessage(ih->handle, TVM_GETNEXTITEM, TVGN_PARENT, (LPARAM)hParent);
    if (hParent == hItemSrc)
      return 0;
  }

  /* Copy the node and its children to the new position */
  winTreeCopyMoveNode(ih, hItemSrc, hItemDst, 1);

  return 0;
}

static char* winTreeGetColorAttrib(Ihandle* ih, int id)
{
  unsigned char r, g, b;
  char* str;
  TVITEM item;
  winTreeItemData* itemData;
  HTREEITEM hItem = iupTreeGetNode(ih, id);
  if (!hItem)
    return NULL;

  item.hItem = hItem;
  item.mask = TVIF_HANDLE | TVIF_PARAM;
  SendMessage(ih->handle, TVM_GETITEM, 0, (LPARAM)(LPTVITEM)&item);
  itemData = (winTreeItemData*)item.lParam;

  r = GetRValue(itemData->color);
  g = GetGValue(itemData->color);
  b = GetBValue(itemData->color);

  str = iupStrGetMemory(12);
  sprintf(str, "%d %d %d", r, g, b);
  return str;
}
 
static int winTreeSetColorAttrib(Ihandle* ih, int id, const char* value)
{
  unsigned char r, g, b;
  TVITEM item;
  winTreeItemData* itemData;
  HTREEITEM hItem = iupTreeGetNode(ih, id);
  if (!hItem)
    return 0;

  item.hItem = hItem;
  item.mask = TVIF_HANDLE | TVIF_PARAM;
  SendMessage(ih->handle, TVM_GETITEM, 0, (LPARAM)(LPTVITEM)&item);
  itemData = (winTreeItemData*)item.lParam;

  if (iupStrToRGB(value, &r, &g, &b))
  {
    itemData->color = RGB(r,g,b);
    iupdrvPostRedraw(ih);
  }

 return 0;
}

static int winTreeSetFgColorAttrib(Ihandle* ih, const char* value)
{
  unsigned char r, g, b;

  if (iupStrToRGB(value, &r, &g, &b))
  {
    COLORREF color = RGB(r,g,b);
    SendMessage(ih->handle, TVM_SETTEXTCOLOR, 0, (LPARAM)color);
  }
  else
    SendMessage(ih->handle, TVM_SETTEXTCOLOR, 0, (LPARAM)CLR_DEFAULT);

  return 0;
}

static int winTreeSetTipVisibleAttrib(Ihandle* ih, const char* value)
{
  HWND tips_hwnd = (HWND)SendMessage(ih->handle, TVM_GETTOOLTIPS, 0, 0);
  if (!tips_hwnd)
    return 0;

  if (iupStrBoolean(value))
    SendMessage(tips_hwnd, TTM_POPUP, 0, 0);  /* Works in Visual Styles Only */
  else
    SendMessage(tips_hwnd, TTM_POP, 0, 0);

  return 0;
}

static char* winTreeGetTipVisibleAttrib(Ihandle* ih)
{
  HWND tips_hwnd = (HWND)SendMessage(ih->handle, TVM_GETTOOLTIPS, 0, 0);
  if (!tips_hwnd)
    return NULL;

  if (IsWindowVisible(tips_hwnd))
    return "Yes";
  else
    return "No";
}

static char* winTreeGetChildCountAttrib(Ihandle* ih, int id)
{
  int count;
  char* str;
  HTREEITEM hItem = iupTreeGetNode(ih, id);
  if (!hItem)
    return NULL;

  count = 0;
  hItem = (HTREEITEM)SendMessage(ih->handle, TVM_GETNEXTITEM, TVGN_CHILD, (LPARAM)hItem);
  while(hItem != NULL)
  {
    count++;

    /* Go to next sibling item */
    hItem = (HTREEITEM)SendMessage(ih->handle, TVM_GETNEXTITEM, TVGN_NEXT, (LPARAM)hItem);
  }

  str = iupStrGetMemory(10);
  sprintf(str, "%d", count);
  return str;
}

static char* winTreeGetKindAttrib(Ihandle* ih, int id)
{
  TVITEM item; 
  winTreeItemData* itemData;
  HTREEITEM hItem = iupTreeGetNode(ih, id);
  if (!hItem)
    return NULL;

  item.hItem = hItem;
  item.mask = TVIF_HANDLE|TVIF_PARAM; 
  SendMessage(ih->handle, TVM_GETITEM, 0, (LPARAM)(LPTVITEM)&item);
  itemData = (winTreeItemData*)item.lParam;

  if(itemData->kind == ITREE_BRANCH)
    return "BRANCH";
  else
    return "LEAF";
}

static char* winTreeGetParentAttrib(Ihandle* ih, int id)
{
  char* str;
  HTREEITEM hItem = iupTreeGetNode(ih, id);
  if (!hItem)
    return NULL;

  hItem = (HTREEITEM)SendMessage(ih->handle, TVM_GETNEXTITEM, TVGN_PARENT, (LPARAM)hItem);
  if (!hItem)
    return NULL;

  str = iupStrGetMemory(10);
  sprintf(str, "%d", iupTreeFindNodeId(ih, hItem));
  return str;
}

static void winTreeRemoveItemData(Ihandle* ih, HTREEITEM hItem, IFns cb, int id)
{
  TVITEM item; 
  item.hItem = hItem;
  item.mask = TVIF_HANDLE|TVIF_PARAM; 
  if (SendMessage(ih->handle, TVM_GETITEM, 0, (LPARAM)(LPTVITEM)&item))
  {
    winTreeItemData* itemData = (winTreeItemData*)item.lParam;
    if (itemData)
    {
      if (cb) 
        cb(ih, (char*)ih->data->node_cache[id].userdata);

      free(itemData);
      item.lParam = (LPARAM)NULL;
      SendMessage(ih->handle, TVM_SETITEM, 0, (LPARAM)(LPTVITEM)&item);
    }
  }
}

static void winTreeRemoveNodeDataRec(Ihandle* ih, HTREEITEM hItem, IFns cb, int *id)
{
  int old_id = *id;

  /* Check whether we have child items */
  /* remove from children first */
  HTREEITEM hChildItem = (HTREEITEM)SendMessage(ih->handle, TVM_GETNEXTITEM, TVGN_CHILD, (LPARAM)hItem);
  while (hChildItem)
  {
    /* go recursive to children */
    winTreeRemoveNodeDataRec(ih, hChildItem, cb, id);

    /* Go to next sibling item */
    hChildItem = (HTREEITEM)SendMessage(ih->handle, TVM_GETNEXTITEM, TVGN_NEXT, (LPARAM)hChildItem);
  }

  /* actually do it for the node */
  ih->data->node_count--;
  (*id)++;

  winTreeRemoveItemData(ih, hItem, cb, old_id);
}

static void winTreeRemoveNodeData(Ihandle* ih, HTREEITEM hItem, int call_cb)
{
  IFns cb = call_cb? (IFns)IupGetCallback(ih, "NODEREMOVED_CB"): NULL;
  int old_count = ih->data->node_count;
  int id = iupTreeFindNodeId(ih, hItem);
  int old_id = id;

  winTreeRemoveNodeDataRec(ih, hItem, cb, &id);

  if (call_cb)
    iupTreeDelFromCache(ih, old_id, old_count-ih->data->node_count);
}

static void winTreeRemoveAllNodeData(Ihandle* ih, int call_cb)
{
  IFns cb = call_cb? (IFns)IupGetCallback(ih, "NODEREMOVED_CB"): NULL;
  int i, old_count = ih->data->node_count;
  HTREEITEM hItem;

  for (i = 0; i < ih->data->node_count; i++)
  {
    hItem = ih->data->node_cache[i].node_handle;
    winTreeRemoveItemData(ih, hItem, cb, i);
  }

  ih->data->node_count = 0;

  if (call_cb)
    iupTreeDelFromCache(ih, 0, old_count);
}

static int winTreeSetDelNodeAttrib(Ihandle* ih, int id, const char* value)
{
  if (!ih->handle)  /* do not do the action before map */
    return 0;
  if (iupStrEqualNoCase(value, "ALL"))
  {
    winTreeRemoveAllNodeData(ih, 1);

    iupAttribSetStr(ih, "_IUPTREE_IGNORE_SELECTION_CB", "1");
    SendMessage(ih->handle, TVM_DELETEITEM, 0, (LPARAM)TVI_ROOT);
    iupAttribSetStr(ih, "_IUPTREE_IGNORE_SELECTION_CB", NULL);
    return 0;
  }
  if (iupStrEqualNoCase(value, "SELECTED")) /* selected here means the reference one */
  {
    HTREEITEM hItem = iupTreeGetNode(ih, id);
    if(!hItem)
      return 0;

    /* deleting the reference node (and it's children) */
    winTreeRemoveNodeData(ih, hItem, 1);
    iupAttribSetStr(ih, "_IUPTREE_IGNORE_SELECTION_CB", "1");
    SendMessage(ih->handle, TVM_DELETEITEM, 0, (LPARAM)hItem);
    iupAttribSetStr(ih, "_IUPTREE_IGNORE_SELECTION_CB", NULL);
    
    return 0;
  }
  else if(iupStrEqualNoCase(value, "CHILDREN"))  /* children of the reference node */
  {
    HTREEITEM hItem = iupTreeGetNode(ih, id);
    HTREEITEM hChildItem = (HTREEITEM)SendMessage(ih->handle, TVM_GETNEXTITEM, TVGN_CHILD, (LPARAM)hItem);

    if(!hItem)
      return 0;

    iupAttribSetStr(ih, "_IUPTREE_IGNORE_SELECTION_CB", "1");

    /* deleting the reference node children */
    while (hChildItem)
    {
      winTreeRemoveNodeData(ih, hChildItem, 1);
      SendMessage(ih->handle, TVM_DELETEITEM, 0, (LPARAM)hChildItem);
      hChildItem = (HTREEITEM)SendMessage(ih->handle, TVM_GETNEXTITEM, TVGN_CHILD, (LPARAM)hItem);
    }

    iupAttribSetStr(ih, "_IUPTREE_IGNORE_SELECTION_CB", NULL);
    return 0;
  }
  else if(iupStrEqualNoCase(value, "MARKED"))
  {
    int i, del_focus = 0;
    HTREEITEM hItemFocus;

    iupAttribSetStr(ih, "_IUPTREE_IGNORE_SELECTION_CB", "1");
    hItemFocus = iupdrvTreeGetFocusNode(ih);

    for(i = 0; i < ih->data->node_count; /* increment only if not removed */)
    {
      if (winTreeIsNodeSelected(ih, ih->data->node_cache[i].node_handle))
      {
        HTREEITEM hItem = ih->data->node_cache[i].node_handle;
        if (hItemFocus == hItem)
        {
          del_focus = 1;
          i++;
        }
        else
        {
          winTreeRemoveNodeData(ih, hItem, 1);
          SendMessage(ih->handle, TVM_DELETEITEM, 0, (LPARAM)hItem);
        }
      }
      else
        i++;
    }

    if (del_focus)
    {
      winTreeRemoveNodeData(ih, hItemFocus, 1);
      SendMessage(ih->handle, TVM_DELETEITEM, 0, (LPARAM)hItemFocus);
    }

    iupAttribSetStr(ih, "_IUPTREE_IGNORE_SELECTION_CB", NULL);

    return 0;
  }

  return 0;
}

static int winTreeSetRenameAttrib(Ihandle* ih, const char* value)
{  
  if (ih->data->show_rename)
  {
    HTREEITEM hItemFocus;
    SetFocus(ih->handle); /* the tree must have focus to activate the edit */
    hItemFocus = iupdrvTreeGetFocusNode(ih);
    SendMessage(ih->handle, TVM_EDITLABEL, 0, (LPARAM)hItemFocus);
  }
  
  (void)value;
  return 0;
}

static char* winTreeGetMarkedAttrib(Ihandle* ih, int id)
{
  HTREEITEM hItem = iupTreeGetNode(ih, id);
  if (!hItem)
    return NULL;

  if (winTreeIsNodeSelected(ih, hItem))
    return "YES";
  else 
    return "NO";
}

static int winTreeSetMarkedAttrib(Ihandle* ih, int id, const char* value)
{
  HTREEITEM hItem = iupTreeGetNode(ih, id);
  if (!hItem)
    return 0;

  if (ih->data->mark_mode==ITREE_MARK_SINGLE)
  {
    HTREEITEM hItemFocus = iupdrvTreeGetFocusNode(ih);
    winTreeSelectNode(ih, hItemFocus, 0);
  }

  winTreeSelectNode(ih, hItem, iupStrBoolean(value));

  if (ih->data->mark_mode==ITREE_MARK_SINGLE)
    winTreeSetFocusNode(ih, hItem);

  return 0;
}

static int winTreeSetMarkStartAttrib(Ihandle* ih, const char* value)
{
  HTREEITEM hItem = iupTreeGetNodeFromString(ih, value);
  if (!hItem)
    return 0;

  iupAttribSetStr(ih, "_IUPTREE_MARKSTART_NODE", (char*)hItem);

  return 1;
}

static char* winTreeGetValueAttrib(Ihandle* ih)
{
  char* str;
  HTREEITEM hItemFocus = iupdrvTreeGetFocusNode(ih);
  if (!hItemFocus)
  {
    if (ih->data->node_count)
      return "0"; /* default VALUE is root */
    else
      return "-1";
  }

  str = iupStrGetMemory(16);
  sprintf(str, "%d", iupTreeFindNodeId(ih, hItemFocus));
  return str;
}

static char* winTreeGetMarkedNodesAttrib(Ihandle* ih)
{
  char* str = iupStrGetMemory(ih->data->node_count+1);
  int i;

  for (i=0; i<ih->data->node_count; i++)
  {
    if (winTreeIsNodeSelected(ih, ih->data->node_cache[i].node_handle))
      str[i] = '+';
    else
      str[i] = '-';
  }

  str[ih->data->node_count] = 0;
  return str;
}

static int winTreeSetMarkedNodesAttrib(Ihandle* ih, const char* value)
{
  int count, i;

  if (ih->data->mark_mode==ITREE_MARK_SINGLE || !value)
    return 0;

  count = strlen(value);
  if (count > ih->data->node_count)
    count = ih->data->node_count;

  for (i=0; i<count; i++)
  {
    if (value[i] == '+')
      winTreeSelectNode(ih, ih->data->node_cache[i].node_handle, 1);
    else
      winTreeSelectNode(ih, ih->data->node_cache[i].node_handle, 0);
  }

  return 0;
}

static int winTreeSetMarkAttrib(Ihandle* ih, const char* value)
{
  if (ih->data->mark_mode==ITREE_MARK_SINGLE)
    return 0;

  if(iupStrEqualNoCase(value, "BLOCK"))
  {
    HTREEITEM hItemFocus = iupdrvTreeGetFocusNode(ih);
    winTreeSelectRange(ih, (HTREEITEM)iupAttribGet(ih, "_IUPTREE_MARKSTART_NODE"), hItemFocus, 0);
  }
  else if(iupStrEqualNoCase(value, "CLEARALL"))
    winTreeClearSelection(ih, NULL);
  else if(iupStrEqualNoCase(value, "MARKALL"))
    winTreeSelectAll(ih);
  else if(iupStrEqualNoCase(value, "INVERTALL")) /* INVERTALL *MUST* appear before INVERT, or else INVERTALL will never be called. */
    iupTreeForEach(ih, (iupTreeNodeFunc)winTreeInvertSelectFunc, NULL);
  else if(iupStrEqualPartial(value, "INVERT")) /* iupStrEqualPartial allows the use of "INVERTid" form */
  {
    HTREEITEM hItem = iupTreeGetNodeFromString(ih, &value[strlen("INVERT")]);
    if (!hItem)
      return 0;

    winTreeSelectNode(ih, hItem, -1); /* toggle */
  }
  else
  {
    HTREEITEM hItem1, hItem2;

    char str1[50], str2[50];
    if (iupStrToStrStr(value, str1, str2, '-')!=2)
      return 0;

    hItem1 = iupTreeGetNodeFromString(ih, str1);
    if (!hItem1)  
      return 0;
    hItem2 = iupTreeGetNodeFromString(ih, str2);
    if (!hItem2)  
      return 0;

    winTreeSelectRange(ih, hItem1, hItem2, 0);
  }

  return 1;
} 

static int winTreeSetValueAttrib(Ihandle* ih, const char* value)
{
  HTREEITEM hItem = NULL;
  HTREEITEM hItemFocus;

  if (winTreeSetMarkAttrib(ih, value))
    return 0;

  hItemFocus = iupdrvTreeGetFocusNode(ih);

  if(iupStrEqualNoCase(value, "ROOT") || iupStrEqualNoCase(value, "FIRST"))
    hItem = (HTREEITEM)SendMessage(ih->handle, TVM_GETNEXTITEM, TVGN_ROOT, 0);
  else if(iupStrEqualNoCase(value, "LAST"))
    hItem = (HTREEITEM)SendMessage(ih->handle, TVM_GETNEXTITEM, TVGN_LASTVISIBLE, 0);
  else if(iupStrEqualNoCase(value, "PGUP"))
  {
    int i;
    HTREEITEM hItemPrev = hItemFocus;
    HTREEITEM hItemNext = hItemFocus;
    for(i = 0; i < 10; i++)
    {
      hItemNext = hItemPrev;
      hItemPrev = (HTREEITEM)SendMessage(ih->handle, TVM_GETNEXTITEM, TVGN_PREVIOUSVISIBLE, (LPARAM)hItemPrev);
      if(hItemPrev == NULL)
      {
        hItemPrev = hItemNext;
        break;
      }
    }
    
    hItem = hItemPrev;
  }
  else if(iupStrEqualNoCase(value, "PGDN"))
  {
    int i;
    HTREEITEM hItemPrev = hItemFocus;
    HTREEITEM hItemNext = hItemFocus;
    
    for(i = 0; i < 10; i++)
    {
      hItemPrev = hItemNext;
      hItemNext = (HTREEITEM)SendMessage(ih->handle, TVM_GETNEXTITEM, TVGN_NEXTVISIBLE, (LPARAM)hItemNext);
      if(hItemNext == NULL)
      {
        hItemNext = hItemPrev;
        break;
      }
    }
    
    hItem = hItemNext;
  }
  else if(iupStrEqualNoCase(value, "NEXT"))
    hItem = (HTREEITEM)SendMessage(ih->handle, TVM_GETNEXTITEM, TVGN_NEXTVISIBLE, (LPARAM)hItemFocus);
  else if(iupStrEqualNoCase(value, "PREVIOUS"))
    hItem = (HTREEITEM)SendMessage(ih->handle, TVM_GETNEXTITEM, TVGN_PREVIOUSVISIBLE, (LPARAM)hItemFocus);
  else
    hItem = iupTreeGetNodeFromString(ih, value);

  if (hItem)
  {
    if (ih->data->mark_mode==ITREE_MARK_SINGLE)
    {
      winTreeSelectNode(ih, hItemFocus, 0);
      winTreeSelectNode(ih, hItem, 1);
    }
    winTreeSetFocusNode(ih, hItem);
  }

  return 0;
} 

void iupdrvTreeUpdateMarkMode(Ihandle *ih)
{
  /* does nothing, must handle single and multiple selection manually in Windows */
  (void)ih;
}

/*********************************************************************************************************/


static int winTreeEditProc(Ihandle* ih, HWND cbedit, UINT msg, WPARAM wp, LPARAM lp, LRESULT *result)
{
  switch (msg)
  {
    case WM_GETDLGCODE:
    {
      MSG* pMsg = (MSG*)lp;

      if (pMsg && (pMsg->message == WM_KEYDOWN || pMsg->message == WM_SYSKEYDOWN))
      {
        if (pMsg->wParam == VK_ESCAPE || pMsg->wParam == VK_RETURN)
        {
          /* these keys are not processed if the return code is not this */
          *result = DLGC_WANTALLKEYS;
          return 1;
        }
      }
    }
  }
  
  (void)wp;
  (void)cbedit;
  (void)ih;
  return 0;
}

static LRESULT CALLBACK winTreeEditWinProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{   
  int ret = 0;
  LRESULT result = 0;
  WNDPROC oldProc;
  Ihandle *ih;

  ih = iupwinHandleGet(hwnd); 
  if (!ih)
    return DefWindowProc(hwnd, msg, wp, lp);  /* should never happen */

  /* retrieve the control previous procedure for subclassing */
  oldProc = (WNDPROC)IupGetCallback(ih, "_IUPWIN_EDITOLDPROC_CB");

  ret = winTreeEditProc(ih, hwnd, msg, wp, lp, &result);

  if (ret)
    return result;
  else
    return CallWindowProc(oldProc, hwnd, msg, wp, lp);
}

static void winTreeDragBegin(Ihandle* ih, int x, int y)
{
  HIMAGELIST  dragImageList;

  HTREEITEM hItemDrag = winTreeFindNodeXY(ih, x, y);
  if (!hItemDrag)
    return;

  SendMessage(ih->handle, TVM_ENDEDITLABELNOW, TRUE, 0);

  /* store the drag-and-drop item */
  iupAttribSetStr(ih, "_IUPTREE_DRAGITEM", (char*)hItemDrag);

  /* get the image list for dragging */
  dragImageList = (HIMAGELIST)SendMessage(ih->handle, TVM_CREATEDRAGIMAGE, 0, (LPARAM)hItemDrag);
  if (dragImageList)
  {
    POINT pt;
    ImageList_BeginDrag(dragImageList, 0, 0, 0);

    pt.x = x;
    pt.y = y;

    ClientToScreen(ih->handle, &pt);
    ImageList_DragEnter(NULL, pt.x, pt.y);

    iupAttribSetStr(ih, "_IUPTREE_DRAGIMAGELIST", (char*)dragImageList);
  }

  ShowCursor(FALSE);
  SetCapture(ih->handle);  /* drag only inside the tree */
}

static void winTreeDragMove(Ihandle* ih, int x, int y)
{
  HTREEITEM	hItemDrop = winTreeFindNodeXY(ih, x, y);
  HIMAGELIST dragImageList = (HIMAGELIST)iupAttribGet(ih, "_IUPTREE_DRAGIMAGELIST");

  if (dragImageList)
  {
    POINT pt;
    pt.x = x;
    pt.y = y;
    ClientToScreen(ih->handle, &pt);
    ImageList_DragMove(pt.x, pt.y);
  }

  if (hItemDrop)
  {
    if (!iupAttribGetBoolean(ih, "DROPEQUALDRAG"))
    {
      HTREEITEM hItemDrag = (HTREEITEM)iupAttribGet(ih, "_IUPTREE_DRAGITEM");
      if (hItemDrop == hItemDrag)
      {
        iupAttribSetStr(ih, "_IUPTREE_DROPITEM", NULL);
        return;
      }
    }

    if(dragImageList)
      ImageList_DragShowNolock(FALSE);

    SendMessage(ih->handle, TVM_SETINSERTMARK, TRUE, (LPARAM)hItemDrop);

    /* store the drop item to be executed */
    iupAttribSetStr(ih, "_IUPTREE_DROPITEM", (char*)hItemDrop);

    if(dragImageList)
      ImageList_DragShowNolock(TRUE);
  }
  else
    iupAttribSetStr(ih, "_IUPTREE_DROPITEM", NULL);
}

static void winTreeDragDrop(Ihandle* ih)
{
  HTREEITEM	 hItemDrag     =  (HTREEITEM)iupAttribGet(ih, "_IUPTREE_DRAGITEM");
  HTREEITEM	 hItemDrop     =  (HTREEITEM)iupAttribGet(ih, "_IUPTREE_DROPITEM");
  HIMAGELIST dragImageList = (HIMAGELIST)iupAttribGet(ih, "_IUPTREE_DRAGIMAGELIST");
  HTREEITEM  hParent;
  int equal_nodes = 0;
  int is_ctrl;

  if (dragImageList)
  {
    ImageList_DragLeave(ih->handle);
    ImageList_EndDrag();
    ImageList_Destroy(dragImageList);
    iupAttribSetStr(ih, "_IUPTREE_DRAGIMAGELIST", NULL);
  }

  ReleaseCapture();
  ShowCursor(TRUE);

  /* Remove drop target highlighting */
  SendMessage(ih->handle, TVM_SETINSERTMARK, 0, (LPARAM)NULL);

  iupAttribSetStr(ih, "_IUPTREE_DRAGITEM", NULL);
  iupAttribSetStr(ih, "_IUPTREE_DROPITEM", NULL);

  if (!hItemDrop || !hItemDrag)
    return;

  /* If Drag item is an ancestor or equal to Drop item then return */
  hParent = hItemDrop;
  while(hParent)
  {
    if (hParent == hItemDrag)
    {
      if (!iupAttribGetBoolean(ih, "DROPEQUALDRAG"))
        return;

      equal_nodes = 1;
      break;
    }

    hParent = (HTREEITEM)SendMessage(ih->handle, TVM_GETNEXTITEM, TVGN_PARENT, (LPARAM)hParent);
  }

  if (winTreeCallDragDropCb(ih, hItemDrag, hItemDrop, &is_ctrl) == IUP_CONTINUE && !equal_nodes)
  {
    /* Copy or move the dragged item to the new position. */
    HTREEITEM  hItemNew = winTreeCopyMoveNode(ih, hItemDrag, hItemDrop, is_ctrl);

    /* Set focus and selection */
    if (hItemNew)
      SendMessage(ih->handle, TVM_SELECTITEM, TVGN_CARET, (LPARAM)hItemNew); 
  }
}  

static void winTreeExtendSelect(Ihandle* ih, int x, int y)
{
  HTREEITEM hItemFirstSel;
  TVHITTESTINFO info;
  HTREEITEM hItem;
  info.pt.x = x;
  info.pt.y = y;
  hItem = (HTREEITEM)SendMessage(ih->handle, TVM_HITTEST, 0, (LPARAM)&info);

  if (!(info.flags & TVHT_ONITEM) || !hItem)
    return;

  hItemFirstSel = (HTREEITEM)iupAttribGet(ih, "_IUPTREE_FIRSTSELITEM");
  if (hItemFirstSel)
  {
    winTreeSelectRange(ih, hItemFirstSel, hItem, 1);

    iupAttribSetStr(ih, "_IUPTREE_LASTSELITEM", (char*)hItem);
    winTreeSetFocusNode(ih, hItem);
  }
}

static int winTreeMouseMultiSelect(Ihandle* ih, int x, int y)
{
  TVHITTESTINFO info;
  HTREEITEM hItem;
  info.pt.x = x;
  info.pt.y = y;
  hItem = (HTREEITEM)SendMessage(ih->handle, TVM_HITTEST, 0, (LPARAM)&info);

  if (!(info.flags & TVHT_ONITEM) || !hItem)
    return 0;

  if (GetKeyState(VK_CONTROL) & 0x8000) /* Control key is down */
  {
    /* Toggle selection state */
    winTreeSelectNode(ih, hItem, -1);
    iupAttribSetStr(ih, "_IUPTREE_FIRSTSELITEM", (char*)hItem);

    winTreeCallSelectionCb(ih, winTreeIsNodeSelected(ih, hItem), hItem);
    winTreeSetFocusNode(ih, hItem);

    return 1;
  }
  else if (GetKeyState(VK_SHIFT) & 0x8000) /* Shift key is down */
  {
    HTREEITEM hItemFirstSel = (HTREEITEM)iupAttribGet(ih, "_IUPTREE_FIRSTSELITEM");
    if (hItemFirstSel)
    {
      int last_id = iupTreeFindNodeId(ih, hItem);
      winTreeSelectRange(ih, hItemFirstSel, hItem, 1);

      /* if last selected item is a branch, then select its children */
      iupTreeSelectLastCollapsedBranch(ih, &last_id);

      winTreeCallMultiSelectionCb(ih);
      winTreeSetFocusNode(ih, hItem);
      return 1;
    }
  }

  winTreeCallMultiUnSelectionCb(ih);

  /* simple click */
  winTreeClearSelection(ih, hItem);
  iupAttribSetStr(ih, "_IUPTREE_FIRSTSELITEM", (char*)hItem);
  iupAttribSetStr(ih, "_IUPTREE_EXTENDSELECT", "1");

  /* Call SELECT_CB for all unselected nodes */

  return 0;
}

static void winTreeCallRightClickCb(Ihandle* ih, int x, int y)
{
  HTREEITEM hItem = winTreeFindNodeXY(ih, x, y);
  if (hItem)
  {
    IFni cbRightClick = (IFni)IupGetCallback(ih, "RIGHTCLICK_CB");
    if (cbRightClick)
      cbRightClick(ih, iupTreeFindNodeId(ih, hItem));
  }
}

static int winTreeProc(Ihandle* ih, UINT msg, WPARAM wp, LPARAM lp, LRESULT *result)
{
  switch (msg)
  {
  case WM_CTLCOLOREDIT:
    {
      HWND hEdit = (HWND)iupAttribGet(ih, "_IUPWIN_EDITBOX");
      if (hEdit)
      {
        winTreeItemData* itemData = (winTreeItemData*)iupAttribGet(ih, "_IUPWIN_EDIT_DATA");
        HDC hDC = (HDC)wp;
        COLORREF cr;

        SetTextColor(hDC, itemData->color);

        cr = (COLORREF)SendMessage(ih->handle, TVM_GETBKCOLOR, 0, 0);
        SetBkColor(hDC, cr);
        SetDCBrushColor(hDC, cr);
        *result = (LRESULT)GetStockObject(DC_BRUSH);
        return 1;
      }

      break;
    }
  case WM_SETFOCUS:
  case WM_KILLFOCUS:
    {
      /*   ------------Comment from wxWidgets--------------------
        the tree control greys out the selected item when it loses focus and
        paints it as selected again when it regains it, but it won't do it
        for the other items itself - help it */
      if (ih->data->mark_mode == ITREE_MARK_MULTIPLE)
      {
        HTREEITEM hItemChild = (HTREEITEM)SendMessage(ih->handle, TVM_GETNEXTITEM, TVGN_FIRSTVISIBLE, 0);
        RECT rect;

        while(hItemChild != NULL)
        {
          *(HTREEITEM*)&rect = hItemChild;
          if (SendMessage(ih->handle, TVM_GETITEMRECT, TRUE, (LPARAM)&rect))
            InvalidateRect(ih->handle, &rect, FALSE);

          /* Go to next visible item */
          hItemChild = (HTREEITEM)SendMessage(ih->handle, TVM_GETNEXTITEM, TVGN_NEXTVISIBLE, (LPARAM)hItemChild);
        }
      }
      break;
    }
  case WM_KEYDOWN:
  case WM_SYSKEYDOWN:
    {
      if (iupwinBaseProc(ih, msg, wp, lp, result)==1)
        return 1;

      if (wp == VK_RETURN)
      {
        HTREEITEM hItemFocus = iupdrvTreeGetFocusNode(ih);
        if (winTreeCallBranchLeafCb(ih, hItemFocus) != IUP_IGNORE)
          winTreeExpandItem(ih, hItemFocus, -1);

        *result = 0;
        return 1;
      }      
      else if (wp == VK_F2)
      {
        winTreeSetRenameAttrib(ih, NULL);
        *result = 0;
        return 1;
      }
      else if (wp == VK_SPACE)
      {
        if (GetKeyState(VK_CONTROL) & 0x8000)
        {
          HTREEITEM hItemFocus = iupdrvTreeGetFocusNode(ih);
          /* Toggle selection state */
          winTreeSelectNode(ih, hItemFocus, -1);
        }
      }
      else if (wp == VK_UP || wp == VK_DOWN)
      {
        HTREEITEM hItemFocus = iupdrvTreeGetFocusNode(ih);
        if (wp == VK_UP)
          hItemFocus = (HTREEITEM)SendMessage(ih->handle, TVM_GETNEXTITEM, TVGN_PREVIOUSVISIBLE, (LPARAM)hItemFocus);
        else
          hItemFocus = (HTREEITEM)SendMessage(ih->handle, TVM_GETNEXTITEM, TVGN_NEXTVISIBLE, (LPARAM)hItemFocus);
        if (!hItemFocus)
          return 0;

        if (GetKeyState(VK_CONTROL) & 0x8000)
        {
          /* Only move focus */
          winTreeSetFocusNode(ih, hItemFocus);

          *result = 0;
          return 1;
        }
        else if (GetKeyState(VK_SHIFT) & 0x8000)
        {
          HTREEITEM hItemFirstSel = (HTREEITEM)iupAttribGet(ih, "_IUPTREE_FIRSTSELITEM");
          if (hItemFirstSel)
          {
            winTreeSelectRange(ih, hItemFirstSel, hItemFocus, 1);

            winTreeCallMultiSelectionCb(ih);
            winTreeSetFocusNode(ih, hItemFocus);

            *result = 0;
            return 1;
          }
        }

        if (ih->data->mark_mode==ITREE_MARK_MULTIPLE)
        {
          iupAttribSetStr(ih, "_IUPTREE_FIRSTSELITEM", (char*)hItemFocus);
          winTreeClearSelection(ih, NULL);
          /* normal processing will select the focus item */
        }
      }

      return 0;
    }
  case WM_LBUTTONDOWN:
    if (iupwinButtonDown(ih, msg, wp, lp)==-1)
    {
      *result = 0;
      return 1;
    }

    if (ih->data->mark_mode==ITREE_MARK_MULTIPLE)
    {
      /* must set focus on left button down or the tree won't show its focus */
      if (iupAttribGetBoolean(ih, "CANFOCUS"))
        SetFocus(ih->handle);

      if (winTreeMouseMultiSelect(ih, (int)(short)LOWORD(lp), (int)(short)HIWORD(lp)))
      {
        *result = 0; /* abort the normal processing if we process multiple selection */
        return 1;
      }
    }
    break;
  case WM_RBUTTONDOWN:
    winTreeCallRightClickCb(ih, (int)(short)LOWORD(lp), (int)(short)HIWORD(lp));
    *result = 0;
    return 1;  /* must abort the normal behavior, because it is weird and just causes trouble */
  case WM_MBUTTONDOWN:
  case WM_LBUTTONDBLCLK:
  case WM_MBUTTONDBLCLK:
  case WM_RBUTTONDBLCLK:
    if (iupwinButtonDown(ih, msg, wp, lp)==-1)
    {
      *result = 0;
      return 1;
    }
    break;
  case WM_MOUSEMOVE:
    if (ih->data->show_dragdrop && (wp & MK_LBUTTON))
    {
      if (!iupAttribGet(ih, "_IUPTREE_DRAGITEM"))
        winTreeDragBegin(ih, (int)(short)LOWORD(lp), (int)(short)HIWORD(lp));
      else 
        winTreeDragMove(ih, (int)(short)LOWORD(lp), (int)(short)HIWORD(lp));
    }
    else if (iupAttribGet(ih, "_IUPTREE_EXTENDSELECT"))
    {
      if (wp & MK_LBUTTON)
        winTreeExtendSelect(ih, (int)(short)LOWORD(lp), (int)(short)HIWORD(lp));
      else
        iupAttribSetStr(ih, "_IUPTREE_EXTENDSELECT", NULL);
    }

    iupwinMouseMove(ih, msg, wp, lp);
    break;
  case WM_LBUTTONUP:
  case WM_MBUTTONUP:
  case WM_RBUTTONUP:
    if (iupwinButtonUp(ih, msg, wp, lp)==-1)
    {
      *result = 0;
      return 1;
    }

    if (iupAttribGet(ih, "_IUPTREE_EXTENDSELECT"))
    {
      iupAttribSetStr(ih, "_IUPTREE_EXTENDSELECT", NULL);

      if (iupAttribGet(ih, "_IUPTREE_LASTSELITEM"))
      {
        winTreeCallMultiSelectionCb(ih);
        iupAttribSetStr(ih, "_IUPTREE_LASTSELITEM", NULL);
      }
    }

    if (ih->data->show_dragdrop && (HTREEITEM)iupAttribGet(ih, "_IUPTREE_DRAGITEM") != NULL)
      winTreeDragDrop(ih);

    break;
  case WM_CHAR:
    {
      if (wp==VK_TAB)  /* the keys have the same definitions as the chars */
      {
        *result = 0;
        return 1;   /* abort default processing to avoid beep */
      }
      break;
    }
  }

  return iupwinBaseProc(ih, msg, wp, lp, result);
}

static COLORREF winTreeInvertColor(COLORREF color)
{
  return RGB(~GetRValue(color), ~GetGValue(color), ~GetBValue(color));
}

static int winTreeWmNotify(Ihandle* ih, NMHDR* msg_info, int *result)
{
  if (msg_info->code == TVN_ITEMCHANGINGA || msg_info->code == TVN_ITEMCHANGINGW) /* Vista Only */
  {
    if (ih->data->mark_mode==ITREE_MARK_MULTIPLE)
    {
      NMTVITEMCHANGE* info = (NMTVITEMCHANGE*)msg_info;
      HTREEITEM hItem = (HTREEITEM)iupAttribGet(ih, "_IUPTREE_ALLOW_CHANGE");
      if (hItem && hItem != info->hItem)  /* Workaround for Vista SetFocus */
      {
        *result = TRUE;  /* prevent the change */
        return 1;
      }
    }
  }
  else if (msg_info->code == TVN_SELCHANGED)
  {
    NMTREEVIEW* info = (NMTREEVIEW*)msg_info;
    if (ih->data->mark_mode!=ITREE_MARK_MULTIPLE || /* (NOT) Multiple selection with Control or Shift key is down */
        !(GetKeyState(VK_CONTROL) & 0x8000 || GetKeyState(VK_SHIFT) & 0x8000)) 
    {
      winTreeCallSelectionCb(ih, 0, info->itemOld.hItem);  /* node unselected */
      winTreeCallSelectionCb(ih, 1, info->itemNew.hItem);  /* node selected */
    }
  }
  else if(msg_info->code == TVN_BEGINLABELEDIT)
  {
    char* value;
    HWND hEdit;
    NMTVDISPINFO* info = (NMTVDISPINFO*)msg_info;
    IFni cbShowRename;

    if (!ih->data->show_rename)
    {
      *result = TRUE;  /* prevent the change */
      return 1;
    }
           
    if (iupAttribGet(ih, "_IUPTREE_EXTENDSELECT"))
    {
      *result = TRUE;  /* prevent the change */
      return 1;
    }

    cbShowRename = (IFni)IupGetCallback(ih, "SHOWRENAME_CB");
    if (cbShowRename && cbShowRename(ih, iupTreeFindNodeId(ih, info->item.hItem))==IUP_IGNORE)
    {
      *result = TRUE;  /* prevent the change */
      return 1;
    }

    hEdit = (HWND)SendMessage(ih->handle, TVM_GETEDITCONTROL, 0, 0);

    /* save the edit box. */
    iupwinHandleAdd(ih, hEdit);
    iupAttribSetStr(ih, "_IUPWIN_EDITBOX", (char*)hEdit);

    /* subclass the edit box. */
    IupSetCallback(ih, "_IUPWIN_EDITOLDPROC_CB", (Icallback)GetWindowLongPtr(hEdit, GWLP_WNDPROC));
    SetWindowLongPtr(hEdit, GWLP_WNDPROC, (LONG_PTR)winTreeEditWinProc);

    value = iupAttribGetStr(ih, "RENAMECARET");
    if (value)
      winTreeSetRenameCaretPos(hEdit, value);

    value = iupAttribGetStr(ih, "RENAMESELECTION");
    if (value)
      winTreeSetRenameSelectionPos(hEdit, value);

    {
      winTreeItemData* itemData;
      TVITEM item; 
      item.hItem = info->item.hItem;
      item.mask = TVIF_HANDLE | TVIF_PARAM;
      SendMessage(ih->handle, TVM_GETITEM, 0, (LPARAM)(LPTVITEM)&item);
      itemData = (winTreeItemData*)item.lParam;
      iupAttribSetStr(ih, "_IUPWIN_EDIT_DATA", (char*)itemData);

      if (itemData->hFont)
        SendMessage(hEdit, WM_SETFONT, (WPARAM)itemData->hFont, MAKELPARAM(TRUE,0));
    }
  }
  else if(msg_info->code == TVN_ENDLABELEDIT)
  {
    IFnis cbRename;
    NMTVDISPINFO* info = (NMTVDISPINFO*)msg_info;

    iupAttribSetStr(ih, "_IUPWIN_EDITBOX", NULL);

    if (!info->item.pszText)  /* cancel, so abort */
      return 0;

    cbRename = (IFnis)IupGetCallback(ih, "RENAME_CB");
    if (cbRename)
    {
      if (cbRename(ih, iupTreeFindNodeId(ih, info->item.hItem), info->item.pszText) == IUP_IGNORE)
      {
        *result = FALSE;
        return 1;
      }
    }

    *result = TRUE;
    return 1;
  }
  else if(msg_info->code == NM_DBLCLK)
  {
    HTREEITEM hItemFocus = iupdrvTreeGetFocusNode(ih);
    TVITEM item;
    winTreeItemData* itemData;

    /* Get Children: branch or leaf */
    item.mask = TVIF_HANDLE|TVIF_PARAM; 
    item.hItem = hItemFocus;
    SendMessage(ih->handle, TVM_GETITEM, 0, (LPARAM)(LPTVITEM)&item);
    itemData = (winTreeItemData*)item.lParam;

    if (itemData->kind == ITREE_LEAF)
    {
      IFni cbExecuteLeaf = (IFni)IupGetCallback(ih, "EXECUTELEAF_CB");
      if(cbExecuteLeaf)
        cbExecuteLeaf(ih, iupTreeFindNodeId(ih, hItemFocus));
    }
  }
  else if(msg_info->code == TVN_ITEMEXPANDING)
  {
    /* mouse click by user: click on the "+" sign or double click on the selected node */
    NMTREEVIEW* tree_info = (NMTREEVIEW*)msg_info;
    HTREEITEM hItem = tree_info->itemNew.hItem;

    if (winTreeCallBranchLeafCb(ih, hItem) != IUP_IGNORE)
    {
      TVITEM item;
      winTreeItemData* itemData = (winTreeItemData*)tree_info->itemNew.lParam;

      if (tree_info->action == TVE_EXPAND)
        item.iSelectedImage = item.iImage = (itemData->image_expanded!=-1)? itemData->image_expanded: (int)ih->data->def_image_expanded;
      else
        item.iSelectedImage = item.iImage = (itemData->image!=-1)? itemData->image: (int)ih->data->def_image_collapsed;

      item.hItem = hItem;
      item.mask = TVIF_HANDLE | TVIF_IMAGE | TVIF_SELECTEDIMAGE;
      SendMessage(ih->handle, TVM_SETITEM, 0, (LPARAM)(LPTVITEM)&item);
    }
    else
    {
      *result = TRUE;  /* prevent the change */
      return 1;
    }
  }
  else if (msg_info->code == NM_CUSTOMDRAW)
  {
    NMTVCUSTOMDRAW *customdraw = (NMTVCUSTOMDRAW*)msg_info;

    if (customdraw->nmcd.dwDrawStage == CDDS_PREPAINT)
    {
      *result = CDRF_NOTIFYPOSTPAINT|CDRF_NOTIFYITEMDRAW;
      return 1;
    }
 
    if (customdraw->nmcd.dwDrawStage == CDDS_ITEMPREPAINT)
    {
      TVITEM item;
      winTreeItemData* itemData;
      HTREEITEM hItem = (HTREEITEM)customdraw->nmcd.dwItemSpec;

      item.hItem = hItem;
      item.mask = TVIF_HANDLE | TVIF_PARAM;
      SendMessage(ih->handle, TVM_GETITEM, 0, (LPARAM)(LPTVITEM)&item);
      itemData = (winTreeItemData*)item.lParam;

      /* In XP after deleting an item there can be a redraw for that item */
      if (!itemData)
        return 0;

      if (GetFocus()==ih->handle && (customdraw->nmcd.uItemState & CDIS_SELECTED))
        customdraw->clrText = winTreeInvertColor(itemData->color);
      else
        customdraw->clrText = itemData->color;

      *result = CDRF_NOTIFYSUBITEMDRAW|CDRF_NOTIFYPOSTPAINT;

      if (itemData->hFont)
      {
        SelectObject(customdraw->nmcd.hdc, itemData->hFont);
        *result |= CDRF_NEWFONT;
      }

      return 1;
    }
  }
  else if (msg_info->code == TVN_GETINFOTIP)
  {
    NMTVGETINFOTIP* tip_info = (NMTVGETINFOTIP*)msg_info;
    IFnii cb;
    char* value;

    HWND tips_hwnd = (HWND)SendMessage(ih->handle, TVM_GETTOOLTIPS, 0, 0);
    if (!tips_hwnd)
      return 0;

    value = IupGetAttribute(ih, "TIP");  /* must use IupGetAttribute to use inheritance */
    if (!value)
      return 0;

    cb = (IFnii)IupGetCallback(ih, "TIPS_CB");
    if (cb)
    {
      int x, y;
      iupdrvGetCursorPos(&x, &y);
      iupdrvScreenToClient(ih, &x, &y);
      cb(ih, x, y);

      value = IupGetAttribute(ih, "TIP");  /* get again, because it could has been changed inside the callback */
    }

    iupStrCopyN(tip_info->pszText, tip_info->cchTextMax, value);

    iupwinTipsUpdateInfo(ih, tips_hwnd);
  }

  return 0;  /* allow the default processsing */
}

static int winTreeConvertXYToPos(Ihandle* ih, int x, int y)
{
  TVHITTESTINFO info;
  HTREEITEM hItem;
  info.pt.x = x;
  info.pt.y = y;
  hItem = (HTREEITEM)SendMessage(ih->handle, TVM_HITTEST, 0, (LPARAM)&info);
  if (hItem)
    return iupTreeFindNodeId(ih, hItem);
  return -1;
}


/*******************************************************************************************/


static int winTreeMapMethod(Ihandle* ih)
{
  DWORD dwStyle = WS_CHILD | WS_CLIPSIBLINGS | WS_BORDER | TVS_SHOWSELALWAYS | TVS_INFOTIP;

  /* styles can be set only on the Tree View creation */

  /* always disable the internal drag&drop, because it affects our selection and drawing */
  dwStyle |= TVS_DISABLEDRAGDROP;

  if (ih->data->show_rename)
    dwStyle |= TVS_EDITLABELS;

  if (!iupAttribGetBoolean(ih, "HIDELINES"))
  {
    dwStyle |= TVS_HASLINES;

    if (!iupAttribGetInt(ih, "ADDROOT"))
      dwStyle |= TVS_LINESATROOT;
  }

  if (!iupAttribGetBoolean(ih, "HIDEBUTTONS"))
    dwStyle |= TVS_HASBUTTONS;

  if (iupAttribGetBoolean(ih, "CANFOCUS"))
    dwStyle |= WS_TABSTOP;

  if (!ih->parent)
    return IUP_ERROR;

  if (!iupwinCreateWindowEx(ih, WC_TREEVIEW, 0, dwStyle))
    return IUP_ERROR;

  if (!iupwin_comctl32ver6)  /* To improve drawing of items when TITLEFONT is set */
    SendMessage(ih->handle, CCM_SETVERSION, 5, 0); 
  else
    SendMessage(ih->handle, TVM_SETEXTENDEDSTYLE, TVS_EX_DOUBLEBUFFER, TVS_EX_DOUBLEBUFFER); 

  IupSetCallback(ih, "_IUPWIN_CTRLPROC_CB", (Icallback)winTreeProc);
  IupSetCallback(ih, "_IUPWIN_NOTIFY_CB",   (Icallback)winTreeWmNotify);

  /* Force background update before setting the images */
  {
    char* value = iupAttribGet(ih, "BGCOLOR");
    if (value)
    {
      winTreeSetBgColorAttrib(ih, value);
      iupAttribSetStr(ih, "BGCOLOR", NULL);
    }
    else if (!iupwin_comctl32ver6 || iupwinGetSystemMajorVersion()<6) /* force background in XP because of the editbox background */
      winTreeSetBgColorAttrib(ih, IupGetGlobal("TXTBGCOLOR"));
  }

  /* Initialize the default images */
  ih->data->def_image_leaf = (void*)winTreeGetImageIndex(ih, "IMGLEAF");
  ih->data->def_image_collapsed = (void*)winTreeGetImageIndex(ih, "IMGCOLLAPSED");
  ih->data->def_image_expanded = (void*)winTreeGetImageIndex(ih, "IMGEXPANDED");

  if (iupAttribGetInt(ih, "ADDROOT"))
    iupdrvTreeAddNode(ih, -1, ITREE_BRANCH, "", 0);

  /* configure for DRAG&DROP of files */
  if (IupGetCallback(ih, "DROPFILES_CB"))
    iupAttribSetStr(ih, "DRAGDROP", "YES");

  IupSetCallback(ih, "_IUP_XY2POS_CB", (Icallback)winTreeConvertXYToPos);

  iupdrvTreeUpdateMarkMode(ih);

  return IUP_NOERROR;
}

static void winTreeUnMapMethod(Ihandle* ih)
{
  Iarray* bmp_array;
  HIMAGELIST image_list;

  winTreeRemoveAllNodeData(ih, 0);

  ih->data->node_count = 0;

  image_list = (HIMAGELIST)SendMessage(ih->handle, TVM_GETIMAGELIST, TVSIL_NORMAL, 0);
  if (image_list)
    ImageList_Destroy(image_list);

  bmp_array = (Iarray*)iupAttribGet(ih, "_IUPWIN_BMPARRAY");
  if (bmp_array)
    iupArrayDestroy(bmp_array);

  iupdrvBaseUnMapMethod(ih);
}

void iupdrvTreeInitClass(Iclass* ic)
{
  /* Driver Dependent Class functions */
  ic->Map = winTreeMapMethod;
  ic->UnMap = winTreeUnMapMethod;

  /* Visual */
  iupClassRegisterAttribute(ic, "BGCOLOR", winTreeGetBgColorAttrib, winTreeSetBgColorAttrib, IUPAF_SAMEASSYSTEM, "TXTBGCOLOR", IUPAF_NO_SAVE|IUPAF_DEFAULT);
  iupClassRegisterAttribute(ic, "FGCOLOR", NULL, winTreeSetFgColorAttrib, IUPAF_SAMEASSYSTEM, "TXTFGCOLOR", IUPAF_DEFAULT);
  iupClassRegisterAttribute(ic, "AUTOREDRAW", NULL, iupwinSetAutoRedrawAttrib, IUPAF_SAMEASSYSTEM, "Yes", IUPAF_WRITEONLY|IUPAF_NO_INHERIT);

  /* Redefined */
  iupClassRegisterAttribute(ic, "TIP", NULL, NULL, NULL, NULL, IUPAF_NO_DEFAULTVALUE|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "TIPVISIBLE", winTreeGetTipVisibleAttrib, winTreeSetTipVisibleAttrib, NULL, NULL, IUPAF_NO_INHERIT);

  /* IupTree Attributes - GENERAL */
  iupClassRegisterAttribute(ic, "EXPANDALL",  NULL, winTreeSetExpandAllAttrib, NULL, NULL, IUPAF_WRITEONLY|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "INDENTATION", winTreeGetIndentationAttrib, winTreeSetIndentationAttrib, NULL, NULL, IUPAF_DEFAULT);
  iupClassRegisterAttribute(ic, "DRAGDROP", NULL, iupwinSetDragDropAttrib, NULL, NULL, IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "SPACING", iupTreeGetSpacingAttrib, winTreeSetSpacingAttrib, IUPAF_SAMEASSYSTEM, "0", IUPAF_NOT_MAPPED);
  iupClassRegisterAttribute(ic, "TOPITEM", NULL, winTreeSetTopItemAttrib, NULL, NULL, IUPAF_WRITEONLY|IUPAF_NO_INHERIT);

  /* IupTree Attributes - IMAGES */
  iupClassRegisterAttributeId(ic, "IMAGE", NULL, winTreeSetImageAttrib, IUPAF_IHANDLENAME|IUPAF_WRITEONLY|IUPAF_NO_INHERIT);
  iupClassRegisterAttributeId(ic, "IMAGEEXPANDED", NULL, winTreeSetImageExpandedAttrib, IUPAF_IHANDLENAME|IUPAF_WRITEONLY|IUPAF_NO_INHERIT);
  
  iupClassRegisterAttribute(ic, "IMAGELEAF",            NULL, winTreeSetImageLeafAttrib, IUPAF_SAMEASSYSTEM, "IMGLEAF", IUPAF_IHANDLENAME|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "IMAGEBRANCHCOLLAPSED", NULL, winTreeSetImageBranchCollapsedAttrib, IUPAF_SAMEASSYSTEM, "IMGCOLLAPSED", IUPAF_IHANDLENAME|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "IMAGEBRANCHEXPANDED",  NULL, winTreeSetImageBranchExpandedAttrib, IUPAF_SAMEASSYSTEM, "IMGEXPANDED", IUPAF_IHANDLENAME|IUPAF_NO_INHERIT);

  /* IupTree Attributes - NODES */
  iupClassRegisterAttributeId(ic, "STATE",  winTreeGetStateAttrib,  winTreeSetStateAttrib, IUPAF_NO_INHERIT);
  iupClassRegisterAttributeId(ic, "DEPTH",  winTreeGetDepthAttrib,  NULL, IUPAF_READONLY|IUPAF_NO_INHERIT);
  iupClassRegisterAttributeId(ic, "KIND",   winTreeGetKindAttrib,   NULL, IUPAF_READONLY|IUPAF_NO_INHERIT);
  iupClassRegisterAttributeId(ic, "PARENT", winTreeGetParentAttrib, NULL, IUPAF_READONLY|IUPAF_NO_INHERIT);
  iupClassRegisterAttributeId(ic, "NAME",   winTreeGetTitleAttrib,   winTreeSetTitleAttrib, IUPAF_NO_INHERIT);
  iupClassRegisterAttributeId(ic, "TITLE",   winTreeGetTitleAttrib,   winTreeSetTitleAttrib, IUPAF_NO_INHERIT);
  iupClassRegisterAttributeId(ic, "CHILDCOUNT",   winTreeGetChildCountAttrib,   NULL, IUPAF_READONLY|IUPAF_NO_INHERIT);
  iupClassRegisterAttributeId(ic, "COLOR",  winTreeGetColorAttrib,  winTreeSetColorAttrib, IUPAF_NO_INHERIT);
  iupClassRegisterAttributeId(ic, "TITLEFONT", winTreeGetTitleFontAttrib, winTreeSetTitleFontAttrib, IUPAF_NO_INHERIT);

  /* IupTree Attributes - MARKS */
  iupClassRegisterAttributeId(ic, "MARKED",   winTreeGetMarkedAttrib,   winTreeSetMarkedAttrib,   IUPAF_NO_INHERIT);
  iupClassRegisterAttribute  (ic, "MARK",    NULL,    winTreeSetMarkAttrib,    NULL, NULL, IUPAF_WRITEONLY|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute  (ic, "STARTING", NULL, winTreeSetMarkStartAttrib, NULL, NULL, IUPAF_NO_DEFAULTVALUE|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute  (ic, "MARKSTART", NULL, winTreeSetMarkStartAttrib, NULL, NULL, IUPAF_NO_DEFAULTVALUE|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute  (ic, "MARKEDNODES",    winTreeGetMarkedNodesAttrib,    winTreeSetMarkedNodesAttrib,    NULL, NULL, IUPAF_NO_SAVE|IUPAF_NO_DEFAULTVALUE|IUPAF_NO_INHERIT);

  iupClassRegisterAttribute  (ic, "VALUE",    winTreeGetValueAttrib,    winTreeSetValueAttrib,    NULL, NULL, IUPAF_NO_DEFAULTVALUE|IUPAF_NO_INHERIT);

  /* IupTree Attributes - ACTION */
  iupClassRegisterAttributeId(ic, "DELNODE", NULL, winTreeSetDelNodeAttrib, IUPAF_NOT_MAPPED|IUPAF_WRITEONLY|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "RENAME",  NULL, winTreeSetRenameAttrib,  NULL, NULL, IUPAF_WRITEONLY|IUPAF_NO_INHERIT);
  iupClassRegisterAttributeId(ic, "MOVENODE",  NULL, winTreeSetMoveNodeAttrib,  IUPAF_NOT_MAPPED|IUPAF_WRITEONLY|IUPAF_NO_INHERIT);
  iupClassRegisterAttributeId(ic, "COPYNODE",  NULL, winTreeSetCopyNodeAttrib,  IUPAF_NOT_MAPPED|IUPAF_WRITEONLY|IUPAF_NO_INHERIT);

  /* necessary because transparent background does not work when not using visual styles */
  if (!iupwin_comctl32ver6)  /* Used by iupdrvImageCreateImage */
    iupClassRegisterAttribute(ic, "FLAT_ALPHA", NULL, NULL, IUPAF_SAMEASSYSTEM, "YES", IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);
}
