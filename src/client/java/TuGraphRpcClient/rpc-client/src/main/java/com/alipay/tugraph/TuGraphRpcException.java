package com.alipay.tugraph;
import lgraph.Lgraph;

public class TuGraphRpcException extends RuntimeException {
    private static final long serialVersionUID = 7382581401295703844L;
    public Lgraph.LGraphResponse.ErrorCode errorCode;
    public String error;
    public String errorMethod;
    
    public TuGraphRpcException(Lgraph.LGraphResponse.ErrorCode errorCode, String error, String errorMethod) {
        super(error);
        this.errorCode = errorCode;
        this.error = error;
        this.errorMethod = errorMethod;
    }
    
    public Lgraph.LGraphResponse.ErrorCode GetErrorCode() {
        return errorCode;
    }
    
    public String GetErrorCodeName() {
        return errorCode.name();
    }
    
    public String GetError() {
        return error;
    }
    
    public String GetErrorMethod() {
        return errorMethod;
    }
}
