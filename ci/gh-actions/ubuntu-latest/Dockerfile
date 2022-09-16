# build for lgraph
FROM tugraph/tugraph-compile-centos7:latest

RUN mkdir -p /root/tugraph-db/
ENV WORKSPACE=/root/tugraph-db/
COPY . /root/tugraph-db/
WORKDIR /root/tugraph-db/

RUN bash /root/tugraph-db/ci/ci_cmd.sh
