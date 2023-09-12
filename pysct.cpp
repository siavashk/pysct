#include <pybind11/pybind11.h>


class Matrix {
public:
    Matrix(size_t rows, size_t cols) : m_rows(rows), m_cols(cols) {
        m_data = new float[rows * cols];
    }
    float* data() { return m_data; }
    size_t rows() const { return m_rows; }
    size_t cols() const { return m_cols; }
private:
    size_t m_rows, m_cols;
    float* m_data;
};


namespace py = pybind11;

PYBIND11_MODULE(pysct, m)
{
    m.doc() = "pybind11 matrix example";

    py::class_<Matrix>(m, "Matrix")
        .def(py::init<size_t&, size_t&>())
        .def("data", &Matrix::data)
        .def("rows", &Matrix::rows)
        .def("cols", &Matrix::cols)
        .def("__repr__", [](const Matrix& m) {return "<example.Matrix>";});
}