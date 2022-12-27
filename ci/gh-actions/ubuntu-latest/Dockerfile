# build for lgraph
FROM tugraph/tugraph-compile-centos7:latest

RUN mkdir -p /root/tugraph-db/
ENV WORKSPACE=/root/tugraph-db/
COPY . /root/tugraph-db/
WORKDIR /root/tugraph-db/

RUN set -e && bash ./cpplint/check_all.sh
#RUN --mount=type=secret,id=CODECOV_TOKEN \
#    export CODECOV_TOKEN=$(cat /run/secrets/CODECOV_TOKEN) && \
#    bash /root/tugraph-db/ci/ci_cmd.sh
