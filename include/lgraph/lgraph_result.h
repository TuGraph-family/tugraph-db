/* Copyright (c) 2022 AntGroup. All Rights Reserved. */

/**
 * Result interface for plugins and built-in procedures. The result of a plugin should be provided
 * in this format in order for the Cypher engine and the graph visualizer to understand.
 */

#pragma once
#include "lgraph/lgraph.h"
#include "tools/json.hpp"

namespace lgraph_api {

/**
 * @brief result type
 * @param INTEGER
 * @param FLOAT
 * @param DOUBLE
 * @param BOOLEAN
 * @param STRING
 * @param MAP <string, FieldData>
 * @param NODE VertexIterator, VertexId
 * @param RELATIONSHIP InEdgeIterator || OutEdgeIterator, EdgeUid
 * @param PATH TODO
 * @param LIST <string, FieldData>
 * @param FIELD INTEGER, FLOAT, DOUBLE, BOOLEAN, STRING
 * @param GRAPH_ELEMENT TODO. NODE, RELATIONSHIP, PATH
 * @param COLLECTION TODO. MAP, LIST
 * @param ANY TODO. any type
 */
enum class ResultElementType {
    NUL = 0x0,
    INTEGER = 0x11,
    FLOAT = 0x12,
    DOUBLE = 0x13,
    BOOLEAN = 0x14,
    STRING = 0x15,
    NODE = 0x21,
    RELATIONSHIP = 0x22,
    PATH = 0x23,
    LIST = 0x41,
    MAP = 0x42,
    FIELD = 0x10,
    GRAPH_ELEMENT = 0x20,
    COLLECTION = 0x40,
    ANY = 0x80
};

ResultElementType ResultElementTypeUpcast(ResultElementType);

struct ResultElement;

/**
 * @brief   You only initialize the class by Result instance. Record provide some insert method
 *          to insert data to the record. eg. Insert, InsertVertexByID, InsertEdgeByID.
 */
class Record {
    std::unordered_map<std::string, std::shared_ptr<ResultElement>> record;
    std::unordered_map<std::string, ResultElementType> header;
    int64_t length_;

    explicit Record(const std::vector<std::pair<std::string, ResultElementType>> &);

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
 * @brief   Result table, result instance provide [NewRecord], [ResetHeader], [Dump] and [Load]
 *          method. Table also provide some method to view content of table. For example,
 *          [Header] and [Recordview].
 *          
 *          It's worth noting that you are best to define your header before using result table.
 *          eg.
 *                       auto result = Result({title, ResultElementType}...)
 *          If you do not define header and initialize by Result(), you will get a empty table
 *          without header, you just use the table after using [ResetHeader] method to set your
 *          header.
 */
class Result {
    std::vector<Record> result;
    std::vector<std::pair<std::string, ResultElementType>> header;
    int64_t row_count_;

 public:
    Result();
    Result(const std::initializer_list<std::pair<std::string, ResultElementType>> &);
    explicit Result(const std::vector<std::pair<std::string, ResultElementType>> &);

    /**
     * @brief   Get type of title.
     *
     * @param   title   One of titles in table.
     *
     * @returns ResultElementType.
     */
    ResultElementType GetType(std::string title);

    /**
     * @brief   Reset your header, the operation will clear the original data and header, please
     *          use the function carefully.
     *
     * @param   header  List of &lt;title, ResultElementType&gt;
     */
    void ResetHeader(const std::vector<std::pair<std::string, ResultElementType>> &header);

    /**
     * @brief   Reset your header, the operation will clear the original data and header, please
     *          use the function carefully.
     *
     * @param   header  List of &lt;title, ResultElementType&gt;
     */
    void ResetHeader(
        const std::initializer_list<std::pair<std::string, ResultElementType>> &header);

    /**
     * @brief   Create a new record in the table and return the record. The record is the
     *          reference of record in the table, if you want to modify the record, you must
     *          assign return value to a reference variable.
     *
     * @returns The reference of record.
     */
    Record &NewRecord();

    /**
     * @brief   return header of the table.
     *
     * @returns header.
     */
    const std::vector<std::pair<std::string, ResultElementType>> &Header();

    /**
     * @brief   Get record of the table, if row number is more bigger than max length of table,
     *          throw a exception.
     *
     * @param   row_num row number of the table.
     *
     * @returns One Record.
     */
    const std::unordered_map<std::string, std::shared_ptr<ResultElement>> &RecordView(
        int64_t row_num);

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
};

}  // namespace lgraph_api
