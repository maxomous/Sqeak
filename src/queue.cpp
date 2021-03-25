// Luke Mitchell & Max Peglar-Willis, 2021
// <hi@lukemitchell.co>

#include "common.hpp"

// Initialise a new queue of the 
// specified size
Queue::Queue(size_t size) {
	head = 0;
	tail = 0;
	this->size = size;
	fflush(stdin);
	buf = new size_t[size];
	assert(buf);
}

Queue::~Queue() {
	delete[] buf;
}


// Add a new value to the queue
// Returns TRUE on success, FALSE
// when the queue is full.
int Queue::enqueue(size_t val) {	
	
	fflush(stdin);
	
	// If the pointers meet, the queue is full.
	if (head == tail - 1)
		throw "ERROR: Queue is full!";
	
	// If the head pointer is at the end of the buffer
	// and the tail pointer at the beginning,
	// the queue is also full.
	if (head == size) {
		if (tail == 0) 
		throw "ERROR: Queue is full!";
	}
	
	buf[head] = val;
	
	// If the head pointer is at the end of the
	// buffer, loop around to the start.
	head = (head == size)  ?  0  :  head + 1;
	
	return true;
}

// Remove the oldest value in the queue
// and return it.
// throws error if queue if empty
size_t Queue::dequeue() {
	// Queue is empty, 
	// nothing to return
	if (head == tail)
		throw "ERROR: You cannot remove from an empty queue!";
	
	size_t oldTail = tail;
	
	// If the tail pointer is at
	// the end of the buffer,
	// loop around to the start.
	if (tail == size)
		tail = 0;
	else 
		tail += 1;
	
	return buf[oldTail];
}

// Return the oldest value in the thisueue.
// This is the next value that will be
// removed when dethisueue is called.
size_t Queue::peek() {
	return buf[tail];
}
