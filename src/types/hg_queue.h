
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

typedef struct _QueueNode QueueNode;

typedef struct _Queue {
  QueueNode* head;
  QueueNode* tail;
} Queue;

Queue* mkQueue(void);
void freeQueue(Queue* q);
// WARNING: You are responsible for freeing anything you add to the
// queue, preferably after removing it or freeing the queue.
void queue_push(Queue* q, void* sv);
void* queue_pop(Queue* q);
int queue_empty(Queue* q);
