import org.apache.lucene.queryparser.classic.ParseException;
import org.junit.After;
import org.junit.Assert;
import org.junit.Before;
import org.junit.Test;

import java.io.IOException;

public class LuceneTest {
    private Lucene lucene;

    @Before
    public void init() throws IOException {
        lucene = new Lucene("test","StandardAnalyzer", 10, 1);
        lucene.clear();
    }

    @After
    public void close() throws IOException {
        lucene.close();
    }

    @Test
    public void testAddVertex() throws IOException, ParseException {
        int labelId = 10;
        for (int i = 0; i < 10; i++) {
            String[] keys = {"field_1"};
            String[] values = {"field_1_" + i};
            lucene.addVertex(i, 10, keys, values);
        }
        lucene.maybeRefresh();
        ScoreVid[] ids = lucene.queryVertex(labelId, "field_1:field_1_5", 100);
        Assert.assertTrue(ids.length == 1);
        Assert.assertTrue(ids[0].vid == 5);

        ids = lucene.queryVertex(labelId, "field_1_5", 100);
        Assert.assertTrue(ids.length == 0);
    }

    @Test
    public void testDeleteVertex() throws IOException, ParseException {
        int labelId1 = 10;
        for (int i = 0; i < 10; i++) {
            String[] keys = {"field_1"};
            String[] values = {"field_1_" + i};
            lucene.addVertex(i, labelId1, keys, values);
        }
        lucene.deleteVertex(5);
        lucene.maybeRefresh();
        ScoreVid[] ids = lucene.queryVertex(labelId1, "field_1:field_1_5", 100);
        Assert.assertTrue(ids.length == 0);

        lucene.deleteVertex(20);
        lucene.maybeRefresh();
        ids = lucene.queryVertex(labelId1, "field_1:field_1_*", 100);
        Assert.assertTrue(ids.length == 9);

        int labelId2 = 20;
        for (int i = 10; i < 20; i++) {
            String[] keys = {"field_2"};
            String[] values = {"field_2_" + i};
            lucene.addVertex(i, labelId2, keys, values);
        }
        lucene.maybeRefresh();
        ids = lucene.queryVertex(labelId2, "field_2:field_*", 100);
        Assert.assertTrue(ids.length == 10);
        ids = lucene.queryVertex(labelId1, "field_1:field_*", 100);
        Assert.assertTrue(ids.length == 9);

        lucene.deleteLabel(true, labelId1);
        lucene.maybeRefresh();
        ids = lucene.queryVertex(labelId2, "field_2:field_*", 100);
        Assert.assertTrue(ids.length == 10);
        ids = lucene.queryVertex(labelId1, "field_1:field_*", 100);
        Assert.assertTrue(ids.length == 0);
    }

    @Test
    public void testAddEdge() throws IOException, ParseException {
        int labelId = 11;
        for (int i = 0; i < 10; i++) {
            String[] keys = {"field_1"};
            String[] values = {"field_1_" + i};
            lucene.addEdge(i, i+1, labelId, i, keys, values);
        }
        lucene.maybeRefresh();
        ScoreEdgeUid[] ids = lucene.queryEdge(labelId, "field_1:field_1_5", 100);
        Assert.assertTrue(ids.length == 1);
        Assert.assertTrue(ids[0].srcId == 5);
        Assert.assertTrue(ids[0].destId == 6);
        Assert.assertTrue(ids[0].labelId == labelId);
        Assert.assertTrue(ids[0].edgeId == 5);
    }

    @Test
    public void testDeleteEdge() throws IOException, ParseException {
        int labelId = 11;
        for (int i = 0; i < 10; i++) {
            String[] keys = {"field_1"};
            String[] values = {"field_1_" + i};
            lucene.addEdge(i, i+1, labelId, i, keys, values);
        }
        lucene.deleteEdge(5, 6, labelId, 5);
        lucene.maybeRefresh();
        ScoreEdgeUid[] ids = lucene.queryEdge(labelId, "field_1:field_1_5", 100);
        Assert.assertTrue(ids.length == 0);
    }
}