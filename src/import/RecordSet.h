#ifndef RECORDSET_H
#define RECORDSET_H

#include <stdlib.h>
#include <vector>

using namespace std;

struct RecordSet {
    char* text;
    unsigned int columns;
    unsigned long abs_size;
    unsigned long rec_size;
    bool headline;
    bool quotesample;
    bool delete_text;

    RecordSet() {
        text = NULL;
        columns = 0;
        abs_size = 0;
        rec_size = 0;
        headline = false;
        quotesample = false;
        delete_text = true;
    }

    ~RecordSet() {
        if (delete_text == true && text != NULL) {
            free(text);
            text = NULL;
            columns = 0;
            abs_size = 0;
            rec_size = 0;
            headline = false;
            quotesample = false;
            delete_text = false;
        }
    }
};
#endif