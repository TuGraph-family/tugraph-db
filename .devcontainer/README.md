# TuGraph-DB Dev Container

This devcontainer provides a complete build and test environment for TuGraph-DB.

## Usage

1. **Open in Dev Container**: In VS Code or Cursor, open this folder and run "Dev Containers: Reopen in Container" (or use the prompt when opening the folder).

2. **Build**: The project is automatically configured and built when the container is first created. To rebuild manually:
   ```bash
   cd build && cmake .. && make -j$(nproc)
   ```

3. **Run tests**:
   ```bash
   ./build/test_case
   ```

4. **Run the server**:
   ```bash
   ./build/lgraph_server --directory /tmp/lgraph_data
   ```

5. **CLI**:
   ```bash
   ./build/lgraph_cli --help
   ```

## Included Dependencies

All build dependencies are pre-installed: gflags, jemalloc, lz4, zstd, rocksdb, spdlog, antlr4, protobuf, boost, vsag, OpenBLAS, faiss, nlohmann-json, tabulate, date, and Rust (for ftindex). GTest is fetched via CMake FetchContent.
