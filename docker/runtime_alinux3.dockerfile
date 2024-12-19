FROM alibaba-cloud-linux-3-registry.cn-hangzhou.cr.aliyuncs.com/alinux3/alinux3:3.9.1

RUN cd /etc/yum.repos.d && sed -i '/mirrors\.aliyun\.com/d' * && \
    sed -i 's/mirrors\.cloud\.aliyuncs\.com/mirrors.aliyun.com/g' * && yum clean all && yum makecache

RUN yum install -y gfortran && yum clean all

# install tugraph
# specifies the path of the object storage where the installation package resides
ARG FILEPATH
# specifies installation package name for tugraph
ARG FILENAME
RUN wget ${FILEPATH}/${FILENAME} && rpm -ivh ${FILENAME} && rm -f /${FILENAME}

ENV LD_LIBRARY_PATH=/usr/local/lib64/lgraph:$LD_LIBRARY_PATH

CMD lgraph_server --mode run