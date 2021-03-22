// Luke Mitchell & Max Peglar-Willis, 2021
// <hi@lukemitchell.co>

#include "common.hpp"

// Initialise a new queue of the 
// specified size
Queue::Queue(size_t size) {
	this->head = 0;
	this->tail = 0;
	this->size = size;
	this->buf = (size_t*)malloc(size * sizeof(size_t));
	assert(this->buf);
}

// Add a new value to the queue
// Returns TRUE on success, FALSE
// when the queue is full.
int Queue::enqueue(size_t val) {	
	// If the pointers meet, the queue is full.
	if (this->head == this->tail - 1)
		throw "ERROR: Queue is full!";
	
	// If the head pointer is at the end of the buffer
	// and the tail pointer at the beginning,
	// the queue is also full.
	if (this->head == this->size) {
		if (this->tail == 0) 
		throw "ERROR: Queue is full!";
	}
	
	this->buf[this->head] = val;
	
	// If the head pointer is at the end of the
	// buffer, loop around to the start.
	this->head = (this->head == this->size)  ?  0  :  this->head + 1;
	
	return true;
}

// Remove the oldest value in the queue
// and return it.
// throws error if queue if empty
size_t Queue::dequeue() {
	// Queue is empty, 
	// nothing to return
	if (this->head == this->tail)
		throw "ERROR: You cannot remove from an empty queue!";
	
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
size_t Queue::peek() {
	return this->buf[this->tail];
}
