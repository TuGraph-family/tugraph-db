package com.antgroup.tugraph;
import com.baidu.brpc.protocol.BrpcMeta;

import lgraph.Lgraph;

public interface TuGraphService {
    /**
     * brpc/sofa：
     * serviceName默认是包名 + 类名，methodName是proto文件Service内对应方法名，
     * hulu/public_pbrpc：
     * serviceName默认是类名，不需要加包名，methodName是proto文件Service内对应方法index，默认从0开始。
     */
    @BrpcMeta(serviceName = "lgraph.LGraphRPCService", methodName = "HandleRequest")
    Lgraph.LGraphResponse HandleRequest(Lgraph.LGraphRequest request);
}
