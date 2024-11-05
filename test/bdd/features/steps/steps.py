import os
import math
import yaml
import neo4j
from behave import *
from neo4j import basic_auth, GraphDatabase
from neo4j.graph import Node, Path, Relationship
import parser
import subprocess

use_step_matcher("re")
def run_cypher(cypher, context):
    records = []
    session = context.driver.session(database="default")
    context.exception = None
    try:
        ret = session.run(cypher, context.parameters)
        records = list(ret)
    except Exception as e:
        context.exception = e
        print("context.exception: ", context.exception)
    finally:
        session.close()
    return records

def parse_props(props_key_value):
    if not props_key_value:
        return ""
    properties = "{"
    for key, value in props_key_value:
        if value is None:
            properties += key + ": null, "
        elif isinstance(value, str):
            properties += key + ": " + "'" + value + "', "
        elif isinstance(value, bool):
            if value:
                properties += key + ": true, "
            else:
                properties += key + ": false, "
        elif isinstance(value, neo4j.time.Date):
            properties += key + ": " +  '\'' + str(value) + '\'' + ", "
        elif isinstance(value, neo4j.time.Time):
            properties += key + ": " +  '\'' + str(value) + '\'' + ", "
        elif isinstance(value, neo4j.time.DateTime):
            properties += key + ": " +  '\'' + str(value) + '\'' + ", "
        elif isinstance(value, neo4j.time.Duration):
            properties += key + ": " +  '\'' + str(value) + '\'' + ", "
        else:
            properties += key + ": " + str(value) + ", "
    properties = properties[:-2]
    properties += "}"
    return properties

def to_string(element):
    if element is None:
        return "null"

    if isinstance(element, Node):
        sol = "("
        if element.labels:
            sol += ':' + ': '.join(element.labels)

        if element.keys():
            if element.labels:
                sol += ' '
            sol += parse_props(element.items())

        sol += ")"
        return sol

    elif isinstance(element, Relationship):
        sol = "[:"
        if element.type:
            sol += element.type
        if element.keys():
            sol += ' '
        sol += parse_props(element.items())
        sol += "]"
        return sol

    elif isinstance(element, Path):
        edges = []
        nodes = []

        for rel in element.relationships:
            edges.append([rel.start_node.id, to_string(rel)])

        for node in element.nodes:
            nodes.append([node.id, to_string(node)])

        sol = "<"
        for i in range(0, len(edges)):
            if edges[i][0] == nodes[i][0]:
                sol += nodes[i][1] + "-" + edges[i][1] + "->"
            else:
                sol += nodes[i][1] + "<-" + edges[i][1] + "-"

        sol += nodes[len(edges)][1]
        sol += ">"

        return sol

    elif isinstance(element, str):
        return "'" + element + "'"

    elif isinstance(element, list):
        sol = '['
        el_str = []
        for el in element:
            el_str.append(to_string(el))
        sol += ', '.join(el_str)
        sol += ']'
        return sol

    elif isinstance(element, bool):
        if element:
            return "true"
        return "false"

    elif isinstance(element, dict):
        if len(element) == 0:
            return '{}'
        sol = '{'
        for key, val in element.items():
            sol += key + ':' + to_string(val) + ','
        sol = sol[:-1] + '}'
        return sol

    elif isinstance(element, float):
        if 'e' in str(element):
            if str(element)[-3] == '-':
                zeroes = int(str(element)[-2:]) - 1
                num_str = ''
                if str(element)[0] == '-':
                    num_str += '-'
                num_str += '.' + zeroes * '0' + \
                    str(element)[:-4].replace("-", "").replace(".", "")
                return num_str
        elif math.isnan(element):
            return 'NaN'
    elif isinstance(element, neo4j.time.Date):
        return '\'' + str(element) + '\''
    elif isinstance(element, neo4j.time.Time):
        return '\'' + str(element) + '\''
    elif isinstance(element, neo4j.time.DateTime):
        return '\'' + str(element) + '\''
    elif isinstance(element, neo4j.time.Duration):
        return '\'' + str(element) + '\''
    return str(element)


def get_result_rows(context, ignore_order):
    result_rows = []
    for result in context.results:
        keys = result.keys()
        values = result.values()
        for i in range(0, len(keys)):
            result_rows.append(keys[i] + ":" + parser.parse(
                to_string(values[i]).replace("\n", "\\n").replace(" ", ""),
                ignore_order))
    return result_rows


def get_expected_rows(context, ignore_order):
    expected_rows = []
    for row in context.table:
        for col in context.table.headings:
            expected_rows.append(
                col + ":" + parser.parse(row[col].replace(" ", ""),
                                         ignore_order))
    return expected_rows


def validate(context, ignore_order):
    result_rows = get_result_rows(context, ignore_order)
    expected_rows = get_expected_rows(context, ignore_order)

    print("Expected: %s" %(str(expected_rows)))
    print("Results : %s" %(str(result_rows)))
    assert(len(expected_rows) == len(result_rows))

    for i in range(0, len(expected_rows)):
        if expected_rows[i] in result_rows:
            result_rows.remove(expected_rows[i])
        else:
            assert(False)


def validate_in_order(context, ignore_order):
    result_rows = get_result_rows(context, ignore_order)
    expected_rows = get_expected_rows(context, ignore_order)

    print("Expected: %s" %(str(expected_rows)))
    print("Results : %s" %(str(result_rows)))
    assert(len(expected_rows) == len(result_rows))

    for i in range(0, len(expected_rows)):
        if expected_rows[i] != result_rows[i]:
            assert(False)

def check_exception(context):
    if context.exception is not None:
        print("Exception when executing query: ", context.exception)
        assert(False)

@given("an initialized database")
def step_impl(context):
    result = subprocess.run('./features/steps/init_db.sh')
    assert result.returncode == 0
    url = "bolt://{}:{}".format("127.0.0.1", "7687")
    auth_token = basic_auth("admin", "73@TuGraph")
    context.driver = GraphDatabase.driver(url, auth=auth_token, encrypted=False)

@when("executing query")
def step_impl(context):
    context.results = run_cypher(context.text, context)

@then("the result should be empty")
def step_impl(context):
    assert(len(context.results) == 0)
    check_exception(context)

@given("an empty graph")
def step_impl(context):
    run_cypher("CALL db.dropDB()", context)

@given("yago graph")
def step_impl(context):
    run_cypher("CALL db.dropDB()", context)
    check_exception(context)
    yago_graph = """
CREATE (rachel:Person {name: 'Rachel Kempson', birthyear: 1910})
CREATE (michael:Person {name: 'Michael Redgrave', birthyear: 1908})
CREATE (vanessa:Person {name: 'Vanessa Redgrave', birthyear: 1937})
CREATE (corin:Person {name: 'Corin Redgrave', birthyear: 1939})
CREATE (liam:Person {name: 'Liam Neeson', birthyear: 1952})
CREATE (natasha:Person {name: 'Natasha Richardson', birthyear: 1963})
CREATE (richard:Person {name: 'Richard Harris', birthyear: 1930})
CREATE (dennis:Person {name: 'Dennis Quaid', birthyear: 1954})
CREATE (lindsay:Person {name: 'Lindsay Lohan', birthyear: 1986})
CREATE (jemma:Person {name: 'Jemma Redgrave', birthyear: 1965})
CREATE (roy:Person {name: 'Roy Redgrave', birthyear: 1873})

CREATE (john:Person {name: 'John Williams', birthyear: 1932})
CREATE (christopher:Person {name: 'Christopher Nolan', birthyear: 1970})

CREATE (newyork:City {name: 'New York'})
CREATE (london:City {name: 'London'})
CREATE (houston:City {name: 'Houston'})

CREATE (mrchips:Film {title: 'Goodbye Mr. Chips'})
CREATE (batmanbegins:Film {title: 'Batman Begins'})
CREATE (harrypotter:Film {title: "Harry Potter and the Sorcerer's Stone"})
CREATE (parent:Film {title: 'The Parent Trap'})
CREATE (camelot:Film {title: 'Camelot'})

CREATE (rachel)-[:HAS_CHILD]->(vanessa),
       (rachel)-[:HAS_CHILD]->(corin),
       (michael)-[:HAS_CHILD]->(vanessa),
       (michael)-[:HAS_CHILD]->(corin),
       (corin)-[:HAS_CHILD]->(jemma),
       (vanessa)-[:HAS_CHILD]->(natasha),
       (roy)-[:HAS_CHILD]->(michael),

       (rachel)-[:MARRIED]->(michael),
       (michael)-[:MARRIED]->(rachel),
       (natasha)-[:MARRIED]->(liam),
       (liam)-[:MARRIED]->(natasha),

       (vanessa)-[:BORN_IN {weight:20.21}]->(london),
       (natasha)-[:BORN_IN {weight:20.18}]->(london),
       (christopher)-[:BORN_IN {weight:19.93}]->(london),
       (dennis)-[:BORN_IN {weight:19.11}]->(houston),
       (lindsay)-[:BORN_IN {weight:20.62}]->(newyork),
       (john)-[:BORN_IN {weight:20.55}]->(newyork),

       (christopher)-[:DIRECTED]->(batmanbegins),

       (john)-[:WROTE_MUSIC_FOR]->(harrypotter),
       (john)-[:WROTE_MUSIC_FOR]->(mrchips),

       (michael)-[:ACTED_IN {charactername: 'The Headmaster'}]->(mrchips),
       (vanessa)-[:ACTED_IN {charactername: 'Guenevere'}]->(camelot),
       (richard)-[:ACTED_IN {charactername: 'King Arthur'}]->(camelot),
       (richard)-[:ACTED_IN {charactername: 'Albus Dumbledore'}]->(harrypotter),
       (natasha)-[:ACTED_IN {charactername: 'Liz James'}]->(parent),
       (dennis)-[:ACTED_IN {charactername: 'Nick Parker'}]->(parent),
       (lindsay)-[:ACTED_IN {charactername: 'Halle/Annie'}]->(parent),
       (liam)-[:ACTED_IN {charactername: 'Henri Ducard'}]->(batmanbegins)
    """
    run_cypher(yago_graph, context)
    check_exception(context)

@then("the result should be, in any order")
def step_impl(context):
    validate(context, False)
    check_exception(context)

@when("executing control query")
def step_impl(context):
    context.results = run_cypher(context.text, context)
    check_exception(context)

@step("having executed")
def step_impl(context):
    for query in context.text.strip().split(';'):
        query = query.strip()
        if len(query) == 0:
            continue
        run_cypher(query, context)
        check_exception(context)

@step("parameters are")
def step_impl(context):
    assert len(context.table.rows) == 1
    for row in context.table:
        for index, header in enumerate(context.table.headings):
            ret = yaml.load(row[index], Loader=yaml.FullLoader)
            if isinstance(ret, str) and ret.startswith("'") and ret.endswith("'"):
                ret = ret[1: len(ret) - 1]
            context.parameters[header] = ret

@then("the result should be \(ignoring element order for lists\)")
def step_impl(context):
    validate(context, True)
    check_exception(context)

@then("the result should be, in order \(ignoring element order for lists\)")
def step_impl(context):
    validate_in_order(context, True)
    check_exception(context)

@then("the result should be, in order")
def step_impl(context):
    validate_in_order(context, False)
    check_exception(context)

@then("an Error should be raised")
def step_impl(context):
    assert context.exception is not None
