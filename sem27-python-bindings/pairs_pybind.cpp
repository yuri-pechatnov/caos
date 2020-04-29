// %%cpp pairs_pybind.cpp

#include <vector>
#include <algorithm>
#include <sstream>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h> // неявные преобразования python-объектов и стандартных C++ классов

struct TPairs {
    std::vector<std::pair<int, float>> Vector;
};

inline void SortPairs(TPairs& pairs) {
    std::sort(pairs.Vector.begin(), pairs.Vector.end());
}

inline void AppendPairs(TPairs& pairs, const TPairs& other) {
    pairs.Vector.insert(pairs.Vector.end(), other.Vector.begin(), other.Vector.end());
}

// -------------------

namespace py = pybind11;

PYBIND11_MODULE(pairs_pybind, m) {
    py::class_<TPairs>(m, "Pairs")
        .def(py::init<std::vector<std::pair<int, float>>>(), 
             "Class constructor", py::arg("vector") = std::vector<std::pair<int, float>>{}) 
        .def("sorted", [](TPairs pairs) { SortPairs(pairs); return pairs; })
        .def("__add__", [](TPairs a, const TPairs& b) { AppendPairs(a, b); return a; })
        .def("__repr__", [](const TPairs& p) { 
            std::stringstream ss;
            ss << "[";
            for (auto pair : p.Vector) { ss << "(" << pair.first << "," << pair.second << "),"; }
            ss << "]";
            return ss.str(); 
        })
        .def_readwrite("Vector", &TPairs::Vector) ; 
};

