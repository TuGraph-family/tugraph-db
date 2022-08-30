/* Copyright (c) 2022 AntGroup. All Rights Reserved. */

#pragma once

namespace lgraph {
namespace import_v2 {

class FileCutter {
 public:
    explicit FileCutter(const std::string& filename) : filename_(filename) {
        file_.Open(filename_);
        if (!file_.Good()) {
            throw std::runtime_error("cannot open " + filename);
        }
        buf_.resize(ONLINE_IMPORT_LIMIT_HARD);
    }

    ~FileCutter() {}

    /**
     * Cut a block from file, store the block in [begin, end) and return them.
     * The block size is between ONLINE_IMPORT_LIMIT_SOFT and ONLINE_IMPORT_LIMIT_HARD
     * The block will end with '\n' unless encountered EOF
     *
     * @param begin  [out] the begin of the block cut
     * @param end    [out] the end of the block cut
     * @return       true if success, false at EOF
     * @exception    std::runtime_error if failed to read or input line too long
     */
    bool Cut(char*& begin, char*& end) {
        begin = (char*)buf_.data();
        size_t n_read = file_.Read(begin, ONLINE_IMPORT_LIMIT_SOFT);
        if (!n_read) {
            FMA_LOG() << "Finished reading file " << filename_;
            return false;
        }
        end = begin + n_read;
        if (n_read == ONLINE_IMPORT_LIMIT_SOFT) {
            while (end[-1] != '\n' && (size_t)(end - begin) < ONLINE_IMPORT_LIMIT_HARD) {
                if (!file_.Read(end, 1)) break;
                end++;
            }
            if ((size_t)(end - begin) >= ONLINE_IMPORT_LIMIT_HARD) {
                throw std::runtime_error("too long input line");
            }
        }
        return true;
    }

 protected:
    fma_common::InputFmaStream file_;
    std::string filename_;
    std::string buf_;
};

}  // end of namespace import_v2
}  // end of namespace lgraph
