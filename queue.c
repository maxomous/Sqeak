// Luke Mitchell & Max Peglar-Willis, 2020
// <hi@lukemitchell.co>

#include <stdlib.h>
#include <stdbool.h>

#include "queue.h"

// Initialise a new queue of the 
// specified size
queue_t* init_queue(size_t size) {
	queue_t* q = malloc(sizeof(queue_t));
	if (q == NULL) {
		return NULL;
	}
	
	q->head = 0;
	q->tail = 0;
	q->size = size;
	
	q->buf = malloc(size * sizeof(size_t));
	if (q->buf == NULL) {
		return NULL;
	}
	
	return q;
}

// Add a new value to the queue
// Returns TRUE on success, FALSE
// when the queue is full.
int enqueue(queue_t* q, size_t val) {	
	// If the pointers meet, the queue is full.
	if (q->head == q->tail - 1) {
		return false;
	}
	
	// If the head pointer is at the end of the buffer
	// and the tail pointer at the beginning,
	// the queue is also full.
	if (q->head == q->size) {
		if (q->tail == 0) {
			return false;
		}
	}
	
	q->buf[q->head] = val;
	
	// If the head pointer is at the end of the
	// buffer, loop around to the start.
	if (q->head == q->size) {
		q->head = 0;
	} else {
		q->head += 1;
	}
	
	return true;
}

// Remove the oldest value in the queue
// and return it.
size_t dequeue(queue_t* q) {
	size_t oldTail = q->tail;
	
	// If the tail pointer is at
	// the end of the buffer,
	// loop around to the start.
	if (q->tail == q->size) {
		q->tail = 0;
	} else {
		q->tail += 1;
	}
	
	return q->buf[oldTail];
}

// Return the oldest value in the queue.
// This is the next value that will be
// removed when dequeue is called.
size_t peek(queue_t* q) {
	return q->buf[q->tail];
}
