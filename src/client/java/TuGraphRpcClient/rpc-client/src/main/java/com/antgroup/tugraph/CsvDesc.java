package com.antgroup.tugraph;
import java.io.UnsupportedEncodingException;
import java.util.ArrayList;
import java.util.List;
import com.alibaba.fastjson.JSONObject;
import com.alibaba.fastjson.JSONArray;



public class CsvDesc implements Comparable<CsvDesc> {
    private String path;
    private long size;
    private String dataFormat;
    private String label;
    private int nHeaderLine;
    private boolean isVertexFile;
    private String edgeSrcLabel;
    private String edgeDstLabel;
    private List<String> columns;

    public CsvDesc() {
        columns = new ArrayList<String>();
    }

    public String getPath() {
        return path;
    }

    public void setPath(String path) {
        this.path = path;
    }

    public void setSize(long size) {
        this.size = size;
    }

    public long getSize() {
        return size;
    }

    public void setDataFormat(String dataFormat) {
        this.dataFormat = dataFormat;
    }

    public void setLabel(String label) {
        this.label = label;
    }

    public int getHeaderLine() {
        return nHeaderLine;
    }

    public void setHeaderLine(int nHeaderLine) {
        this.nHeaderLine = nHeaderLine;
    }

    public void setFileType(boolean isVertexFile) {
        this.isVertexFile = isVertexFile;
    }

    public void setEdgeSrcLabel(String edgeSrcLabel) {
        this.edgeSrcLabel = edgeSrcLabel;
    }

    public void setEdgeDstLabel(String edgeDstLabel) {
        this.edgeDstLabel = edgeDstLabel;
    }

    public void addColumn(String column) {
        this.columns.add(column);
    }

    public int compareTo(CsvDesc csv) {
        if (isVertexFile) {
            if (csv.isVertexFile) {
                return 0;
            }
            if (!csv.isVertexFile) {
                return -1;
            }
        } else {
            if (csv.isVertexFile) {
                return 1;
            }
            if (!csv.isVertexFile) {
                return 0;
            }
        }
        return 0;
    }

    public byte[] dump(boolean hasPath) throws UnsupportedEncodingException {
        JSONObject jsonObject = new JSONObject();
        if (hasPath) {
            jsonObject.put("path", path);
        }
        jsonObject.put("header", nHeaderLine);
        jsonObject.put("format", dataFormat);
        jsonObject.put("label", label);
        JSONArray tmp = new JSONArray();
        for (String column : columns) {
            tmp.add(column);
        }
        jsonObject.put("columns", tmp);
        if (!isVertexFile) {
            jsonObject.put("SRC_ID", edgeSrcLabel);
            jsonObject.put("DST_ID", edgeDstLabel);
        }
        JSONArray array = new JSONArray();
        array.add(jsonObject);
        JSONObject obj = new JSONObject();
        obj.put("files", array);
        String desc = obj.toJSONString();
        byte[] textByteDesc = new byte[0];
        try {
            textByteDesc = desc.getBytes("UTF-8");
        } catch (UnsupportedEncodingException e) {
            throw e;
        }
        return textByteDesc;
    }
}
