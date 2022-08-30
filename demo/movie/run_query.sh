#!/bin/bash

CYPHER="lgraph_cypher -u admin -P 73@TuGraph -p 7090 -f "
QUERY_DIR="./query"

cat $QUERY_DIR/e1.cypher
$CYPHER $QUERY_DIR/e1.cypher
echo

cat $QUERY_DIR/e2.cypher
$CYPHER $QUERY_DIR/e2.cypher
echo

cat $QUERY_DIR/e3.cypher
$CYPHER $QUERY_DIR/e3.cypher
echo

cat $QUERY_DIR/e4.cypher
$CYPHER $QUERY_DIR/e4.cypher
echo

cat $QUERY_DIR/e5.cypher
$CYPHER $QUERY_DIR/e5.cypher
echo

cat $QUERY_DIR/e6.cypher
$CYPHER $QUERY_DIR/e6.cypher
echo

cat $QUERY_DIR/e7.cypher
$CYPHER $QUERY_DIR/e7.cypher
echo

echo "QUERY DONE."