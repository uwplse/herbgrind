/*--------------------------------------------------------------------*/
/*--- HerbGrind: a valgrind tool for Herbie                stack.h ---*/
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

#ifndef _STACK_H
#define _STACK_H

#include "pub_tool_basics.h"
#include "pub_tool_tooliface.h"


// Dodgy C inheritance heavily encouraged.
typedef struct _StackNode {
  struct _StackNode* next;
} StackNode;

/* typedef struct _Stack Stack; */
typedef struct _Stack {
  StackNode* head;
} Stack;


Stack* mkStack(void);
void freeStack(Stack* s);
// WARNING: You are responsible for freeing anything you add to the
// stack, preferably after removing it or freeing the stack.
void stack_push(Stack* s, StackNode* item);
VG_REGPARM(2) void stack_push2(Stack* s, StackNode* item_node);
StackNode* stack_pop(Stack* s);
int stack_empty(Stack* s);

void addStackPushG(IRSB* sbOut, IRTemp guard, Stack* s, IRTemp node);
void addStackPush(IRSB* sbOut, Stack* s, IRTemp node);
IRTemp runStackPop(IRSB* sbOut, Stack* s);
IRTemp runStackPopG(IRSB* sbOut, IRTemp guard_temp, Stack* s);
IRTemp runStackEmpty(IRSB* sbOut, Stack* s);
IRTemp runStackEmptyG(IRSB* sbOut, IRTemp guard_temp, Stack* s);

#endif
