#include <catch.hpp>

#include <iostream>

#include "ShpLoader.h"

#pragma pack(push, 1)
struct DbfRecordRaw {
    uint8_t deleted;
    std::array<uint8_t, 19> area;
    std::array<uint8_t, 18> perimeter;
    std::array<uint8_t, 11> gravimet;
    std::array<uint8_t, 11> gravimetI;
    std::array<uint8_t, 8> polyColor;
    std::array<uint8_t, 49> name;
};
#pragma pack(pop)

struct DbfRecord {
    float area;
    float perimeter;
    int32_t gravimet;
    int32_t gravimetI;
    int32_t polyColor;
    std::string name;
};

int firstNonWhitespaceChar(const std::string &str) {
    for (size_t i = 0; i < str.size(); i++) {
        if (str[i] != ' ') {
            return i;
        }
    }
    return -1;
}

template <class Itr> float toFloat(Itr begin, Itr end) {
    auto str = std::string(begin, end);
    return std::stof(str.substr(firstNonWhitespaceChar(str)));
}

template <class Itr> int32_t toInt(Itr begin, Itr end) {
    auto str = std::string(begin, end);
    return std::stoi(str.substr(firstNonWhitespaceChar(str)));
}

TEST_CASE("Can load shp file") {
    ShpData shpData;
    bool success = loadShpFile("../../gis_data/gravimetry/gravi.shp", shpData);
    REQUIRE(success);
    REQUIRE(shpData.records.size() == 605);
}

TEST_CASE("Can load dbf file") {
    DbfData dbfData;
    bool success = loadDbfFile("../../gis_data/gravimetry/gravi.dbf", dbfData);
    REQUIRE(success);

    REQUIRE(dbfData.header.numRecords == 605);
    REQUIRE(dbfData.header.numBytesRecord == 117);
    REQUIRE(dbfData.numFieldDescriptors == 6);
    // TODO check each field descriptor
#if 0
    for (int i = 0; i < dbfData.numFieldDescriptors; i++) {
        const std::array<char, 11> &fieldName = dbfData.fieldDescriptors[i].fieldName;
        std::cout << "\tField name: " << std::string(fieldName.begin(), fieldName.end()) << std::endl;
        std::cout << "\tField type: " << dbfData.fieldDescriptors[i].fieldType << std::endl;
        std::cout << "\tField size: " << int(dbfData.fieldDescriptors[i].fieldLength) << std::endl;
        std::cout << std::endl;
    }
#endif

    std::vector<DbfRecord> records = convertTo<DbfRecordRaw, DbfRecord>(dbfData, [](const DbfRecordRaw &raw) {
        DbfRecord result = {};
        result.area = toFloat(raw.area.begin(), raw.area.end());
        result.perimeter = toFloat(raw.perimeter.begin(), raw.perimeter.end());
        result.gravimet = toInt(raw.gravimet.begin(), raw.gravimet.end());
        result.gravimetI = toInt(raw.gravimetI.begin(), raw.gravimetI.end());
        result.polyColor = toInt(raw.polyColor.begin(), raw.polyColor.end());
        result.name = std::string(raw.name.begin(), raw.name.end());
        return result;
    });

    // TODO check the first record
#if 0
    for (const auto &record : records) {
        std::cout << record.area << ", " << record.perimeter << ", " << record.gravimet << ", " << record.gravimetI
                  << ", " << record.polyColor << std::endl;
    }
#endif
}
