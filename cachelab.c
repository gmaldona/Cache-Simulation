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

struct file {
    int length;
    char** content;
};

struct CacheLine {
    int validBit;
    int tag;
    struct CacheBlock* cacheBlock;
    int LRUCounter;
};

struct CacheBlock {
    int* memoryBytes;
};

struct CacheStructure {
    int s;
    int b;
    int sets;
    int lines;
    int blockSize;
    int totalSize;

    struct CacheLine*** cacheLines;
};

struct addressBlock {
    int tag;
    int set;
    int blockOffset;
};

enum algorithm {
    LRU, OPTIMAL, FIFO
};

int binaryToDecimal(const char* binary) {
    int decimal = 0;
    int exp = 0;
    char* str = malloc(sizeof(char) * 100);
    strcpy(str, binary);
    int length = (int) strlen(str);
    for (int k = length - 1; k >=0  ; k--) {
        int bit = *(binary + k) == '0'? 0 : 1;
        int pow = 1;
        for (int j = 0; j < exp; j++) {
            pow = pow * 2 ;
        }
        decimal = decimal + bit * pow;
        exp ++;
    }
    free(str);
    return decimal;
}

char* hexToBinary(const char* hex) {
    char* binary = malloc( sizeof(char) * strlen(hex) * 4 );
    for (int k = 0; k < strlen(hex); k++) {
        char* temp;
        switch (*(hex + k)) {
            case '0': temp = "0000"; break;
            case '1': temp = "0001"; break;
            case '2': temp = "0010"; break;
            case '3': temp = "0011"; break;
            case '4': temp = "0100"; break;
            case '5': temp = "0101"; break;
            case '6': temp = "0110"; break;
            case '7': temp = "0111"; break;
            case '8': temp = "1000"; break;
            case '9': temp = "1001"; break;
            case 'A': temp = "1010"; break;
            case 'B': temp = "1011"; break;
            case 'C': temp = "1100"; break;
            case 'D': temp = "1101"; break;
            case 'E': temp = "1110"; break;
            case 'F': temp = "1111"; break;

        }
        strcat(binary, temp);
    }
    return binary;
}

// print result of cache simulation showing hit number, miss number, miss rate, and total running time
void printResult(int hits, int misses, int missRate, int runTime) {
    printf("[result] hits: %d misses: %d miss rate: %d%% total running time: %d cycle\n",hits, misses, missRate, runTime);
}

struct file* loadFile(const char* fileName) {
    FILE *fp;
    struct file* file = malloc(sizeof(struct file));
    int index = 0;
    file->length = 0;

    fp = fopen(fileName, "r");
    if (fp == NULL) return NULL;
    char string[10];
    while (fscanf(fp, "%s", string) > 0) file->length ++;
    file->content = malloc(sizeof(char*) * file->length);
    for (int i = 0; i < file->length; i++) *(file->content + i) = malloc(sizeof(char) * 100);
    fclose(fp);
    fp = fopen(fileName, "r");
    char buffer[10];
    while (fscanf(fp, "%s", buffer) > 0) {
        if (strcmp(buffer, "\0") != 0) {
            strcpy( *(file->content + index), buffer );
            index++;
        }
    }
    fclose(fp);
    return file;
}

void displayBlock(struct CacheStructure* cache, struct CacheBlock* block) {
    printf("[");
    for (int k = 0; k < cache->blockSize; k++) {
        printf("%d",*(block->memoryBytes + k));
    }
    printf("]\t");
}

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
    free(tagBuffer);

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
    printf("%d %d %d\t" ,addressBlock->tag, addressBlock->set, addressBlock->blockOffset);

    return addressBlock;
}

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
        for (int j = 0; j < cacheStructure->lines + 1; j++) {
            *(*(cacheStructure->cacheLines + k) + j) = malloc(sizeof(struct CacheLine));
            struct CacheLine* cacheLine = *(*(cacheStructure->cacheLines + k) + j);
            cacheLine->cacheBlock = malloc(sizeof(struct CacheBlock));
            cacheLine->cacheBlock->memoryBytes = malloc(sizeof(int) * cacheStructure->blockSize);
        }
    }

     return cacheStructure;
}

int checkCache(const struct addressBlock* address, struct CacheStructure* cache) {
    struct CacheLine** cacheRow = *(cache->cacheLines + address->set);

    //TODO: Check if empty first

    int tagMatch = 0;
    struct CacheLine* cacheLine;
    int emptyLines = 0;
    struct CacheLine** emptyCacheLines = malloc(sizeof(struct CacheLine*) * cache->lines);
    *emptyCacheLines = NULL;
    for (int k = 0; k < cache->lines; k++) {
        cacheLine = *(*(cache->cacheLines + address->set) + k);
        if (cacheLine->validBit == 0) {
            *(emptyCacheLines + emptyLines) = cacheLine;
            *(emptyCacheLines + emptyLines + 1) = NULL;
            if (k < cache->lines - 1) continue;
        }
        // found a tag match
        if (address->tag == cacheLine->tag) {
            tagMatch = 1;
            if (cacheLine->validBit == 1) {
                if ((cacheLine->cacheBlock->memoryBytes + address->blockOffset) != NULL) {
                    printf("HIT!\t\t");
                    displayBlock(cache, cacheLine->cacheBlock);
                    return 1;
                }
            }
        }
        // If no tag matches in the set
        if (k == cache->lines - 1 && tagMatch == 0) {
            if (*emptyCacheLines != NULL) {
                struct CacheLine *emptyCacheLine = *emptyCacheLines;
                emptyCacheLine->validBit = 1;
                emptyCacheLine->tag = address->tag;
                printf("miss\t\t");
                displayBlock(cache, emptyCacheLine->cacheBlock);
                if (address->blockOffset % 2 == 0) {
                    *(emptyCacheLine->cacheBlock->memoryBytes + address->blockOffset) = 1;
                    *(emptyCacheLine->cacheBlock->memoryBytes + address->blockOffset + 1) = 1;
                } else {
                    *(emptyCacheLine->cacheBlock->memoryBytes + address->blockOffset) = 1;
                    *(emptyCacheLine->cacheBlock->memoryBytes + address->blockOffset - 1) = 1;
                }
                displayBlock(cache, emptyCacheLine->cacheBlock);
                return -1;
            } else {
                printf("Replacement\t");
                displayBlock(cache, cacheLine->cacheBlock);
                return -1;
            }
        };
    }


    return 0;
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
        m = 64; s = 4; e = 1; b = 4; i = "address02"; r = "lru";
    }
    struct file* file = loadFile(i);
    struct CacheStructure* cache = buildCache(m, s, e, b);
    struct addressBlock* addressBlock;
    printf("Hex Mem\tBinary Mem\tAddr\tTarget\t\tStarting Cache\t\tTargeted Cache\n");
    for (int k = 0; k < file->length; k++) {
        printf("%s\t",*(file->content + k));
        char* binary = hexToBinary( *(file->content + k) );
        printf("%s\t", binary);
        addressBlock = getAddressBlock(binary, cache);

        checkCache(addressBlock, cache);
        printf("\n");
    }
    printf("\n\t\ttag - set - blockOffset\n");
    printf("\nCache Sets\n");
    for (int j = 0; j < s; j++) {
        for (int k = 0; k < e + 1; k++) {
            struct CacheLine* cacheLine = *(*(cache->cacheLines + j)+k);
            printf("(%d, %d)   %p\t",j, k, cacheLine);
        }
        printf("\n");
    }

    //struct CacheStructure* cacheStructure = buildCache(m, s, e, b);

    free(file);
    free(cache);
    free(addressBlock);
    return 0;
}
// GOOD LUCK!
