typedef struct HashTableObjectTag HashTableObject;
typedef HashTableObject *HashTablePTR;
typedef struct HashTableInfoTag
{
    unsigned int bucketCount; // current number of buckets
    float loadFactor; // ( number of entries / number of buckets )
    float useFactor; // ( number of buckets with one or more entries / number of buckets )
    unsigned int largestBucketSize; // number of entries in the bucket containing the most entries
    int dynamicBehaviour; // whether or not the Hash Table will resize dynamically
    float expandUseFactor; // the value of useFactor that will trigger an expansion of the number of buckets
    float contractUseFactor; // the value of useFactor that will trigger a contraction in the number of buckets
} HashTableInfo;
 
int CreateHashTable( HashTablePTR *hashTableHandle, unsigned int initialSize ); // Creates a HashTable Object
 
int DestroyHashTable( HashTablePTR *hashTableHandle ); // Destroys a HashTable Object

int InsertEntry( HashTablePTR hashTable, char *key, void *data, void **previousDataHandle ); //Inserts a key/data pair into the HashTable. Returns 0 on insertion into blank space, 1 on hash collision, and 2 on key collision (in which case the data will be pointed to by previousDataHandle, and the data will update to the new data)

int DeleteEntry( HashTablePTR hashTable, char *key, void **dataHandle ); // Deletes a key/data pair, and has dataHandle point to the data (which will then be freed by the user).
 
int FindEntry( HashTablePTR hashTable, char *key, void **dataHandle ); // Finds key/data pair, and points to the data by dataHandle
 
int GetKeys( HashTablePTR hashTable, char ***keysArrayHandle, unsigned int *keyCount ); // Returns an array with copies of all keys

int GetHashTableInfo( HashTablePTR hashTable, HashTableInfo *pHashTableInfo ); // Fills out a pre-malloc'd hashtable info object

int SetResizeBehaviour( HashTablePTR hashTable, int dynamicBehaviour, float expandUseFactor, float contractUseFactor ); // Sets the resize behavior of the hashtable object

