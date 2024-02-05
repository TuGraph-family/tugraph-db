# build for lgraph
FROM tugraph/tugraph-compile-centos7:latest

ARG BUILD_TYPE
ARG TEST_TYPE
RUN echo "The value of BUILD_TYPE is: ${BUILD_TYPE}"
RUN echo "The value of TEST_TYPE is: ${TEST_TYPE}"

RUN mkdir -p /root/tugraph-db/
ENV WORKSPACE=/root/tugraph-db/
COPY . /root/tugraph-db/
WORKDIR /root/tugraph-db/

RUN --mount=type=secret,id=CODECOV_TOKEN \
    export CODECOV_TOKEN=$(cat /run/secrets/CODECOV_TOKEN) && \
    bash /root/tugraph-db/ci/github_ci.sh ${BUILD_TYPE} ${TEST_TYPE}
