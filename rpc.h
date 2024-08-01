#ifndef RPC_H
#define RPC_H

#include <QString>
#include <tiffio.h>

#pragma pack (push, 1)
typedef struct {
    double ERR_BIAS = -1;
    double ERR_RAND = -1;
    double LINE_OFF = -1;
    double SAMP_OFF = -1;
    double LAT_OFF = -1;
    double LONG_OFF = -1;
    double HEIGHT_OFF = -1;
    double LINE_SCALE = -1;
    double SAMP_SCALE = -1;
    double LAT_SCALE = -1;
    double LONG_SCALE = -1;
    double HEIGHT_SCALE = -1;
    double LINE_NUM_COEFF[20];
    double LINE_DEN_COEFF[20];
    double SAMP_NUM_COEFF[20];
    double SAMP_DEN_COEFF[20];
} RPC;
#pragma pack (pop)

#pragma pack (push, 1)
//file header
typedef struct {
    uint16_t byteOrder;
    uint16_t identity;
    uint32_t offsetIFD;
} IMAGEFILEHEADER;
#pragma pack (pop)

#pragma pack (push, 1)
//IFD структура
typedef struct {
    uint16_t tag;
    uint16_t valuesType;
    uint32_t valuesNumber;
    uint32_t valueOffset;
} IFD;
#pragma pack (pop)

class Rpc
{
public:
    Rpc();
    RPC data;

    bool readData(TIFF* tif/*QString fileName*/);
    bool geo2planar(double B, double L, double H, double& row, double& column);
    bool planar2geo(double x, double y, double H,  double& B, double& L, int& iterationNumber);
    void setIsEmpty(bool val);
    bool getIsEmpty();

private:
    bool isEmpty = true;

    double PCoeff = 0;
    double LCoeff = 0;
    double HCoeff = 0;

    bool getRow(double& row);
    bool getColumn(double& column);
};

#endif // RPC_H
