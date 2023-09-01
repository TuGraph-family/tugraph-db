## Resource preparation for building docker image

The docker files here uses many Cpp library resources which are uploaded to oss service.

Most of them are the same as the original resource. However, some exceptional ones are carefully
modified to accelerate the building process with these problems:

- Download packages directly from some slow website in CMake files
- Revise the cmake files to make it work

The pacakges revised from the original ones include,

- GraphAr package
- Gemoetry package

### GraphAr Package

GraphAr has Apache arrow embedded, which downloads lots of 3rd-party packages during the building
process. To accelerate the building process, take these steps:

- Download GraphAr package from github and Apache Arrow package referring to the link inside GraphAr, Eg:
    - Gar package link: https://github.com/alibaba/GraphAr/archive/refs/tags/v0.7.0.tar.gz
    - Arrow package
      link: https://www.apache.org/dyn/closer.lua?action=download&filename=arrow/arrow-${ARROW_VERSION_TO_BUILD}/apache-arrow-${ARROW_VERSION_TO_BUILD}.tar.gz
- Apache Arrow package
    - Upload Apache Arrow package to oss
        - Eg: https://tugraph-web.oss-cn-beijing.aliyuncs.com/tugraph/deps/graphar/apache-arrow-10.0.1.tar.gz
- GraphAr package
    - Replace the GAR_ARROW_SOURCE_FILE variable with the oss url.
    - Repackage GraphAr and upload it to oss
- Download the third-party packages required by Apache Arrow
    - Script located in `apache-arrow-10.0.1/cpp/thirdparty/download_dependencies.sh`
    - Use the script to download the third-party packages required by Apache Arrow to local
    - Upload the packages to oss and update the Dockerfile Arrow related variable with the oss urls in batch