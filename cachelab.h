//
//  cachelab.h - header file for cachelab
//  : defining global variabls and functions
//

#ifndef cachelab_h
#define cachelab_h

#define HIT_TIME 1          // hit time fixed for calculating running time
#define MISS_PENALTY 100    // miss penalty fixed for calculating running time

//
// printResult
// : providing a standard way for your cache simulator to display its following result
//  - hits: number of hits
//  - misses: number of misses
//  - missRate: miss rate in percentage (=hits/(hits+misses))
//  - rumTime: total running time in cycle
//

struct file {
    int length;
    char** content;
};

struct CacheLine {
    int validBit;
    int tag;
    int LRUCounter;
};

struct CacheStructure {
    int s;
    int b;
    int sets;
    int lines;
    int blockSize;
    int totalSize;

    struct CacheLine*** cacheLines;
    struct Queue** setQueues;
};

struct addressBlock {
    int tag;
    int set;
    int blockOffset;
};

int binaryToDecimal(const char* binary) {
    if (binary == NULL) return 0;
    int decimal = 0;
    int exp = 0;
    char str[1000];
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
    return decimal;
}

char* hexToBinary(char* hex) {

    /*char* retHex = malloc(sizeof(char) * 3);
    if (strlen(hex) < 3) {
        char* tempHex = malloc(sizeof(char) * 3);
        int index = 0;

        for (int i = 3 - (int) strlen(hex); i > 0; i-- ) {
            *(tempHex + index) = '0';
            index ++;
        }
        strcat(tempHex, hex);
        strcpy(retHex, tempHex);
    } else {
        strcpy(retHex, hex);
    }*/
    char retHex[4] = {};
    if (strlen(hex) < 3) {
        char tempHex[4] = {};
        int index = 0;
        for (int i = 3 - (int) strlen(hex); i > 0; i--) {
            tempHex[index] = '0';
            index ++;
        }
        strcat(tempHex, hex);
        strcpy(retHex, tempHex);
    }else {
        strcpy(retHex, hex);
    }

    char binary[15] = {};
    for (int k = 0; k < strlen(retHex); k++) {
        char temp[5];
        switch (*(retHex + k)) {
            case '0': strcpy(temp, "0000"); break;
            case '1': strcpy(temp, "0001"); break;
            case '2': strcpy(temp, "0010"); break;
            case '3': strcpy(temp, "0011"); break;
            case '4': strcpy(temp, "0100"); break;
            case '5': strcpy(temp, "0101"); break;
            case '6': strcpy(temp, "0110"); break;
            case '7': strcpy(temp, "0111"); break;
            case '8': strcpy(temp, "1000"); break;
            case '9': strcpy(temp, "1001"); break;
            case 'A': strcpy(temp, "1010"); break;
            case 'B': strcpy(temp, "1011"); break;
            case 'C': strcpy(temp, "1100"); break;
            case 'D': strcpy(temp, "1101"); break;
            case 'E': strcpy(temp, "1110"); break;
            case 'F': strcpy(temp, "1111"); break;

        }
        strcat(binary, temp);
    }
    char* retBinary = malloc(sizeof(char) * 12);
    strcpy(retBinary, binary);
    return retBinary;
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

void printResult(int hits, int misses, int missRate, int runTime);


#endif /* cachelab_h */
