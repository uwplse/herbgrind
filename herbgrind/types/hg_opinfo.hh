
/*--------------------------------------------------------------------*/
/*--- HerbGrind: a valgrind tool for Herbie           hg_opinfo.hh ---*/
/*--------------------------------------------------------------------*/

/*
   This file is part of HerbGrind, a valgrind tool for diagnosing
   floating point accuracy problems in binary programs and extracting
   problematic expressions.

   Copyright (C) 2016 Alex Sanchez-Stern

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License as
   published by the Free Software Foundation; either version 3 of the
   License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307, USA.

   The GNU General Public License is contained in the file COPYING.
*/

#ifndef _HG_OPINFO_HH
#define _HG_OPINFO_HH

// You probably don't want to include this outside of the .h files in
// this directory.
typedef struct _LoadG_Info LoadG_Info;
typedef struct _OpDebug_Info OpDebug_Info;
typedef struct _Eval_Info Eval_Info;
typedef struct _Unary_Args Unary_Args;
typedef struct _Binary_Args Binary_Args;
typedef struct _Ternary_Args Ternary_Args;
typedef struct _Quadnary_Args Quadnary_Args;
typedef struct _Op_Info Op_Info;

#endif
