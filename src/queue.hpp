// Luke Mitchell & Max Peglar-Willis, 2021
// <hi@lukemitchell.co>

#ifndef QUEUE_HPP
#define QUEUE_HPP

class Queue {
	private:
		size_t* buf;
		size_t head;
		size_t tail;
		size_t size;
	
	public:
		// Initialise a new queue of the 
		// specified size
		Queue(size_t size);
		~Queue();
		// Add a new value to the queue
		// Returns TRUE on success, FALSE
		// when the queue is full.
		int enqueue(size_t val);

		// Remove the oldest value in the queue
		// and return it.
		// throws error if queue if empty
		size_t dequeue();

		// Return the oldest value in the queue.
		// This is the next value that will be
		// removed when dequeue is called.
		size_t peek();
		
		// reset the queue, size remains the same
		void clear();
};
#endif
