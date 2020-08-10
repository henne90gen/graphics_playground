#pragma once

#include <cstdint>

/* https://www.esri.com/library/whitepapers/pdfs/shapefile.pdf */
struct ShpHeader {
    int32_t fileCode;   // big endian
    int32_t unused[5];  // big endian
    int32_t fileLength; // big endian
    int32_t version;    // little endian
    int32_t shapeType;  //  little endian
    double minX;        // little endian
    double minY;        // little endian
    double maxX;        // little endian
    double maxY;        // little endian
    double minZ;        // little endian, optional
    double maxZ;        // little endian, optional
    double minM;        // little endian, optional
    double maxM;        // little endian, optional
};

struct ShiHeader {};

/* http://www.dbfree.org/webdocs/1-documentation/b-dbf_header_specifications.htm */
struct DbfHeader {};
