#pragma once

#include "../core/fastVector.hpp"
#include "../io/io.hpp"
#include <cassert>
#include <cstddef>
#include <initializer_list>
#include <utility>

namespace calafite {
    namespace mathematics {

        template<typename Type>
        struct Matrix {
            size_t rowsValue, colsValue;
            core::FastVector<Type> data;

            Matrix() : rowsValue(0), colsValue(0) {}

            Matrix(size_t rows, size_t cols, const Type& initialValue = Type(0))
                : rowsValue(rows), colsValue(cols), data(rows * cols, initialValue) {}

            Matrix(const core::FastVector<core::FastVector<Type>>& grid) {
                rowsValue = grid.size();
                colsValue = rowsValue > 0 ? grid[0].size() : 0;
                data.reserve(rowsValue * colsValue);
                for (size_t r = 0; r < rowsValue; ++r) {
                    assert(grid[r].size() == colsValue && "All rows must have the same number of columns.");
                    for (size_t c = 0; c < colsValue; ++c) {
                        data.pushBack(grid[r][c]);
                    }
                }
            }

            Matrix(std::initializer_list<std::initializer_list<Type>> list) {
                rowsValue = list.size();
                colsValue = rowsValue > 0 ? list.begin()->size() : 0;
                data.reserve(rowsValue * colsValue);
                for (const auto& row : list) {
                    assert(row.size() == colsValue && "All rows must have the same number of columns.");
                    for (const auto& value : row) {
                        data.pushBack(value);
                    }
                }
            }

            static Matrix identity(size_t size) {
                Matrix result(size, size, Type(0));
                for (size_t i = 0; i < size; ++i) {
                    result[i][i] = Type(1);
                }
                return result;
            }

            inline size_t rows() const { return rowsValue; }
            inline size_t cols() const { return colsValue; }
            inline bool empty() const { return rowsValue == 0 || colsValue == 0; }
            inline void clear() { rowsValue = 0; colsValue = 0; data.clear(); }

            inline Type* operator[](size_t row) {
                assert(row < rowsValue);
                return data.begin() + row * colsValue;
            }

            inline const Type* operator[](size_t row) const {
                assert(row < rowsValue);
                return data.begin() + row * colsValue;
            }

            bool operator==(const Matrix& other) const {
                if (rowsValue != other.rowsValue || colsValue != other.colsValue) return false;
                for (size_t i = 0; i < data.size(); ++i) {
                    if (data[i] != other.data[i]) return false;
                }
                return true;
            }

            bool operator!=(const Matrix& other) const { return !(*this == other); }

            Matrix& operator+=(const Matrix& other) {
                assert(rowsValue == other.rowsValue && colsValue == other.colsValue);
                for (size_t i = 0; i < data.size(); ++i) data[i] = data[i] + other.data[i];
                return *this;
            }

            Matrix operator+(const Matrix& other) const { return Matrix(*this) += other; }

            Matrix& operator-=(const Matrix& other) {
                assert(rowsValue == other.rowsValue && colsValue == other.colsValue);
                for (size_t i = 0; i < data.size(); ++i) data[i] = data[i] - other.data[i];
                return *this;
            }

            Matrix operator-(const Matrix& other) const { return Matrix(*this) -= other; }

            Matrix operator*(const Matrix& other) const {
                assert(colsValue == other.rowsValue);
                Matrix result(rowsValue, other.colsValue, Type(0));
                for (size_t i = 0; i < rowsValue; ++i) {
                    for (size_t k = 0; k < colsValue; ++k) {
                        Type temp = (*this)[i][k];
                        if (temp == Type(0)) continue;
                        for (size_t j = 0; j < other.colsValue; ++j) {
                            result[i][j] = result[i][j] + temp * other[k][j];
                        }
                    }
                }
                return result;
            }

            Matrix& operator*=(const Matrix& other) { return *this = *this * other; }

            Matrix& operator*=(const Type& scalar) {
                for (size_t i = 0; i < data.size(); ++i) data[i] = data[i] * scalar;
                return *this;
            }
            Matrix operator*(const Type& scalar) const { return Matrix(*this) *= scalar; }

            Matrix& operator/=(const Type& scalar) {
                for (size_t i = 0; i < data.size(); ++i) data[i] = data[i] / scalar;
                return *this;
            }
            Matrix operator/(const Type& scalar) const { return Matrix(*this) /= scalar; }

            [[nodiscard]] Matrix power(long long exponent) const {
                assert(rowsValue == colsValue);
                Matrix result = identity(rowsValue);
                Matrix base = *this;
                while (exponent > 0) {
                    if (exponent & 1) result *= base;
                    base *= base;
                    exponent >>= 1;
                }
                return result;
            }

            [[nodiscard]] Matrix transpose() const {
                Matrix result(colsValue, rowsValue);
                for (size_t i = 0; i < rowsValue; ++i) {
                    for (size_t j = 0; j < colsValue; ++j) {
                        result[j][i] = (*this)[i][j];
                    }
                }
                return result;
            }

            [[nodiscard]] Type determinant() const {
                assert(rowsValue == colsValue);
                Matrix mat = *this;
                Type det = Type(1);
                size_t n = rowsValue;

                for (size_t i = 0; i < n; ++i) {
                    size_t pivot = i;
                    for (size_t j = i + 1; j < n; ++j) {
                        if (mat[j][i] != Type(0)) {
                            pivot = j;
                            break;
                        }
                    }
                    if (mat[pivot][i] == Type(0)) return Type(0);
                    
                    if (pivot != i) {
                        for (size_t j = i; j < n; ++j) std::swap(mat[i][j], mat[pivot][j]);
                        det = Type(0) - det;
                    }
                    
                    det = det * mat[i][i];
                    Type inversePivot = Type(1) / mat[i][i];
                    for (size_t j = i + 1; j < n; ++j) {
                        if (mat[j][i] != Type(0)) {
                            Type factor = mat[j][i] * inversePivot;
                            for (size_t k = i; k < n; ++k) {
                                mat[j][k] = mat[j][k] - factor * mat[i][k];
                            }
                        }
                    }
                }
                return det;
            }

            [[nodiscard]] Matrix inverse(bool* possible = nullptr) const {
                assert(rowsValue == colsValue);
                size_t n = rowsValue;
                Matrix mat = *this;
                Matrix res = identity(n);

                for (size_t i = 0; i < n; ++i) {
                    size_t pivot = i;
                    for (size_t j = i + 1; j < n; ++j) {
                        if (mat[j][i] != Type(0)) {
                            pivot = j;
                            break;
                        }
                    }
                    if (mat[pivot][i] == Type(0)) {
                        if (possible) *possible = false;
                        return res; // Singular matrix
                    }
                    if (pivot != i) {
                        for (size_t j = 0; j < n; ++j) {
                            std::swap(mat[i][j], mat[pivot][j]);
                            std::swap(res[i][j], res[pivot][j]);
                        }
                    }
                    
                    Type inversePivot = Type(1) / mat[i][i];
                    for (size_t j = 0; j < n; ++j) {
                        mat[i][j] = mat[i][j] * inversePivot;
                        res[i][j] = res[i][j] * inversePivot;
                    }
                    for (size_t j = 0; j < n; ++j) {
                        if (i != j && mat[j][i] != Type(0)) {
                            Type factor = mat[j][i];
                            for (size_t k = 0; k < n; ++k) {
                                mat[j][k] = mat[j][k] - factor * mat[i][k];
                                res[j][k] = res[j][k] - factor * res[i][k];
                            }
                        }
                    }
                }
                if (possible) *possible = true;
                return res;
            }

            core::FastVector<Type> solve(const core::FastVector<Type>& b) const {
                assert(rowsValue == colsValue && rowsValue == b.size());
                size_t n = rowsValue;
                Matrix mat = *this;
                core::FastVector<Type> res = b;

                for (size_t i = 0; i < n; ++i) {
                    size_t pivot = i;
                    for (size_t j = i + 1; j < n; ++j) {
                        if (mat[j][i] != Type(0)) {
                            pivot = j;
                            break;
                        }
                    }
                    if (mat[pivot][i] == Type(0)) return {};
                    
                    if (pivot != i) {
                        for (size_t j = i; j < n; ++j) std::swap(mat[i][j], mat[pivot][j]);
                        std::swap(res[i], res[pivot]);
                    }
                    
                    Type inversePivot = Type(1) / mat[i][i];
                    for (size_t j = i + 1; j < n; ++j) {
                        if (mat[j][i] != Type(0)) {
                            Type factor = mat[j][i] * inversePivot;
                            for (size_t k = i; k < n; ++k) {
                                mat[j][k] = mat[j][k] - factor * mat[i][k];
                            }
                            res[j] = res[j] - factor * res[i];
                        }
                    }
                }
                
                for (size_t i = n; i-- > 0;) {
                    for (size_t j = i + 1; j < n; ++j) {
                        res[i] = res[i] - mat[i][j] * res[j];
                    }
                    res[i] = res[i] / mat[i][i];
                }
                return res;
            }
        };

        template<typename Type>
        inline io::Printer& operator<<(io::Printer& printer, const Matrix<Type>& matrix) {
            for (size_t r = 0; r < matrix.rows(); ++r) {
                for (size_t c = 0; c < matrix.cols(); ++c) {
                    printer << matrix[r][c];
                    if (c + 1 < matrix.cols()) printer << ' ';
                }
                if (r + 1 < matrix.rows()) printer << '\n';
            }
            return printer;
        }

        template<typename Type>
        inline io::Scanner& operator>>(io::Scanner& scanner, Matrix<Type>& matrix) {
            for (size_t i = 0; i < matrix.data.size(); ++i) {
                scanner >> matrix.data[i];
            }
            return scanner;
        }

    }
}
