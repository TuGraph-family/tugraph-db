package com.antgroup.tugraph;
import java.io.IOException;
import java.io.FileNotFoundException;
import java.io.FileInputStream;
import java.util.Arrays;

public class FileCutter {
    private FileInputStream stream;
    private long readBytes;

    static final int ONLINE_IMPORT_LIMIT_SOFT = 16 << 20;
    static final int ONLINE_IMPORT_LIMIT_HARD = 17 << 20;

    public FileCutter(String filename) throws FileNotFoundException {
        readBytes = 0;
        try {
            stream = new FileInputStream(filename);
        } catch (FileNotFoundException e) {
            throw e;
        }
    }

    public byte[] cut() throws IOException {
        byte[] buf = new byte[ONLINE_IMPORT_LIMIT_HARD];
        int bytes = 0;
        try {
            bytes = stream.read(buf, 0, ONLINE_IMPORT_LIMIT_SOFT);
        } catch (IOException e) {
            throw e;
        }
        if (bytes == -1) {
            return null;
        }
        readBytes += bytes;
        while (buf[bytes - 1] != '\n') {
            try {
                stream.read(buf, bytes, 1);
            } catch (IOException e) {
               throw e;
            }
            ++bytes;
            ++readBytes;
        }
        if (bytes >= ONLINE_IMPORT_LIMIT_HARD) {
            throw new InputException("too long input line");
        }
        return Arrays.copyOf(buf, bytes);
    }

    int lineCount(byte[] buf) {
        int count = 0;
        for (int idx = 0; idx < readBytes; ++idx) {
            if (buf[idx] == '\n') {
                ++count;
            }
        }
        return count;
    }
}
