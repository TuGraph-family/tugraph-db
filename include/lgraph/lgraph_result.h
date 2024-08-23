//  Copyright 2022 AntGroup CO., Ltd.
//  Licensed under the Apache License, Version 2.0 (the "License");
//  you may not use this file except in compliance with the License.
//  You may obtain a copy of the License at
//  http://www.apache.org/licenses/LICENSE-2.0
//  Unless required by applicable law or agreed to in writing, software
//  distributed under the License is distributed on an "AS IS" BASIS,
//  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.

/**
 *  @file   lgraph_result.h
 *  @brief  Result interface for plugins and built-in procedures. The result of a plugin should be provided
 *          in this format in order for the Cypher engine and the graph visualizer to understand.
 */

#pragma once
#include <any>
#include "lgraph/lgraph.h"
#include "tools/json.hpp"
#include "lgraph/lgraph_types.h"

#ifndef _WIN32
#include "lgraph/lgraph_traversal.h"
#endif


namespace lgraph {
class StateMachine;
}

namespace cypher {
class PluginAdapter;
}

namespace lgraph_api {

struct ResultElement;

namespace lgraph_result {
struct Node;
struct Relationship;
}

typedef std::unordered_map<size_t, std::shared_ptr<lgraph_result::Node>> NODEMAP;
typedef std::unordered_map<EdgeUid, std::shared_ptr<lgraph_result::Relationship>,
        EdgeUid::Hash> RELPMAP;

/**
 * @brief   You only initialize the class by Result instance. Record provide some insert method
 *          to insert data to the record. eg. Insert, InsertVertexByID, InsertEdgeByID.
 */
class Record {
    std::unordered_map<std::string, std::shared_ptr<ResultElement>> record;
    std::unordered_map<std::string, LGraphType> header;
    int64_t length_;

    explicit Record(const std::vector<std::pair<std::string, LGraphType>> &);

 public:
    friend class Result;
    Record(const Record &);
    Record(Record &&);
    Record &operator=(const Record &);
    Record &operator=(Record &&);

    /**
     * @brief   insert a value to result table. You can insert a value by insert function, and
     *          value type must be same as you defined earlier.
     *
     * @param   fname   Field name you defined earlier.
     * @param   fv      Field value.
     */
    void Insert(const std::string &fname, const FieldData &fv);

    /**
     * @brief   insert a value to result table. You can insert a value by insert function,
     *          and value type must be same as you defined earlier.  You can get properties
     *          and label from the interface, this is different from InsertVertexByID.
     * @param   fname     title name you defined earlier.
     * @param   vid       VertextId
     * @param   txn       Transaction
     */
    void Insert(const std::string &fname, const int64_t vid, lgraph_api::Transaction* txn);

    /**
     * @brief   insert a value to result table. You can insert a value by insert function,
     *          and value type must be same as you defined earlier. You can get properties
     *          and label from the interface, this is different from InsertEdgeByID.
     * @param   fname   title name you defined earlier.
     * @param   euid    EdgeUid
     * @param   txn     Transaction
     */
    void Insert(const std::string &fname, EdgeUid &euid, lgraph_api::Transaction* txn);

    /**
     * @brief   insert a value to result table. You can insert a value by insert function, and
     *          value type must be same as you defined earlier.
     *
     * @param   fname   Field name you defined earlier.
     * @param   vid     VertextId.
     */
    void InsertVertexByID(const std::string &fname, int64_t vid);

    /**
     * @brief   insert a value to result table. You can insert a value by insert function, and
     *          value type must be same as you defined earlier.
     *
     * @param   fname   title name you defined earlier.
     * @param   uid     EdgeUid.
     */
    void InsertEdgeByID(const std::string &fname, const EdgeUid &uid);

    /**
     * @brief   insert a value to result table. You can insert a value by insert function, and
     *          value type must be same as you defined earlier.
     *
     * @param   fname   one of title name you defined earlier.
     * @param   vit     VertexIterator.
     */
    void Insert(const std::string &fname, const lgraph_api::VertexIterator &vit);

    /**
     * @brief   insert a value to result table. You can insert a value by insert function, and
     *          value type must be same as you defined earlier.
     *
     * @param   fname   one of title name you defined earlier.
     * @param   ieit    InEdgeIterator.
     */
    void Insert(const std::string &fname, const lgraph_api::InEdgeIterator &ieit);

    /**
     * @brief   insert a value to result table. You can insert a value by insert function, and
     *          value type must be same as you defined earlier.
     *
     * @param   fname   one of title name you defined earlier.
     * @param   oeit    OutEdgeIterator.
     */
    void Insert(const std::string &fname, const lgraph_api::OutEdgeIterator &oeit);

    /**
     * @brief   Insert a value to result table. You can insert a value by insert function, and
     *          value type must be same as you defined earlier.
     *
     * @param   fname   one of title name you defined earlier.
     * @param   list    LIST OF FieldData.
     */
    void Insert(const std::string &fname, const std::vector<FieldData> &list);

    /**
     * @brief   insert a value to result table. You can insert a value by insert function, and
     *          value type must be same as you defined earlier.
     *
     * @param   fname   one of title name you defined earlier.
     * @param   map     MAP OF &lt;string, FieldData&gt;
     */
    void Insert(const std::string &fname, const std::map<std::string, FieldData> &map);

#ifndef _WIN32
    /**
     * @brief   insert value into result table. You can insert a value by the function,
     *          and value must be same as you defined earlier.
     * @param  fname    one of title name you defined earlier.
     * @param  path     Path of traverse api.
     * @param  txn      Trasaction
     */
    void Insert(const std::string &fname, const traversal::Path &path,
                lgraph_api::Transaction* txn,
                NODEMAP& node_map,
                RELPMAP& relp_map);
#endif

    /**
     * @brief   Get the size of record. If record is empty, return 0, max size is not beyond the
     *          length of your defined param list
     *
     * @returns Size of record.
     */
    int64_t Size() const { return length_; }

    /**
     * @brief   Check a key is exist or not. The key is the one of titile your defined earlier.
     *
     * @param   key The key.
     *
     * @returns True if exsit, otherwise fasle.
     */
    bool HasKey(const std::string &key) { return header.find(key) != header.end(); }
};
/**
 * @brief   Result table, result instance provide [MutableRecord], [ResetHeader], [Dump] and [Load]
 *          method. Table also provide some method to view content of table. For example,
 *          [Header] and [Recordview].
 *
 *          It's worth noting that you are best to define your header before using result table.
 *          eg.
 *                       auto result = Result({title, LGraphType}...)
 *          If you do not define header and initialize by Result(), you will get a empty table
 *          without header, you just use the table after using [ResetHeader] method to set your
 *          header.
 */
class Result {
    friend class lgraph::StateMachine;
    friend class cypher::PluginAdapter;

    std::vector<Record> result;
    std::vector<std::pair<std::string, LGraphType>> header;
    int64_t row_count_;
    bool is_python_driver_ = false;
    int64_t v_eid_ = 0;  // virtual edge id

 public:
    Result();
    Result(const std::initializer_list<std::pair<std::string, LGraphType>> &);
    explicit Result(const std::vector<std::pair<std::string, LGraphType>> &);

    /**
     * @brief   Get type of title.
     *
     * @param   title   One of titles in table.
     *
     * @returns LGraphType.
     */
    LGraphType GetType(std::string title);

    /**
     * @brief   Reset your header, the operation will clear the original data and header, please
     *          use the function carefully.
     *
     * @param   header  List of &lt;title, LGraphType&gt;
     */
    void ResetHeader(const std::vector<std::pair<std::string, LGraphType>> &header);

    /**
     * @brief   Reset your header, the operation will clear the original data and header, please
     *          use the function carefully.
     *
     * @param   header  List of &lt;title, LGraphType&gt;
     */
    void ResetHeader(
        const std::initializer_list<std::pair<std::string, LGraphType>> &header);

    /**
     * @brief   Create a new record in the table and return the record. The record is the
     *          reference of record in the table, if you want to modify the record, you must
     *          assign return value to a reference variable.
     *
     * @returns The reference of record.
     */
    Record *MutableRecord();

    /**
     * @brief   This function attempts to reserve enough memory for the result vector to hold
     *          the specified number of elements.
     *
     */
    void Reserve(size_t n);

    /**
     * @brief   This function will resize the vector to the specified number of elements
     *
     */
    void Resize(size_t n);

    /**
     * @brief   Provides access to the data contained in the vector.
     *
     */
    Record* At(size_t n);

    /**
     * @brief   return header of the table.
     *
     * @returns header.
     */
    const std::vector<std::pair<std::string, LGraphType>> &Header();

    /**
     * @brief   return size of the table.
     *
     * @returns table size.
     */
    int64_t Size() const;

    /**
     * @brief   Serialize the table.
     *
     * @param   is_standard (Optional) If true, the result will serialize to a standard result,
     *                      the standard result can be Visualized in web. If false, result will
     *                      serialize a json object -- SDK result.
     *
     * @returns Serialize result.
     */
    std::string Dump(bool is_standard = true);

    /**
     * @brief   Deserialize  data to result table. This will be clear original data and header,
     *          please use this function carefully.
     *
     * @param   json    Json string to be deserialized.
     */
    void Load(const std::string &json);

    /**
     * @brief Clear all the records, Size() will be 0.
     */
    void ClearRecords();

    std::vector<std::string> BoltHeader();
    std::vector<std::vector<std::any>> BoltRecords();
    /**
     * @brief Mark that the result is returned to python driver.
     *  Python driver is special, use the virtual edge id instead of the real edge id
     */
    void MarkPythonDriver(bool is_python_driver) {
        is_python_driver_ = is_python_driver;
    }

 private:
    /**
     * @brief   Get record of the table, if row number is more bigger than max length of table,
     *          throw a exception.
     *
     * @param   row_num row number of the table.
     *
     * @returns One Record.
     */
    const std::unordered_map<std::string, std::shared_ptr<ResultElement>>&
    RecordView(int64_t row_num);
};

}  // namespace lgraph_api
