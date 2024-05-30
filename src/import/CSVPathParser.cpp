#include "./lib/CSVPathParser.h"

const int DFATable[4][3] = {
    {0, 3, 1},
    {0, 1, 1},
    {0, 3, 4},
    {3, 2, 3},
};

int* bitmap::stateMetrix(int blockNum) {
    int* Metrix = new int[blockNum * 4];
    return Metrix;
}

int* sortMetrix(int* metrix, int blockNum) {
    int* stMetrix;
    stMetrix = new int[blockNum];
    int j = 0;
    stMetrix[0] = 0;
    for (int i = 0; i < blockNum - 1; i++) {
        stMetrix[i + 1] = metrix[i * 4 + j];
        j = metrix[i * 4 + j];
    }
    free(metrix);
    return stMetrix;
}

// ---bitmap--- //
BitmapSet* bitmap::bmAlloc(unsigned long length, unsigned int blockNum, unsigned int blockSize,
                           bool quote, char*& fileBegin, int threads) {
    bool* bitmapField;
    bool* bitmapRecord;
    bool* Field_IR;
    bool* Field_OR;
    bool* Field_IQ;
    bool* Field_OQ;
    bool* Record_IR;
    bool* Record_OR;
    bool* Record_IQ;
    bool* Record_OQ;
    int* Metrix;
    if (quote == false) {
        bitmapField = new bool[length];
        bitmapRecord = new bool[length];
    } else {
        Field_IR = new bool[length];
        Field_OR = new bool[length];
        Field_IQ = new bool[length];
        Field_OQ = new bool[length];
        Record_IR = new bool[length];
        Record_OR = new bool[length];
        Record_IQ = new bool[length];
        Record_OQ = new bool[length];
        Metrix = bitmap::stateMetrix(blockNum);
    }

#pragma omp parallel for schedule(dynamic) num_threads(threads)
    for (int i = 0; i < blockNum - 1; i++) {
        char* linebegin;
        char* lineend;
        linebegin = (char*)(fileBegin + blockSize * i);
        lineend = (char*)(fileBegin + blockSize * (i + 1) - 1);

        if (i == blockNum - 1) {
            if (quote == false) {
                bool* mbitmapField;
                bool* mbitmapRecord;
                mbitmapField = (bool*)(bitmapField + (unsigned long)blockSize * i);
                mbitmapRecord = (bool*)(bitmapRecord + (unsigned long)blockSize * i);
                char* linebegin = (char*)(fileBegin + (unsigned long)blockSize * i);
                char* lineend = (char*)(fileBegin + length - 1);
                noquote::parse(i, (length - (unsigned long)i * blockSize) / 32, linebegin, lineend,
                               mbitmapField, mbitmapRecord);
            } else {
                bool* mField_IR;
                bool* mField_OR;
                bool* mField_IQ;
                bool* mField_OQ;
                bool* mRecord_IR;
                bool* mRecord_OR;
                bool* mRecord_IQ;
                bool* mRecord_OQ;
                mField_IR = (bool*)(Field_IR + (unsigned long)blockSize * i);
                mField_OR = (bool*)(Field_OR + (unsigned long)blockSize * i);
                mField_IQ = (bool*)(Field_IQ + (unsigned long)blockSize * i);
                mField_OQ = (bool*)(Field_OQ + (unsigned long)blockSize * i);
                mRecord_IR = (bool*)(Record_IR + (unsigned long)blockSize * i);
                mRecord_OR = (bool*)(Record_OR + (unsigned long)blockSize * i);
                mRecord_IQ = (bool*)(Record_IQ + (unsigned long)blockSize * i);
                mRecord_OQ = (bool*)(Record_OQ + (unsigned long)blockSize * i);
                char* linebegin = (char*)(fileBegin + (unsigned long)blockSize * i);
                char* lineend = (char*)(fileBegin + length - 1);
                DFA::getDFA(i, linebegin, lineend, mField_IR, mField_OR, mField_IQ, mField_OQ,
                            mRecord_IR, mRecord_OR, mRecord_IQ, mRecord_OQ, Metrix);
            }
        } else {
            if (quote == false) {
                bool* mbitmapField;
                bool* mbitmapRecord;
                mbitmapField = (bool*)(bitmapField + blockSize * i);
                mbitmapRecord = (bool*)(bitmapRecord + blockSize * i);
                noquote::parse(i, blockSize / 32, linebegin, lineend, mbitmapField, mbitmapRecord);
            } else {
                bool* mField_IR;
                bool* mField_OR;
                bool* mField_IQ;
                bool* mField_OQ;
                bool* mRecord_IR;
                bool* mRecord_OR;
                bool* mRecord_IQ;
                bool* mRecord_OQ;
                mField_IR = (bool*)(Field_IR + blockSize * i);
                mField_OR = (bool*)(Field_OR + blockSize * i);
                mField_IQ = (bool*)(Field_IQ + blockSize * i);
                mField_OQ = (bool*)(Field_OQ + blockSize * i);
                mRecord_IR = (bool*)(Record_IR + blockSize * i);
                mRecord_OR = (bool*)(Record_OR + blockSize * i);
                mRecord_IQ = (bool*)(Record_IQ + blockSize * i);
                mRecord_OQ = (bool*)(Record_OQ + blockSize * i);
                DFA::getDFA(i, linebegin, lineend, mField_IR, mField_OR, mField_IQ, mField_OQ,
                            mRecord_IR, mRecord_OR, mRecord_IQ, mRecord_OQ, Metrix);
            }
        }
    }

    int* standMetrix;
    if (quote == true) {
        standMetrix = sortMetrix(Metrix, blockNum);
        bitmapField = new bool[length];
        bitmapRecord = new bool[length];
#pragma omp parallel for schedule(dynamic) num_threads(threads)
        for (int counter = 0; counter < blockNum; counter++) {
            bool* totalBTMPF;
            bool* totalBTMPR;
            bool* fenbtmpF;
            bool* fenbtmpR;
            if (counter == blockNum - 1) {
                if (standMetrix[counter] == 0) {
                    totalBTMPF = (bool*)(bitmapField + blockSize * counter);
                    totalBTMPR = (bool*)(bitmapRecord + blockSize * counter);
                    fenbtmpF = (bool*)(Field_OR + blockSize * counter);
                    fenbtmpR = (bool*)(Record_OR + blockSize * counter);
                    bitmap::mergebitmap(totalBTMPF, totalBTMPR, fenbtmpF, fenbtmpR,
                                        (unsigned long)(length - blockSize * counter));
                } else if (standMetrix[counter] == 1) {
                    totalBTMPF = (bool*)(bitmapField + blockSize * counter);
                    totalBTMPR = (bool*)(bitmapRecord + blockSize * counter);
                    fenbtmpF = (bool*)(Field_IR + blockSize * counter);
                    fenbtmpR = (bool*)(Record_IR + blockSize * counter);
                    bitmap::mergebitmap(totalBTMPF, totalBTMPR, fenbtmpF, fenbtmpR,
                                        (unsigned long)(length - blockSize * counter));
                } else if (standMetrix[counter] == 2) {
                    totalBTMPF = (bool*)(bitmapField + blockSize * counter);
                    totalBTMPR = (bool*)(bitmapRecord + blockSize * counter);
                    fenbtmpF = (bool*)(Field_OQ + blockSize * counter);
                    fenbtmpR = (bool*)(Record_OQ + blockSize * counter);
                    bitmap::mergebitmap(totalBTMPF, totalBTMPR, fenbtmpF, fenbtmpR,
                                        (unsigned long)(length - blockSize * counter));
                } else if (standMetrix[counter] == 3) {
                    totalBTMPF = (bool*)(bitmapField + blockSize * counter);
                    totalBTMPR = (bool*)(bitmapRecord + blockSize * counter);
                    fenbtmpF = (bool*)(Field_IQ + blockSize * counter);
                    fenbtmpR = (bool*)(Record_IQ + blockSize * counter);
                    bitmap::mergebitmap(totalBTMPF, totalBTMPR, fenbtmpF, fenbtmpR,
                                        (unsigned long)(length - blockSize * counter));
                }
            } else {
                if (standMetrix[counter] == 0) {
                    totalBTMPF = (bool*)(bitmapField + blockSize * counter);
                    totalBTMPR = (bool*)(bitmapRecord + blockSize * counter);
                    fenbtmpF = (bool*)(Field_OR + blockSize * counter);
                    fenbtmpR = (bool*)(Record_OR + blockSize * counter);
                    bitmap::mergebitmap(totalBTMPF, totalBTMPR, fenbtmpF, fenbtmpR, blockSize);
                } else if (standMetrix[counter] == 1) {
                    totalBTMPF = (bool*)(bitmapField + blockSize * counter);
                    totalBTMPR = (bool*)(bitmapRecord + blockSize * counter);
                    fenbtmpF = (bool*)(Field_IR + blockSize * counter);
                    fenbtmpR = (bool*)(Record_IR + blockSize * counter);
                    bitmap::mergebitmap(totalBTMPF, totalBTMPR, fenbtmpF, fenbtmpR, blockSize);
                } else if (standMetrix[counter] == 2) {
                    totalBTMPF = (bool*)(bitmapField + blockSize * counter);
                    totalBTMPR = (bool*)(bitmapRecord + blockSize * counter);
                    fenbtmpF = (bool*)(Field_OQ + blockSize * counter);
                    fenbtmpR = (bool*)(Record_OQ + blockSize * counter);
                    bitmap::mergebitmap(totalBTMPF, totalBTMPR, fenbtmpF, fenbtmpR, blockSize);
                } else if (standMetrix[counter] == 3) {
                    totalBTMPF = (bool*)(bitmapField + blockSize * counter);
                    totalBTMPR = (bool*)(bitmapRecord + blockSize * counter);
                    fenbtmpF = (bool*)(Field_IQ + blockSize * counter);
                    fenbtmpR = (bool*)(Record_IQ + blockSize * counter);
                    bitmap::mergebitmap(totalBTMPF, totalBTMPR, fenbtmpF, fenbtmpR, blockSize);
                }
            }
        }
    }
    BitmapSet* BTMP = new BitmapSet();

    BTMP->bitmapComma = bitmapField;
    BTMP->bitmapLineBreak = bitmapRecord;
    BTMP->length = length;
    BTMP->delete_bitmap = false;
    return BTMP;
}

void bitmap::mergebitmap(bool* totalBTMPF, bool* totalBTMPR, bool* fenbtmpF, bool* fenbtmpR,
                         unsigned int blockSize) {
    memcpy(totalBTMPF, fenbtmpF, blockSize * sizeof(bool));
    memcpy(totalBTMPR, fenbtmpR, blockSize * sizeof(bool));
}

// -----noquote----- //
noquote::noquote(bool done) { Done = done; }

void checkCR(char*& lineBegin, bool*& columnBegin, bool*& recordBegin) {
    const __m256i column = _mm256_set1_epi8(',');
    const __m256i record = _mm256_set1_epi8('\n');
    __m256i op = _mm256_loadu_si256((__m256i*)lineBegin);
    __m256i eqC = _mm256_cmpeq_epi8(op, column);
    __m256i eqR = _mm256_cmpeq_epi8(op, record);
    int maskC = _mm256_movemask_epi8(eqC);
    int maskR = _mm256_movemask_epi8(eqR);
    while (maskC) {
        int CP = bitScan(maskC) - 1;
        columnBegin[CP] = true;
        maskC &= ~(0x1 << CP);
    }
    while (maskR) {
        int RP = bitScan(maskR) - 1;
        recordBegin[RP] = true;
        maskR &= ~(0x1 << RP);
    }
}

noquote* noquote::parse(int block, int frequence, char*& line_begin, char*& line_end,
                        bool*& col_column_begin, bool*& col_record_begin) {
    char* mlinebegin;
    bool* mcolumnbegin;
    bool* mrecordbegin;
    for (int i = 0; i < frequence; i++) {
        mlinebegin = (char*)(line_begin + 32 * i);
        mcolumnbegin = (bool*)(col_column_begin + 32 * i);
        mrecordbegin = (bool*)(col_record_begin + 32 * i);
        checkCR(mlinebegin, mcolumnbegin, mrecordbegin);
    }
    return NULL;
}

bool noquote::get_noquoteState() { return Done; }

/*---class DFA---*/
DFA::DFA(int block, int* metrix) {
    Block = block;
    Metrix = metrix;
}

DFA* DFA::getDFA(int block, char*& line_begin, char*& line_end, bool*& field_IR, bool*& field_OR,
                 bool*& field_IQ, bool*& field_OQ, bool*& record_IR, bool*& record_OR,
                 bool*& record_IQ, bool*& record_OQ, int*& Metrix) {
    int start[4];
    int result[4];
    int symbol;
    unsigned int sig_sym = 0;

    bool sig[4];
    for (int i = 0; i < 4; i++) {
        start[i] = i;
        result[i] = i;
        sig[i] = 0;
    }
    // cout << "test " << endl;
    while (line_begin <= line_end) {
        if (*line_begin == ',') {
            symbol = 0;
            sig_sym = 44;
        } else if (*line_begin == '\n') {
            symbol = 0;
            sig_sym = 10;
        } else if (*line_begin == '\"') {
            symbol = 1;
            sig_sym = 0;
        } else {
            symbol = 2;
            sig_sym = 0;
        }
        for (int j = 0; j < 4; j++) {
            if (result[j] != 4) {
                result[j] = DFATable[result[j]][symbol];
                if (sig[j] == 0 && result[j] == 3) sig[j] = 1;
                if (sig[j] == 1 && result[j] == 2) sig[j] = 0;
                if (sig[j] == 0) {
                    if (sig_sym == 44) {
                        if (j == 0)
                            *field_OR = true;
                        else if (j == 1)
                            *field_IR = true;
                        else if (j == 2)
                            *field_OQ = true;
                        else
                            *field_IQ = true;
                    } else if (sig_sym == 10) {
                        if (j == 0)
                            *record_OR = true;
                        else if (j == 1)
                            *record_IR = true;
                        else if (j == 2)
                            *record_OQ = true;
                        else
                            *record_IQ = true;
                    }
                }
            }
        }
        ++field_OR;
        ++field_IR;
        ++field_OQ;
        ++field_IQ;
        ++record_OR;
        ++record_IR;
        ++record_OQ;
        ++record_IQ;
        ++line_begin;
    }
    for (int k = 0; k < 4; k++) {
        Metrix[block * 4 + k] = result[k];
    }
    DFA* object = new DFA(block, Metrix);
    return object;
}