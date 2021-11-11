//
// cachelab.c - Cache lab template
//
//

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <unistd.h>
#include <string.h>
#include "cachelab.h"
// more libraries if needed for your program

struct node {
    struct CacheLine* cacheLine;
    struct node* next;
};

struct Queue {
    struct node* head;
};

// Method for queuing cacheLines
void queue(struct CacheLine* cacheLine, struct node* node, struct Queue* q) {
    if (q->head==NULL) {
        q->head = malloc(sizeof(struct node));
        q->head->cacheLine = malloc(sizeof(struct CacheLine));
        q->head->cacheLine = cacheLine;
        q->head->next = NULL;
        return;
    }

    if (node != NULL) {
        if (node->next != NULL) queue(cacheLine, node->next, q);
        else {
            node->next = malloc(sizeof(struct node));
            node->next->cacheLine = cacheLine;
            node->next->next = NULL;
        }
    }
}

// Method for dequeuing cacheLines
struct CacheLine* dequeue(struct Queue* q) {

    if (q->head != NULL) {
        struct node* node = q->head;
        struct CacheLine* cacheLine = node->cacheLine;
        q->head = q->head->next;
        return cacheLine;
    }
    return NULL;
}

//FIFO Algorithm for cachline replacement, first in, first replaced
int fifo(struct addressBlock* address, const struct CacheStructure* cache) {
    if (address->set > cache->sets) {
        printf("Memory Address is greater than Cache Address");
        return 0;
    }
    int hasMissed = 1;
    // Checks if cache exists
    for (int line = 0; line < cache->lines; line++) {
        struct CacheLine* cacheLine = *(*(cache->cacheLines + address->set) + line);
        if (address->tag == cacheLine->tag && cacheLine->validBit == 1) {
            // if miss and last line in the set
            hasMissed = 0;
        }
    }

    // Has hit
    if (hasMissed == 0) {
        printf("H\n");
        return 1;
    }
    // if missed
    // first check if there are any empty lines in the set
     struct CacheLine** lines_in_address_set = *(cache->cacheLines + address->set);
     for (int line = 0; line < cache->lines; line++) {
         struct CacheLine* cacheLine = *(lines_in_address_set + line);
         if (cacheLine->validBit == 0) {
             cacheLine->validBit = 1;
             cacheLine->tag = address->tag;
             struct Queue* setQueue = *(cache->setQueues + address->set);
             queue(cacheLine, setQueue->head, setQueue);
             printf("M\n");
             return -1;
         }
    }

    // if none empty, then replacement
    struct Queue* setQueue = *(cache->setQueues + address->set);
    struct CacheLine* cacheLine = dequeue(setQueue);
    cacheLine->tag = address->tag;
    queue(cacheLine, setQueue->head, setQueue);
    printf("M\n");
    return -1;
}

// print result of cache simulation showing hit number, miss number, miss rate, and total running time
void printResult(int hits, int misses, int missRate, int runTime) {
    printf("[result] hits: %d misses: %d miss rate: %d%% total running time: %d cycle\n",hits, misses, missRate, runTime);
}

// Frees cache memory
void freeCache(struct CacheStructure* cache) {
    for (int k = 0; k < cache->sets; k++) {
        for (int j = 0; j < cache->lines; j++) {
            struct CacheLine* cacheLine = *(*(cache->cacheLines + k) + j);
            free(cacheLine);
        }
    }
}

/* LRU algorithm for replacement of old cache by checking the least recently used cache block and replacing the cache
 * with a new cache block
 */
int lru(struct addressBlock* address, const struct CacheStructure* cache) {
    if (address->set > cache->sets) {
        printf("Memory Address is greater than Cache Address");
        return 0;
    }
    int hasMissed = 1;
    // Checks if cache exists
    for (int line = 0; line < cache->lines; line++) {
        struct CacheLine* cacheLine = *(*(cache->cacheLines + address->set) + line);
        if (address->tag == cacheLine->tag && cacheLine->validBit == 1) {
            // if miss and last line in the set
            hasMissed = 0;
            cacheLine->LRUCounter =-1;
        }
    }

    for (int set = 0; set < cache->sets; set++) {
        for (int line = 0; line < cache->lines; line++) {
            struct CacheLine* cacheLine = *(*(cache->cacheLines + set) + line);
            if (cacheLine->validBit == 1)
                cacheLine->LRUCounter ++;
        }
    }

    // Has hit
    if (hasMissed == 0) {
        printf("H\n");
        return 1;
    }

    // if missed
    // first check if there are any empty lines in the set
    struct CacheLine** lines_in_address_set = *(cache->cacheLines + address->set);
    for (int line = 0; line < cache->lines; line++) {
        struct CacheLine* cacheLine = *(lines_in_address_set + line);
        if (cacheLine->validBit == 0) {
            cacheLine->validBit = 1;
            cacheLine->tag = address->tag;
            cacheLine->LRUCounter = 1;
            printf("M\n");
            return -1;
        }
    }

    int lruIndex = 0;
    for (int i = 0; i < cache->lines; i++) {
        struct CacheLine* cacheLine = *(lines_in_address_set + i);
        struct CacheLine* prev_cacheLine = *(lines_in_address_set + lruIndex);
        if (cacheLine->LRUCounter >= prev_cacheLine->LRUCounter) lruIndex = i;
    }

    struct CacheLine* LRUCacheLine = *(*(cache->cacheLines + address->set) + lruIndex);
    LRUCacheLine->tag = address->tag;
    LRUCacheLine->LRUCounter = 0;
    printf("M\n");
    return -1;
}

// Parses the binary address into a struct to ease of use
struct addressBlock* getAddressBlock(const char* address, const struct CacheStructure* cache) {
    // address is a bit vector of length 1 + s + b
    struct addressBlock* addressBlock = malloc(sizeof(struct addressBlock));

    int setBits = cache->s;
    int blockBits = cache->b;
    int tagBits = ((int) strlen(address)) - blockBits - setBits ;

    // Parsing tag bits
    char* tagBuffer = malloc(sizeof(char) * tagBits);
    for (int k = 0; k < tagBits; k++) {
        *(tagBuffer + k) = *(address + k);
    }
    char* tag = malloc(sizeof(char) * tagBits);
    strncpy(tag, tagBuffer, tagBits + 1);
    *(tag + tagBits) = '\0';
    //printf("tag: %s\t", tag);
    addressBlock->tag = binaryToDecimal(tag);

    // Parsing Set Bits
    char* setBuffer = malloc(sizeof(char) * setBits);
    for (int k = tagBits; k < tagBits + setBits; k++) {
        *(setBuffer + k - tagBits) = *(address + k);
    }
    char* set = malloc(sizeof(char) * setBits);
    strncpy(set, setBuffer, setBits);
    *(set + setBits) = '\0';
    //printf("set: %s\t", set);
    addressBlock->set = binaryToDecimal(set);

    // Parsing Block Bits
    char* blockOffsetBuffer = malloc(sizeof(char) * blockBits + 1);
    for (int k = tagBits + setBits; k < tagBits + setBits + blockBits; k++) {
        *(blockOffsetBuffer + k - tagBits - setBits) = *(address + k);
    }
    char* offset = malloc(sizeof(char) * blockBits);
    strncpy(offset, blockOffsetBuffer, blockBits);
    *(offset + blockBits) = '\0';
    //printf("offset: %s\t", offset); 
    addressBlock->blockOffset = binaryToDecimal(blockOffsetBuffer);

    //printf("%d %d %d\t", addressBlock->tag, addressBlock->set, addressBlock->blockOffset);
    return addressBlock;
}

// Builds the cache structure with parameters from the stdin
struct CacheStructure* buildCache(int m, int s, int e, int b) {
    struct CacheStructure* cacheStructure = malloc(sizeof(struct CacheStructure));
    cacheStructure->s = s;
    cacheStructure->b = b;
    cacheStructure->sets = (int) pow(2, s);
    cacheStructure->lines = e + 1;
    cacheStructure->blockSize = (int) pow(2, b);
    cacheStructure->totalSize = cacheStructure->sets * cacheStructure->lines * cacheStructure->blockSize;
    cacheStructure->cacheLines = malloc(sizeof(struct CacheLine**) * cacheStructure->sets);
    for (int k = 0; k < cacheStructure->sets; k++) {
        *(cacheStructure->cacheLines + k) = malloc(sizeof(struct CacheLine*) * cacheStructure->lines);
        for (int j = 0; j < cacheStructure->lines ; j++) {
            *(*(cacheStructure->cacheLines + k) + j) = malloc(sizeof(struct CacheLine));
	    struct CacheLine* cacheLine = *(*(cacheStructure->cacheLines + k) + j);
	    cacheLine->validBit = 0; 
        }
    }
     return cacheStructure;
}

// main function should be coded here
int main(int argc, char** argv) {
    int opt;
    int m, s, e, b;
    char* i;
    char* r;
    if (argc > 1) {
        while (-1 != (opt = getopt(argc, argv, "m:s:e:b:i:r:"))) {
            switch (opt) {
                case 'm':
                    m = atoi(optarg);
                    break;
                case 's':
                    s = atoi(optarg);
                    break;
                case 'e':
                    e = atoi(optarg);
                    break;
                case 'b':
                    b = atoi(optarg);
                    break;
                case 'i':
                    if (strcmp(optarg, "address01") == 0 || strcmp(optarg, "address02") == 0 ||
                        strcmp(optarg, "address03") == 0) {
                        i = optarg;
                        break;
                    } else return -1;

                case 'r':
                    if (strcmp(optarg, "fifo") == 0 || strcmp(optarg, "optimal") == 0 || strcmp(optarg, "lru") == 0) {
                        r = optarg;
                        break;
                    } else return -1;
                default:
                    printf("Wrong arguments... exiting\n");
                    return -1;
            }
        }
    }
    // Debugging quick variables in the IDE
    else {
        m = 32; s = 2; e = 1; b = 3; i = "address02"; r = "fifo";
    }
    if (m == 0 || s == 0 || b == 0 || i == NULL) {
        printf("Wrong arguments... exiting\n");
        exit(1);
    }
    struct file* file = loadFile(i);
    struct CacheStructure* cache = buildCache(m, s, e, b);
    struct addressBlock* addressBlock;

    int misses = 0;
    int hits = 0;
    cache->setQueues = malloc(sizeof(struct Queue*) * cache->sets);
    for (int set = 0; set < cache->sets; set++) {
        *(cache->setQueues + set) = malloc(sizeof(struct Queue));
        struct Queue* queue = *(cache->setQueues + set);
        queue->head =NULL;
    }
    for (int k = 0; k < file->length; k++) {
        printf("%s\t",*(file->content + k));
        char* binary = hexToBinary( *(file->content + k) );
        //printf("%s\t", binary);
        addressBlock = getAddressBlock(binary, cache);
	    //printf("%d %d %d\t", addressBlock->tag, addressBlock->set, addressBlock->blockOffset);
        if  (strcmp(r, "lru") == 0 || strcmp(r, "LRU") == 0) {
            int returnVal = lru(addressBlock, cache);
            if (returnVal == -1) misses ++;
            else hits ++;
        }
        else if (strcmp(r, "fifo") == 0 || strcmp(r, "FIFO") == 0) {
            int returnVal = fifo(addressBlock, cache);
            if (returnVal == -1) misses++;
            else hits ++;
        }
    }
    double missRate = ( (double) misses / file->length) ;
    double averageAccessTime = (HIT_TIME + MISS_PENALTY * missRate) ;
    int runTime = file->length * averageAccessTime;
    printResult(hits, misses,  missRate * 100, runTime);
    free(file);
    freeCache(cache);
    free(addressBlock);
    if (strcmp(r, "fifo") == 0 || strcmp(r, "FIFO") == 0) {
    }
    return 0;
}
// GOOD LUCK!
