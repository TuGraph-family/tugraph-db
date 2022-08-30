
cypher=`cat $1`
echo $cypher

curl -XPOST \
    "http://0.0.0.0:7078/db/cypher" \
    -H "Accept: application/json" \
    -H "Content-Type: application/json" \
    -H "Authorization: Basic YWRtaW46YWRtaW4xMjM0NTY=" \
    -d "{\"script\":\"${cypher}\"}"

