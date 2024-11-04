#!/usr/bin/bash

DOCKER_IMAGE="tugraph/tugraph-compile-centos8:1.3.3-dev"

WORKING_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
REPO_DIR="$(git rev-parse --show-toplevel)"
echo "Running directory ${WORKING_DIR}"
echo "Repo directory ${REPO_DIR}"

function usage() {
  echo
  echo "Usage: start_dev.sh [<args>]"
  echo
  echo "Options:"
  echo "  -h|--help        print the help info"
  echo "  -p|--port        docker port, must be set!"
  echo "  -n|--name        docker name, default: username-port"
  echo
}

######### parsing arguments #############
while [[ $# -gt 0 ]]; do
  key="$1"
  case $key in
    -h|--help)
      usage
      exit 0
      ;;
    -p|--port)
      PORT="$2"
      shift
      ;;
    -n|--name)
      CONTAINER_NAME="$2"
      shift
      ;;
    *)
      echo "ERROR: unknown option \"$key\""
      usage
      exit 1
      ;;
  esac
  shift
done

if [[ -z "${PORT}" ]]; then
  echo "port must be set"
  usage
  exit 1
fi

USER_ID=$(id -u)
USER_NAME=$(id -u -n)
GROUP_ID=$(id -g)
GROUP_NAME=$(id -g -n)

if [[ -z "${CONTAINER_NAME}" ]]; then
  CONTAINER_NAME=${USER_NAME}-${PORT}
fi
echo "Run container ${CONTAINER_NAME}"

CONTAINER_ID=$(docker ps -a | grep "${CONTAINER_NAME}" | awk '{print $1}')
if [ -z "$CONTAINER_ID" ]; then
    docker run -p ${PORT}:22 -i -d \
        --cap-add SYS_PTRACE \
        --privileged \
        --name "${CONTAINER_NAME}" \
        --hostname "${CONTAINER_NAME}" \
        -v ~/.ssh/:/home/${USER_NAME}/.ssh/ \
        -v /etc/group:/etc/group:ro \
        -v /etc/passwd:/etc/passwd:ro \
        -v /etc/shadow:/etc/shadow:ro \
        -v /etc/sudoers:/etc/sudoers:ro \
        -v /etc/ssh:/etc/ssh:ro \
        -v ${REPO_DIR}:/home/${USER_NAME}/tugraph-db \
        -w /home/${USER_NAME} \
        ${DOCKER_IMAGE} \
        bash -c "chown ${USER_NAME}:${GROUP_NAME} /home/${USER_NAME} &&
          echo 'export JAVA_HOME=/usr/lib/jvm/java-11-openjdk' > /home/${USER_NAME}/.bashrc &&
          echo 'export LD_LIBRARY_PATH=/usr/local/lib64/lgraph:/usr/lib64:/usr/local/lib64:/usr/local/lib:/usr/lib/jvm/java-11-openjdk/lib/server' > /home/${USER_NAME}/.bashrc &&
          echo 'export JAVA_HOME=/usr/lib/jvm/java-11-openjdk' > /home/${USER_NAME}/.bash_profile &&
          /usr/sbin/sshd -D"
fi
