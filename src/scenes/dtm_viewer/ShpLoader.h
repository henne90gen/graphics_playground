#pragma once

#include <array>
#include <cstdint>
#include <fstream>
#include <functional>
#include <iostream>
#include <string>
#include <vector>

#pragma pack(push, 1)

/* https://www.esri.com/library/whitepapers/pdfs/shapefile.pdf */
struct ShpHeader {
    int32_t fileCode;              // big endian
    std::array<int32_t, 5> unused; // big endian
    int32_t fileLength;            // big endian
    int32_t version;               // little endian
    int32_t shapeType;             // little endian
    double minX;                   // little endian
    double minY;                   // little endian
    double maxX;                   // little endian
    double maxY;                   // little endian
    double minZ;                   // little endian, optional
    double maxZ;                   // little endian, optional
    double minM;                   // little endian, optional
    double maxM;                   // little endian, optional
};

struct ShpRecordHeader {
    int32_t recordNumber;  // big endian
    int32_t contentLength; // big endian
};

struct ShpRecord {
    int32_t shapeType = 0;
    void *data = nullptr;
};

struct ShpPoint {
    double x;
    double y;
};

struct ShpMultiPoint {
    std::array<double, 4> box = {};
    int32_t numPoints = -1;
    ShpPoint *points = nullptr;
};

struct ShpPolyLine {
    std::array<double, 4> box = {};
    int32_t numParts = -1;
    int32_t numPoints = -1;
    int32_t *parts = nullptr;
    ShpPoint *points = nullptr;
};

struct ShpPolygon {
    std::array<double, 4> box = {};
    int32_t numParts = -1;
    int32_t numPoints = -1;
    int32_t *parts = nullptr;
    ShpPoint *points = nullptr;
};

struct ShpData {
    ShpHeader header;
    std::vector<ShpRecord> records;
};

struct DbfLastUpdate {
    uint8_t yearFrom1900;
    uint8_t month;
    uint8_t day;
};

/*
    https://www.dbase.com/Knowledgebase/INT/db7_file_fmt.htm
    http://www.dbfree.org/webdocs/1-documentation/b-dbf_header_specifications.htm
*/
struct DbfHeader {
    uint8_t fileType;
    DbfLastUpdate lastUpdate;
    int32_t numRecords;
    int16_t positionOfFirstData;
    int16_t numBytesRecord;
    std::array<uint8_t, 16> reserved0;
    uint8_t tableFlags;
    uint8_t codePageMark;
    std::array<uint8_t, 2> reserved1;
};

struct DbfFieldDescriptor {
    std::array<char, 11> fieldName;
    uint8_t fieldType;
    uint32_t fieldOffset;
    uint8_t fieldLength;
    uint8_t fieldDecimalCount;
    uint8_t flags;
    uint32_t autoIncrementNext;
    uint8_t autoIncrementStep;
    std::array<uint8_t, 8> reserved2;
};

struct DbfData {
    DbfHeader header;
    int32_t numFieldDescriptors;
    DbfFieldDescriptor *fieldDescriptors;
    void *data;
};

struct DbfNumeric {
    float value;
};

#pragma pack(pop)

bool loadShpFile(const std::string &fileName, ShpData &data);
bool loadDbfFile(const std::string &fileName, DbfData &data);

template <typename Raw, typename R>
std::vector<R> convertTo(const DbfData &data, const std::function<R(const Raw &)> convertFunc) {
    auto records = std::vector<R>(data.header.numRecords);
    for (size_t i = 0; i < (size_t)data.header.numRecords; i++) {
        auto record = reinterpret_cast<Raw *>(data.data)[i];
        records[i] = convertFunc(record);
    }
    return records;
}
