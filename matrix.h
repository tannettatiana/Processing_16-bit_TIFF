//#pragma once

#ifndef MATRIX
#define MATRIX

#include <iostream>
#include <vector>

template <typename Type>
class Matrix{

    protected:
        std::vector<std::vector<Type> > matr;
        int row;
        int column;
        bool isEmpty;

    public:
        Matrix(){
            row = 0;
            column = 0;
            this -> isEmpty = true;
        }

        Matrix(int rowSize, int colSize){
            initial(rowSize, colSize);
        }

        void initial(int rowSize, int colSize){
            row = rowSize;
            column = colSize;
            matr.resize(rowSize);
            for (int i = 0; i < matr.size(); i++)
            {
                matr[i].resize(colSize, 0);
            }
            this -> isEmpty = false;
        }

        void free(){
            if (!isEmpty){
                for (int i = 0; i < row; i++)
                {
                    std::vector<Type>().swap(matr[i]);
                }
                row = 0;
                column = 0;
                isEmpty = true;
            }
        }

        bool getEmpty(){
            return this -> isEmpty;
        }

        Type* operator [] (int i){
            return this -> matr[i].data();
        }


//        Type operator () (int i, int j) const{
//            return matr[i][j];
//        }

        /*Type& operator [] (int i){
            return matr[i];
        }*/

        int getRow() const{
            return this -> row;
        }

        int getColumn() const{
            return this -> column;
        }

        void setItem(int i, int j, Type& val){
            matr[i][j] = val;
        }

        /*void printMatr(){
            for (int i = 0; i < row; i++){
                for (int j = 0; j < column; j++){
                    std::cout << std::fixed << std::setprecision(3) << matr[i][j] << "  ";
                }
                std::cout << std::endl;
            }
        }*/

};

#endif // MATRIX

