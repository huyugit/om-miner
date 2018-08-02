#ifndef RING_QUEUE_H
#define RING_QUEUE_H
/*
 * Contains RingQueue class declaration.
 */

#include <stdlib.h>
#include <assert.h>


// This is a FIFO queue data structure that uses a fixed size array of T
// as if it were connected end-to-end (similar to Circular buffer,
// see http://en.wikipedia.org/wiki/Circular_buffer).
// 
// First element enqueued is refered as "head", and the last as "tail".
// Note that if the number of elements in the queue reaches the capacity
// and a new element is added, the oldest ("head") element is lost.
// Elements are dequeued in the order they appeared (from "head" to "tail").
// 
// Class T used as a template argument should implement default
// constructor, copy constructor and assignment operator.
//
template<class T, size_t CAPACITY>
class RingQueue
{
// Construction/destruction.
public:
    // Default constructor.
    RingQueue()
        : m_head(0)
        , m_tail(0)
        , m_size(0)
    {}

    // Copy constructor.
    RingQueue(const RingQueue& src)
        : m_head(0)
        , m_tail(0)
        , m_size(0)
    {
        copy(src);
    }

    // Operators.
public:
    // Assignment operator.
    RingQueue& operator=(const RingQueue& right)
    {
        if (&right != this)
            copy(right);
        
        return *this;
    }

    // Array subscript operators.
    // Returns a reference to an element with the specified
    // offset from head (serial). These will allow to iterate
    // over the queue elements from 0 to getSize().
    inline const T& operator[](size_t serial) const throw()
    {
        assert(serial < m_size);
        return m_elements[serialToIndex(serial)];
    }
    inline T& operator[](size_t serial) throw()
    {
        assert(serial < m_size);
        return m_elements[serialToIndex(serial)];
    }

// Public interface.
public:
    // Returns the size of the underlying array.
    inline size_t getCapacity() const throw()  { return CAPACITY; }

    // Returns a number of elements currently stored in the queue.
    inline size_t getSize() const throw()  { return m_size; }

    // Returns true if the queue is empty (no element currently stored).
    inline bool isEmpty() const throw()  { return (m_size == 0); }
    
    // Returns a reference to the head (first added) element.
    inline const T& head() const throw()
    {
        assert(m_size > 0);
        return m_elements[m_head];
    }
    inline T& head() throw()
    {
        assert(m_size > 0);
        return m_elements[m_head];
    }
    
    // Returns a reference to the tail (last added) element.
    inline const T& tail() const throw()
    {
        assert(m_size > 0);
        return m_elements[getTailIndex()];
    }
    inline T& tail() throw()
    {
        assert(m_size > 0);
        return m_elements[getTailIndex()];
    }

    // Adds a new element to the end of the queue (as "tail").
    // If the buffer is full, the oldest element ("head") is discarded.
    inline void enqueue(const T& element) throw()
    {
        enqueue() = element;
    }
    
    T& enqueue() throw()
    {
        assert(CAPACITY > 0);
        
        if (++m_tail >= CAPACITY)
            m_tail = 0;
        
        if (m_size < CAPACITY)
            ++m_size;
        else if (++m_head >= CAPACITY)
            m_head = 0;
        
        return tail();
    }

    // Removes "head" element from the queue.
    // Returns false if the queue is empty (nothing to dequeue).
    bool dequeue() throw()
    {
        if (m_size == 0)
            return false;
        
        if (++m_head >= CAPACITY)
            m_head = 0;

        --m_size;
        return true;
    }

    // Stores the "head" element into the "element" argument
    // and removes it from the queue. Returns false if the queue
    // is empty (nothing to dequeue).
    bool dequeue(T& element) throw()
    {
        if (m_size == 0)
            return false;
    
        element = head();
        return dequeue();
    }

    // Removes "tail" element from the queue.
    // Returns false if the queue is empty (nothing to pop).
    bool pop() throw()
    {
        if (m_size == 0)
            return false;

        m_tail = (m_tail > 0 ? m_tail : CAPACITY) - 1;
        --m_size;
        return true;
    }

    // Clears the queue.
    inline void clear() throw()
    {
        m_head = 0;
        m_tail = 0;
        m_size = 0;
    }

// Implementation methods.
private:
    // Returns the index of the last added element.
    inline size_t getTailIndex() const throw()
    {
        return (m_tail > 0 ? m_tail : CAPACITY) - 1;
    }

    // Based on the given offset in rage from 0 to getSize()
    // returns the element index in the internal storage array.
    //
    // serial - element offset from the head of the queue.
    inline size_t serialToIndex(size_t serial) const throw()
    {
        return (serial < CAPACITY - m_head)
            ? m_head + serial
            : serial - (CAPACITY - m_head);
    }

    // Copies data from another RingQueue object.
    void copy(const RingQueue& src)
    {
        m_head = 0;
        m_tail = 0;
        m_size = 0;
        
        for (; m_size < src.m_size; ++m_tail, ++m_size)
            m_elements[m_tail] = src.m_elements[m_tail];
        
        if (m_tail >= CAPACITY)
            m_tail = 0;
    }

// Member variables.
private:
    // Pointer to an internal storage array.
    T m_elements[CAPACITY];
    
    // Head/tail element indices.
    size_t m_head;  // Index of the oldest element added into the queue.
    size_t m_tail;  // Index of the next element after the last added one.
    
    // A number of elements currently stored in this queue.
    size_t m_size;
};

#endif  // RING_QUEUE_H
