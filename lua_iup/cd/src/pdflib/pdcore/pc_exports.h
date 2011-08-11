/*---------------------------------------------------------------------------*
 |              PDFlib - A library for generating PDF on the fly             |
 +---------------------------------------------------------------------------+
 | Copyright (c) 1997-2006 Thomas Merz and PDFlib GmbH. All rights reserved. |
 +---------------------------------------------------------------------------+
 |                                                                           |
 |    This software is subject to the PDFlib license. It is NOT in the       |
 |    public domain. Extended versions and commercial licenses are           |
 |    available, please check http://www.pdflib.com.                         |
 |                                                                           |
 *---------------------------------------------------------------------------*/

/* $Id: pc_exports.h,v 1.2 2009/10/20 18:12:26 scuri Exp $
 *
 * Header for CodeWarrior to create a PDFlib DLL
 *
 */

/*
 * Force a DLL to be built.
 * This is useful as a prefix file when building a DLL with CodeWarrior.
 */

#define PDFLIB_EXPORTS
