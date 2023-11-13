export HADOOP_HOME=~/hadoop/current

export JRE_HOME=$JAVA_HOME/jre
export HADOOP_BIN_PATH=$HADOOP_HOME/bin
export HADOOP_CONF_DIR=$HADOOP_HOME/etc/hadoop

export PATH=$HADOOP_HOME/bin:$PATH:$JAVA_HOME/bin:$JRE_HOME/bin:$JRE_HOME/bin/server:$PATH

export LD_LIBRARY_PATH=$HADOOP_HOME/lib/native:$JAVA_HOME/jre/lib/amd64/server:$LD_LIBRARY_PATH

#export HC=
#for f in `find $HADOOP_HOME/share -name *.jar`; do
#	export HC=$HADOOP_HOME/$f:$HC
#done

export HC=$(hadoop classpath --glob)

export CLASSPATH=$CLASSPATH:.:$JAVA_HOME/lib:$JRE_HOME/lib
export CLASSPATH=$HC:$CLASSPATH
