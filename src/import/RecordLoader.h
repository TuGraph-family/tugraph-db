#ifndef RECORDLOADER_H
#define RECORDLOADER_H
#include <stdio.h>
#include <immintrin.h>
#include <math.h>
#include <time.h>
#if defined(__MACH__)
#include <stdlib.h>
#else
#include <malloc.h>
#endif
#include <string.h>
#include <ctype.h>
#include <pthread.h>
#include <sys/time.h>
#include <sys/file.h>
#include <unistd.h>
#include <sched.h>
#include <iostream>
#include <string>
#include <vector>
#include "RecordSet.h"

#define bitScan(x) __builtin_ffs(x)

using namespace std;

class RecordLoader {
 public:
    static RecordSet* loadRecords(char* file_path, bool head_line);
    static unsigned int scanColumns(char* first_line, bool head_line, bool quote_sample);
    static bool intervalSample(char* file_Begin, unsigned long length);
    static bool quoteBool(char*& sampleLine);
};

class blSize {
 public:
    blSize(unsigned int block, unsigned int blnumber);
    static blSize* matchingBlock(unsigned long length, int avcore);
    unsigned int getBlSize();
    unsigned int getBlNum();

 private:
    unsigned int blockSize;
    unsigned int blockNumber;
};
#endif