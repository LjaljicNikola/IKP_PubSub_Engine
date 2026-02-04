#ifndef HASH_MAP_H
#define HASH_MAP_H

#include <cstring>

template<typename K, typename V>
class HashMap {
private:
    // Entry structure for key-value pairs
    struct Entry {
        K key;
        V value;
        bool occupied;
        bool deleted;
        
        Entry() : occupied(false), deleted(false) {}
    };
    
    Entry* table;
    int capacity;
    int numElements;
    
    // Simple hash function for strings
    int hashString(const char* str) const {
        unsigned long hash = 5381;
        int c;
        while ((c = *str++)) {
            hash = ((hash << 5) + hash) + c;
        }
        return hash % capacity;
    }
    
    // Hash function for integers
    int hashInt(int key) const {
        return key % capacity;
    }
    
    // Generic hash function (specializations below)
    int hash(const K& key) const;
    
    // Compare keys
    bool keysEqual(const K& k1, const K& k2) const;
    
    // Resize table when load factor > 0.7
    void resize() {
        int oldCapacity = capacity;
        Entry* oldTable = table;
        
        capacity = capacity * 2;
        table = new Entry[capacity];
        numElements = 0;
        
        // Rehash all elements
        for (int i = 0; i < oldCapacity; i++) {
            if (oldTable[i].occupied && !oldTable[i].deleted) {
                insert(oldTable[i].key, oldTable[i].value);
            }
        }
        
        delete[] oldTable;
    }
    
public:
    // Constructor
    explicit HashMap(int initialCapacity = 16) 
        : capacity(initialCapacity), numElements(0) {
        table = new Entry[capacity];
    }
    
    // Destructor
    ~HashMap() {
        delete[] table;
    }
    
    // Copy constructor
    HashMap(const HashMap& other) 
        : capacity(other.capacity), numElements(other.numElements) {
        table = new Entry[capacity];
        for (int i = 0; i < capacity; i++) {
            table[i] = other.table[i];
        }
    }
    
    // Assignment operator
    HashMap& operator=(const HashMap& other) {
        if (this != &other) {
            delete[] table;
            capacity = other.capacity;
            numElements = other.numElements;
            table = new Entry[capacity];
            for (int i = 0; i < capacity; i++) {
                table[i] = other.table[i];
            }
        }
        return *this;
    }
    
    // Insert or update key-value pair
    void insert(const K& key, const V& value) {
        // Resize if load factor too high
        if ((float)numElements / capacity > 0.7f) {
            resize();
        }
        
        int index = hash(key);
        int originalIndex = index;
        int firstDeleted = -1;
        
        // Linear probing
        while (table[index].occupied) {
            if (table[index].deleted && firstDeleted == -1) {
                firstDeleted = index;
            }
            
            if (!table[index].deleted && keysEqual(table[index].key, key)) {
                // Key exists, update value
                table[index].value = value;
                return;
            }
            
            index = (index + 1) % capacity;
            
            // Table full
            if (index == originalIndex) {
                resize();
                insert(key, value);
                return;
            }
        }
        
        // Insert new entry
        if (firstDeleted != -1) {
            index = firstDeleted;
        }
        
        table[index].key = key;
        table[index].value = value;
        table[index].occupied = true;
        table[index].deleted = false;
        numElements++;
    }
    
    // Get value by key
    bool get(const K& key, V& value) const {
        int index = hash(key);
        int originalIndex = index;
        
        while (table[index].occupied) {
            if (!table[index].deleted && keysEqual(table[index].key, key)) {
                value = table[index].value;
                return true;
            }
            
            index = (index + 1) % capacity;
            if (index == originalIndex) {
                break;
            }
        }
        return false;
    }
    
    // Check if key exists
    bool contains(const K& key) const {
        V dummy;
        return get(key, dummy);
    }
    
    // Remove key
    bool remove(const K& key) {
        int index = hash(key);
        int originalIndex = index;
        
        while (table[index].occupied) {
            if (!table[index].deleted && keysEqual(table[index].key, key)) {
                table[index].deleted = true;
                numElements--;
                return true;
            }
            
            index = (index + 1) % capacity;
            if (index == originalIndex) {
                break;
            }
        }
        return false;
    }
    
    // Get size
    int size() const {
        return numElements;
    }
    
    // Check if empty
    bool isEmpty() const {
        return numElements == 0;
    }
    
    // Clear all entries
    void clear() {
        delete[] table;
        table = new Entry[capacity];
        numElements = 0;
    }
    
    // Get all keys
    void getKeys(K* keys, int& count) const {
        count = 0;
        for (int i = 0; i < capacity && count < numElements; i++) {
            if (table[i].occupied && !table[i].deleted) {
                keys[count++] = table[i].key;
            }
        }
    }
};

// Specialization for const char* keys
template<>
inline int HashMap<const char*, void*>::hash(const char* const& key) const {
    return hashString(key);
}

template<>
inline bool HashMap<const char*, void*>::keysEqual(const char* const& k1, const char* const& k2) const {
    return strcmp(k1, k2) == 0;
}

// Specialization for int keys
template<typename V>
inline int HashMap<int, V>::hash(const int& key) const {
    return hashInt(key);
}

template<typename V>
inline bool HashMap<int, V>::keysEqual(const int& k1, const int& k2) const {
    return k1 == k2;
}

#endif // HASH_MAP_H
