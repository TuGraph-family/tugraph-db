package main

import (
	"context"
	"fmt"
	"github.com/neo4j/neo4j-go-driver/v5/neo4j"
)

func main() {
	driver, err := neo4j.NewDriverWithContext("bolt://localhost:7687", neo4j.BasicAuth("admin", "73@TuGraph", ""))
	if err != nil {
		panic(err)
	}
	ctx := context.Background()
	defer driver.Close(ctx)

	session := driver.NewSession(ctx, neo4j.SessionConfig{DatabaseName: "default"})
	defer session.Close(ctx)

	_, err = session.Run(ctx, "CREATE (n:person {id:1, name: 'James'})", nil)
	if err != nil {
		panic(err)
		return
	}
	_, err = session.Run(ctx, "CREATE (n:person {id:2, name: 'Robert'})", nil)
	if err != nil {
		panic(err)
		return
	}
	_, err = session.Run(ctx, "MATCH (n1:person {id:1}), (n2:person {id:2}) CREATE (n1)-[r:is_friend]->(n2)", nil)
	if err != nil {
		panic(err)
		return
	}
	res, err := session.Run(ctx, "match (n)-[r]->(m) return n,r,m", nil)
	if err != nil {
		panic(err)
		return
	}
	records, err := res.Collect(ctx)
	if err != nil {
		panic(err)
		return
	}
	for _, record := range records {
		fmt.Printf("record = %#v\n", record)
	}
}