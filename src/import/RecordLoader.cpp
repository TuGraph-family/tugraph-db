#include <sys/time.h>
#include "./lib/RecordLoader.h"
using namespace std;

#define MAX_PAD 64

/*---To pursue speed, all CSV data is loaded into memory---*/
RecordSet* RecordLoader::loadRecords(char* file_path, bool head_line) {
    unsigned long size;
    FILE* fp = fopen(file_path, "rb+");
    if (fp == NULL) {
        return NULL;
    }
    fseek(fp, 0, SEEK_END);
    size = ftell(fp);
    rewind(fp);
    void* p;
    if (posix_memalign(&p, 64, (size + MAX_PAD) * sizeof(char)) != 0) {
        cout << "Memory space is not enough! " << endl;
    }
    char* record_text = (char*)p;
    size_t load_size = fread(record_text, 1, size, fp);
    if (load_size == 0) {
        cout << "Fail to load the input record into memory! " << endl;
    }
    int remain = 64 - (size % 64);
    int counter = 0;
    // pad the input data where its size can be divided by 64
    while (counter < remain) {
        record_text[size + counter] = 'd';
        counter++;
    }
    record_text[size + counter] = '\0';
    fclose(fp);
    bool quote = false;
    quote = intervalSample((char*)p, size);
    if (quote == 0)
        cout << "Go to Template without quote" << endl;
    else
        cout << "Go to Template with quote" << endl;
    unsigned int columns = scanColumns(record_text, head_line, quote);
    // only one single record
    RecordSet* record = new RecordSet();
    record->text = record_text;
    record->columns = columns;
    record->headline = head_line;
    record->quotesample = quote;
    record->abs_size = size;
    record->rec_size = strlen(record_text);
    record->delete_text = false;
    return record;
}

/*---Determine the number of columns by scanning the head line---*/
unsigned int RecordLoader::scanColumns(char* first_line, bool head_line, bool quote_sample) {
    unsigned int columns = 0;
    char* pos = (char*)&first_line[0];
    if (head_line == true) {
        while (1) {
            if (*pos != ',' && *pos != '\n') {
                pos++;
            } else if (*pos == ',') {
                pos++;
                columns++;
            } else if (*pos == '\n') {
                columns++;
                break;
            }
        }
    } else {
        if (quote_sample == false) {
            while (1) {
                if (*pos == ',') {
                    columns++;
                    pos++;
                } else if (*pos == '\n') {
                    columns++;
                    break;
                } else
                    pos++;
            }
        } else {
            bool sigquote = false;
            while (1) {
                if (*pos == '\n' && sigquote == false) {
                    columns++;
                    break;
                } else if (*pos == ',' && sigquote == false) {
                    pos++;
                    columns++;
                } else if (*pos == '\"') {
                    if (*(++pos) == ',' || *(++pos) == '\n' && sigquote == true) {
                        pos++;
                        sigquote = false;

                    } else if (sigquote == false) {
                        pos++;
                        sigquote = true;
                    }
                } else
                    pos++;
            }
        }
    }
    return columns;
}

/*---Exponential sampling---*/
bool RecordLoader::intervalSample(char* fileBegin, unsigned long length) {
    srand((unsigned)time(NULL));
    int Time = rand() % 1000 + 100;
    unsigned long initial_Time = Time;
    bool quote;
    for (int sa = 5; (pow(2, sa) + initial_Time) < length; sa++) {
        char* sampleLine;
        sampleLine = new char[32];
        sampleLine = fileBegin + initial_Time;
        quote = quoteBool(sampleLine);
        if (quote == true) return quote;
        initial_Time = initial_Time + pow(2, sa);
    }
    return quote;
}

/*---Check if double quotes exist in the sample line---*/
bool RecordLoader::quoteBool(char*& sampleLine) {
    const __m256i emitting = _mm256_set1_epi8('"');
    char* sample32;
    sample32 = new char[32];
    sample32 = sampleLine;
    __m256i op = _mm256_loadu_si256((__m256i*)sample32);
    __m256i eqC = _mm256_cmpeq_epi8(op, emitting);
    int mask = _mm256_movemask_epi8(eqC);
    int gg = -1;
    while (mask) {
        gg = bitScan(mask) - 1;
        if (gg != -1) return true;
    }
    return false;
}

// -----blSize----- //
blSize::blSize(unsigned int block, unsigned int blnumber) {
    blockSize = block;
    blockNumber = blnumber;
}

/*---Determine the size of the blocksize based on the size of the file---*/
blSize* blSize::matchingBlock(unsigned long length, int avcore) {
    int fileSize = length / 1024 / 1024 / 1024 + 1;
    int blockSize = 0;
    int blnumber = 0;
    if (fileSize < 2)
        blockSize = 128 * 1024;
    else if (fileSize < 5)
        blockSize = 256 * 1024;
    else if (fileSize < 10)
        blockSize = 512 * 1024;
    else
        blockSize = 1024 * 1024;
    if (length / blockSize < avcore) blockSize = length / avcore;
    blnumber = length / blockSize + 1;
    blSize* object = new blSize(blockSize, blnumber);
    return object;
}

unsigned int blSize::getBlSize() { return blockSize; }

unsigned int blSize::getBlNum() { return blockNumber; }