#include <boost/python.hpp>
#include <boost/python/implicit.hpp>
#include <boost/python/extract.hpp>
#include <boost/python/make_constructor.hpp>
#include <boost/variant.hpp>
#include <boost/python/copy_const_reference.hpp>
#include <boost/python/return_value_policy.hpp>
#include "type_printer.hpp"

namespace backend {
template<class T>
T* get_pointer(std::shared_ptr<T> const &p) {
    return p.get();
}


template<class T, class P=repr_type_printer>
const std::string to_string(std::shared_ptr<T> &p) {
    std::ostringstream os;
    P pp(os);
    pp(*p);
    return os.str();
}

template<class T, class P=repr_type_printer>
const std::string to_string_apply(std::shared_ptr<T> &p) {
    std::ostringstream os;
    P pp(os);
    boost::apply_visitor(pp, *p);
    return os.str();
}

template<class T>
const std::string repr(std::shared_ptr<T> &p) {
    return to_string<T, repr_type_printer>(p);
}
template<class T>
const std::string repr_apply(std::shared_ptr<T> &p) {
    return to_string_apply<T, repr_type_printer>(p);
}

}

using namespace boost::python;
using namespace backend;

template<typename S, typename T>
static std::shared_ptr<T> make_from_list(list vals) {
    std::vector<std::shared_ptr<S> > values;
    boost::python::ssize_t n = len(vals);
    for(boost::python::ssize_t i=0; i<n; i++) {
        object elem = vals[i];
        std::shared_ptr<S> p_elem = extract<std::shared_ptr<S> >(elem);
        values.push_back(p_elem);
    }
    auto result = std::shared_ptr<T>(new T(std::move(values)));
    return result;
}

BOOST_PYTHON_MODULE(core_syntax) {
    class_<type_t, std::shared_ptr<type_t>, boost::noncopyable >("Type", no_init)
        .def("__repr__", &backend::repr_apply<type_t>);
    class_<monotype_t, std::shared_ptr<monotype_t>, bases<type_t>, boost::noncopyable >("Monotype", no_init)
        .def("__repr__", &backend::repr<monotype_t>);
    class_<int32_mt, std::shared_ptr<int32_mt>, bases<monotype_t, type_t> >("Int32", init<>())
        .def("__repr__", &backend::repr<int32_mt>);
    class_<int64_mt, std::shared_ptr<int64_mt>, bases<monotype_t, type_t> >("Int64", init<>())
        .def("__repr__", &backend::repr<int32_mt>);
    class_<uint32_mt, std::shared_ptr<uint32_mt>, bases<monotype_t, type_t> >("Uint32", init<>())
        .def("__repr__", &backend::repr<int32_mt>);
    class_<uint64_mt, std::shared_ptr<uint64_mt>, bases<monotype_t, type_t> >("Uint64", init<>())
        .def("__repr__", &backend::repr<int32_mt>);
    class_<float32_mt, std::shared_ptr<float32_mt>, bases<monotype_t, type_t> >("Float32", init<>())
        .def("__repr__", &backend::repr<int32_mt>);
    class_<float64_mt, std::shared_ptr<float64_mt>, bases<monotype_t, type_t> >("Float64", init<>())
        .def("__repr__", &backend::repr<int32_mt>);
    class_<bool_mt, std::shared_ptr<bool_mt>, bases<monotype_t, type_t> >("Bool", init<>())
        .def("__repr__", &backend::repr<int32_mt>);
    class_<void_mt, std::shared_ptr<void_mt>, bases<monotype_t, type_t> >("Void", init<>())
        .def("__repr__", &backend::repr<int32_mt>);
    class_<sequence_t, std::shared_ptr<sequence_t>, bases<monotype_t, type_t> >("Sequence", init<std::shared_ptr<type_t> >())
        .def("__repr__", &backend::repr<sequence_t>);
}
