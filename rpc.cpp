#include "rpc.h"
#include <fstream>
#include <iostream>

Rpc::Rpc()
{

}

void Rpc::setIsEmpty(bool val){
    isEmpty = val;
}

bool Rpc::getIsEmpty(){
    return isEmpty;
}

bool Rpc::readData(TIFF* tif/*QString fileName*/){

//    QByteArray ba = fileName.toLocal8Bit();
//    const char *charFileName = ba.data();
//    TIFF* tif = TIFFOpen(charFileName, "r");
    uint32_t count;
    void *RPCdata;
    if (tif){
        int RPCErr = TIFFGetField(tif, 50844, &count, &RPCdata);
        if (!RPCErr){
//            TIFFClose(tif);
            return false;
        }
        data = *(RPC *)RPCdata;
        isEmpty = false;
//        TIFFClose(tif);
    }

    return true;
}

double summ(double C[], double P, double L, double H){
    return (C[0] + C[1] * L + C[2] * P + C[3] * H + C[4] * L * P
            + C[5] * L * H + C[6] * P * H + C[7] * L * L + C[8] * P * P
            + C[9] * H * H + C[10] * P * L * H + C[11] * L * L * L
            + C[12] * L * P * P + C[13] * L * H * H + C[14] * L * L * P
            + C[15] * P * P * P + C[16] * P * H * H + C[17] * L * L * H
            + C[18] * P * P * H + C[19] * H * H * H);
}

bool Rpc::getRow(double &row){
    double den = summ(data.LINE_DEN_COEFF, PCoeff, LCoeff, HCoeff);
    if (den == 0){
        return false;
    } else {
        double r_n = summ(data.LINE_NUM_COEFF, PCoeff, LCoeff, HCoeff) / den;
        row = r_n * data.LINE_SCALE + data.LINE_OFF;
        return true;
    }
}

bool Rpc::getColumn(double &column){
    double den = summ(data.SAMP_DEN_COEFF, PCoeff, LCoeff, HCoeff);
    if (den == 0){
        return false;
    }
    else{
        double c_n = summ(data.SAMP_NUM_COEFF, PCoeff, LCoeff, HCoeff) / den;
        column = c_n * data.SAMP_SCALE + data.SAMP_OFF;
        return true;
    }
}

bool Rpc::geo2planar(double B, double L, double H, double& row, double& column){
    if (isEmpty){
        return false;
    }
    if ((data.LAT_SCALE == 0) || data.LONG_SCALE == 0 || data.HEIGHT_SCALE == 0){
        return false;
    }
    PCoeff = (B - data.LAT_OFF) / data.LAT_SCALE;
    LCoeff = (L - data.LONG_OFF) / data.LONG_SCALE;
    HCoeff = (H - data.HEIGHT_OFF) / data.HEIGHT_SCALE;
    bool wasRowCount = getRow(row);
    bool wasColumnCount = getColumn(column);
    return wasRowCount && wasColumnCount;
}

bool Rpc::planar2geo(double x, double y, double H, double& B, double& L, int& iterationNumber){
    if (isEmpty){
        return false;
    }
    double y0, x0; //row и column
    double B0, L0, H0;
    double dL = 1;
    double dB = 1;
    double dg = 0.00001;
    B0 = data.LAT_OFF;
    L0 = data.LONG_OFF;
    H0 = H;
    double dmin = 0.000002778;
    int k = 0; //счетчик
    while ((std::abs(dB) > dmin) || (std::abs(dL) > dmin)){
        bool wasPlanarCount = geo2planar(B0, L0, H0, y0, x0);
        double xb, yb, xl, yl;
        wasPlanarCount = wasPlanarCount && geo2planar(B0 + dg, L0, H0, yb, xb);
        wasPlanarCount = wasPlanarCount && geo2planar(B0, L0 + dg, H0, yl, xl);
        if (!wasPlanarCount){
            return false;
        }
        double dxDb = (xb - x0) / dg;
        double dyDb = (yb - y0) / dg;
        double dxDl = (xl - x0) / dg;
        double dyDl = (yl - y0) / dg;
        if ((dxDb == 0) || ((dyDl - dyDb * dxDl / dxDb) == 0)){
            return false;
        }
        dL = (y - y0 - dyDb / dxDb * x + dyDb / dxDb * x0) / (dyDl - dyDb * dxDl / dxDb);
        dB = (x - x0 - dxDl * dL) / dxDb;
        B0 += dB;
        L0 += dL;
        if (k > 1000000){
            break;
        }
        k += 1;
    }
    B = B0;
    L = L0;
    iterationNumber = k;
    return true;
}
