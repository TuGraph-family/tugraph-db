import argparse
import rdflib

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
}
queries = [
    '''
    SELECT DISTINCT ?vav WHERE {
    ?vav rdf:type brick:VAV .
    }
    ''',  # 1
    '''
    SELECT DISTINCT ?sensor WHERE {
      ?sensor rdf:type/rdfs:subClassOf* brick:Zone_Temperature_Sensor .
    }
    ''',  # 2
    '''
    SELECT DISTINCT ?x WHERE {
      ?ahu rdf:type brick:AHU .
      ?ahu bf:feeds+ ?x .
    }
    ''',  # 3
    '''
    SELECT DISTINCT ?floor ?room ?zone WHERE {
      ?floor rdf:type brick:Floor .
      ?room rdf:type brick:Room .
      ?zone rdf:type brick:HVAC_Zone .
      ?floor bf:hasPart+ ?room .
      ?zone bf:hasPart+ ?room .
    }
    ''',  # 4
    '''
    SELECT DISTINCT ?sensor ?room
    WHERE {
      { ?sensor rdf:type/rdfs:subClassOf* brick:Zone_Temperature_Sensor . }
      UNION
      { ?sensor rdf:type/rdfs:subClassOf* brick:Discharge_Air_Temperature_Sensor . }
      UNION
      { ?sensor rdf:type/rdfs:subClassOf* brick:Occupancy_Sensor . }
      UNION
      { ?sensor rdf:type/rdfs:subClassOf* brick:CO2_Sensor . }
      ?vav rdf:type brick:VAV .
      ?zone rdf:type brick:HVAC_Zone .
      ?room rdf:type brick:Room .
      ?vav bf:feeds+ ?zone .
      ?zone bf:hasPart ?room .
      {?vav bf:hasPoint ?sensor }
      UNION
      {?room bf:hasPoint ?sensor }
    }
    ''',  # 5
    '''
    SELECT DISTINCT ?vav ?x ?y ?z ?a  WHERE {
      ?vav rdf:type brick:VAV .
      ?vav bf:feeds+ ?x .
      ?y bf:feeds+ ?vav .
      ?vav bf:hasPoint+ ?z .
      ?a bf:isPartOf+ ?vav .
    }
    ''',  # 6
    '''
    SELECT DISTINCT ?vav ?room ?tempsensor ?valvesensor ?setpoint WHERE {
      ?vav rdf:type brick:VAV .
      ?vav bf:hasPoint ?tempsensor .
      ?tempsensor rdf:type/rdfs:subClassOf* brick:Temperature_Sensor .
      ?vav bf:hasPoint ?valvesensor .
      ?valvesensor rdf:type/rdfs:subClassOf* brick:Valve_Command .
      ?vav bf:hasPoint ?setpoint .
      ?setpoint rdf:type/rdfs:subClassOf* brick:Zone_Temperature_Setpoint .
      ?room rdf:type brick:Room .
      ?tempsensor bf:isLocatedIn ?room .
    }
    ''',  # 7
]


def new_brick_graph(brick_building_filepath):
    g = rdflib.Graph()
    for key, value in prefix.items():
        g.bind(value, rdflib.Namespace(key))
    g.parse('schema/Brick.ttl', format='turtle')
    g.parse('schema/BrickFrame.ttl', format='turtle')
    g.parse('schema/BrickTag.ttl', format='turtle')
    g.parse(brick_building_filepath, format='turtle')
    return g


ap = argparse.ArgumentParser(description='Query Brick Building With RDFLib')
ap.add_argument('-i', dest='input', action='store', help='input rdf file (*.ttl)')
ap.add_argument('-q', dest='query_number', action='store', type=int, default=0,
                choices=[n for n in range(0, len(queries) + 1)], help='query number (0 for all)')
args = ap.parse_args()
input = args.input
qn = args.query_number

g = new_brick_graph(input)
count = 0
for q in queries:
    count += 1
    if qn > 0 and count < qn: continue
    if qn > 0 and count > qn: break
    res = list(g.query(q))
    print
    q
    for r in res:
        print
        r
