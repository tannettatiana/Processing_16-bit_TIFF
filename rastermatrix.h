//#pragma once

#ifndef RASTERMATRIX
#define RASTERMATRIX

#include "matrix.h"
#include <vector>

template <typename T>
class RasterMatrix: public Matrix<T>{

    int colorsCount;

    public:
        RasterMatrix(): Matrix<T>(){
            this -> colorsCount = 0;
        }

        void initial(int rowSize, int colSize, int colorsCount){
            Matrix<T>::initial(rowSize, colSize * colorsCount);
            this -> colorsCount = colorsCount;
        }

        void free(){
            if (!this -> getEmpty()){
                Matrix<T>::free();
                this -> colorsCount = 0;
            }
        }

        int getColorsCount() const{
            return this -> colorsCount;
        }

        T const getPixel(int i, int j, int channel = 0){
            if (this -> colorsCount == 1){
                return this -> matr[i][j];
            } else {
                return this -> matr[i][this -> colorsCount * j + channel];
            }
        }

        void setPixel(T& val, int i, int j, int channel = 0){
            if (this -> colorsCount == 1){
                this -> matr[i][j] = val;
            } else {
                this -> matr[i][this -> colorsCount * j + channel] = val;
            }
        }

        int getColumn() const{
            return this -> column / this -> colorsCount;
        }

        typename std::vector <T> :: iterator getBegin(int i){
            return this -> matr[i].begin();
        }

        typename std::vector <T> :: iterator getEnd(int i){
            return this -> matr[i].end();
        }


//        T* operator [] (int i){
//            return this -> matr[i].data();
//        }

        void setString(int i, T* str){
            *this -> matr[i].data() = *str;
        }



};
#endif // RASTERMATRIX

