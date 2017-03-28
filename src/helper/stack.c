/*--------------------------------------------------------------------*/
/*--- Herbgrind: a valgrind tool for Herbie                stack.c ---*/
/*--------------------------------------------------------------------*/

/*
   This file is part of Herbgrind, a valgrind tool for diagnosing
   floating point accuracy problems in binary programs and extracting
   problematic expressions.

   Copyright (C) 2016-2017 Alex Sanchez-Stern

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

#include "stack.h"

#include <stddef.h>

#include "pub_tool_mallocfree.h"
#include "pub_tool_libcassert.h"
#include "pub_tool_machine.h"
#include "instrument-util.h"


Stack* mkStack(void){
  Stack* newStack = VG_(malloc)("stack", sizeof(Stack));
  newStack->head = NULL;
  return newStack;
}
void freeStack(Stack* s){
  VG_(free)(s);
}
void stack_push(Stack* s, StackNode* item_node){
  item_node->next = s->head;
  s->head = item_node;
}
StackNode* stack_pop(Stack* s){
  StackNode* oldHead = s->head;
  s->head = oldHead->next;
  return oldHead;
}
int stack_empty(Stack* s){
  return (s->head == NULL);
}
void addStackPushG(IRSB* sbOut, IRExpr* guard, Stack* s, IRExpr* node){
  IRExpr* sHead = runLoad64C(sbOut, &(s->head));
  addStoreArrowG(sbOut, guard, node, StackNode,
                 next, sHead);
  addStoreGC(sbOut, guard, node, &(s->head));
}
void addStackPush(IRSB* sbOut, Stack* s, IRExpr* node){
  IRExpr* sHead = runLoad64C(sbOut, &(s->head));
  addStoreArrow(sbOut, node, StackNode, next, sHead);
  addStoreC(sbOut, node, &(s->head));
}
IRExpr* runStackPop(IRSB* sbOut, Stack* s){
  IRExpr* oldHead = runLoad64C(sbOut, &(s->head));
  IRExpr* oldHeadNext = runArrow(sbOut,
                                 oldHead, StackNode, next);
  addStoreC(sbOut, oldHeadNext, &(s->head));
  return oldHead;
}
IRExpr* runStackPopG(IRSB* sbOut, IRExpr* guard, Stack* s){
  IRExpr* oldHead = runLoadG64C(sbOut, &(s->head), guard);
  IRExpr* oldHeadNext = runArrowG(sbOut, guard,
                                  oldHead, StackNode, next);
  addStoreGC(sbOut, guard, oldHeadNext, &(s->head));
  return oldHead;
}
IRExpr* runStackEmpty(IRSB* sbOut, Stack* s){
  return runZeroCheck64(sbOut, runLoad64C(sbOut, &(s->head)));
}
IRExpr* runStackEmptyG(IRSB* sbOut, IRExpr* guard, Stack* s){
  return runZeroCheck64(sbOut,
                        runLoadG64C(sbOut, &(s->head), guard));
}
