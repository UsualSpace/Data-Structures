/*
  Filename: LRU_index_cache.cpp
  Author: Abdurrahman Alyajouri
  Date Created: 8/24/2024
  Date Modified: 9/6/2024 
  Description: A slightly modified version of the LRU cache concept, in which instead of being able to insert a key and value pair, 
               you can only enter in a key, and the data structure will assign to this key an available index in an array or other container.
               As the cache fills up, it assigns sequential indices to incoming keys. When all indices have been taken up or used (dependent on the capacity variable),
               The structure begins to follow the rules of the "least recently used" (LRU) principle, and evicts the least recently used/accessed key from the cache, and recycles its index
               to pair to the new incoming key. 
               
               TLDR: This structure auto assigns to a key the integer index of the next available spot in some integer-indexed fixed-size container, based on LRU policies. 
                    
               Note: This data structure simply maps keys to indices for a theoretical container of the same capacity as defined for the structure below. The retrieved indices can be used to access elements
                     in some container of the same size.
                    
               One use case of this structure is in one of my projects involving chunk based terrain rendering. The problem was that each chunk in the terrain required a unique texture, 
               and all the textures were stored in a limited array of textures. Because the only unique and known aspect of a chunk was its position in the world, I used said chunk positions as keys and paired
               them to indices in the array of textures, so that a texture could be easily corresponded to a chunk based on it's position. Given a list of chunks that need to be rendered, for each chunk, I would check if
               its world position was already a key in the cache. If the key does exist already, nothing happens. If not, I insert said chunk position into the cache, and it is paired with 
               the next available index into the array of textures. A texture is then generated at this index.
               When actually rendering, a list of chunks to be rendered would be given to the GPU, as well as a parallel list of indices into the texture array for a given chunk in the chunk list, 
               that was obtained by using the functions of the below data structure. It allowed easy access and correspondence between terrain chunks and textures within a texture array.

               Function roles:
                   bool Exists(const K& key) const : Returns true if a key is already in the cache, false otherwise.
                   int Get(const K& key) : Returns the integer index in some fixed-size container assigned to the given key value. Returns -1 if the given key was not in the cache.
                   int GetUsed() const : Returns the number of used up indices/spots in the cache.
                   int GetCapacity() const : Returns the capacity of the cache.
                   void Put(const K& key): For the given key, inserts it into the cache and assigns it the next available index decided by said cache.
                   void PrintCacheState() const : Prints a list of all the key-index pairs in the cache at any given moment.
*/

template<typename K>
class LRUIndexCache {
    public:
        typedef std::pair<K, int> kv;
        LRUIndexCache() {}

        LRUIndexCache(const int& capacity) {
            this->capacity = capacity;
            this->used = 0;
        }

        bool Exists(const K& key) const {
            return kvmap.find(key) != kvmap.end();
        }
        
        int Get(const K& key) {
            if(Exists(key)) { // If the given key already exists in our cache...
                // Make temporary copy of key-index pair.
                kv temp = *kvmap[key]; 
                
                // Erase node containing said key-index pair.
                kvlist.erase(kvmap[key]);

                // Push back the temporary key-value copy to the back of the list, 
                // indicating it is the most recently used key, 
                // therefore should be furthest from being evicted from the cache.
                kvlist.push_back(temp);

                // For the key in question, update its pointer counterpart in the map to point to
                // the new location of the key-value pair in the list (which is the back of the list since we just used 'push_back()').
                kvmap[key] = prev(kvlist.end());

                // Since we've already copied the key-value pair at the beginning, just return the copied value rather than performing
                // another access operation.
                return temp.second;
            }
            return -1; //invalid index
        }

        int GetUsed() const {
            // May be useful to know how many spots have been taken up in the cache.
            return this->used;
        }

        int GetCapacity() const {
            // May be useful to know how many spots have been taken up in the cache.
            return this->capacity;
        }
        
        void Put(const K& key) { 
            if(Exists(key)) { // If the given key exists in our cache already...
                // Make temporary copy of key-index pair. 
                kv temp = *kvmap[key];

                // Erase node containing said key-index pair.
                kvlist.erase(kvmap[key]);

                // Push back the temporary key-value copy to the back of the list, 
                // indicating it is the most recently used key, 
                // therefore should be furthest from being evicted from the cache.
                kvlist.push_back(temp);

                // For the input key, update its pointer counterpart in the map to point to
                // the new location of the key-value pair in the list (which is the back of the list since we just used 'push_back()').
                kvmap[key] = prev(kvlist.end());
              
            } else if(this->used + 1 > this->capacity) { // If the addition of one more key breaches capacity...
                // Make a temporary copy of the value corresponding to the least recently used key (located at the front of list), 
                // so we can assign it to the new key that will be added.
                int temp_v = kvlist.begin()->second;

                // Remove the least recently used key from our map.
                kvmap.erase(kvlist.begin()->first);

                // Finally, evict the least recently used key from the front of the list.
                kvlist.pop_front();

                // Now add the new key-value pair, by combining the input key and recycling the index from the previously evicted key.
                kvlist.push_back(std::make_pair(key, temp_v));

                // Insert the new key into our map, and set its counterpart to point to the location of new key-value pair in the back of our list.
                kvmap[key] = prev(kvlist.end());
            } else { // Else...
                // Push a new pair to the back of the list, consisting of the input key, and the next available index in our cache.
                kvlist.push_back(std::make_pair(key, this->used));
                kvmap[key] = prev(kvlist.end());
                ++this->used;
            }
        }

        //Assuming a key can be printed via cout.
        void PrintCacheState() const {
            std::cout << "=========CacheState=========\n";
            for(auto const& [key, value] : kvlist) {
                std::cout << key << " : " << value << std::endl;
            }
            std::cout << "============================\n";
        }

    private:
        int capacity; // The maximum amount of indices to keep track of.
        int used; // The amount of used indices.
        std::list<kv> kvlist; // Doubly linked list for keeping track of key-value pairs
        std::unordered_map<K, class std::list<kv>::iterator> kvmap; // Unordered map that for a given key, stores the location to the node in the kvlist that contains said key.
};
