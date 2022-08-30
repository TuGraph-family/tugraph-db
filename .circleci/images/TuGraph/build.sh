# INPUT
TuGraphPath=${1}
TuGraphEnvDockerImage=${2}
TuGraphDockerImage=${3}
DataX=${4}

# COPY TuGraph
list=".git copyright CMakeLists.txt demo deps include plugins release src test toolkits ha"
mkdir -p TuGraph.Build
for i in ${list}; do
    cp -r ${TuGraphPath}/$i TuGraph.Build
done
cp -r ${TuGraphPath}/demo demo
cp -r ${DataX} DataX
docker build -t ${TuGraphDockerImage} --build-arg TuGraphEnvDockerImage=${TuGraphEnvDockerImage} .
rm -rf TuGraph.Build
rm -rf demo
rm -rf DataX
