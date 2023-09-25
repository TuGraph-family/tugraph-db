import org.apache.lucene.queryparser.classic.ParseException;

import java.io.IOException;

public class LuceneTest {

    private static class InnerLucene extends Lucene{
        public InnerLucene() throws IOException {
            super("test", "StandardAnalyzer", 10, 1);
            super.clear();
        }

        @Override
        protected void finalize() throws Throwable {
            super.close();
        }
    }

    public static void testAddVertex() throws IOException, ParseException {
        InnerLucene lucene = new InnerLucene();
        int labelId = 10;
        for (int i = 0; i < 10; i++) {
            String[] keys = {"field_1"};
            String[] values = {"field_1_" + i};
            lucene.addVertex(i, 10, keys, values);
        }
        lucene.maybeRefresh();
        ScoreVid[] ids = lucene.queryVertex(labelId, "field_1:field_1_5", 100);
        assert ids.length == 1;
        assert ids[0].vid == 5;

        ids = lucene.queryVertex(labelId, "field_1_5", 100);
        assert ids.length == 0;
    }

    public static void testDeleteVertex() throws IOException, ParseException {
        InnerLucene lucene = new InnerLucene();
        int labelId1 = 10;
        for (int i = 0; i < 10; i++) {
            String[] keys = {"field_1"};
            String[] values = {"field_1_" + i};
            lucene.addVertex(i, labelId1, keys, values);
        }
        lucene.deleteVertex(5);
        lucene.maybeRefresh();
        ScoreVid[] ids = lucene.queryVertex(labelId1, "field_1:field_1_5", 100);
        assert ids.length == 0;

        lucene.deleteVertex(20);
        lucene.maybeRefresh();
        ids = lucene.queryVertex(labelId1, "field_1:field_1_*", 100);
        assert ids.length == 9;

        int labelId2 = 20;
        for (int i = 10; i < 20; i++) {
            String[] keys = {"field_2"};
            String[] values = {"field_2_" + i};
            lucene.addVertex(i, labelId2, keys, values);
        }
        lucene.maybeRefresh();
        ids = lucene.queryVertex(labelId2, "field_2:field_*", 100);
        assert ids.length == 10;
        ids = lucene.queryVertex(labelId1, "field_1:field_*", 100);
        assert ids.length == 9;

        lucene.deleteLabel(true, labelId1);
        lucene.maybeRefresh();
        ids = lucene.queryVertex(labelId2, "field_2:field_*", 100);
        assert ids.length == 10;
        ids = lucene.queryVertex(labelId1, "field_1:field_*", 100);
        assert ids.length == 0;
    }

    public static void testAddEdge() throws IOException, ParseException {
        InnerLucene lucene = new InnerLucene();
        int labelId = 11;
        for (int i = 0; i < 10; i++) {
            String[] keys = {"field_1"};
            String[] values = {"field_1_" + i};
            lucene.addEdge(i, i+1, labelId, i, keys, values);
        }
        lucene.maybeRefresh();
        ScoreEdgeUid[] ids = lucene.queryEdge(labelId, "field_1:field_1_5", 100);
        assert ids.length == 1;
        assert ids[0].srcId == 5;
        assert ids[0].destId == 6;
        assert ids[0].labelId == labelId;
        assert ids[0].edgeId == 5;
    }

    public static void testDeleteEdge() throws IOException, ParseException {
        InnerLucene lucene = new InnerLucene();
        int labelId = 11;
        for (int i = 0; i < 10; i++) {
            String[] keys = {"field_1"};
            String[] values = {"field_1_" + i};
            lucene.addEdge(i, i+1, labelId, i, keys, values);
        }
        lucene.deleteEdge(5, 6, labelId, 5);
        lucene.maybeRefresh();
        ScoreEdgeUid[] ids = lucene.queryEdge(labelId, "field_1:field_1_5", 100);
        assert ids.length == 0;
    }

    public static void main(String[] args) throws IOException, ParseException {
        testAddVertex();
        testDeleteVertex();
        testAddEdge();
        testDeleteEdge();
    }
}