# INPUT
TuGraphPath=${1}
CompileDockerImage=${2}
RuntimeDockerImage=${3}
DataX=${4}

# COPY TuGraph
list=".git copyright CMakeLists.txt Options.cmake demo deps include plugins release src test toolkits"
mkdir -p tugraph-db.build
for i in ${list}; do
    cp -r ${TuGraphPath}/$i tugraph-db.build
done
cp -r ${TuGraphPath}/demo demo
cp -r ${DataX} DataX
docker build -t ${RuntimeDockerImage} --build-arg CompileDockerImage=${CompileDockerImage} .
rm -rf tugraph-db.build
rm -rf demo
rm -rf DataX
