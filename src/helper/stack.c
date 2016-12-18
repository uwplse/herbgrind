/*--------------------------------------------------------------------*/
/*--- HerbGrind: a valgrind tool for Herbie                stack.c ---*/
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

#include "stack.h"

#include "pub_tool_mallocfree.h"
#include "pub_tool_libcassert.h"
#include "pub_tool_machine.h"
#include "instrument-util.h"

#include <stddef.h>

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
void addStackPushG(IRSB* sbOut, IRTemp guard, Stack* s, IRTemp node){
  IRTemp sHead = runLoad64C(sbOut, &(s->head));
  addStoreArrowG(sbOut, guard, node, StackNode,
                 next, IRExpr_RdTmp(sHead));
  addStoreGC(sbOut, guard, IRExpr_RdTmp(node), &(s->head));
}
void addStackPush(IRSB* sbOut, Stack* s, IRTemp node){
  IRTemp sHead = runLoad64C(sbOut, &(s->head));
  addStoreArrow(sbOut, node, StackNode, next, IRExpr_RdTmp(sHead));
  addStoreC(sbOut, IRExpr_RdTmp(node), &(s->head));
}
IRTemp runStackPop(IRSB* sbOut, Stack* s){
  IRTemp oldHead = runLoad64C(sbOut, &(s->head));
  IRTemp oldHeadNext = runArrow(sbOut,
                                oldHead, StackNode, next);
  addStoreC(sbOut, IRExpr_RdTmp(oldHeadNext), &(s->head));
  return oldHead;
}
IRTemp runStackPopG(IRSB* sbOut, IRTemp guard_temp, Stack* s){
  IRTemp oldHead = runLoadG64C(sbOut, &(s->head), guard_temp);
  IRTemp oldHeadNext = runArrowG(sbOut, guard_temp,
                                 oldHead, StackNode, next);
  addStoreGC(sbOut, guard_temp, IRExpr_RdTmp(oldHeadNext), &(s->head));
  return oldHead;
}
IRTemp runStackEmpty(IRSB* sbOut, Stack* s){
  return runZeroCheck64(sbOut, runLoad64C(sbOut, &(s->head)));
}
IRTemp runStackEmptyG(IRSB* sbOut, IRTemp guard_temp, Stack* s){
  return runZeroCheck64(sbOut,
                        runLoadG64C(sbOut, &(s->head), guard_temp));
}
