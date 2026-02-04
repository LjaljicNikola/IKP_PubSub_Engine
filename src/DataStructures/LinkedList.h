#ifndef LINKED_LIST_H
#define LINKED_LIST_H

template<typename T>
class LinkedList {
private:
    // Node structure
    struct Node {
        T data;
        Node* next;
        
        Node(const T& value) : data(value), next(nullptr) {}
    };
    
    Node* head;
    Node* tail;
    int listSize;
    
public:
    // Constructor
    LinkedList() : head(nullptr), tail(nullptr), listSize(0) {}
    
    // Destructor
    ~LinkedList() {
        clear();
    }
    
    // Copy constructor
    LinkedList(const LinkedList& other) : head(nullptr), tail(nullptr), listSize(0) {
        Node* current = other.head;
        while (current != nullptr) {
            pushBack(current->data);
            current = current->next;
        }
    }
    
    // Assignment operator
    LinkedList& operator=(const LinkedList& other) {
        if (this != &other) {
            clear();
            Node* current = other.head;
            while (current != nullptr) {
                pushBack(current->data);
                current = current->next;
            }
        }
        return *this;
    }
    
    // Add element to end
    void pushBack(const T& value) {
        Node* newNode = new Node(value);
        
        if (tail == nullptr) {
            head = tail = newNode;
        } else {
            tail->next = newNode;
            tail = newNode;
        }
        listSize++;
    }
    
    // Add element to front
    void pushFront(const T& value) {
        Node* newNode = new Node(value);
        newNode->next = head;
        head = newNode;
        
        if (tail == nullptr) {
            tail = newNode;
        }
        listSize++;
    }
    
    // Remove element by value (first occurrence)
    bool remove(const T& value) {
        Node* current = head;
        Node* previous = nullptr;
        
        while (current != nullptr) {
            if (current->data == value) {
                if (previous == nullptr) {
                    // Removing head
                    head = current->next;
                    if (head == nullptr) {
                        tail = nullptr;
                    }
                } else {
                    previous->next = current->next;
                    if (current == tail) {
                        tail = previous;
                    }
                }
                delete current;
                listSize--;
                return true;
            }
            previous = current;
            current = current->next;
        }
        return false;
    }
    
    // Check if value exists
    bool contains(const T& value) const {
        Node* current = head;
        while (current != nullptr) {
            if (current->data == value) {
                return true;
            }
            current = current->next;
        }
        return false;
    }
    
    // Get size
    int size() const {
        return listSize;
    }
    
    // Check if empty
    bool isEmpty() const {
        return listSize == 0;
    }
    
    // Clear all elements
    void clear() {
        Node* current = head;
        while (current != nullptr) {
            Node* next = current->next;
            delete current;
            current = next;
        }
        head = tail = nullptr;
        listSize = 0;
    }
    
    // Iterator class for range-based for loops
    class Iterator {
    private:
        Node* current;
    public:
        Iterator(Node* node) : current(node) {}
        
        T& operator*() { return current->data; }
        T* operator->() { return &(current->data); }
        
        Iterator& operator++() {
            if (current != nullptr) {
                current = current->next;
            }
            return *this;
        }
        
        bool operator!=(const Iterator& other) const {
            return current != other.current;
        }
    };
    
    Iterator begin() { return Iterator(head); }
    Iterator end() { return Iterator(nullptr); }
    
    // Const iterator
    class ConstIterator {
    private:
        const Node* current;
    public:
        ConstIterator(const Node* node) : current(node) {}
        
        const T& operator*() const { return current->data; }
        const T* operator->() const { return &(current->data); }
        
        ConstIterator& operator++() {
            if (current != nullptr) {
                current = current->next;
            }
            return *this;
        }
        
        bool operator!=(const ConstIterator& other) const {
            return current != other.current;
        }
    };
    
    ConstIterator begin() const { return ConstIterator(head); }
    ConstIterator end() const { return ConstIterator(nullptr); }
};

#endif // LINKED_LIST_H
