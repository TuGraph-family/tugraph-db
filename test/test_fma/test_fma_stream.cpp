/**
 * Copyright 2022 AntGroup CO., Ltd.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 */

#include <string>
#include <vector>

#include "fma-common/binary_read_write_helper.h"
#include "fma-common/buffered_file_stream.h"
#include "fma-common/configuration.h"
#include "fma-common/file_system.h"
#include "fma-common/fma_stream.h"
#include "fma-common/logging.h"
#include "fma-common/string_formatter.h"
#include "./unit_test_utils.h"
#include "fma-common/utils.h"

using namespace fma_common;

FMA_SET_TEST_PARAMS(FmaStream, "-f tmpfile -s 67", "-f tmpfile -s 65536 -n 13 -b 2 -u 1023 -r 3",
                    "-f tmpfile -s 65536 -u 0");

FMA_UNIT_TEST(FmaStream) {
    lgraph_log::LoggerManager::GetInstance().EnableBufferMode();

    typedef size_t T;
    ArgParser parser;
    parser.Add<std::string>("file,f").Comment("File to write to").SetDefault("./tmpfile");
    parser.Add<size_t>("size,s").Comment("Size of each int vector").SetDefault(65536);
    parser.Add<size_t>("nIter,n").Comment("Number of vectors to write").SetDefault(1024);
    parser.Add<size_t>("nBuffer,b").Comment("Number of write buffers to use").SetDefault(1);
    parser.Add<size_t>("bufSize,u").Comment("Buffer size to use").SetDefault(1023);
    parser.Add<size_t>("nRead,r").Comment("Number of times to read the data").SetDefault(1);
    parser.AddBinary("compress,c").Comment("Use SNAPPY compression");
    parser.Parse(argc, argv);
    parser.Finalize();

    std::string path = parser.GetValue<std::string>("file");
    size_t vector_size = parser.GetValue<size_t>("size");
    size_t n_vectors = parser.GetValue<size_t>("nIter");
    // size_t n_buffers = parser.GetValue<size_t>("nBuffer");
    size_t buf_size = parser.GetValue<size_t>("bufSize");
    size_t n_read = parser.GetValue<size_t>("nRead");
    bool compress = parser.GetValue<bool>("compress");

    {
        typedef OutputBufferedFileStream<UnbufferedOutputLocalFileStream,
                                         ThreadPoolOutputStreamBuffer>
            TPBS;
        const char *dir = "test_buffer_stream";
        static const size_t nf = 10;
        static const size_t bsize = 1024;
        file_system::MkDir(dir);
        ThreadPool pool(3);
        std::vector<TPBS> outs(nf);
        for (size_t i = 0; i < outs.size(); i++) {
            auto &f = outs[i];
            f.Open(StringFormatter::Format("{}/{}", dir, i), bsize, std::iostream::trunc,
                   std::make_tuple(), std::make_tuple(&pool));
            FMA_UT_ASSERT(f.Good());
        }
        size_t size = 0;
        for (size_t i = 0; i < bsize * 3; i++) {
            std::string buf(i, i % 128);
            size += buf.size();
            for (auto &f : outs) {
                f.Write(buf.data(), buf.size());
            }
        }
        outs.clear();
        // validate content
        for (size_t i = 0; i < nf; i++) {
            InputFmaStream in(StringFormatter::Format("{}/{}", dir, i));
            FMA_UT_ASSERT(in.Good());
            FMA_UT_CHECK_EQ(in.Size(), size);
            std::string buf(bsize * 3, 0);
            for (size_t i = 0; i < bsize * 3; i++) {
                size_t r = in.Read(&buf[0], i);
                FMA_UT_CHECK_EQ(r, i);
                for (size_t j = 0; j < r; j++) FMA_UT_CHECK_EQ(buf[j], i % 128);
            }
        }
    }

    uint64_t sum = 0;
    std::vector<T> v(vector_size);
    for (size_t i = 0; i < v.size(); i++) {
        v[i] = i;
        sum += v[i];
    }

    double t1 = GetTime();
    OutputFmaStream out(path, buf_size, std::ofstream::trunc, compress, 1);
    for (size_t i = 0; i < n_vectors; i++) {
        BinaryWrite(out, v);
    }
    out.Close();
    double t2 = GetTime();
    LOG() << "Write completed at "
          << (double)vector_size * sizeof(T) * n_vectors / 1024 / 1024 / (t2 - t1) << "MB/s";

    for (size_t r = 0; r < n_read; r++) {
        double t = 0;
        InputFmaStream in(path, buf_size, compress);
        FMA_UT_ASSERT(in.Seek(v.size() * sizeof(T) + sizeof(size_t)));
        for (size_t i = 0; i < n_vectors - 1; i++) {
            std::fill(v.begin(), v.end(), 0);
            t1 = GetTime();
            BinaryRead(in, v);
            t2 = GetTime();
            t += (t2 - t1);
            uint64_t s = 0;
            for (auto d : v) s += d;
            FMA_UT_CHECK_EQ(s, sum) << "inconsistent content";
        }
        LOG() << "Read completed at "
              << (double)vector_size * sizeof(T) * n_vectors / 1024 / 1024 / t << "MB/s";
    }

    lgraph_log::LoggerManager::GetInstance().DisableBufferMode();
    return 0;
}
