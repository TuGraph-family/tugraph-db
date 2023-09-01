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

### Geometry Package

Boost.Geometry (aka Generic Geometry Library, GGL), part of collection of the Boost C++ Libraries, defines concepts,
primitives and algorithms for solving geometry problems. We use Boost.Geometry(including extensions) to support our
spatial data.
To build the package, take these steps:

- Get the Boost 1.68.0 package as shown in images:
    - https://boostorg.jfrog.io/artifactory/main/release/1.68.0/source/boost_1_68_0.tar.gz
    - wget https://tugraph-web.oss-cn-beijing.aliyuncs.com/tugraph/deps/boost_1_68_0.tar.gz \
    - && tar xf boost_1_68_0.tar.gz && cd boost_1_68_0 \

- find and change the geometry library in boost:
    - case the INVALID_INDEX(occur only once) in the 513 line of boost/geometry/index/detail/rtree/predicates.hpp
    - contridicts with the INVALID_INDEX in antlr4-common.h, we need to change the INVALID_INDEX in
    - geometry library to INVALID_INDEX_
        - cd boost/geometry
        - Replace the INVALID_INDEX in 513 line of boost/geometry/index/detail/rtree/predicates.hpp to INVALID_INDEX_

- add the extensions to geometry libraray:
    - Download the extensions of boost.geometry develop branch from github
        - extensions link: https://github.com/boostorg/geometry.git
    - reset the branch to version corresponding to the 1.68.0 version of boost
        - git reset --hard 5fbd2a11027f658decda5e43af73935e98a84ee5
    - move the extensions to boost/geometry directly(cause it's head only library)
        - cp -r extensions boost/geometry/

- Package the whole geometry library as .tar.gz and upload it to oss.