#include "ShpLoader.h"

int32_t swapOrder(int32_t in) {
    std::array<unsigned char, 4> buffer = {};
    for (int i = 0; i < 4; i++) {
        buffer[3 - i] = (in >> (i * 8));
    }
    return (int)buffer[0] | (int)buffer[1] << 8 | (int)buffer[2] << 16 | (int)buffer[3] << 24;
}

std::istream &operator>>(std::istream &is, ShpHeader &header) {
    is.read(reinterpret_cast<char *>(&header), sizeof(header));
    if (is.fail()) {
        std::cout << "Failed to read file header" << std::endl;
        return is;
    }

    header.fileCode = swapOrder(header.fileCode);
    header.fileLength = swapOrder(header.fileLength);

    return is;
}

std::istream &operator>>(std::istream &is, ShpRecordHeader &header) {
    is.read(reinterpret_cast<char *>(&header), sizeof(header));
    if (is.fail()) {
        std::cout << "Failed to read record header" << std::endl;
        return is;
    }

    header.recordNumber = swapOrder(header.recordNumber);
    header.contentLength = swapOrder(header.contentLength);

    return is;
}

std::istream &operator>>(std::istream &is, ShpMultiPoint *multiPoint) {
    is.read(reinterpret_cast<char *>(multiPoint), sizeof(double) * 4 + sizeof(int32_t));
    if (is.fail()) {
        std::cout << "Failed to read multi point" << std::endl;
        return is;
    }

    size_t pointsSize = multiPoint->numPoints * sizeof(ShpPoint);
    multiPoint->points = static_cast<ShpPoint *>(std::malloc(pointsSize));
    is.read(reinterpret_cast<char *>(multiPoint->points), pointsSize);

    return is;
}

std::istream &operator>>(std::istream &is, ShpPolyLine *polyLine) {
    is.read(reinterpret_cast<char *>(polyLine), sizeof(double) * 4 + sizeof(int32_t) * 2);
    if (is.fail()) {
        std::cout << "Failed to read poly line" << std::endl;
        return is;
    }

    size_t partsSize = polyLine->numParts * sizeof(int32_t);
    polyLine->parts = static_cast<int32_t *>(std::malloc(partsSize));
    is.read(reinterpret_cast<char *>(polyLine->parts), partsSize);

    size_t pointsSize = polyLine->numPoints * sizeof(ShpPoint);
    polyLine->points = static_cast<ShpPoint *>(std::malloc(pointsSize));
    is.read(reinterpret_cast<char *>(polyLine->points), pointsSize);

    return is;
}

std::istream &operator>>(std::istream &is, ShpPolygon *polygon) {
    is.read(reinterpret_cast<char *>(polygon), sizeof(double) * 4 + sizeof(int32_t) * 2);
    if (is.fail()) {
        std::cout << "Failed to read polygon" << std::endl;
        return is;
    }

    size_t partsSize = polygon->numParts * sizeof(int32_t);
    polygon->parts = static_cast<int32_t *>(std::malloc(partsSize));
    is.read(reinterpret_cast<char *>(polygon->parts), partsSize);

    size_t pointsSize = polygon->numPoints * sizeof(ShpPoint);
    polygon->points = static_cast<ShpPoint *>(std::malloc(pointsSize));
    is.read(reinterpret_cast<char *>(polygon->points), pointsSize);

    return is;
}

std::istream &operator>>(std::istream &is, ShpRecord &record) {
    is.read(reinterpret_cast<char *>(&record), sizeof(record.shapeType));
    if (is.fail()) {
        std::cout << "Failed to read record shape type" << std::endl;
        return is;
    }

    switch (record.shapeType) {
    case 1: {
        auto *point = new ShpPoint();
        is.read(reinterpret_cast<char *>(point), sizeof(ShpPoint));
        record.data = point;
        break;
    }
    case 8: {
        auto *multiPoint = new ShpMultiPoint();
        is >> multiPoint;
        record.data = multiPoint;
        break;
    }
    case 3: {
        auto *polyLine = new ShpPolyLine();
        is >> polyLine;
        record.data = polyLine;
        break;
    }
    case 5: {
        auto *polygon = new ShpPolygon();
        is >> polygon;
        record.data = polygon;
        break;
    }
    default:
        std::cout << "Shape type " << record.shapeType << " not supported" << std::endl;
        break;
    }

    return is;
}

std::istream &operator>>(std::istream &is, ShpData &data) {
    is >> data.header;
    if (is.rdstate() != std::ios_base::goodbit) {
        std::cout << "Failed to read shp header" << std::endl;
        return is;
    }

    data.records.clear();
    while (is.tellg() < data.header.fileLength * 2) {
        ShpRecordHeader recordHeader = {};
        is >> recordHeader;
        if (is.fail()) {
            break;
        }

        ShpRecord record = {};
        is >> record;
        if (is.fail()) {
            break;
        }

        if (record.shapeType != 5) {
            is.seekg(recordHeader.contentLength * 2 - sizeof(record.shapeType), std::ios::cur);
        }

        data.records.push_back(record);
    }

    return is;
}

bool loadShpFile(const std::string &fileName, ShpData &data) {
    std::ifstream shpFile;
    shpFile.open(fileName, std::ios::in | std::ios::binary);
    if (!shpFile.is_open()) {
        std::cout << "Failed to open file" << std::endl;
        return false;
    }

    shpFile >> data;
    if (shpFile.fail()) {
        shpFile.close();
        return false;
    }

    shpFile.close();
    return true;
}

std::istream &operator>>(std::istream &is, DbfHeader &header) {
    is.read(reinterpret_cast<char *>(&header), sizeof(header));
    return is;
}

std::istream &operator>>(std::istream &is, DbfData &data) {
    is >> data.header;
    if (is.fail()) {
        std::cout << "Failed to read dbf header" << std::endl;
        return is;
    }

    size_t fieldDescriptorsSize = data.header.positionOfFirstData - sizeof(DbfHeader) - 1;
    data.numFieldDescriptors = fieldDescriptorsSize / sizeof(DbfFieldDescriptor);
    data.fieldDescriptors = reinterpret_cast<DbfFieldDescriptor *>(std::malloc(fieldDescriptorsSize));
    is.read(reinterpret_cast<char *>(data.fieldDescriptors), fieldDescriptorsSize);

    size_t recordsSize = data.header.numRecords * data.header.numBytesRecord;
    data.data = std::malloc(recordsSize);
    is.read(reinterpret_cast<char *>(data.data), recordsSize);

    return is;
}

bool loadDbfFile(const std::string &fileName, DbfData &data) {
    std::ifstream dbfFile;
    dbfFile.open(fileName, std::ios::in | std::ios::binary);

    if (!dbfFile.is_open()) {
        std::cout << "Failed to open file" << std::endl;
        return false;
    }

    dbfFile >> data;
    if (dbfFile.fail()) {
        dbfFile.close();
        return false;
    }

    dbfFile.close();
    return true;
}
