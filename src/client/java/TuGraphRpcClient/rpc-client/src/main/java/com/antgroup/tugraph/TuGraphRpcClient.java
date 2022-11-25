
package com.antgroup.tugraph;
import com.baidu.brpc.client.BrpcProxy;
import com.baidu.brpc.client.RpcClient;
import com.baidu.brpc.client.RpcClientOptions;
import com.baidu.brpc.client.loadbalance.LoadBalanceStrategy;
import com.baidu.brpc.protocol.Options;
import lombok.extern.slf4j.Slf4j;
import java.io.File;
import java.util.ArrayList;
import java.util.List;
import java.util.Base64;
import java.util.Collections;
import java.lang.StringBuilder;
import java.io.IOException;
import java.io.FileInputStream;
import java.io.BufferedInputStream;
import java.io.UnsupportedEncodingException;
import org.apache.commons.io.FileUtils;
import com.alibaba.fastjson.JSON;
import com.alibaba.fastjson.JSONObject;
import com.alibaba.fastjson.JSONArray;
import org.apache.commons.lang3.StringUtils;


import lgraph.Lgraph;

@SuppressWarnings("unchecked")
@Slf4j
public class TuGraphRpcClient {

    private RpcClient client;
    private TuGraphService tuGraphService;
    private String token;
    private String url;
    private long serverVersion;
    private static final int TIMEOUTINMS = 60 * 60 * 1000;

    public TuGraphRpcClient(String url, String user, String pass) {
        RpcClientOptions options = new RpcClientOptions();
        options.setProtocolType(Options.ProtocolType.PROTOCOL_BAIDU_STD_VALUE);
        options.setLoadBalanceType(LoadBalanceStrategy.LOAD_BALANCE_FAIR);
        options.setWriteTimeoutMillis(TIMEOUTINMS);
        options.setReadTimeoutMillis(TIMEOUTINMS);
        client = new RpcClient(url, options);
        tuGraphService = BrpcProxy.getProxy(client, TuGraphService.class);
        Lgraph.LoginRequest loginReq = Lgraph.LoginRequest.newBuilder().setUser(user).setPassword(pass).build();
        Lgraph.AuthRequest authReq = Lgraph.AuthRequest.newBuilder().setLogin(loginReq).build();
        Lgraph.AclRequest req = Lgraph.AclRequest.newBuilder().setAuthRequest(authReq).build();
        Lgraph.LGraphRequest request = Lgraph.LGraphRequest.newBuilder().setAclRequest(req).setToken("").setIsWriteOp(false).build();

        Lgraph.LGraphResponse response = tuGraphService.HandleRequest(request);
        if (response.getErrorCode().getNumber() != Lgraph.LGraphResponse.ErrorCode.SUCCESS_VALUE) {
            throw new TuGraphRpcException(response.getErrorCode(), response.getError(), "TuGraphRpcClient");
        }
        this.token = response.getAclResponse().getAuthResponse().getToken();
        this.serverVersion = response.getServerVersion();
        this.url = url;
    }

    public void stopClient() {
        client.stop();
    }

    private String handleCypherRequest(String query, String graph, double timeout) {
        Lgraph.CypherRequest cypherRequest = Lgraph.CypherRequest.newBuilder().setQuery(query).setResultInJsonFormat(true).setGraph(graph).setTimeout(timeout).build();
        Lgraph.LGraphRequest request = Lgraph.LGraphRequest.newBuilder().setCypherRequest(cypherRequest).setToken(this.token).setClientVersion(serverVersion).build();
        Lgraph.LGraphResponse response = tuGraphService.HandleRequest(request);
        if (response.getErrorCode().getNumber() != Lgraph.LGraphResponse.ErrorCode.SUCCESS_VALUE) {
            throw new TuGraphRpcException(response.getErrorCode(), response.getError(), "CallCypher");
        }
        serverVersion = response.getServerVersion() > serverVersion ? response.getServerVersion() : serverVersion;
        return response.getCypherResponse().getJsonResult();
    }

    // parse delimiter and process strings like \r \n \002 \0xA
    private String parseDelimiter(String delimiter) {
        int size = delimiter.length();
        StringBuilder sb = new StringBuilder();
        for (int idx = 0; idx < size; ++idx) {
            if (delimiter.charAt(idx) == '\\') {
                int begin = idx;
                ++idx;
                if (idx == size) {
                    throw new InputException("Illegal escape sequence, do you mean \\?");
                }
                char c = delimiter.charAt(idx);
                switch (c) {
                    case ('\\'): {
                        sb.append('\\');
                        break;
                    }
                    case ('a'): {
                        throw new InputException("unsupported delimiter in java language");
//                         sb.append('\a');
//                         break;
                    }
                    case ('f'): {
                        sb.append('\f');
                        break;
                    }
                    case ('n'): {
                        sb.append('\n');
                        break;
                    }
                    case ('r'): {
                        sb.append('\r');
                        break;
                    }
                    case ('t'): {
                        sb.append('\t');
                        break;
                    }
                    case ('v'): {
                        throw new InputException("unsupported delimiter in java language");
//                         sb.append('\v');
//                         break;
                    }
                    case ('x'): {
                        // \xnn hex numbers
                        ++idx;
                        int temp = 0;
                        for (int i = 0; i < 2; i++) {
                            if (idx == size) {
                                throw new InputException("Illegal escape sequence: " + delimiter.substring(begin, idx));
                            }
                            if (delimiter.charAt(idx) >= '0' && delimiter.charAt(idx) <= '9') {
                                temp = temp * 16 + delimiter.charAt(idx) - '0';
                            } else if (delimiter.charAt(idx) >= 'a' && delimiter.charAt(idx) <= 'f') {
                                temp = temp * 16 + (delimiter.charAt(idx) - 'a' + 10);
                            } else if (delimiter.charAt(idx) >= 'A' && delimiter.charAt(idx) <= 'F') {
                                temp = temp * 16 + (delimiter.charAt(idx) - 'A' + 10);
                            } else {
                                throw new InputException("Illegal escape sequence: " + delimiter.substring(begin, idx + 1));
                            }
                            ++idx;
                        }
                        sb.append((char)(temp + '0'));
                        break;
                    }
                    default: {
                        if (c >= '0' && c <= '9') {
                            // \nnn octal numbers
                            int temp = 0;
                            for (int i = 0; i < 3; i++) {
                                char sc = delimiter.charAt(idx);
                                if (idx == size) {
                                    throw new InputException("Illegal escape sequence: " + delimiter.substring(begin, idx));
                                }
                                if (sc < '0' || sc > '7') {
                                    throw new InputException("Illegal escape sequence: " + delimiter.substring(begin, idx + 1));
                                }
                                temp = temp * 8 + sc - '0';
                                ++idx;
                            }
                            if (temp >= 256) {
                                throw new InputException("Illegal escape sequence: " + delimiter.substring(begin, idx));
                            }
                            sb.append((char)(temp + '0'));
                            break;
                        } else {
                            throw new InputException("Illegal escape sequence: " + delimiter.substring(begin, idx + 1));
                        }
                    }
                }
            } else {
                sb.append(delimiter.charAt(idx));
                ++idx;
            }
        }
        return sb.toString();
    }

    private String textFileReader(String path) throws IOException {
        String result = null;
        File file = new File(path);
        if (!file.exists() || !file.isFile()) {
           throw new InputException("Illegal file: " + path);
        }
        try {
            result = FileUtils.readFileToString(file, "utf-8");
        } catch (IOException e) {
            throw e;
        }
        return result;
    }

    private byte[] binaryFileReader(String path) throws IOException {
         File file = new File(path);
         if (!file.exists() || !file.isFile()) {
            throw new InputException("Illegal file: " + path);
         }
         long fileSize = file.length();
         BufferedInputStream in = null;
         try {
            in = new BufferedInputStream(new FileInputStream(file), (int)fileSize);
            byte[] buf = new byte[(int)fileSize];
            int readBytes = in.read(buf, 0, (int)fileSize);
            if (readBytes != fileSize) {
                return null;
            }
            return buf;
         } catch (IOException e) {
            throw e;
         } finally {
            if (in != null) {
                try {
                    in.close();
                } catch(IOException e) {
                    throw e;
                }
            }
         }
    }

    private List<CsvDesc> parseConfigurationFile(JSONObject conf, boolean hasPath) {
        if (!conf.containsKey("files")) {
            return null;
        }
        List<CsvDesc> list = new ArrayList<CsvDesc>();
        JSONArray array = conf.getJSONArray("files");
        for (int idx = 0; idx < array.size(); ++idx) {
            JSONObject obj = (JSONObject)array.get(idx);
            if (!obj.containsKey("path") || !obj.containsKey("format") || !obj.containsKey("label")
                    || !obj.containsKey("columns")) {
                throw new InputException("Missing \"path\" or \"format\" or \"label\" or \"columns\" in json");
            }
            List<File> files = new ArrayList<File>();
            if (hasPath) {
                File path = new File(obj.getString("path"));
                if (!path.exists()) {
                    throw new InputException("Path " + obj.getString("path") + " does not exist in json {}");
                }
                if (path.isFile()) {
                    files.add(path);
                }
                if (path.isDirectory()) {
                    File[] temp = path.listFiles();
                    for (File f : temp) {
                        files.add(f);
                    }
                }
            } else {
                files.add(new File(""));
            }
            for (File file : files) {
                CsvDesc csv = new CsvDesc();
                if (!StringUtils.isBlank(file.getPath())) {
                    csv.setPath(file.getPath());
                    csv.setSize(file.length());
                }
                String format = obj.getString("format");
                if (!"JSON".equals(format) &&  !"CSV".equals(format)) {
                    throw new InputException("\"format\" value error : " + format + "should be CSV or JSON");
                }
                csv.setDataFormat(format);
                csv.setLabel(obj.getString("label"));
                if (obj.containsKey("header")) {
                    csv.setHeaderLine(obj.getIntValue("header"));
                }
                boolean isVertexFile = !(obj.containsKey("SRC_ID") || obj.containsKey("DST_ID"));
                csv.setFileType(isVertexFile);
                if (!isVertexFile) {
                    if (!obj.containsKey("SRC_ID") || !obj.containsKey("DST_ID")) {
                        throw new InputException("Missing \"SRC_ID\" or \"DST_ID\"");
                    }
                    csv.setEdgeSrcLabel(obj.getString("SRC_ID"));
                    csv.setEdgeDstLabel(obj.getString("DST_ID"));
                }

                JSONArray columns = obj.getJSONArray("columns");
                for (int i = 0; i < columns.size(); ++i) {
                    String column = (String)columns.get(i);
                    if ("".equals(column)) {
                        throw new InputException("Found empty filed in json " + file.getPath());
                    }
                    csv.addColumn(column);
                }
                list.add(csv);
            }
        }
        return list;
    }

    public String getUrl() {
        return url;
    }

    public String callCypher(String cypher, String graph, double timeout) {
        return handleCypherRequest(cypher, graph, timeout);
    }

    public boolean loadPlugin(String sourceFile, String pluginType, String pluginName, String codeType,
            String pluginDescription, boolean readOnly, String graph, double timeout) throws IOException {
        byte[] content = binaryFileReader(sourceFile);
        if (content == null) {
            return false;
        }
        String content64 = Base64.getEncoder().encodeToString(content);
        StringBuilder sb = new StringBuilder();
        sb.append("CALL db.plugin.loadPlugin('");
        sb.append(pluginType);
        sb.append("','");
        sb.append(pluginName);
        sb.append("','");
        sb.append(content64);
        sb.append("','");
        sb.append(codeType);
        sb.append("','");
        sb.append(pluginDescription);
        sb.append("',");
        sb.append(readOnly);
        sb.append(")");
        callCypher(sb.toString(), graph, timeout);
        return true;
    }

    public String callPlugin(String pluginType, String pluginName, String param, double pluginTimeOut,
            boolean inProcess, String graph, double timeout) {
        StringBuilder sb = new StringBuilder();
        sb.append("CALL db.plugin.callPlugin('");
        sb.append(pluginType);
        sb.append("','");
        sb.append(pluginName);
        sb.append("','");
        sb.append(param);
        sb.append("',");
        sb.append(pluginTimeOut);
        sb.append(",");
        sb.append(inProcess);
        sb.append(")");
        return callCypher(sb.toString(), graph, timeout);
    }

    public boolean importSchemaFromContent(String schema, String graph, double timeout) throws UnsupportedEncodingException {
        byte[] textByte = new byte[0];
        try {
            textByte = schema.getBytes("UTF-8");
        } catch (UnsupportedEncodingException e) {
            throw e;
        }
        String schema64 = Base64.getEncoder().encodeToString(textByte);
        StringBuilder sb = new StringBuilder();
        sb.append("CALL db.importor.schemaImportor('");
        sb.append(schema64);
        sb.append("')");
        String res = callCypher(sb.toString(), graph, timeout);
        if (!"null".equals(res)) {
            throw new InputException(res);
        }
        // TODO
//         if (!StringUtils.isBlank(res)) {
//             throw new InputException(res);
//         }
        return true;
    }

    public boolean importDataFromContent(String desc, String data, String delimiter,
            boolean continueOnError, int threadNums, String graph, double timeout) throws UnsupportedEncodingException {
        byte[] textByteDesc = new byte[0];
        byte[] textByteData = new byte[0];
        try {
            textByteDesc = desc.getBytes("UTF-8");
            textByteData = data.getBytes("UTF-8");
        } catch (UnsupportedEncodingException e) {
            throw e;
        }
        String desc64 = Base64.getEncoder().encodeToString(textByteDesc);
        String data64 = Base64.getEncoder().encodeToString(textByteData);
        StringBuilder sb = new StringBuilder();
        sb.append("CALL db.importor.dataImportor('");
        sb.append(desc64);
        sb.append("','");
        sb.append(data64);
        sb.append("',");
        sb.append(continueOnError);
        sb.append(",");
        sb.append(threadNums);
        sb.append(",'");
        sb.append(parseDelimiter(delimiter));
        sb.append("')");
        String res = callCypher(sb.toString(), graph, timeout);
        if (!"null".equals(res)) {
            throw new InputException(res);
        }
        // TODO
//         if (!StringUtils.isBlank(res)) {
//             throw new InputException(res);
//         }
        return true;
    }

    public boolean importSchemaFromFile(String schemaFile, String graph, double timeout) throws UnsupportedEncodingException, IOException {
        String content = textFileReader(schemaFile);
        if ("".equals(content)) {
            throw new InputException("Illegal schema_file : " + schemaFile);
        }
        JSONObject jsonObject = JSON.parseObject(content);
        if (jsonObject.isEmpty() || !jsonObject.containsKey("schema")) {
            throw new InputException("Illegal schema_file : " + schemaFile);
        }
        JSONObject schema = new JSONObject();
        schema.put("schema", jsonObject.getJSONArray("schema"));
        byte[] textByte = new byte[0];
        try {
            textByte = schema.toJSONString().getBytes("UTF-8");
        } catch (UnsupportedEncodingException e) {
            throw e;
        }
        String schema64 = Base64.getEncoder().encodeToString(textByte);
        StringBuilder sb = new StringBuilder();
        sb.append("CALL db.importor.schemaImportor('");
        sb.append(schema64);
        sb.append("')");
        String res = callCypher(sb.toString(), graph, timeout);
        if (!"null".equals(res)) {
            throw new InputException(res);
        }
        // TODO
//         if (!StringUtils.isBlank(res)) {
//             throw new InputException(res);
//         }
        return true;
    }

    public boolean importDataFromFile(String confFile, String delimiter, boolean continueOnError, int threadNums,
            int skipPackages, String graph, double timeout) throws IOException, UnsupportedEncodingException {
        String content = textFileReader(confFile);
        if ("".equals(content)) {
            throw new InputException("Illegal conf_file : " + confFile);
        }
        JSONObject jsonObject = JSON.parseObject(content);
        if (jsonObject.isEmpty()) {
            throw new InputException("Illegal conf_file : " + confFile);
        }
        List<CsvDesc> files = parseConfigurationFile(jsonObject, true);
        if (files.isEmpty()) {
            return true;
        }
        Collections.sort(files);
        long bytesTotal = 0;
        for (CsvDesc cd : files) {
            bytesTotal += cd.getSize();
        }

        for (CsvDesc cd : files) {
            byte[] desc = cd.dump(false);
            boolean isFirstPackage = true;
            FileCutter cutter = new FileCutter(cd.getPath());
            byte[] buf = null;
            for (buf = cutter.cut(); buf != null; isFirstPackage = false) {
                if (skipPackages > 0) {
                    --skipPackages;
                    continue;
                }
                if (isFirstPackage) {
                    int lines = cutter.lineCount(buf);
                    if (lines < cd.getHeaderLine()) {
                        throw new InputException("HEADER too large");
                    }
                } else {
                    cd.setHeaderLine(0);
                    desc = cd.dump(false);
                }
                String desc64 = Base64.getEncoder().encodeToString(desc);
                String content64 = Base64.getEncoder().encodeToString(buf);
                StringBuilder sb = new StringBuilder();
                sb.append("CALL db.importor.dataImportor('");
                sb.append(desc64);
                sb.append("','");
                sb.append(content64);
                sb.append("',");
                sb.append(continueOnError);
                sb.append(",");
                sb.append(threadNums);
                sb.append(",'");
                sb.append(parseDelimiter(delimiter));
                sb.append("')");
                String res = callCypher(sb.toString(), graph, timeout);
                if (!"null".equals(res)) {
                    throw new InputException(res);
                }
                // TODO
        //         if (!StringUtils.isBlank(res)) {
        //             throw new InputException(res);
        //         }
                buf = cutter.cut();
            }
        }
        return true;
    }

    // When the scope of the client object ends, the token is automatically deleted, making the token invalid.
    public void logout() {
        Lgraph.LogoutRequest logoutReq = Lgraph.LogoutRequest.newBuilder().setToken(this.token).build();
        Lgraph.AuthRequest authReq = Lgraph.AuthRequest.newBuilder().setLogout(logoutReq).build();
        Lgraph.AclRequest req = Lgraph.AclRequest.newBuilder().setAuthRequest(authReq).build();
        Lgraph.LGraphRequest request = Lgraph.LGraphRequest.newBuilder().setAclRequest(req).setToken(this.token).setIsWriteOp(false).build();

        Lgraph.LGraphResponse response = tuGraphService.HandleRequest(request);
        if (response.getErrorCode().getNumber() != Lgraph.LGraphResponse.ErrorCode.SUCCESS_VALUE) {
            throw new TuGraphRpcException(response.getErrorCode(), response.getError(), "TuGraphRpcClient");
        }
    }

    protected void finalize() throws Throwable {
        try {
            logout();
        } catch (Exception e) {
            System.out.print(e.getClass().getName());
        }
    }
}
