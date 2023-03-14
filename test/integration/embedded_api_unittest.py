#!/usr/bin/env python

import math
import datetime

from liblgraph_python_api import *


class ExceptionWasNotThrown(Exception):
    pass


def FMA_EXPECT_EXCEPTION(code, glbs, lcls):
    exec('''try:
    {}
    raise ExceptionWasNotThrown()
except ExceptionWasNotThrown:
    raise ExceptionWasNotThrown("expecting an exception, but it was not thrown")
except Exception as e:
    print("expected exception: " + str(e))
    '''.format(code), glbs, lcls)


class LGraphUnitTests:

    def test_field_data(self):
        assert (FieldData(11) == 11)
        assert (FieldData.Int8(11) == 11)
        assert (FieldData(11) > 10)
        assert (FieldData(11.1) <= 12)
        assert (FieldData(12) != 12.3)
        assert (FieldData('abc') < 'abcd')

    def test_datetime(self):
        fd = FieldData.DateTime("2020-09-11 18:00:01")
        assert (fd.AsDateTime() == datetime.datetime(2020, 9, 11, 18, 0, 1))
        assert (fd.ToPython() == datetime.datetime(2020, 9, 11, 18, 0, 1))
        assert (fd == FieldData.DateTime(datetime.datetime(2020, 9, 11, 18, 0, 1)))
        assert (fd != FieldData.DateTime(datetime.datetime(2020, 9, 11, 18, 0, 2)))
        assert (fd > FieldData.DateTime(datetime.datetime(2020, 9, 11, 18, 0, 0)))
        assert (fd < FieldData.DateTime(datetime.datetime(2020, 9, 11, 18, 0, 2)))
        assert (fd >= FieldData.DateTime(datetime.datetime(2020, 9, 11, 18, 0, 1)))
        assert (fd >= FieldData.DateTime(datetime.datetime(2020, 9, 11, 18, 0, 0)))
        assert (fd < FieldData.DateTime(datetime.datetime(2020, 9, 11, 18, 0, 3)))
        assert (fd < FieldData.DateTime(datetime.datetime(2020, 9, 11, 18, 0, 3)))
        assert (fd <= FieldData.DateTime(datetime.datetime(2020, 9, 11, 18, 0, 1)))
        assert (fd <= FieldData.DateTime(datetime.datetime(2020, 9, 11, 18, 0, 2)))
        assert (fd > FieldData.DateTime(datetime.datetime(2020, 9, 11, 18, 0, 0)))
        fd = FieldData.Date("2020-11-12")
        assert (fd == FieldData.Date(datetime.datetime(2020, 11, 12)))
        assert (fd.AsDate() == datetime.datetime(2020, 11, 12))

    def test_db(self):
        with Galaxy("./testdb", True, True) as galaxy:
            galaxy.SetCurrentUser("admin", "73@TuGraph")
            res = galaxy.Cypher("default", "match(a) return a")
            print("test Cypher")
            print(res)

            with galaxy.OpenGraph("default") as db:
                db.DropAllData()
                lv_0 = "person"
                fds_0 = [
                    FieldSpec("id", FieldType.INT64, False),
                    FieldSpec("name", FieldType.STRING, False),
                    FieldSpec("age", FieldType.INT32, True)
                ]
                lv_1 = "software"
                fds_1 = [
                    FieldSpec("id", FieldType.INT64, False),
                    FieldSpec("name", FieldType.STRING, False),
                    FieldSpec("lang", FieldType.STRING, True)
                ]
                lv_2 = "software_plus"
                fds_4 = [
                    FieldSpec("id", FieldType.INT64, False),
                    FieldSpec("name", FieldType.STRING, False),
                    FieldSpec("os", FieldType.STRING, True),
                    FieldSpec("version", FieldType.INT8, True)
                ]
                le_0 = "knows"
                fds_2 = [
                    FieldSpec("weight", FieldType.FLOAT, False)
                ]
                le_1 = "created"
                fds_3 = [
                    FieldSpec("weight", FieldType.FLOAT, False)
                ]
                le_2 = "created_plus"
                fds_5 = [
                    FieldSpec("duration", FieldType.INT32, False),
                    FieldSpec("weight", FieldType.FLOAT, False)
                ]
                le_3 = "deleted_label"
                fds_7 = [
                    FieldSpec("deleted", FieldType.FLOAT, False)
                ]
                defalut_spec = [FieldSpec("modified", FieldType.FLOAT, False)]
                mod_spec = [FieldSpec("modified", FieldType.FLOAT, True)]
                defalut_value = [FieldData(23.1)]
                print("\nadd labels,indexes")
                assert (db.AddVertexLabel(lv_0, fds_0, "id"))
                assert (db.AddVertexLabel(lv_1, fds_1, "id"))
                assert (db.AddEdgeLabel(le_0, fds_2))
                assert (db.AddEdgeLabel(le_1, fds_3))
                assert (db.AddVertexLabel(lv_2, fds_4, "id"))
                assert (db.AddEdgeLabel(le_2, fds_5))
                assert (db.AddEdgeLabel(le_3, fds_7))
                assert (~db.AlterEdgeLabelAddFields(le_3, defalut_spec, defalut_value))
                assert (~db.AlterEdgeLabelModFields(le_3, mod_spec))
                assert (~db.AlterEdgeLabelDelFields(le_3, ["modified"]))
                assert (~db.DeleteEdgeLabel(le_3))
                FMA_EXPECT_EXCEPTION('db.DeleteEdgeLabel(le_3)', globals(), locals())
                FMA_EXPECT_EXCEPTION('db.AlterEdgeLabelAddFields(le_3, defalut_spec, defalut_value)', globals(),
                                     locals())
                FMA_EXPECT_EXCEPTION('db.AlterEdgeLabelModFields(le_3, defalut_spec, defalut_value)', globals(),
                                     locals())
                FMA_EXPECT_EXCEPTION('db.AlterEdgeLabelDeleteFields(le_3, defalut_spec, defalut_value)', globals(),
                                     locals())

                assert (db.AddVertexLabel(le_3, fds_7, "deleted"))
                assert (~db.AlterVertexLabelAddFields(le_3, defalut_spec, defalut_value))
                assert (~db.AlterVertexLabelModFields(le_3, mod_spec))
                assert (~db.AlterVertexLabelDelFields(le_3, ["modified"]))
                assert (~db.DeleteVertexLabel(le_3))
                FMA_EXPECT_EXCEPTION('db.DeleteVertexLabel(le_3)', globals(), locals())
                FMA_EXPECT_EXCEPTION('db.AlterVertexLabelAddFields(le_3, defalut_spec, defalut_value)', globals(),
                                     locals())
                FMA_EXPECT_EXCEPTION('db.AlterVertexLabelModFields(le_3, defalut_spec, defalut_value)', globals(),
                                     locals())
                FMA_EXPECT_EXCEPTION('db.AlterVertexLabelDeleteFields(le_3, defalut_spec, defalut_value)', globals(),
                                     locals())

                assert (db.AddVertexIndex(lv_0, fds_0[1].name, False))
                assert (db.AddVertexIndex(lv_0, fds_0[2].name, False))
                assert (db.AddVertexIndex(lv_1, fds_1[1].name, False))

                print("\nlist indexes")
                with db.CreateReadTxn() as txn:
                    person_lid = txn.GetVertexLabelId("person")
                    software_lid = txn.GetVertexLabelId("software")
                    software_plus_lid = txn.GetVertexLabelId("software_plus")
                    knows_lid = txn.GetEdgeLabelId("knows")
                    created_lid = txn.GetEdgeLabelId("created")
                    created_plus_lid = txn.GetEdgeLabelId("created_plus")

                    indexes = txn.ListVertexIndexes()
                    assert (len(indexes) == 6)

                print("\ndelete indexes")
                assert (db.DeleteVertexIndex(lv_0, fds_0[1].name))
                assert (db.DeleteVertexIndex(lv_0, fds_0[1].name) == False)
                with db.CreateReadTxn() as txn:
                    assert (len(txn.ListVertexIndexes()) == 5)

                print("\nlist labels")
                with db.CreateReadTxn() as txn:
                    vlabels = txn.ListVertexLabels()
                    assert (len(vlabels) == 3)
                    elabels = txn.ListEdgeLabels()
                    assert (len(elabels) == 3)

                print("\nget schema")
                with db.CreateReadTxn() as txn:
                    assert (len(txn.GetVertexSchema(lv_1)) == 3)
                    assert (len(txn.GetEdgeSchema(le_2)) == 2)
                    assert (txn.GetVertexLabelId(lv_2) == 2)
                    assert (txn.GetEdgeLabelId(le_0) == 0)
                    f_names = ["version", "os"]
                    assert (txn.GetVertexFieldIds(2, f_names) == [1, 3])

                print("\nadd vertices and edges")
                with db.CreateWriteTxn() as txn:
                    vid = []
                    person_fields = ["id", "name", "age"]
                    person_fids = txn.GetVertexFieldIds(person_lid, person_fields)
                    software_fields = ["id", "name", "lang"]
                    software_fids = txn.GetVertexFieldIds(software_lid, software_fields)

                    vid.append(txn.AddVertex(person_lid, person_fids,
                                             [FieldData(2), FieldData("vadas"), FieldData(27)]))
                    vid.append(txn.AddVertex(software_lid, software_fids,
                                             [FieldData(3), FieldData("lop"), FieldData("java")]))
                    vid.append(txn.AddVertex(person_lid, person_fids,
                                             [FieldData(4), FieldData("josh"), FieldData(32)]))
                    vid.append(txn.AddVertex(software_lid,
                                             txn.GetVertexFieldIds(software_lid, ["id", "lang", "name"]),
                                             [FieldData(5), FieldData("c++"), FieldData("ripple")]))
                    vid.append(txn.AddVertex("person", {"id": 6, "age": 35, "name": "peter"}))
                    assert (vid == [0, 1, 2, 3, 4])
                    print("\nadd edges")
                    eids = []
                    eids.append((txn.AddEdge(vid[0], vid[1], "knows", ["weight"], ["0.5"])))
                    eids.append(txn.AddEdge(vid[0], vid[2], created_lid, [0], [FieldData(3)]))
                    eids.append(txn.AddEdge(vid[0], vid[3], "knows", {"weight": 0.78}))
                    eids.append(txn.AddEdge(vid[3], vid[2], "created", {"weight": 0.68}))
                    eids.append(txn.AddEdge(vid[3], vid[4], "created", {"weight": 0.68}))
                    eids.append(txn.AddEdge(vid[3], vid[2], "created", {"weight": 0.68}))
                    print(eids)
                    print(txn.DumpGraph())
                    it = txn.GetVertexIterator(0)
                    assert (it.IsValid())
                    ni = 0
                    eit = it.GetInEdgeIterator()
                    while eit.IsValid():
                        ni = ni + 1
                        eit.Next()
                    assert (ni == 0)
                    no = 0
                    eit = it.GetOutEdgeIterator()
                    while eit.IsValid():
                        no = no + 1
                        eit.Next()
                    assert (no == 3)
                    txn.Commit()

                with db.CreateWriteTxn() as txn:
                    # duplicate index
                    FMA_EXPECT_EXCEPTION("txn.AddVertex('person', {'id':2, 'name':'vad', 'age':22})", globals(),
                                         locals())
                    # missing required field
                    FMA_EXPECT_EXCEPTION("txn.AddVertex('person', {'id':11})", globals(), locals())

                print("\nget vertex/edge property")
                with db.CreateReadTxn() as txn:
                    assert (txn.GetVertexIterator(vid[1]).GetFields(['lang']) == ['java'])
                    assert (txn.GetVertexIterator(vid[1])['id'] == 3)
                    assert (math.isclose(txn.GetOutEdgeIterator(EdgeUid(vid[0], vid[2], 0, 0, 0), True)[0], 0.78,
                                         rel_tol=0.1))
                    assert (txn.GetVertexIterator(vid[0]).GetFields(["id", "name", "edge"]) == [2, 'vadas', None])
                    assert (math.isclose(txn.GetOutEdgeIterator(EdgeUid(vid[0], vid[1], 0, 0, 0), True)['weight'], 0.5))

                print("\nupdate vertex & edge (label include)")
                with db.CreateWriteTxn() as txn:
                    txn.GetVertexIterator()
                    vit = txn.GetVertexIterator(vid[4])
                    assert (txn.VertexToString(vit.GetId()).find("person") != -1)
                    vit.SetFields(["id", "name"], ["5", "scrapy"])
                    assert (vit['name'] == 'scrapy')
                    txn.Commit()

                print("\ndelete vertex & edge")
                with db.CreateWriteTxn() as txn:
                    print(txn.DumpGraph())
                    txn.GetVertexIterator(vid[3]).Delete()
                    assert (txn.GetVertexIterator(vid[3]).IsValid() == False)
                    eit = txn.GetOutEdgeIterator(EdgeUid(vid[0], vid[2], 0, 0, 0), True)
                    assert (eit.IsValid())
                    euid = eit.GetUid()
                    eit.Delete()
                    assert (txn.GetOutEdgeIterator(euid, False).IsValid() == False)
                    txn.Commit()

                print("\nUpsert Edge")
                with db.CreateWriteTxn() as txn:
                    print(txn.DumpGraph())
                    txn.UpsertEdge(0, 1, "knows", {"weight": 0.5})
                    txn.UpsertEdge(1, 0, "knows", {"weight": 0.5})
                    txn.Commit()

                print("\nGet/Set Field")
                with db.CreateWriteTxn() as txn:
                    print(txn.DumpGraph())
                    vit = txn.GetVertexIterator()
                    assert (vit.GetField('id') == 2)
                    assert (vit.GetField(0) == 2)
                    assert (vit.GetFields([0])[0] == 2)
                    assert (vit.GetAllFields()['id'] == 2)
                    vit.SetFields({'age': 27, 'id': 2, 'name': 'vadas'})
                    assert (vit.GetField(0) == 2)
                    vit.SetFields(['age', 'id', 'name'], [FieldData(27), FieldData(2), FieldData('vadas')])
                    assert (vit.GetField(0) == 2)
                    vit.SetFields([1, 0, 2], [FieldData(27), FieldData(2), FieldData('vadas')])
                    assert (vit.GetField(0) == 2)
                    assert (vit.GetNumInEdges()[0] == 1)
                    assert (vit.GetNumOutEdges()[0] == 1)
                    oit = vit.GetOutEdgeIterator()
                    assert (oit.IsValid())
                    assert (oit.GetField('weight') == 0.5)
                    assert (oit.GetField(0) == 0.5)
                    assert (oit.GetFields([0])[0] == 0.5)
                    assert (oit.GetFields(['weight'])[0] == 0.5)
                    assert (oit.GetAllFields()['weight'] == 0.5)
                    oit.SetField('weight', 0.5)
                    assert (oit.GetField('weight') == 0.5)
                    oit.SetFields(['weight'], ['0.5'])
                    assert (oit.GetField('weight') == 0.5)
                    oit.SetFields(['weight'], [FieldData(0.5)])
                    assert (oit.GetField('weight') == 0.5)
                    oit.SetFields({'weight': 0.5})
                    assert (oit.GetField('weight') == 0.5)
                    oit.SetFields([0], [FieldData(0.5)])
                    assert (oit.GetField('weight') == 0.5)
                    iit = vit.GetInEdgeIterator()
                    assert (iit.IsValid())
                    assert (iit.GetField('weight') == 0.5)
                    assert (iit.GetField(0) == 0.5)
                    assert (iit.GetField(0) == 0.5)
                    assert (iit.GetFields([0])[0] == 0.5)
                    assert (iit.GetFields(['weight'])[0] == 0.5)
                    assert (iit.GetAllFields()['weight'] == 0.5)
                    iit.SetField('weight', 0.5)
                    assert (iit.GetField('weight') == 0.5)
                    iit.SetFields(['weight'], ['0.5'])
                    assert (iit.GetField('weight') == 0.5)
                    iit.SetFields(['weight'], [FieldData(0.5)])
                    assert (iit.GetField('weight') == 0.5)
                    iit.SetFields({'weight': 0.5})
                    assert (iit.GetField('weight') == 0.5)
                    iit.SetFields([0], [FieldData(0.5)])
                    assert (iit.GetField('weight') == 0.5)
                    txn.Commit()

                print("\niterator")
                with db.CreateReadTxn() as txn:
                    print(txn.DumpGraph())
                    it = txn.GetVertexIterator(vid[2])
                    assert (it.GetLabel() == "person")
                    eit = it.GetOutEdgeIterator()
                    assert (it['name'] == 'josh')
                    assert (it.Next())
                    assert (it['name'] == 'scrapy')
                    it = txn.GetVertexIterator(vid[2])
                    eit = it.GetInEdgeIterator()
                    assert (eit.IsValid() == False)
                    it = txn.GetVertexIterator(vid[1])
                    eit = it.GetInEdgeIterator()
                    assert (eit.GetSrc() == 0)
                    assert (eit.GetDst() == 1)
                    assert (not eit.Next())

                print("\niterator write")
                with db.CreateWriteTxn() as txn:
                    it = txn.GetVertexIterator(vid[0])
                    eit = it.GetOutEdgeIterator()
                    it.SetField('age', 39)
                    assert (it['age'] == 39)
                    assert (math.isclose(eit['weight'], 0.5))
                    txn.Commit()

                print("\nindex lookup")
                with db.CreateReadTxn() as txn:
                    print(txn.DumpGraph())
                    it = txn.GetVertexIndexIterator("person", 'age', "32", "100")
                    assert (it.IsValid());
                    assert (it.GetIndexValue() == 32);
                    assert (it.GetVid() == 2);
                    assert (it.Next())
                    assert (it.GetIndexValue() == 35);
                    assert (it.GetVid() == 4);
                    it = txn.GetVertexByUniqueIndex("person", "id", 2)
                    assert (it.IsValid())
                    assert (it.GetId() == 0)

                print("\nlist out/in edges")
                with db.CreateReadTxn() as txn:
                    assert (txn.GetVertexIterator(1).ListSrcVids()[0] == [0])
                    assert (txn.GetVertexIterator(1).ListDstVids()[0] == [0])


if __name__ == '__main__':
    embedded = LGraphUnitTests()
    embedded.test_field_data()
    embedded.test_datetime()
    embedded.test_db()
