/*
// scenario 2:
match path = (na:vertex {id:'order1'})-[ra]-(nb)-[rb]-(nc)
where ra.type in [1,2,3,4] and rb.type in [1,2,3,4] and nb.type in [1,2.3,4,5,6,7]
return distinct nc
*/

// scenario 3:
match path = (na:vertex {id:'order1'})-[ra]-(nb)-[rb]-(nc)
where ra.type in [1,2,3,4] and rb.type in [1,2,3,4] 
return count(path)

/*
// scenario 4:
match path = (na:vertex {id:'order1'})-[ra]-(nb)-[rb]-(nc)
where ra.type in [1,2,3,4] and rb.type in [1,2,3,4] 
return count(distinct nc)

// scenario 5:
match path = (na:vertex {id:'order1'})-[ra]-(nb)-[rb]-(nc)
where ra.type in [1,2,3,4] and rb.type in [1,2,3,4] 
return path

// scenario 6:
match path = (na:vertex {id:'order1'})-[ra]-(nb)-[rb]-(nc)
where ra.type in [1,2] and rb.type in [1,2] and nb.type in [3,4] 
return path
*/
