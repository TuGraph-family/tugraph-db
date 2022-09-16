ARG CompileDockerImage
FROM ${CompileDockerImage}

COPY tugraph-db.build tugraph-db.build
COPY demo /root/demo
COPY DataX /root/DataX

RUN cd /tugraph-db.build/deps && bash build_deps.sh -j4 \
    && cd .. && mkdir build && cd build \
    && cmake .. && make -j4 install && rm -r /tugraph-db.build && rm -r /root/.npm

RUN cd /root/DataX && mvn clean package -DskipTests assembly:assembly && cd target/datax/datax \
    && DATAX_PREFIX=/usr/local && mkdir -p ${DATAX_PREFIX}/bin \
    && mkdir -p ${DATAX_PREFIX}/conf && mkdir -p ${DATAX_PREFIX}/job \
    && mkdir -p ${DATAX_PREFIX}/plugin/reader && mkdir -p ${DATAX_PREFIX}/plugin/writer \
    && cp bin/* ${DATAX_PREFIX}/bin && cp conf/* ${DATAX_PREFIX}/conf \
    && cp job/* ${DATAX_PREFIX}/job && cp lib/* ${DATAX_PREFIX}/lib \
    && cp -r plugin/reader/mysqlreader ${DATAX_PREFIX}/plugin/reader/ \
    && cp -r plugin/reader/hdfsreader ${DATAX_PREFIX}/plugin/reader/ \
    && cp -r plugin/writer/mysqlwriter ${DATAX_PREFIX}/plugin/writer/ \
    && cp -r plugin/writer/hdfswriter ${DATAX_PREFIX}/plugin/writer/ \
    && cp -r plugin/writer/tugraphwriter ${DATAX_PREFIX}/plugin/writer/ \
    && chmod 755 /usr/local/bin/* && rm -r /root/DataX && rm -r /root/.m2
