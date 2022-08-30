import org.apache.lucene.analysis.Analyzer;
import org.apache.lucene.analysis.cn.smart.SmartChineseAnalyzer;
import org.apache.lucene.analysis.standard.StandardAnalyzer;
import org.apache.lucene.document.*;
import org.apache.lucene.index.*;
import org.apache.lucene.queryparser.classic.ParseException;
import org.apache.lucene.queryparser.classic.QueryParser;
import org.apache.lucene.search.*;
import org.apache.lucene.store.Directory;
import org.apache.lucene.store.FSDirectory;
import org.apache.commons.io.FileUtils;

import java.io.File;
import java.io.IOException;
import java.nio.file.Paths;
import java.util.Collection;
import java.util.concurrent.*;


public class Lucene {
    private final IndexWriter writer;
    private final SearcherManager searcherManager;
    private final QueryParser parser;
    private final ScheduledExecutorService scheduledExecutor;
    private Future commitFuture = null;
    private Future maybeRefreshFuture = null;
    private final String indexPath;

    public Lucene(String indexPath, String analyzerName, int commitInterval, int refreshInterval) throws IOException {
        this.indexPath = indexPath;
        Directory dir = FSDirectory.open(Paths.get(indexPath));
        Analyzer analyzer = null;
        if (analyzerName.equals("StandardAnalyzer")) {
            analyzer = new StandardAnalyzer();
        } else if (analyzerName.equals("SmartChineseAnalyzer")) {
            analyzer = new SmartChineseAnalyzer();
        } else {
            throw new IllegalArgumentException("Unknown analyzer : " + analyzerName);
        }
        IndexWriterConfig iwc = new IndexWriterConfig(analyzer);
        iwc.setIndexDeletionPolicy(new SnapshotDeletionPolicy(new KeepOnlyLastCommitDeletionPolicy()));
        iwc.setRAMBufferSizeMB(256.0);
        writer = new IndexWriter(dir, iwc);
        searcherManager = new SearcherManager(writer,null);
        scheduledExecutor = Executors.newScheduledThreadPool(3);
        if (commitInterval > 0) {
            commitFuture = scheduledExecutor.scheduleWithFixedDelay(() -> {
                try {
                    writer.commit();
                } catch (IOException e) {
                    e.printStackTrace();
                }
            }, commitInterval, commitInterval, TimeUnit.SECONDS);
        }
        if (refreshInterval > 0) {
            maybeRefreshFuture = scheduledExecutor.scheduleWithFixedDelay(() -> {
                try {
                    searcherManager.maybeRefresh();
                } catch (IOException e) {
                    e.printStackTrace();
                }
            }, 0, refreshInterval, TimeUnit.SECONDS);
        }
        parser = new QueryParser("labelId", analyzer);
    }

    public void backup(String backupDir) throws IOException {
        IndexWriterConfig config = (IndexWriterConfig)writer.getConfig();
        SnapshotDeletionPolicy snapshotDeletionPolicy = (SnapshotDeletionPolicy)config.getIndexDeletionPolicy();
        IndexCommit snapshot = null;
        try {
            snapshot = snapshotDeletionPolicy.snapshot();
            Collection<String> fileNames = snapshot.getFileNames();
            for (String fileName : fileNames) {
                File from = new File(indexPath + File.separator + fileName);
                File to = new File(backupDir + File.separator + fileName);
                FileUtils.copyFile(from, to);
            }
        } finally {
            if (snapshot != null) {
                snapshotDeletionPolicy.release(snapshot);
            }
            writer.deleteUnusedFiles();
        }
    }

    public void clear() throws IOException {
        writer.deleteAll();
        writer.commit();
    }
    public void close() throws IOException {
        if (maybeRefreshFuture != null) {
            maybeRefreshFuture.cancel(false);
        }
        searcherManager.close();
        if (commitFuture != null) {
            commitFuture.cancel(false);
        }
        writer.close();
        scheduledExecutor.shutdown();
    }

    public void maybeRefresh() throws IOException {
        searcherManager.maybeRefresh();
    }

    public void addVertex(long id, int labelId, String[] fieldName, String[] fieldvalue) throws IOException {
        if (fieldName.length != fieldvalue.length) {
            throw new IOException("fieldName and fieldvalue size mismatch");
        }
        Document doc = new Document();
        doc.add(new LongPoint("id", id));
        doc.add(new StoredField("id", id));
        doc.add(new IntPoint("labelId", labelId));
        doc.add(new StringField("type", "v", Field.Store.YES));
        for (int i = 0; i < fieldName.length; i++) {
            doc.add(new TextField(fieldName[i], fieldvalue[i], Field.Store.NO));
        }
        writer.addDocument(doc);
    }

    public void deleteVertex(long id) throws IOException {
        BooleanQuery.Builder builder = new BooleanQuery.Builder();
        builder.add(LongPoint.newExactQuery("id", id), BooleanClause.Occur.MUST);
        writer.deleteDocuments(builder.build());
    }

    public void deleteLabel(boolean is_vertex, int labelId) throws IOException {
        BooleanQuery.Builder builder = new BooleanQuery.Builder();
        builder.add(IntPoint.newExactQuery("labelId", labelId), BooleanClause.Occur.MUST);
        builder.add(new TermQuery(new Term("type", is_vertex ? "v" : "e")), BooleanClause.Occur.MUST);
        writer.deleteDocuments(builder.build());
    }

    public void addEdge(long srcId, long destId, int labelId, int edgeId, String[] fieldName, String[] fieldvalue) throws IOException {
        if (fieldName.length != fieldvalue.length) {
            throw new IOException("fieldName and fieldvalue size mismatch");
        }
        Document doc = new Document();
        doc.add(new LongPoint("srcId", srcId));
        doc.add(new LongPoint("destId", destId));
        doc.add(new IntPoint("labelId", labelId));
        doc.add(new IntPoint("edgeId", edgeId));

        doc.add(new StoredField("srcId", srcId));
        doc.add(new StoredField("destId", destId));
        doc.add(new StoredField("labelId", labelId));
        doc.add(new StoredField("edgeId", edgeId));
        doc.add(new StringField("type", "e", Field.Store.YES));
        for (int i = 0; i < fieldName.length; i++) {
            doc.add(new TextField(fieldName[i], fieldvalue[i], Field.Store.NO));
        }
        writer.addDocument(doc);
    }

    public void deleteEdge(long srcId, long destId, int labelId, int edgeId) throws IOException {
        BooleanQuery.Builder builder = new BooleanQuery.Builder();
        builder.add(LongPoint.newExactQuery("srcId", srcId), BooleanClause.Occur.MUST);
        builder.add(LongPoint.newExactQuery("destId", destId), BooleanClause.Occur.MUST);
        builder.add(IntPoint.newExactQuery("labelId", labelId), BooleanClause.Occur.MUST);
        builder.add(IntPoint.newExactQuery("edgeId", edgeId), BooleanClause.Occur.MUST);
        writer.deleteDocuments(builder.build());
    }

    public void commit() throws IOException {
        writer.commit();
    }

    public ScoreVid[] queryVertex(int labelId, String query, int topN) throws IOException, ParseException {
        IndexSearcher searcher = null;
        ScoreVid[] ids = null;
        try {
            BooleanQuery booleanQuery = new BooleanQuery.Builder()
                    .add(parser.parse(query), BooleanClause.Occur.MUST)
                    .add(IntPoint.newExactQuery("labelId", labelId), BooleanClause.Occur.MUST)
                    .add(new TermQuery(new Term("type",  "v")), BooleanClause.Occur.MUST)
                    .build();
            searcher = searcherManager.acquire();
            TopDocs results = searcher.search(booleanQuery, topN);
            ids = new ScoreVid[results.scoreDocs.length];
            for (int i = 0; i < results.scoreDocs.length; i++) {
                ids[i] = new ScoreVid();
                ids[i].vid = searcher.doc(results.scoreDocs[i].doc).getField("id").numericValue().longValue();
                ids[i].score = results.scoreDocs[i].score;
            }
        } finally {
            if (searcher != null) {
                searcherManager.release(searcher);
            }
        }
        return ids;
    }

    public ScoreEdgeUid[] queryEdge(int labelId, String query, int topN) throws IOException, ParseException {
        IndexSearcher searcher = null;
        ScoreEdgeUid[] ids = null;
        try {
            searcher = searcherManager.acquire();
            BooleanQuery booleanQuery = new BooleanQuery.Builder()
                    .add(parser.parse(query), BooleanClause.Occur.MUST)
                    .add(IntPoint.newExactQuery("labelId", labelId), BooleanClause.Occur.MUST)
                    .add(new TermQuery(new Term("type",  "e")), BooleanClause.Occur.MUST)
                    .build();
            TopDocs results = searcher.search(booleanQuery, topN);
            ids = new ScoreEdgeUid[results.scoreDocs.length];
            for (int i = 0; i < results.scoreDocs.length; i++) {
                Document doc = searcher.doc(results.scoreDocs[i].doc);
                ids[i] = new ScoreEdgeUid();
                ids[i].srcId = doc.getField("srcId").numericValue().longValue();
                ids[i].destId = doc.getField("destId").numericValue().longValue();
                ids[i].labelId = doc.getField("labelId").numericValue().intValue();
                ids[i].edgeId = doc.getField("edgeId").numericValue().intValue();
                ids[i].score = results.scoreDocs[i].score;
            }
        } finally {
            if (searcher != null) {
                searcherManager.release(searcher);
            }
        }
        return ids;
    }

    public static void main(String[] args) throws InterruptedException, IOException, ParseException {
        try {
            Lucene t = new Lucene("test","StandardAnalyzer", 10, 1);
            t.clear();
            for (int i = 0; i < 10; i++) {
                String[] keys = {"fieldA"};
                String[] values = {"haha fieldA_" + i};
                t.addVertex(i, 10, keys, values);
            }
            t.maybeRefresh();
            ScoreVid[] ids = t.queryVertex(10, "fieldA:fieldA_5", 100);
            for (ScoreVid id: ids) {
                System.out.println("1 id : " + id.toString());
            }
            t.deleteVertex(5);
            t.maybeRefresh();
            ids = t.queryVertex(10, "fieldA:fieldA_5", 100);
            for (ScoreVid id: ids) {
                System.out.println("2 id : " + id.toString());
            }

            t.backup("test_back");
            Lucene t_back = new Lucene("test_back","StandardAnalyzer",10, 1);
            ids = t.queryVertex(10, "fieldA:haha", 100);
            System.out.println(ids.length);

        } catch (Exception  e) {
            e.printStackTrace();
        }
    }
}