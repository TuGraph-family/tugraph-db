public class ScoreEdgeUid {
    public float score;
    public long srcId;
    public long destId;
    public int labelId;
    public int edgeId;
    public String toString() {
        return "srcId: " + this.srcId +
                ", destId:" + this.destId +
                ", labelId:" + this.labelId +
                ", edgeId:" + this.edgeId +
                ", score:" + this.score;
    }
}
