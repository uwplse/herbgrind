
/*--------------------------------------------------------------------*/
/*--- HerbGrind: a valgrind tool for Herbie        hg_shadowvals.c ---*/
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

#include "../include/hg_macros.h"
#include "pub_tool_mallocfree.h"
#include "pub_tool_libcassert.h"
#include "hg_queue.h"

struct _QueueNode {
  void* item;
  QueueNode* next;
};

Queue* mkQueue(void){
  Queue* newQueue;
  ALLOC(newQueue, "queue", 1, sizeof(Queue));
  newQueue->head = NULL;
  newQueue->tail = NULL;
  return newQueue;
}
void freeQueue(Queue* q){
  while (q->head != NULL){
    QueueNode* oldHead = q->head;
    q->head = oldHead->next;
    VG_(free)(oldHead);
  }
  VG_(free)(q);
}
void queue_push(Queue* q, void* sv){
  QueueNode* newNode;
  ALLOC(newNode, "queue node", 1, sizeof(QueueNode));
  newNode->item = sv;
  newNode->next = NULL;
  if (q->head == NULL){
    q->head = newNode;
    q->tail = newNode;
  } else {
    q->tail->next = newNode;
    q->tail = newNode;
  }
}
void* queue_pop(Queue* q){
  tl_assert(!queue_empty(q));
  QueueNode* oldHead = q->head;
  q->head = oldHead->next;
  void* item = oldHead->item;
  VG_(free)(oldHead);
  return item;
}
int queue_empty(Queue* q){
  return (q->head == NULL);
}
