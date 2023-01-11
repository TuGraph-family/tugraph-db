/**
 * Copyright 2022 AntGroup CO., Ltd.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 */

#if LGRAPH_ENABLE_FULLTEXT_INDEX

#include "core/full_text_index.h"

namespace lgraph {

JavaVM *LuceneJNIEnv::vm = nullptr;
std::once_flag LuceneJNIEnv::once_flag;

// Each thread must have its own jni environment, otherwise coredump
thread_local LuceneJNIEnv FullTextIndex::jni_env_;

FullTextIndex::FullTextIndex(const std::string &path, const std::string &analyzer,
                             int commit_interval, int refresh_interval)
    : path_(path), commit_interval_(commit_interval), refresh_interval_(refresh_interval) {
    jstring index_path = jni_env_.env->NewStringUTF(path_.c_str());
    jni_env_.CheckException("NewStringUTF");
    jstring analyzer_name = jni_env_.env->NewStringUTF(analyzer.c_str());
    jni_env_.CheckException("NewStringUTF");
    lucene_ = jni_env_.env->NewObject(jni_env_.luceneClass, jni_env_.lucene_constructor, index_path,
                                      analyzer_name, commit_interval, refresh_interval);
    jni_env_.CheckException("lucene_constructor");
}

FullTextIndex::~FullTextIndex() {
    jni_env_.env->CallVoidMethod(lucene_, jni_env_.close);
    jni_env_.CheckException("close");
}

void FullTextIndex::Clear() {
    jni_env_.env->CallVoidMethod(lucene_, jni_env_.clear);
    jni_env_.CheckException("clear");
}

void FullTextIndex::Refresh() {
    jni_env_.env->CallVoidMethod(lucene_, jni_env_.refresh);
    jni_env_.CheckException("refresh");
}

void FullTextIndex::Commit() {
    jni_env_.env->CallVoidMethod(lucene_, jni_env_.commit);
    jni_env_.CheckException("commit");
}

void FullTextIndex::Backup(const std::string &backup_dir) {
    jstring path = jni_env_.env->NewStringUTF(backup_dir.c_str());
    jni_env_.CheckException("NewStringUTF");
    jni_env_.env->CallVoidMethod(lucene_, jni_env_.backup, path);
    jni_env_.CheckException("backup");
}

void FullTextIndex::DeleteLabel(bool is_vertex, LabelId labelId) {
    jni_env_.env->CallVoidMethod(lucene_, jni_env_.delete_label, is_vertex, labelId);
    jni_env_.CheckException("delete_label");
}

void FullTextIndex::AddVertex(int64_t vid, LabelId labelId,
                              const std::vector<std::pair<std::string, std::string>> &kvs) {
    if (kvs.empty()) {
        return;
    }
    int len = kvs.size();
    jobjectArray jobjectKeys = jni_env_.env->NewObjectArray(len, jni_env_.stringClass, nullptr);
    jni_env_.CheckException("NewObjectArray");
    jobjectArray jobjectValues = jni_env_.env->NewObjectArray(len, jni_env_.stringClass, nullptr);
    jni_env_.CheckException("NewObjectArray");
    for (int i = 0; i < len; i++) {
        jni_env_.env->SetObjectArrayElement(jobjectKeys, i,
                                            jni_env_.env->NewStringUTF(kvs[i].first.c_str()));
        jni_env_.env->SetObjectArrayElement(jobjectValues, i,
                                            jni_env_.env->NewStringUTF(kvs[i].second.c_str()));
    }
    jni_env_.env->CallVoidMethod(lucene_, jni_env_.add_vertex, vid, labelId, jobjectKeys,
                                 jobjectValues);
    jni_env_.CheckException("add_vertex");
}

void FullTextIndex::AddEdge(const EdgeUid &euid,
                            const std::vector<std::pair<std::string, std::string>> &kvs) {
    if (kvs.empty()) {
        return;
    }
    int len = kvs.size();
    jobjectArray jobjectKeys = jni_env_.env->NewObjectArray(len, jni_env_.stringClass, nullptr);
    jni_env_.CheckException("NewObjectArray");
    jobjectArray jobjectValues = jni_env_.env->NewObjectArray(len, jni_env_.stringClass, nullptr);
    jni_env_.CheckException("NewObjectArray");
    for (int i = 0; i < len; i++) {
        jni_env_.env->SetObjectArrayElement(jobjectKeys, i,
                                            jni_env_.env->NewStringUTF(kvs[i].first.c_str()));
        jni_env_.env->SetObjectArrayElement(jobjectValues, i,
                                            jni_env_.env->NewStringUTF(kvs[i].second.c_str()));
    }
    jni_env_.env->CallVoidMethod(lucene_, jni_env_.add_edge, euid.src, euid.dst, euid.lid, euid.eid,
                                 jobjectKeys, jobjectValues);
    jni_env_.CheckException("add_edge");
}

void FullTextIndex::DeleteVertex(int64_t vid) {
    jni_env_.env->CallVoidMethod(lucene_, jni_env_.delete_vertex, vid);
    jni_env_.CheckException("delete_vertex");
}

void FullTextIndex::DeleteEdge(const EdgeUid &euid) {
    jni_env_.env->CallVoidMethod(lucene_, jni_env_.delete_edge, euid.src, euid.dst, euid.lid,
                                 euid.eid);
    jni_env_.CheckException("delete_edge");
}

std::vector<std::pair<int64_t, float>> FullTextIndex::QueryVertex(LabelId lid,
                                                                  const std::string &query,
                                                                  int top_n) {
    std::vector<std::pair<int64_t, float>> ret;
    auto ids = (jobjectArray)jni_env_.env->CallObjectMethod(
        lucene_, jni_env_.query_vertex, lid, jni_env_.env->NewStringUTF(query.c_str()), top_n);
    jni_env_.CheckException("query_vertex");
    jsize len = jni_env_.env->GetArrayLength(ids);
    for (int i = 0; i < len; i++) {
        jobject ob = jni_env_.env->GetObjectArrayElement(ids, i);
        jlong vid = jni_env_.env->GetLongField(ob, jni_env_.vid);
        jfloat score = jni_env_.env->GetFloatField(ob, jni_env_.vertexScore);
        ret.emplace_back(vid, score);
    }
    jni_env_.CheckException("ScoreVid GetField");
    return ret;
}

std::vector<std::pair<EdgeUid, float>> FullTextIndex::QueryEdge(LabelId lid,
                                                                const std::string &query,
                                                                int top_n) {
    std::vector<std::pair<EdgeUid, float>> ret;
    auto ids = (jobjectArray)jni_env_.env->CallObjectMethod(
        lucene_, jni_env_.query_edge, lid, jni_env_.env->NewStringUTF(query.c_str()), top_n);
    jni_env_.CheckException("query_edge");
    jsize len = jni_env_.env->GetArrayLength(ids);
    for (int i = 0; i < len; i++) {
        jobject ob = jni_env_.env->GetObjectArrayElement(ids, i);
        jlong srcID = jni_env_.env->GetLongField(ob, jni_env_.srcId);
        jlong destId = jni_env_.env->GetLongField(ob, jni_env_.destId);
        jint labelId = jni_env_.env->GetIntField(ob, jni_env_.labelId);
        jint edgeId = jni_env_.env->GetIntField(ob, jni_env_.edgeId);
        jfloat score = jni_env_.env->GetFloatField(ob, jni_env_.edgeScore);
        ret.emplace_back(EdgeUid{srcID, destId, (LabelId)labelId, 0, edgeId},
                         score);  // TODO(heng):
    }
    jni_env_.CheckException("ScoreEdgeUid GetField");
    return ret;
}

void LuceneJNIEnv::InitJniEnv() {
    JNIEnv *localEnv = nullptr;
    JavaVMOption *options = new JavaVMOption[1];
    options[0].optionString =
        const_cast<char *>("-Djava.class.path=./tugraph_lucene-1.0-SNAPSHOT.jar");
    JavaVMInitArgs vm_args;
    vm_args.version = JNI_VERSION_1_8;
    vm_args.nOptions = 1;
    vm_args.options = options;
    vm_args.ignoreUnrecognized = 1;
    // Construct a VM
    jint res = JNI_CreateJavaVM(&vm, (void **)&localEnv, &vm_args);
    if (res != JNI_OK) {
        throw FTIndexException("Failed to create jvm, ret: " + std::to_string(res));
    }
}

void LuceneJNIEnv::CheckException(const std::string &call_method) {
    jthrowable ex = env->ExceptionOccurred();
    if (ex) {
        std::string exStr;
        env->ExceptionClear();
        jclass exClass = env->GetObjectClass(ex);
        jmethodID toString = env->GetMethodID(exClass, "toString", "()Ljava/lang/String;");
        auto errMsg = (jstring)env->CallObjectMethod(exClass, toString);
        const char *nativeString = env->GetStringUTFChars(errMsg, nullptr);
        exStr = nativeString;
        env->ReleaseStringUTFChars(errMsg, nativeString);
        throw FTIndexException(call_method + " : " + exStr);
    }
}

LuceneJNIEnv::LuceneJNIEnv() {
    std::call_once(once_flag, InitJniEnv);
    if (vm->GetEnv(reinterpret_cast<void **>(&env), JNI_VERSION_1_8) == JNI_EDETACHED) {
        auto s = vm->AttachCurrentThread(reinterpret_cast<void **>(&env), nullptr);
        if (s != JNI_OK) {
            throw FTIndexException("jvm AttachCurrentThread failed");
        }
        need_detach = true;
    }
    luceneClass = env->FindClass("Lucene");
    CheckException("FindClass Lucene");
    lucene_constructor =
        env->GetMethodID(luceneClass, "<init>", "(Ljava/lang/String;Ljava/lang/String;II)V");
    CheckException("GetMethodID init");
    add_vertex =
        env->GetMethodID(luceneClass, "addVertex", "(JI[Ljava/lang/String;[Ljava/lang/String;)V");
    CheckException("GetMethodID addVertex");
    add_edge =
        env->GetMethodID(luceneClass, "addEdge", "(JJII[Ljava/lang/String;[Ljava/lang/String;)V");
    CheckException("GetMethodID addEdge");
    delete_vertex = env->GetMethodID(luceneClass, "deleteVertex", "(J)V");
    CheckException("GetMethodID deleteVertex");
    delete_edge = env->GetMethodID(luceneClass, "deleteEdge", "(JJII)V");
    CheckException("GetMethodID deleteEdge");
    delete_label = env->GetMethodID(luceneClass, "deleteLabel", "(ZI)V");
    CheckException("GetMethodID deleteLabel");
    query_vertex =
        env->GetMethodID(luceneClass, "queryVertex", "(ILjava/lang/String;I)[LScoreVid;");
    CheckException("GetMethodID queryVertex");
    query_edge =
        env->GetMethodID(luceneClass, "queryEdge", "(ILjava/lang/String;I)[LScoreEdgeUid;");
    CheckException("GetMethodID queryEdge");
    close = env->GetMethodID(luceneClass, "close", "()V");
    CheckException("GetMethodID close");
    clear = env->GetMethodID(luceneClass, "clear", "()V");
    CheckException("GetMethodID clear");
    refresh = env->GetMethodID(luceneClass, "maybeRefresh", "()V");
    CheckException("GetMethodID maybeRefresh");
    backup = env->GetMethodID(luceneClass, "backup", "(Ljava/lang/String;)V");
    CheckException("GetMethodID backup");
    commit = env->GetMethodID(luceneClass, "commit", "()V");
    CheckException("GetMethodID commit");
    stringClass = env->FindClass("java/lang/String");
    CheckException("FindClass String");

    ScoreVidClass = env->FindClass("ScoreVid");
    CheckException("FindClass ScoreVid");
    vid = env->GetFieldID(ScoreVidClass, "vid", "J");
    CheckException("GetFieldID vid");
    vertexScore = env->GetFieldID(ScoreVidClass, "score", "F");
    CheckException("GetFieldID score");

    ScoreEdgeUidClass = env->FindClass("ScoreEdgeUid");
    CheckException("FindClass ScoreEdgeUid");
    srcId = env->GetFieldID(ScoreEdgeUidClass, "srcId", "J");
    CheckException("GetFieldID srcId");
    destId = env->GetFieldID(ScoreEdgeUidClass, "destId", "J");
    CheckException("GetFieldID destId");
    labelId = env->GetFieldID(ScoreEdgeUidClass, "labelId", "I");
    CheckException("GetFieldID labelId");
    edgeId = env->GetFieldID(ScoreEdgeUidClass, "edgeId", "I");
    CheckException("GetFieldID edgeId");
    edgeScore = env->GetFieldID(ScoreEdgeUidClass, "score", "F");
    CheckException("GetFieldID score");
}

LuceneJNIEnv::~LuceneJNIEnv() {
    if (need_detach) {
        vm->DetachCurrentThread();
    }
}

}  // namespace lgraph

#endif
