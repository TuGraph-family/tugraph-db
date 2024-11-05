#!/bin/bash
set -o errexit

# This script is used to generate source files from the test grammars in the same folder. The generated files are placed
# into a subfolder "generated" which the demo project uses to compile a demo binary.

# There are 2 ways of running the ANTLR generator here.

# 1) Running from jar. Use the given jar (or replace it by another one you built or downloaded) for generation.
# Build jar:
export SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )
export PROJECT_SOURCE_DIR=${SCRIPT_DIR}/../../../
cd ${PROJECT_SOURCE_DIR}/deps/antlr4
mvn clean
MAVEN_OPTS="-Xmx1G" mvn -DskipTests package
LOCATION=${PROJECT_SOURCE_DIR}/deps/antlr4/tool/target/antlr4-4.13.0-complete.jar
GENERATED_DIR=${PROJECT_SOURCE_DIR}/src/cypher/parser/generated
if [ -d "$GENERATED_DIR" ]; then
    echo "directory '$GENERATED_DIR' already exists!" && exit 1
fi
# Generate
java -jar $LOCATION -Dlanguage=Cpp -no-listener -visitor -o $GENERATED_DIR -package parser \
    ${PROJECT_SOURCE_DIR}/src/cypher/grammar/Lcypher.g4
rm $GENERATED_DIR/LcypherBaseVisitor.*

#java -jar $LOCATION -Dlanguage=Cpp -listener -visitor -o $GENERATED_DIR -package antlrcpptest Lcypher.g4
#java -jar $LOCATION -Dlanguage=Cpp -listener -visitor -o generated/ -package antlrcpptest -XdbgST TLexer.g4 TParser.g4
#java -jar $LOCATION -Dlanguage=Java -listener -visitor -o generated/ -package antlrcpptest TLexer.g4 TParser.g4

# 2) Running from class path. This requires that you have both antlr3 and antlr4 compiled. In this scenario no installation
#    is needed. You just compile the java class files (using "mvn compile" in both the antlr4 and the antlr3 root folders).
#    The script then runs the generation using these class files, by specifying them on the classpath.
#    Also the string template jar is needed. Adjust CLASSPATH if you have stored the jar in a different folder as this script assumes.
#    Furthermore is assumed that the antlr3 folder is located side-by-side with the antlr4 folder. Adjust CLASSPATH if not.
#    This approach is especially useful if you are working on a target stg file, as it doesn't require to regenerate the
#    antlr jar over and over again.
#CLASSPATH=../../../tool/resources/:ST-4.0.8.jar:../../../tool/target/classes:../../../runtime/Java/target/classes:../../../../antlr3/runtime/Java/target/classes

#java -cp $CLASSPATH org.antlr.v4.Tool -Dlanguage=Cpp -listener -visitor -o generated/ -package antlrcpptest TLexer.g4 TParser.g4
#java -cp $CLASSPATH org.antlr.v4.Tool -Dlanguage=Cpp -listener -visitor -o generated/ -package antlrcpptest -XdbgST TLexer.g4 TParser.g4
#java -cp $CLASSPATH org.antlr.v4.Tool -Dlanguage=Java -listener -visitor -o generated/ TLexer.g4 TParser.g4
