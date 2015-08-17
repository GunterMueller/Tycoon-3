/*
 * This file is part of the Tycoon-2 system.
 *
 * The Tycoon-2 system is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation (Version 2).
 *
 * The Tycoon-2 system is distributed in the hope that it will be
 * useful, but WITHOUT ANY WARRANTY; without even the implied warranty
 * of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public
 * License along with the Tycoon-2 system; see the file LICENSE.
 * If not, write to AB 4.02, Softwaresysteme, TU Hamburg-Harburg
 * D-21071 Hamburg, Germany. http://www.sts.tu-harburg.de
 * 
 * Copyright (c) 1996-1998 Higher-Order GmbH, Hamburg. All rights reserved.
 *
 */
/*
  Copyright (c) 1996 Higher-Order GmbH, Hamburg. All rights reserved.

  $File: //depot/tycoon2/stsmain/tycoon2/src/tm/tmccall.h $ $Revision: #3 $ $Date: 2003/10/01 $ Marc Weikard

  CCall Interface
  
*/

#ifndef TMCCALL_H
#define TMCCALL_H

#include "tsp.h"
#include "tyc.h"


#ifdef __cplusplus
extern "C" {
#endif


#define MAX_CCALL_ARGUMENTS 20

extern tsp_OID tmccall_execute(tyc_ExternalMethod *, tsp_OID *);


#ifdef __cplusplus
}
#endif

#endif

