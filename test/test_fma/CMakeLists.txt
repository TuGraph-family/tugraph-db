# unit test cmake
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -DFMA_IN_UNIT_TEST -DNO_STACKTRACE")

include_directories(${LGRAPH_INCLUDE_DIR})

add_executable(fma_unit_test
        main.cpp
        test_arg_parse.cpp
        test_binary_read_write_helper.cpp
        test_bounded_queue.cpp
        test_cache_aligned_vector.cpp
        test_configuration.cpp
        test_data_file.cpp
        test_date.cpp
        test_encrypt.cpp
        test_file_stream.cpp
        test_file_system.cpp
        test_fma_stream.cpp
        test_hard_container.cpp
        test_hardware_info.cpp
        test_leveled_log_device.cpp
        test_logging.cpp
        test_many_streams.cpp
        test_multi_disk_stream.cpp
        test_piped_hdfs_stream.cpp
        test_pipeline.cpp
        test_rotating_files.cpp
        test_rw_lock.cpp
        test_snappy_stream.cpp
        test_stream_buffer.cpp
        test_string_util.cpp
        test_text_dir_stream.cpp
        test_text_parser.cpp
        test_text_parser_utils.cpp
        test_text_writer.cpp
        test_thread_pool.cpp
        test_timed_task.cpp
        test_type_traits.cpp
        test_utils.cpp
        test_level_log.cpp)

target_link_libraries (fma_unit_test
        libstdc++fs.a
        ${Boost_LIBRARIES})
