from TuGraphRestClient import TuGraphRestClient
import os
import unittest
import subprocess
import time
import json

FILES = {
    "person" : '''Rachel Kempson,1910,10086
Michael Redgrave,1908,10087
Vanessa Redgrave,1937,10088
Corin Redgrave,1939,10089
Liam Neeson,1952,10090
Natasha Richardson,1963,10091
Richard Harris,1930,10092
Dennis Quaid,1954,10093
Lindsay Lohan,1986,10094
Jemma Redgrave,1965,10095
Roy Redgrave,1873,10096
John Williams,1932,10097
Christopher Nolan,1970,10098''',

    "city" : '''Header Line
New York
London
Houston''',

    "film" : '''"Goodbye, Mr. Chips"
Batman Begins
Harry Potter and the Sorcerer's Stone
The Parent Trap
Camelot''',

    "has_child" : '''Rachel Kempson,Vanessa Redgrave
Rachel Kempson,Corin Redgrave
Michael Redgrave,Vanessa Redgrave
Michael Redgrave,Corin Redgrave
Corin Redgrave,Jemma Redgrave
Vanessa Redgrave,Natasha Richardson
Roy Redgrave,Michael Redgrave''',

    "married" : '''Rachel Kempson,Michael Redgrave
Michael Redgrave,Rachel Kempson
Natasha Richardson,Liam Neeson
Liam Neeson,Natasha Richardson''',

    "born_in" : '''Vanessa Redgrave,London,20.21
Natasha Richardson,London,20.18
Christopher Nolan,London,19.93
Dennis Quaid,Houston,19.11
Lindsay Lohan,New York,20.62
John Williams,New York,20.55''',

    "directed" : '''Christopher Nolan,Batman Begins''',

    "wrote" : '''John Williams,Harry Potter and the Sorcerer's Stone
John Williams,"Goodbye, Mr. Chips"''',

    "acted_in" : '''Michael Redgrave,"Goodbye, Mr. Chips",The Headmaster
Vanessa Redgrave,Camelot,Guenevere
Richard Harris,Camelot,King Arthur
Richard Harris,Harry Potter and the Sorcerer's Stone,Albus Dumbledore
Natasha Richardson,The Parent Trap,Liz James
Dennis Quaid,The Parent Trap,Nick Parker
Lindsay Lohan,The Parent Trap,Halle/Annie
Liam Neeson,Batman Begins,Henri Ducard'''
}

DESCRIPTION = {
    "person" : '''{"files": [
    {
        "columns": [
            "name",
            "birthyear",
            "phone"
        ],
        "format": "CSV",
        "header": 0,
        "path": "person.csv",
        "label": "Person"
        }
        ]
    }''',

    "city" : '''
    {
    "files": [
        {
            "columns": [
                "name"
            ],
            "format": "CSV",
            "header": 1,
            "path": "city.csv",
            "label": "City"
        }
    ]
    }
    ''',

    "film" : '''
    {
    "files": [
        {
            "columns": [
                "title"
            ],
            "format": "CSV",
            "header": 0,
            "path": "film.csv",
            "label": "Film"
        }
    ]
    }
    ''',

    "has_child" : '''
    {
    "files": [
        {
            "DST_ID": "Person",
            "SRC_ID": "Person",
            "columns": [
                "SRC_ID",
                "DST_ID"
            ],
            "format": "CSV",
            "header": 0,
            "path": "has_child.csv",
            "label": "HAS_CHILD"
        }
    ]
    }
    ''',

    "married" : '''
    {
    "files": [
        {
            "DST_ID": "Person",
            "SRC_ID": "Person",
            "columns": [
                "SRC_ID",
                "DST_ID"
            ],
            "format": "CSV",
            "header": 0,
            "path": "married.csv",
            "label": "MARRIED"
        }
    ]
    }
    ''',

    "born_in" : '''
    {
    "files": [
        {
            "DST_ID": "City",
            "SRC_ID": "Person",
            "columns": [
                "SRC_ID",
                "DST_ID",
                "weight"
            ],
            "format": "CSV",
            "path": "born_in.csv",
            "header": 0,
            "label": "BORN_IN"
        }
    ]
    }
    ''',

    "directed" : '''
    {
    "files": [
        {
            "DST_ID": "Film",
            "SRC_ID": "Person",
            "columns": [
                "SRC_ID",
                "DST_ID"
            ],
            "format": "CSV",
            "path": "directed.csv",
            "header": 0,
            "label": "DIRECTED"
        }
    ]
    }
    ''',

    "wrote" : '''
    {
    "files": [
    {
        "DST_ID": "Film",
        "SRC_ID": "Person",
        "columns": [
            "SRC_ID",
            "DST_ID"
        ],
        "format": "CSV",
        "header": 0,
        "path": "wrote.csv",
        "label": "WROTE_MUSIC_FOR"
    }
    ]
    }
    ''',

    "acted_in" : '''
    {
    "files": [
        {
            "DST_ID": "Film",
            "SRC_ID": "Person",
            "columns": [
                "SRC_ID",
                "DST_ID",
                "charactername"
            ],
            "format": "CSV",
            "path": "acted_in.csv",
            "header": 0,
            "label": "ACTED_IN"
        }
    ]
    }
    '''
}

SCHEMA = '''{"schema" : [
                    {
                        "label" : "Person",
                        "type" : "VERTEX",
                        "primary" : "name",
                        "properties" : [
                            {"name" : "name", "type":"STRING"},
                            {"name" : "birthyear", "type":"INT16", "optional":true},
                            {"name" : "phone", "type":"INT16","unique":true, "index":true}
                        ]
                    },
                {
                    "label" : "City",
                    "type" : "VERTEX",
                    "primary" : "name",
                    "properties" : [
                        {"name" : "name", "type":"STRING"}
                    ]
                },
                {
                    "label" : "Film",
                    "type" : "VERTEX",
                    "primary" : "title",
                    "properties" : [
                        {"name" : "title", "type":"STRING"}
                    ]
                },
                {"label" : "HAS_CHILD", "type" : "EDGE"},
                {"label" : "MARRIED", "type" : "EDGE"},
                {
                    "label" : "BORN_IN",
                    "type" : "EDGE",
                    "properties" : [
                        {"name" : "weight", "type":"FLOAT", "optional":true}
                    ]
                },
                {"label" : "DIRECTED", "type" : "EDGE"},
                {"label" : "WROTE_MUSIC_FOR", "type" : "EDGE"},
                {
                    "label" : "ACTED_IN",
                    "type" : "EDGE",
                    "properties" : [
                        {"name" : "charactername", "type":"STRING"}
                    ]
                },
                {
                    "label": "PLAY_IN",
                    "type": "EDGE",
                    "properties": [{
                        "name": "role",
                        "type": "STRING",
                        "optional": true
                    }],
                    "constraints": [
                        ["Person", "Film"]
                    ]
                }
    ]}'''

class TuGraphRestClientTest(unittest.TestCase):

    def setUp(self):
        self.url = 'http://100.83.30.35:49091/LGraphHttpService/Query/'
        try:
            # start server in ci command
            self.log = open('/tmp/out.log', 'w+')
            pwd = '/data/jzj/project/antgroup/tugraph-db/build/output'
            self.process = subprocess.Popen([
                pwd + '/lgraph_server',
                '-c', pwd + '/lgraph.json'
            ], stdout=self.log, stderr=self.log, close_fds=True)
            time.sleep(3)
        except Exception as e:
            # in dev environment, start server before run tests
            logging.debug(e)
        self.client = TuGraphRestClient(self.url, 'admin', '73@TuGraph')

    def tearDown(self):
        pass
        try:
            self.process.kill()
            self.process.wait()
            self.log.close()
        except Exception as e:
            logging.debug(e)


    def write_files(self):
        l = []
        for key,val in FILES.items():
            file_name = key + ".csv"
            with open(file_name, "w+") as f:
                f.write(val)
            l.append(file_name)
        return l

    def upload_files(self, files):
        for f in files:
            self.client.upload_file(f)
            res = self.client.check_file_with_size(f, str(self.get_file_size(f)))
            assert res["pass"] == True

    def test_rest_client_01(self):
        res = self.client.import_schema("default", json.loads(SCHEMA))
        res = self.client.call_cypher("default", "CALL db.vertexLabels()")
        assert len(res) == 3
        res = self.client.call_cypher("default", "CALL db.edgeLabels()")
        assert len(res) == 7

    def test_rest_client_02(self):
        files = self.write_files()
        self.upload_files(files)
        tasks = []
        for key,val in DESCRIPTION.items():
            res = self.client.import_data("default", json.loads(val), ",", True, "0", "", "1")
            tasks.append(res["taskId"])
        time.sleep(3)
        for task in tasks:
            print(task)
            progress = self.client.import_progress(task)
            print(progress)

    def get_file_size(self, file_name):
        size = os.path.getsize(file_name)
        return size

    def test_rest_client_03(self):
        res = self.client.call_cypher("default", "match (m:Person) return count(m)")
        count = res[0]["count(m)"]
        assert count == 13

        res = self.client.call_cypher("default", "match (m:City) return count(m)")
        count = res[0]["count(m)"]
        assert count == 3

        res = self.client.call_cypher("default", "match (m:Film) return count(m)")
        count = res[0]["count(m)"]
        assert count == 5

        res = self.client.call_cypher("default", "match (n)-[r:HAS_CHILD]->(m) return count(r)")
        count = res[0]["count(r)"]
        assert count == 7

        res = self.client.call_cypher("default", "match (n)-[r:MARRIED]->(m) return count(r)")
        count = res[0]["count(r)"]
        assert count == 4

        res = self.client.call_cypher("default", "match (n)-[r:BORN_IN]->(m) return count(r)")
        count = res[0]["count(r)"]
        assert count == 6

        res = self.client.call_cypher("default", "match (n)-[r:DIRECTED]->(m) return count(r)")
        count = res[0]["count(r)"]
        assert count == 1

        res = self.client.call_cypher("default", "match (n)-[r:WROTE_MUSIC_FOR]->(m) return count(r)")
        count = res[0]["count(r)"]
        assert count == 2

        res = self.client.call_cypher("default", "match (n)-[r:ACTED_IN]->(m) return count(r)")
        count = res[0]["count(r)"]
        assert count == 8

if __name__ == '__main__':
    unittest.main()