import subprocess
from neo4j import GraphDatabase, basic_auth

def before_all(context):
    result = subprocess.run('./features/steps/init_db.sh')
    assert result.returncode == 0
    url = "bolt://{}:{}".format("127.0.0.1", "7687")
    auth_token = basic_auth("admin", "73@TuGraph")
    context.driver = GraphDatabase.driver(url, auth=auth_token, encrypted=False)

def before_scenario(context, scenario):
    context.parameters = {}
    context.exception = None

def after_all(context):
    pass
