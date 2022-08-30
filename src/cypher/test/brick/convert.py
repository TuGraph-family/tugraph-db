import argparse
import rdflib
import os
from collections import defaultdict

prefix = {
    # schema
    'https://brickschema.org/schema/1.0.2/Brick#': 'brick',
    'https://brickschema.org/schema/1.0.2/BrickFrame#': 'bf',
    'https://brickschema.org/schema/1.0.2/BrickTag#': 'btag',
    'http://www.w3.org/2002/07/owl#': 'owl',
    'http://www.w3.org/1999/02/22-rdf-syntax-ns#': 'rdf',
    'http://www.w3.org/2000/01/rdf-schema#': 'rdfs',
    'http://www.w3.org/2004/02/skos/core#': 'skos',
    'http://www.w3.org/XML/1998/namespace': 'xml',
    'http://www.w3.org/2001/XMLSchema#': 'xsd',
    # building
    'https://brickschema.org/schema/1.0.2/building_example#': 'example',
    'http://virginia.edu/building/ontology/rice#': 'rice',
    'http://cmu.edu/building/ontology/ghc#': 'ghc',
    'http://ucsd.edu/building/ontology/ebu3b#': 'ebu3b',
}
null_str = '<null>'


def new_brick_graph(brick_building_filepath):
    g = rdflib.Graph()
    g.parse('schema/Brick.ttl', format='turtle')
    g.parse('schema/BrickFrame.ttl', format='turtle')
    g.parse('schema/BrickTag.ttl', format='turtle')
    g.parse(brick_building_filepath, format='turtle')
    return g


def fold_uri(uri):
    pos = uri.find('#')
    if pos < 0:
        # print 'Warning: no # found: ' + uri
        return uri
    pre = uri[:pos + 1]
    post = uri[pos + 1:]
    if not prefix.has_key(pre):
        prefix[pre] = 'NULL'
    return prefix[pre] + '$' + post


def convert_2_csv(graph, output):
    vertex = set()
    md = defaultdict(list)
    for s, p, o in graph:
        # print s.encode('ascii', 'ignore'),p.encode('ascii', 'ignore'),o.encode('ascii', 'ignore')
        fs = fold_uri(s.encode('ascii', 'ignore'))
        fp = fold_uri(p.encode('ascii', 'ignore'))
        fo = fold_uri(o.encode('ascii', 'ignore'))
        # print ','.join((fs,fp,fo))
        if fs.find(',') >= 0: fs = '"' + fs + '"'
        if fo.find(',') >= 0: fo = '"' + fo + '"'
        if fs == '': fs = null_str
        if fo == '': fo = null_str
        vertex.update([fs, fo])
        md[fp].append(fs + ',' + fo + '\n')
    f = open(output + '/' + 'vertex.csv', 'w+')
    for v in vertex:
        f.write(v + '\n')
    for k, v in md.items():
        f = open(output + '/' + k + '.csv', 'w+')
        f.writelines(v)
        f.close()


def convert_2_csv_neo4j(graph, output):
    vertex = set()
    md = defaultdict(list)
    for s, p, o in graph:
        fs = fold_uri(s.encode('ascii', 'ignore'))
        fp = fold_uri(p.encode('ascii', 'ignore'))
        fo = fold_uri(o.encode('ascii', 'ignore'))
        if fs == '': fs = null_str
        if fo == '': fo = null_str
        vertex.update([fs, fo])
        md[fp].append('"' + fs + '","' + fo + '",' + fp + '\n')
    f = open(output + '/' + 'vertex.csv', 'w+')
    for v in vertex:
        f.write('"' + v + '",vertex\n')
    f.close()
    f = open(output + '/' + 'edge.csv', 'w+')
    for k, v in md.items():
        f.writelines(v)
    f.close()


ap = argparse.ArgumentParser(description='Convert RDF to LGraph or Neo4j CSV Files')
ap.add_argument('-i', dest='input', action='store', help='input rdf file (*.ttl)')
ap.add_argument('-o', dest='output', action='store', help='output directory')
ap.add_argument('-n', '--neo4j', action='store_true', help='neo4j csv format')
args = ap.parse_args()
input = args.input
output = args.output

g = new_brick_graph(input)
if not os.path.exists(output): os.makedirs(output)
if args.neo4j:
    convert_2_csv_neo4j(g, output)
else:
    convert_2_csv(g, output)

# print prefix 
for k, v in prefix.items():
    print
    k, v
