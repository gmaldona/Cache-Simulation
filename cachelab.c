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
            *(cache->LRUSetCounter + address->set) = 1;
            printf("M\n");
            return -1;
        }
    }

    // No empty in set, look for a replacement
    int LRUSet = 0;
    for (int set = 0; set < cache->sets; set ++) {
        if (*(cache->LRUSetCounter + set) > LRUSet ) LRUSet = set;
    }
    // loop through all the lines in the LRUSet and find the LRULine
    struct CacheLine** LRUSetLines = *(cache->cacheLines + LRUSet);
    int LRULine = 0;
    for (int line = 0; line < cache->lines; line++) {
        struct CacheLine* cacheLine = *(*(cache->cacheLines + LRUSet) + line);
        if ( cacheLine->LRUCounter > LRULine) LRULine = line;
    }
    // update old cache with new cache
    struct CacheLine* LRUCacheLine = *(*(cache->cacheLines + LRUSet) + LRULine);
    LRUCacheLine->tag = address->tag;
    LRUCacheLine->LRUCounter = 0;
    printf("M\n") ;
    return -1;
}

// Parses the binary address into a struct to ease of use
struct addressBlock* getAddressBlock(const char* address, const struct CacheStructure* cache) {
    // address is a bit vector of length 1 + s + b

    struct addressBlock* addressBlock = malloc(sizeof(struct addressBlock));

    int setBits = cache->s;
    int blockBits = cache->b;
    int tagBits = ((int) strlen(address)) - blockBits - setBits ;

    char* tagBuffer = malloc(sizeof(char) * tagBits);
    for (int k = 0; k < tagBits; k++) {
        *(tagBuffer + k) = *(address + k);
    }
    addressBlock->tag = binaryToDecimal(tagBuffer);

    char* setBuffer = malloc(sizeof(char) * setBits);
    for (int k = tagBits; k < tagBits + setBits; k++) {
        *(setBuffer + k - tagBits) = *(address + k);
    }
    addressBlock->set = binaryToDecimal(setBuffer);

    char* blockOffsetBuffer = malloc(sizeof(char) * blockBits);
    for (int k = tagBits + setBits; k < tagBits + setBits + blockBits; k++) {
        *(blockOffsetBuffer + k - tagBits - setBits) = *(address + k);
    }
    addressBlock->blockOffset = binaryToDecimal(blockOffsetBuffer);

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
    cacheStructure->LRUSetCounter = malloc(sizeof(int) * cacheStructure->sets);
    for (int k = 0; k < cacheStructure->sets; k++) {
        *(cacheStructure->cacheLines + k) = malloc(sizeof(struct CacheLine*) * cacheStructure->lines);
        *(cacheStructure->LRUSetCounter + k) = 0;
        for (int j = 0; j < cacheStructure->lines + 1; j++) {
            *(*(cacheStructure->cacheLines + k) + j) = malloc(sizeof(struct CacheLine));
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
                    printf("Wrong arguments... \n");
                    return -1;
            }
        }
    }
    // Debugging quick variables in the IDE
    else {
        m = 64; s = 4; e = 0; b = 4; i = "testcase"; r = "lru";
    }
    struct file* file = loadFile(i);
    struct CacheStructure* cache = buildCache(m, s, e, b);
    struct addressBlock* addressBlock;

//    printf("\n\t\ttag - set - blockOffset\n");
//    printf("\nCache Sets\n");
//    for (int j = 0; j < cache->sets; j++) {
//        for (int k = 0; k < e + 1; k++) {
//            struct CacheLine* cacheLine = *(*(cache->cacheLines + j)+k);
//            printf("(%d, %d)   %p\t",j, k, cacheLine);
//        }
//        printf("\n");
//    }
    //printf("\n");
    //printf("Hex Mem\tBinary Mem\tAddr\tTarget\t\tStarting Cache\t\tTargeted Cache\n");
    int misses = 0;
    int hits = 0;
    for (int k = 0; k < file->length; k++) {
        printf("%s\t",*(file->content + k));
        char* binary = hexToBinary( *(file->content + k) );
        //printf("%s\t", binary);
        addressBlock = getAddressBlock(binary, cache);
        if (strcmp(r, "lru") == 0 || strcmp(r, "LRU") == 0) {
            int returnVal = lru(addressBlock, cache);
            if (returnVal == -1) misses ++;
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
    return 0;
}
// GOOD LUCK!
