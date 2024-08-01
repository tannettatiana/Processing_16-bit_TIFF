#include "geoimage.h"
#include "rastermatrix.h"
#include "rpc.h"
#include <QtMath>

#define TIFFTAG_RPCCOEFFICIENT 50844
#define     N(a)    (sizeof (a) / sizeof (a[0]))


static const TIFFFieldInfo xtiffFieldInfo[] = {

    /* XXX Insert Your tags here */
    { TIFFTAG_RPCCOEFFICIENT, -1, -1, TIFF_DOUBLE, FIELD_CUSTOM,
      true, true, "RPC" }
};

GeoImage::GeoImage(Progress* progress) : QImage()
{
    x1.resize(3);
    x2.resize(3);

    histR.resize(brightness_range);
    histG.resize(brightness_range);
    histB.resize(brightness_range);

    this->progress = progress;
}

GeoImage::GeoImage() : QImage()
{
    x1.resize(3);
    x2.resize(3);

    histR.resize(brightness_range);
    histG.resize(brightness_range);
    histB.resize(brightness_range);
}

GeoImage::~GeoImage()
{
}

enum RGB{
    R = 0, G = 1, B = 2
};

//поиск x' или x'' для контрастирования
int findX(QVector<uint32_t>& histogram, uint32_t npixels, int start, int increment) {
    int k = start;
    uint32_t count = 0;
    while (count <= 0.02 * npixels) {
        count += histogram[k];
        k += increment;
    }
    return k;
}

//поиск x' и x'' для контрастирования
void findX1andX2(std::vector<int>& x1, std::vector<int>& x2, int npixels, QVector<uint32_t>& histogram, int channel) {
    x1[channel] = findX(histogram, npixels, 0, 1);
    x2[channel] = findX(histogram, npixels, max_brightness, -1);
    if (x1[channel] >= x2[channel]) {
        x2[channel] = std::min(x1[channel] + 1, max_brightness);
    }
}

//приведение текущей яркости к значению от 0 до 255
uint8_t histogramContrast(uint16_t bright, const int x1, const int x2) {
    uint8_t color = 0;
    if (bright <= x1) {
        color = 0;
    }
    else if (bright >= x2) {
        color = std::pow(2, 8) - 1;
    }
    else {
        double k = 255 / ((double)(x2 - x1));
        double b = -k * x1;
        color = qRound(k * bright + b);
    }
    return color;
}

//контрастирование
template <typename T>
void GeoImage::contrasting(RasterMatrix<T>& originMatrix, QImage& histImg,  std::vector<int>& x1, std::vector<int>& x2){
    try{
        progress->start("Контрастирование: ");

        double per = 0;
        double length = originMatrix.getRow();

        for (int i = 0; i < originMatrix.getRow(); i++) {

            uint8_t newBlue = 0;
            uint8_t newGreen = 0;
            uint8_t newRed = 0;
            for (int j = 0; j < originMatrix.getColumn(); j++) {

                if (originMatrix.getColorsCount() == 1) {
                    newRed = histogramContrast(originMatrix.getPixel(i, j), x1[0], x2[0]);
                    newGreen = newRed;
                    newBlue = newRed;
                }
                else if (originMatrix.getColorsCount() == 3) {
                    newRed = histogramContrast(originMatrix.getPixel(i, j, R), x1[0], x2[0]);
                    newGreen = histogramContrast(originMatrix.getPixel(i, j, G), x1[1], x2[1]);
                    newBlue = histogramContrast(originMatrix.getPixel(i, j, B), x1[2], x2[2]);
                }

                histImg.setPixel(j, i, qRgb(newRed, newGreen, newBlue));
            }

            per = i / (double)(length) * 100;
            if (progress->set_progress(per)){
                return;
            }
        }

        progress->stop();

    } catch (const std::runtime_error& e){
        printToConsole(QString("Ошибка в прогрессе контрастирования! ") + QString(e.what()));
//        std::cout << "Error in contrasting progress! " << e.what() << std::endl;
        return;
    }
}

template <typename T>
void freeBuffers(T* buff){
    if (!buff)
        _TIFFfree(buff);
}

//считывание изображения из файла в матрицу и поиск параметров контрастирования
template <typename T>
int GeoImage::openAsMartixAndFindParameters(RasterMatrix<T>& originMatrix, TIFF* tif, uint16_t colorsCount, QImage& histImg){
    try{
        progress->start("Вычисление параметров контрастирования: ");

        //начало обработки
        uint32_t npixels;

        //гистограмма, historam[i] = G(i)
        histR.fill(0);
        histG.fill(0);
        histB.fill(0);

        //параметры изображения
        uint32_t width, length;
        uint16_t planarConfig;

        if (!TIFFGetField(tif, TIFFTAG_IMAGEWIDTH, &width)){
            progress->stop();
            return 3;
        }
        if (!TIFFGetField(tif, TIFFTAG_IMAGELENGTH, &length)){
            progress->stop();
            return 4;
        }
        if (!TIFFGetField(tif, TIFFTAG_PLANARCONFIG, &planarConfig)){
            progress->stop();
            return 5;
        }

        npixels = width * length;

        histImg = QPixmap(width, length).toImage();

        originMatrix.initial(length, width, colorsCount);

        uint32_t onePixelSize = ((colorsCount == 3)) ? 3 : 1;
        T* buff = (T*)_TIFFmalloc(width * onePixelSize * sizeof(T));

        if (!buff){
            freeBuffers(buff);
            progress->stop();
            return 2;
        }

        double per = 0;

        for (int i = 0; i < length; i++) {

            int readLineErr1 = TIFFReadScanline(tif, buff, i, 0);
            if (!readLineErr1){
                freeBuffers(buff);
                progress->stop();
                return 6;
            }

            std::copy(buff, buff + width * onePixelSize - 1, originMatrix[i]);
            for (int j = 0; j < width; j++) {
                if (originMatrix.getPixel(i, j) < brightness_range){
                    histR[originMatrix.getPixel(i, j)] += 1;
                } else {
                    this->brightnessErr = QString("Красный канал пикселя имеет недопустимое значение! i: %1; j: %2; яркость: %3").arg(i).arg(j).arg(originMatrix.getPixel(i, j, R));
                    progress->stop();
                    return -2;
                }
                if (colorsCount == 3) {
                    if (originMatrix.getPixel(i, j, G) < brightness_range){
                        histG[originMatrix.getPixel(i, j, G)] += 1;
                    } else {
                        this->brightnessErr = QString("Зеленый канал пикселя имеет недопустимое значение! i: %1; j: %2; яркость: %3").arg(i).arg(j).arg(originMatrix.getPixel(i, j, G));
                        progress->stop();
                        return -2;
                    }
                    if (originMatrix.getPixel(i, j, B) < brightness_range){
                        histB[originMatrix.getPixel(i, j, B)] += 1;
                    } else {
                        this->brightnessErr = QString("Синий канал пикселя имеет недопустимое значение! i: %1; j: %2; яркость: %3").arg(i).arg(j).arg(originMatrix.getPixel(i, j, B));
                        progress->stop();
                        return -2;
                    }
                }
            }

            per = i / (double)(length) * 100;
            if (progress->set_progress(per)){
                progress->stop();
                return -1;
            }

        }

        freeBuffers(buff);

        //поиск x' и x'' для контрастирования по гистограмме
        std::fill(x1.begin(), x1.end(), 0);
        std::fill(x2.begin(), x2.end(), 0);

        findX1andX2(x1, x2, npixels, histR, 0);
        if (colorsCount == 3) {
            findX1andX2(x1, x2, npixels, histG, 1);
            findX1andX2(x1, x2, npixels, histB, 2);
        }

        progress->stop();

        return 0;

    } catch (const std::runtime_error& e){
        printToConsole(QString("Ошибка в прогрессе открытия! ") + QString(e.what()));
//        std::cout << "Error in opening and finding parameters progress! " << e.what() << std::endl;
        return -1;
    }

}

//поиск параметров контрастирования
template <typename T>
int GeoImage::findParameters(RasterMatrix<T>& originMatrix, uint16_t colorsCount, QImage& histImg){
    try{
        progress->start("Вычисление параметров контрастирования: ");

        //начало обработки
        uint32_t npixels;

        //гистограмма, historam[i] = G(i)

        histR.fill(0);
        histG.fill(0);
        histB.fill(0);

        //параметры изображения
        uint32_t width = originMatrix.getColumn();
        uint32_t length = originMatrix.getRow();

        npixels = width * length;

        histImg = QPixmap(width, length).toImage();

        double per = 0;

        for (int i = 0; i < length; i++) {

            for (int j = 0; j < width; j++) {

                if (originMatrix.getPixel(i, j) < brightness_range){
                    histR[originMatrix.getPixel(i, j)] += 1;
                } else {
                    this->brightnessErr = QString("Красный канал пикселя имеет недопустимое значение! i: %1; j: %2; яркость: %3").arg(i).arg(j).arg(originMatrix.getPixel(i, j, R));
                    progress->stop();
                    return -2;
                }

                if (colorsCount == 3) {
                    if (originMatrix.getPixel(i, j, G) < brightness_range){
                        histG[originMatrix.getPixel(i, j, G)] += 1;
                    } else {
                        this->brightnessErr = QString("Зеленый канал пикселя имеет недопустимое значение! i: %1; j: %2; яркость: %3").arg(i).arg(j).arg(originMatrix.getPixel(i, j, G));
                        progress->stop();
                        return -2;
                    }
                    if (originMatrix.getPixel(i, j, B) < brightness_range){
                        histB[originMatrix.getPixel(i, j, B)] += 1;
                    } else {
                        this->brightnessErr = QString("Синий канал пикселя имеет недопустимое значение! i: %1; j: %2; яркость: %3").arg(i).arg(j).arg(originMatrix.getPixel(i, j, B));
                        progress->stop();
                        return -2;
                    }
                }

            }

            per = i / (double)(length) * 100;
            if (progress->set_progress(per)){
                progress->stop();
                return -1;
            }
        }

        //поиск x' и x'' для контрастирования по гистограмме
        std::fill(x1.begin(), x1.end(), 0);
        std::fill(x2.begin(), x2.end(), 0);

        findX1andX2(x1, x2, npixels, histR, 0);
        if (colorsCount == 3) {
            findX1andX2(x1, x2, npixels, histG, 1);
            findX1andX2(x1, x2, npixels, histB, 2);
        }

        progress->stop();
        return 0;

    } catch (const std::runtime_error& e){
        printToConsole(QString("Ошибка в прогрессе поиска параметров контрастирования"));
        return -1;
    }

}

//получение ошибки по коду
QString getErrorMessage(int err){
    QString errStr;
    switch (err){
    case 2:
        errStr = "Error in memory allocation";
        break;
    case 3:
        errStr = "Error in readind field of width";
        break;
    case 4:
        errStr = "Error in readind field of length";
        break;
    case 5:
        errStr = "Error in readind field of planar config";
        break;
    case 6:
        errStr = "Error in reading file";
        break;
    default:
        errStr = "Unknown error in openAsMartixAndFindParameters: " + QString(err);
        break;
    }
    return errStr;
}

//загрузка из матрицы и контрастирование
int GeoImage::loadFromMatr(RasterMatrix<uint16_t> &matr){
    int err = findParameters(matr, matr.getColorsCount(), histImg);
    originMatrix = matr;
    if (err == 0){
        contrasting(originMatrix, histImg, x1, x2);
    } else {
        return err;
    }
    return 0;
}

//чтение изображения из файла и контрастирование
int GeoImage::load(const QString &fileName)
{
    name = fileName;
    QByteArray ba = fileName.toLocal8Bit();
    const char *charFileName = ba.data();
    TIFF* tif = TIFFOpen(charFileName, "r");
    uint16_t bitOnColor = 0;
    uint16_t colorsCount = 0;

    if (!tif){
        return -1;
    }

    int bitOnColorErr = TIFFGetField(tif, TIFFTAG_BITSPERSAMPLE, &bitOnColor);
    if (!bitOnColorErr){
        TIFFClose(tif);
        printToConsole("Ошибка в чтении TIFFTAG_BITSPERSAMPLE");
        return -1;
    }
    if (!TIFFGetField(tif, TIFFTAG_SAMPLESPERPIXEL, &colorsCount)) {
        TIFFClose(tif);
        printToConsole("Ошибка в чтении TIFFTAG_SAMPLESPERPIXEL");
        return -1;
    }
    if (colorsCount != 1 && colorsCount != 3){
        TIFFClose(tif);
        printToConsole("Неподдерживаемое количество каналов!");
        return -1;
    }

    if (bitOnColor != 16){
        TIFFClose(tif);
        return -1;
    }

    int err = openAsMartixAndFindParameters(originMatrix, tif, colorsCount, histImg);
    if (err != 0){
        TIFFClose(tif);
        printToConsole(QString("%1").arg(err));
        return -2;
    }

    if (!originMatrix.getEmpty()){
        contrasting(originMatrix, histImg, x1, x2);
        scaledImage = histImg;
    }

    bool wasRpcRead = rpc.readData(tif);
//    if (!wasRpcRead){
//        std::cout << "rpc" << std::endl;
//        return false;
//    }

    TIFFClose(tif);

    return 0;
}

//сохранение изображения
bool GeoImage::save(const QString &fileName){
    QByteArray ba = fileName.toLocal8Bit();
    const char *charFileName = ba.data();
    TIFF* tif = TIFFOpen(charFileName, "w");
    if (!tif){
        printToConsole(QString("Файл сохраняемого изображения не был открыт!"));
        return false;
    }

    TIFFSetField(tif, TIFFTAG_IMAGEWIDTH, originMatrix.getColumn());
    TIFFSetField(tif, TIFFTAG_IMAGELENGTH, originMatrix.getRow());
    TIFFSetField(tif, TIFFTAG_BITSPERSAMPLE, 16);
    TIFFSetField(tif, TIFFTAG_SAMPLESPERPIXEL, originMatrix.getColorsCount());
    TIFFSetField(tif, TIFFTAG_ROWSPERSTRIP, 1);
    TIFFSetField(tif, TIFFTAG_ORIENTATION, ORIENTATION_TOPLEFT);
    TIFFSetField(tif, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG);
    TIFFSetField(tif, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_MINISBLACK);
    TIFFSetField(tif, TIFFTAG_SAMPLEFORMAT, SAMPLEFORMAT_UINT);
    TIFFSetField(tif, TIFFTAG_COMPRESSION, COMPRESSION_NONE);

    TIFFMergeFieldInfo(tif, xtiffFieldInfo, N(xtiffFieldInfo));

//    double* adfRPCTag = (double*)_TIFFmalloc(92 * sizeof(double));
    double adfRPCTag[92];
    adfRPCTag[0] = -1.0;  // Error Bias
    adfRPCTag[1] = -1.0;  // Error Random
    adfRPCTag[2] = rpc.data.LINE_OFF;
    adfRPCTag[3] = rpc.data.SAMP_OFF;
    adfRPCTag[4] = rpc.data.LAT_OFF;
    adfRPCTag[5] = rpc.data.LONG_OFF;
    adfRPCTag[6] = rpc.data.HEIGHT_OFF;
    adfRPCTag[7] = rpc.data.LINE_SCALE;
    adfRPCTag[8] = rpc.data.SAMP_SCALE;
    adfRPCTag[9] = rpc.data.LAT_SCALE;
    adfRPCTag[10] = rpc.data.LONG_SCALE;
    adfRPCTag[11] = rpc.data.HEIGHT_SCALE;
    for (int i = 0; i < 20; i++){
        adfRPCTag[12 + i] = rpc.data.LINE_NUM_COEFF[i];
    }
    for (int i = 0; i < 20; i++){
        adfRPCTag[32 + i] = rpc.data.LINE_DEN_COEFF[i];
    }
    for (int i = 0; i < 20; i++){
        adfRPCTag[52 + i] = rpc.data.SAMP_NUM_COEFF[i];
    }
    for (int i = 0; i < 20; i++){
        adfRPCTag[72 + i] = rpc.data.SAMP_DEN_COEFF[i];
    }
    int err = TIFFSetField(tif, TIFFTAG_RPCCOEFFICIENT, 92, adfRPCTag);
    printToConsole(QString("%1").arg(err));
//    std::cout << err << std::endl;
    uint16_t* buff = (uint16_t*)_TIFFmalloc(originMatrix.getColumn() * sizeof(uint16_t) * originMatrix.getColorsCount());

    for (int i = 0; i < originMatrix.getRow(); i++){
        std::copy(originMatrix.getBegin(i), originMatrix.getEnd(i), buff);
        TIFFWriteScanline(tif, buff, i, 0);
    }

    _TIFFfree(buff);
    TIFFClose(tif);
    return true;

}

void GeoImage::printToConsole(const QString& str){
//    QString var = QString::fromUtf8(str);
    std::cout << str.toLocal8Bit().data() << std::endl;
}

