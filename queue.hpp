// Luke Mitchell & Max Peglar-Willis, 2020
// <hi@lukemitchell.co>

#ifndef QUEUE_HPP
#define QUEUE_HPP

class queue_t {
	private:
		size_t* buf;
		size_t head;
		size_t tail;
		size_t size;
	
	public:
		// Initialise a new queue of the 
		// specified size
		queue_t(size_t size);

		// Add a new value to the queue
		// Returns TRUE on success, FALSE
		// when the queue is full.
		int enqueue(size_t val);

		// Remove the oldest value in the queue
		// and return it.
		// returns 0 to check if queue is full
		size_t dequeue();

		// Return the oldest value in the queue.
		// This is the next value that will be
		// removed when dequeue is called.
		size_t peek();
		
};
/*
// Define a queue data structure.
// This is a First In First Out (FIFO)
// structure.
typedef struct {
	size_t* buf;
	size_t head;
	size_t tail;
	size_t size;
} queue_t;

// Initialise a new queue of the 
// specified size
extern queue_t* init_queue(size_t size);

// Add a new value to the queue
// Returns TRUE on success, FALSE
// when the queue is full.
extern int enqueue(queue_t* q, size_t val);

// Remove the oldest value in the queue
// and return it.
// returns 0 to check if queue is full
extern size_t dequeue(queue_t* q);

// Return the oldest value in the queue.
// This is the next value that will be
// removed when dequeue is called.
extern size_t peek(queue_t* q);
*/
#endif
