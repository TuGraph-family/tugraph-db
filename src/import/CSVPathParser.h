#ifndef CSVPATHPASER_H
#define CSVPATHPASER_H

#include <iostream>
#include <immintrin.h>
#include <math.h>
#include <time.h>
#include <string.h>
#include "BitmapSet.h"

#define bitScan(x) __builtin_ffs(x)

using namespace std;

class bitmap {
 public:
    static BitmapSet* bmAlloc(unsigned long length, unsigned int blockNum, unsigned int blockSize,
                              bool quote, char*& fileBegin, int threads);
    static int* stateMetrix(int blockNum);
    static void mergebitmap(bool* totalBTMPF, bool* totalBTMPR, bool* fenbtmpF, bool* fenbtmpR,
                            unsigned int blockSize);
};

class noquote {
 public:
    noquote();
    noquote(bool done);
    ~noquote();
    static noquote* parse(int block, int frequence, char*& line_begin, char*& line_end,
                          bool*& col_column_begin, bool*& col_record_begin);
    bool get_noquoteState();

 private:
    bool Done;
};

class DFA {
 public:
    DFA();
    DFA(int block, int* metrix);
    ~DFA();
    static DFA* getDFA(int block, char*& line_begin, char*& line_end, bool*& field_IR,
                       bool*& field_OR, bool*& field_IQ, bool*& field_OQ, bool*& record_IR,
                       bool*& record_OR, bool*& record_IQ, bool*& record_OQ, int*& Metrix);
    int* get_Metrix();

 private:
    int Block;
    int* Metrix;
};
#endif