// Luke Mitchell & Max Peglar-Willis, 2020
// <hi@lukemitchell.co>

#include "common.hpp"

// Initialise a new queue of the 
// specified size
queue_t::queue_t(size_t size) {
	this->head = 0;
	this->tail = 0;
	this->size = size;
	this->buf = (size_t*)malloc(size * sizeof(size_t));
	assert(this->buf);
}

// Add a new value to the queue
// Returns TRUE on success, FALSE
// when the queue is full.
int queue_t::enqueue(size_t val) {	
	// If the pointers meet, the queue is full.
	if (this->head == this->tail - 1)
		return false;
	
	// If the head pointer is at the end of the buffer
	// and the tail pointer at the beginning,
	// the queue is also full.
	if (this->head == this->size) {
		if (this->tail == 0) 
			return false;
	}
	
	this->buf[this->head] = val;
	
	// If the head pointer is at the end of the
	// buffer, loop around to the start.
	this->head = (this->head == this->size)  ?  0  :  this->head + 1;
	
	return true;
}

// Remove the oldest value in the queue
// and return it.
// returns 0 to check if queue is full
size_t queue_t::dequeue() {
	
	// Queue is empty, 
	// nothing to return
	if (this->head == this->tail)
		return false;
	
	size_t oldTail = this->tail;
	
	// If the tail pointer is at
	// the end of the buffer,
	// loop around to the start.
	if (this->tail == this->size)
		this->tail = 0;
	else 
		this->tail += 1;
	
	return this->buf[oldTail];
}

// Return the oldest value in the thisueue.
// This is the next value that will be
// removed when dethisueue is called.
size_t queue_t::peek() {
	return this->buf[this->tail];
}



/*// Luke Mitchell & Max Peglar-Willis, 2020
// <hi@lukemitchell.co>

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>

#include "queue.hpp"

// Initialise a new queue of the 
// specified size
queue_t* init_queue(size_t size) {
	queue_t* q = (queue_t*)malloc(sizeof(queue_t));
	assert(q);
	
	q->head = 0;
	q->tail = 0;
	q->size = size;
	
	q->buf = (size_t*)malloc(size * sizeof(size_t));
	assert(q->buf);
	
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
// returns 0 to check if queue is full
size_t dequeue(queue_t* q) {
	
	// Queue is empty, 
	// nothing to return
	if (q->head == q->tail)
		return false;
	
	size_t oldTail = q->tail;
	
	// If the tail pointer is at
	// the end of the buffer,
	// loop around to the start.
	if (q->tail == q->size)
		q->tail = 0;
	else 
		q->tail += 1;
	
	return q->buf[oldTail];
}

// Return the oldest value in the queue.
// This is the next value that will be
// removed when dequeue is called.
size_t peek(queue_t* q) {
	return q->buf[q->tail];
}
*/
