#ifndef CIRCULAR_BUFFER_H
#define CIRCULAR_BUFFER_H

#include "../Message.h"

template<typename T>
class CircularBuffer {
private:
    T* buffer;              // Dynamic array for buffer storage
    int capacity;           // Maximum capacity of buffer
    int head;               // Write position
    int tail;               // Read position
    int count;              // Current number of elements
    
public:
    // Constructor
    explicit CircularBuffer(int size = 100) 
        : capacity(size), head(0), tail(0), count(0) {
        buffer = new T[capacity];
    }
    
    // Destructor
    ~CircularBuffer() {
        delete[] buffer;
    }
    
    // Add element to buffer (overwrites oldest if full)
    void push(const T& item) {
        buffer[head] = item;
        head = (head + 1) % capacity;
        
        if (count < capacity) {
            count++;
        } else {
            // Buffer is full, move tail forward (overwrite oldest)
            tail = (tail + 1) % capacity;
        }
    }
    
    // Remove and return oldest element
    bool pop(T& item) {
        if (count == 0) {
            return false;  // Buffer is empty
        }
        
        item = buffer[tail];
        tail = (tail + 1) % capacity;
        count--;
        return true;
    }
    
    // Peek at oldest element without removing
    bool peek(T& item) const {
        if (count == 0) {
            return false;
        }
        item = buffer[tail];
        return true;
    }
    
    // Check if buffer is empty
    bool isEmpty() const {
        return count == 0;
    }
    
    // Check if buffer is full
    bool isFull() const {
        return count == capacity;
    }
    
    // Get current size
    int size() const {
        return count;
    }
    
    // Get capacity
    int getCapacity() const {
        return capacity;
    }
    
    // Clear buffer
    void clear() {
        head = 0;
        tail = 0;
        count = 0;
    }
    
    // Get element at index (0 = oldest)
    bool getAt(int index, T& item) const {
        if (index < 0 || index >= count) {
            return false;
        }
        int actualIndex = (tail + index) % capacity;
        item = buffer[actualIndex];
        return true;
    }
};

#endif // CIRCULAR_BUFFER_H
