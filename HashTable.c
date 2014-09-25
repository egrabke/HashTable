#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "HashTable.h"

#define MAXHASH 4294967295 // max for 32bit unsigned int. Can be changed to whatever, code will work
#define SENTINEL 0xDEADBEEF // Sentinel value

typedef struct Node Node;
struct Node
{
    char *key;
    void *value;
    Node *left;
    Node *right;
};

struct HashTableObjectTag
{
    int sentinel;
    struct Node **hashes;
    char *(*keys)[];
    unsigned int keycount; // keycount/size = loadFactor
    unsigned int bucketuse; // bucketuse/size = useFactor
    unsigned int size; // Max hash value + 1
    int dynamic; // Boolean
    float expand;
    float contract;
};

static int nohash = 1;
static int tempindex = 0;

//--Finished Declarations--//

int CreateHashTable( HashTablePTR *hashTableHandle, unsigned int initialSize )
{ // Done
    if (*hashTableHandle != NULL)
        DestroyHashTable(hashTableHandle); // In case
    if (initialSize > MAXHASH || initialSize <= 0)
        return -1; // Goofed
    HashTablePTR temp = malloc(sizeof(HashTableObject));
    if (temp == NULL)
        return -1; // Memory couldn't allocate
    
    (*hashTableHandle) = temp;
    (*hashTableHandle)->keys = malloc(0);
    (*hashTableHandle)->sentinel = (int) SENTINEL;
    (*hashTableHandle)->hashes = calloc(initialSize,sizeof(Node));
    (*hashTableHandle)->size = initialSize;
    (*hashTableHandle)->keycount = 0;
    (*hashTableHandle)->bucketuse = 0;
    SetResizeBehaviour( *hashTableHandle, 1, (float) 0.7, (float) 0.2);
    
    return 0;
}

void HashHelper (Node **hashes, Node* node)
{ // Stores pointer to all nodes in a list
    
    if (node != NULL)
    {
        HashHelper(hashes, node->left);
        HashHelper(hashes, node->right);
        hashes[tempindex] = node; // tempindex is static int
        tempindex++;
        //printf("tempindex: %d\n", tempindex); // Debugging
    }
}

void ReHash (HashTablePTR hashTable)
{
    if(hashTable->dynamic == 0 || hashTable->bucketuse == 0 || (hashTable->expand > ((float)hashTable->bucketuse/(float)hashTable->size) && hashTable->contract < ((float)hashTable->bucketuse/(float)hashTable->size)) || hashTable->contract == 0 || hashTable->expand == 0)
        return; // Doesn't need to be changed

    unsigned int size;
    Node **hashes;
        
    nohash = 0;
    
    if (hashTable->contract > ((float)hashTable->bucketuse/(float)hashTable->size))
        size = (unsigned int)((float)hashTable->bucketuse / hashTable->contract - 1); // Time to contract
    else
        size = (unsigned int)((float)hashTable->bucketuse / hashTable->expand + 1); // Otherwise time to expand
    
    if (size < 1)
        size = 1;
    
    //printf("Size: %d\n", size); // Debugging
    //CreateHashTable(&temp, size);
    
    hashes = calloc(hashTable->keycount,sizeof(Node));
    
    for (int hash = 0; hash < hashTable->size; hash++)
    {
        HashHelper(hashes, hashTable->hashes[hash]);
        hashTable->hashes[hash] = NULL;
    }
    
    hashTable->size = size;
    free(hashTable->hashes);
    free(hashTable->keys);
    hashTable->hashes = NULL;
    hashTable->hashes = calloc(size,sizeof(Node));
    hashTable->keys = malloc(0);
    hashTable->bucketuse = 0;
    hashTable->keycount = 0;
    
    void **dummy = NULL;
    
    for (int index = 0; index < tempindex; index++)
    {
        InsertEntry(hashTable, hashes[index]->key, hashes[index]->value, dummy);
        free(hashes[index]->key);
        free(hashes[index]);
    }
    
    free(hashes);
    hashes = NULL;
    
    nohash = 1;
    tempindex = 0;
    return;
}

int SetResizeBehaviour( HashTablePTR hashTable, int dynamicBehaviour, float expandUseFactor, float contractUseFactor )
{ // Done
    if (hashTable == NULL || *((int *) hashTable) != SENTINEL)
        return -1; // In case someone goofed
    
    if (contractUseFactor >= expandUseFactor)
        return 1; // Again, someone goofed
    
    hashTable->dynamic = dynamicBehaviour;
    hashTable->expand = expandUseFactor;
    hashTable->contract = contractUseFactor;
    
    return 0;
}

unsigned int CountNodes(Node* node)
{ // Done
    if (node != NULL)
    {
        return (1 + CountNodes(node->left) + CountNodes(node->right));
    }
    return 0;
}

int GetHashTableInfo( HashTablePTR hashTable, HashTableInfo *pHashTableInfo )
{ // Done
    if (hashTable == NULL || *((int *) hashTable) != SENTINEL)
        return -1; // In case someone goofed
        
    unsigned int count, maxcount;
    count = 0;
    maxcount = 0;
        
    pHashTableInfo->bucketCount = hashTable->size;
    pHashTableInfo->loadFactor = ((float)hashTable->keycount)/((float)hashTable->size);
    pHashTableInfo->useFactor = ((float)hashTable->bucketuse)/((float)hashTable->size);
    pHashTableInfo->dynamicBehaviour = hashTable->dynamic;
    pHashTableInfo->expandUseFactor = hashTable->expand;
    pHashTableInfo->contractUseFactor = hashTable->contract;
    
    for (int hash = 0; hash < hashTable->size; hash++)
    {
        count = 0;
        count = CountNodes(hashTable->hashes[hash]);
        if (count > maxcount)
            maxcount = count;
    }
    pHashTableInfo->largestBucketSize = maxcount;
    
    return 0;
}

void DestroyNodes( Node* node)
{ // Working
    if (node != NULL)
    {
        DestroyNodes(node->left);
        DestroyNodes(node->right);
        free(node->key);
        free(node);
    }
}

int DestroyHashTable( HashTablePTR *hashTableHandle )
{ // Working
    if (hashTableHandle == NULL || *hashTableHandle == NULL || *((int *) *hashTableHandle) != SENTINEL)
        return -1; // Someone goofed
    
    for (int hash = 0; hash < (*hashTableHandle)->size; hash++)
    {
        //printf("Begin for loop\n"); // Debugging
        if ((*hashTableHandle)->hashes[hash] == NULL)
            continue;
        
        DestroyNodes((*hashTableHandle)->hashes[hash]);
        (*hashTableHandle)->hashes[hash] = NULL;
    }
    
    free((*hashTableHandle)->hashes);
    free((*hashTableHandle)->keys);
    free((*hashTableHandle));
    *hashTableHandle = NULL;
    hashTableHandle = NULL;
    
    return 0; // Done properly
}

unsigned int Hash(char *key, unsigned int range)
{ // Working
    unsigned int prehash = 0;
    int count;
    unsigned int ch;
    for (count = 0; count < strlen(key); count++)
    {
        ch = (unsigned int) key[count];
        prehash <<= 8;
        prehash ^= (unsigned int) ch;
    }
    
    unsigned int hash = ((unsigned int) prehash)%((unsigned int) range); // Unsigned so no negative, and range will be less than MAXHASH
    
    return hash;
}

int InsertEntry( HashTablePTR hashTable, char *key, void *data, void **previousDataHandle )
{ // Working
    if (hashTable == NULL || *((int *) hashTable) != SENTINEL)
        return -1; // In case someone goofed
    //printf("Start insert\n"); // Debugging
    unsigned int hash = Hash(key, hashTable->size);
    //printf("hash: %d\n", hash); // Debugging
    Node *first = NULL;
    Node *second = NULL;
    Node *temp = NULL;
    
    for (first = hashTable->hashes[hash]; ; first = second)
    {
        if(first == NULL) // If no entry at that point
        {
            temp = malloc(sizeof(Node));
            if (temp == NULL)
                return -2; // couldn't allocate memory
            
            //printf("Memory Allocated1\n"); // Debugging
            second = temp;
            second->key = calloc((strlen(key) + 1),sizeof(char));
            strcpy(second->key, key);
            second->value = data;
            //*previousDataHandle = second->value;
            second->left = NULL;
            second->right = NULL;
            hashTable->hashes[hash] = second;
            //printf("Key: %s\n", second->key); // Debugging
            
            hashTable->keycount++;
            hashTable->keys = realloc(*hashTable->keys, hashTable->keycount*sizeof(char *));
            (*hashTable->keys)[hashTable->keycount - 1] = second->key;
            
            hashTable->bucketuse++;
            //printf("Keylist: %s\n", (*hashTable->keys)[hashTable->keycount - 1]);
            if (nohash)
                ReHash(hashTable);
            
            return 0; // Successful
        }
        if(strcmp(first->key, key) == 0)
        {
            *previousDataHandle = first->value;
            first->value = data;
            
            return 2; // Collision, same key
        }
        if(strcmp(key, first->key) > 0) // i.e. key bigger than current
        {
            second = first->right;
            //printf("Teehee\n"); // Debugging
            if (second == NULL)
            {
                //printf("Haha\n"); // Debugging
                temp = malloc(sizeof(Node));
                if (temp == NULL)
                    return -2; // couldn't allocate memory
                second = temp;
                second->key = calloc((strlen(key) + 1),sizeof(char));
                strcpy(second->key, key);
                second->value = data;
                //*previousDataHandle = second->value;
                second->right = NULL;
                second->left = NULL;
                first->right = second;
                hashTable->keycount++;
                hashTable->keys = realloc(*hashTable->keys, hashTable->keycount*sizeof(char *));
                (*hashTable->keys)[hashTable->keycount - 1] = second->key;
                if (nohash)
                    ReHash(hashTable);
                
                return 1; // Right collision
            }
        }
        else
        {
            second = first->left;
            //printf("Rwar?\n");
            if (second == NULL)
            {
                //printf("Myesh\n");
                temp = malloc(sizeof(Node));
                if (temp == NULL)
                    return -2; // couldn't allocate memory
                second = temp;
                second->key = calloc((strlen(key) + 1),sizeof(char));
                strcpy(second->key, key);
                second->value = data;
                //*previousDataHandle = second->value;
                second->right = NULL;
                second->left = NULL;
                first->left = second;
                hashTable->keycount++;
                hashTable->keys = realloc(*hashTable->keys, hashTable->keycount*sizeof(char *));
                (*hashTable->keys)[hashTable->keycount - 1] = second->key;
                if (nohash)
                    ReHash(hashTable);
                
                return 1; // Left collision
            }
        }
    }
    
    return -2; // Something didn't go right
}
static int iterator = 0;
void *DeleteNode(Node** root, HashTablePTR hashTable, char *key)
{
    if (*root != NULL)
    {
        iterator += 1;
        if (strcmp(key, (*root)->key) < 0)
            return DeleteNode(&(*root)->left, hashTable, key);
        else if (strcmp(key, (*root)->key) > 0)
            return DeleteNode(&(*root)->right, hashTable, key);
        else // Found it
        {
            struct Node* temp = *root;
            void *data;
            if (temp->left == NULL)
            {
                *root = (*root)->right;
                data = temp->value;
                int index;
                for (index = 0; index < hashTable->keycount; index++)
                {
                    if (strcmp((*hashTable->keys)[index], key) == 0)
                        break; // It found the key
                }
                for (int new = index; new + 1 < hashTable->keycount; new++)
                {
                    (*hashTable->keys)[new] = (*hashTable->keys)[new + 1];
                }
                if (iterator == 1) hashTable->bucketuse--;
                hashTable->keycount--;
                (*hashTable->keys)[hashTable->keycount] = NULL;
                (*hashTable).keys = realloc((*hashTable).keys, hashTable->keycount*sizeof(char *)); // Deep Magic lies here
                free(temp->key);
                free(temp);
                return data;
            }
            else if (temp->right == NULL)
            {
                *root = (*root)->left;
                data = temp->value;
                int index;
                for (index = 0; index < hashTable->keycount; index++)
                {
                    if (strcmp((*hashTable->keys)[index], key) == 0)
                        break; // It found the key
                }
                for (int new = index; new + 1 < hashTable->keycount; new++)
                {
                    (*hashTable->keys)[new] = (*hashTable->keys)[new + 1];
                }
                if (iterator == 1) hashTable->bucketuse--;
                hashTable->keycount--;
                (*hashTable->keys)[hashTable->keycount] = NULL;
                (*hashTable).keys = realloc((*hashTable).keys, hashTable->keycount*sizeof(char *)); // Deep Magic lies here
                free(temp->key);
                free(temp);
                return data;
            }
            struct Node* tempfollow = temp;
            struct Node* holder = *root;
            temp = temp->left;
            if (temp->right == NULL)
            {
                (*root) = (*root)->left;
                (*root)->right = holder->right;
                data = holder->value;
                
                int index;
                for (index = 0; index < hashTable->keycount; index++)
                {
                    if (strcmp((*hashTable->keys)[index], key) == 0)
                        break; // It found the key
                }
                for (int new = index; new + 1 < hashTable->keycount; new++)
                {
                    (*hashTable->keys)[new] = (*hashTable->keys)[new + 1];
                }
                if (iterator == 1) hashTable->bucketuse--;
                hashTable->keycount--;
                (*hashTable->keys)[hashTable->keycount] = NULL;
                (*hashTable).keys = realloc((*hashTable).keys, hashTable->keycount*sizeof(char *)); // Deep Magic lies here
                free(holder->key);
                free(holder);
                return data;
            }
            else
            {
                while (temp->right != NULL)
                {
                    tempfollow = temp;
                    temp = temp->right;
                }
                tempfollow->right = temp->left;
                temp->left = (*root)->left;
                temp->right = (*root)->right;
                (*root) = temp;
            }
            data = holder->value;
            int index;
            for (index = 0; index < hashTable->keycount; index++)
            {
                if (strcmp((*hashTable->keys)[index], key) == 0)
                    break; // It found the key
            }
            for (int new = index; new + 1 < hashTable->keycount; new++)
            {
                (*hashTable->keys)[new] = (*hashTable->keys)[new + 1];
            }
            if (iterator == 1) hashTable->bucketuse--;
            hashTable->keycount--;
            (*hashTable->keys)[hashTable->keycount] = NULL;
            (*hashTable).keys = realloc((*hashTable).keys, hashTable->keycount*sizeof(char *)); // Deep Magic lies here
            //printf("Keything: %s", holder->key);
            //printf("Keythingleft: %p", (void *)(*root)->left);
            //printf("Keythingright: %p", (void *)(*root)->right);
            free(holder->key);
            free(holder);
            return data;
        }
    }
    
    return NULL;
}

int DeleteEntry( HashTablePTR hashTable, char *key, void **dataHandle )
{ // Working
    if (hashTable == NULL || *((int *) hashTable) != SENTINEL)
        return -1; // Isn't a thing
    
    unsigned int hash = Hash(key, hashTable->size);
    
    if ((*dataHandle = DeleteNode(&hashTable->hashes[hash], hashTable, key)) != NULL)
    {
        if (nohash)
            ReHash(hashTable);
        return 0;
    }
    
    return -2; // Couldn't find
}

int FindEntry( HashTablePTR hashTable, char *key, void **dataHandle )
{ // Working
    if (hashTable == NULL || *((int *) hashTable) != SENTINEL)
        return -1; // Doesn't point properly
    
    Node *seeker = NULL;
    
    unsigned int hash = Hash(key, hashTable->size);
    
    for (seeker = hashTable->hashes[hash]; ; )
    {
        if (seeker == NULL)
            break;
        if(strcmp(seeker->key, key) == 0)
        {
            *dataHandle = seeker->value;
            return 0; // Found it
        }
        if(strcmp(key, seeker->key) > 0)
            seeker = seeker->right;
        else
            seeker = seeker->left;
    }
    
    return -2; //Couldn't find anything
}

int GetKeys( HashTablePTR hashTable, char ***keysArrayHandle, unsigned int *keyCount )
{ // Working
    if (hashTable == NULL || *((int *) hashTable) != SENTINEL)
    {
        *keysArrayHandle = calloc(0,0);
        *keyCount = 0; // Inputted to allow for weird tester
        return -1; // Isn't a thing
    }

    *keyCount = hashTable->keycount;
    
    char **temp = calloc((*keyCount),sizeof(char *));
    if (temp == NULL)
        return -2; // Memory not allocated
    
    *keysArrayHandle = temp;
    
    char *temp1;
    
    for (int index = 0; index < *keyCount; index++)
    {
        temp1 = malloc((strlen((*hashTable->keys)[index]) + 1)*sizeof(char));
        if (temp1 == NULL)
            return -2; // Memory not allocated
        (*keysArrayHandle)[index] = temp1;
        strcpy((*keysArrayHandle)[index], (*hashTable->keys)[index]);
    }

    return 0;
}
