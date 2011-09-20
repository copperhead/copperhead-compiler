#include <boost/python.hpp>
#include <boost/python/implicit.hpp>
#include <boost/python/extract.hpp>
#include <boost/python/make_constructor.hpp>
#include <boost/variant.hpp>
#include <boost/python/copy_const_reference.hpp>
#include <boost/python/return_value_policy.hpp>
#include "node.hpp"
#include "wrappers.hpp"
#include "expression.hpp"
#include "py_printer.hpp"
#include "repr_printer.hpp"
#include "compiler.hpp"
#include "cuda_printer.hpp"
#include "type_printer.hpp"

#include <sstream>
#include <iostream>

namespace backend {
template<class T>
T* get_pointer(std::shared_ptr<T> const &p) {
    return p.get();
}


template<class T, class P=py_printer>
const std::string to_string(std::shared_ptr<T> &p) {
    std::ostringstream os;
    P pp(os);
    pp(*p);
    return os.str();
}

template<class T, class P=py_printer>
const std::string to_string_apply(std::shared_ptr<T> &p) {
    std::ostringstream os;
    P pp(os);
    boost::apply_visitor(pp, *p);
    return os.str();
}


template<class T>
const std::string str(std::shared_ptr<T> &p) {
    return to_string<T, py_printer>(p);
}
    
template<class T>
const std::string repr(std::shared_ptr<T> &p) {
    return to_string<T, repr_printer>(p);
}

template<class T>
const std::string repr_type(std::shared_ptr<T> &p) {
    return to_string<T, repr_type_printer>(p);
}

template<class T>
const std::string str_apply(std::shared_ptr<T> &p) {
    return to_string_apply<T, py_printer>(p);
}

template<class T>
const std::string repr_apply(std::shared_ptr<T> &p) {
    return to_string_apply<T, repr_printer>(p);
}

template<class T>
const std::string repr_type_apply(std::shared_ptr<T> &p) {
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


std::string compile(std::shared_ptr<compiler> &c,
                               std::shared_ptr<suite_wrap> &s) {
    std::shared_ptr<suite> rewritten = c->operator()(*s);
    std::string entry_point = c->entry_point();
    std::ostringstream os;
    cuda_printer p(entry_point, os);
    p(*rewritten);
    return os.str();
}

BOOST_PYTHON_MODULE(bindings) {
    class_<node, std::shared_ptr<node>, boost::noncopyable>("Node", no_init)
        .def("__str__", &backend::str_apply<node>)
        .def("__repr__", &backend::repr_apply<node>);
    class_<expression, std::shared_ptr<expression>, bases<node>, boost::noncopyable>("Expression", no_init)
        .def("__str__", &backend::str_apply<expression>)
        .def("__repr__", &backend::repr_apply<expression>);
    class_<literal, std::shared_ptr<literal>, bases<expression, node>, boost::noncopyable>("Literal", no_init)
        .def("__str__", &backend::str_apply<literal>)
        .def("__repr__", &backend::repr_apply<literal>);
    class_<name, std::shared_ptr<name>, bases<literal, expression, node> >("Name", init<std::string>())
        .def("id", &name::id)
        .def("__str__", &backend::str<name>)
        .def("__repr__", &backend::repr<name>);  
    class_<number, std::shared_ptr<number>, bases<literal, expression, node> >("Number", init<std::string>())
        .def("val", &number::val)
        .def("__str__", &backend::str<number>)
        .def("__repr__", &backend::repr<number>);
    class_<backend::tuple_wrap, std::shared_ptr<backend::tuple_wrap>, bases<expression, node> >("Tuple")
        .def("__init__", make_constructor(make_from_list<expression, backend::tuple_wrap>))
        .def("__iter__", range(&tuple_wrap::p_begin, &tuple_wrap::p_end))
        .def("__str__", &backend::str<backend::tuple_wrap>)
        .def("__repr__", &backend::repr<backend::tuple_wrap>);
    class_<apply_wrap, std::shared_ptr<apply_wrap>, bases<expression, node> >("Apply", init<std::shared_ptr<name>, std::shared_ptr<backend::tuple> >())
        .def("fn", &apply_wrap::p_fn) 
        .def("args", &apply_wrap::p_args)
        .def("__str__", &backend::str<apply_wrap>)
        .def("__repr__", &backend::repr<apply_wrap>);
    class_<lambda_wrap, std::shared_ptr<lambda_wrap>, bases<expression, node> >("Lambda", init<std::shared_ptr<backend::tuple>, std::shared_ptr<expression> >())
        .def("args", &lambda_wrap::p_args)
        .def("body", &lambda_wrap::p_body)
        .def("__str__", &backend::str<lambda_wrap>)
        .def("__repr__", &backend::repr<lambda_wrap>);
    class_<statement, std::shared_ptr<statement>, bases<node>, boost::noncopyable>("Statement", no_init)
        .def("__str__", &backend::str_apply<statement>)
        .def("__repr__", &backend::repr_apply<statement>);
    class_<ret_wrap, std::shared_ptr<ret_wrap>, bases<statement, node> >("Return", init<std::shared_ptr<expression> >())
        .def("val", &ret_wrap::p_val)
        .def("__str__", &backend::str<ret_wrap>)
        .def("__repr__", &backend::repr<ret_wrap>);
    class_<bind_wrap, std::shared_ptr<bind_wrap>, bases<statement, node> >("Bind", init<std::shared_ptr<expression>, std::shared_ptr<expression> >())
        .def("lhs", &bind_wrap::p_lhs)
        .def("rhs", &bind_wrap::p_rhs)
        .def("__str__", &backend::str<bind_wrap>)
        .def("__repr__", &backend::repr<bind_wrap>);
    class_<procedure_wrap, std::shared_ptr<procedure_wrap>, bases<statement, node> >("Procedure", init<std::shared_ptr<name>, std::shared_ptr<backend::tuple>, std::shared_ptr<suite> >())
        .def("id", &procedure_wrap::p_id)
        .def("args", &procedure_wrap::p_args)
        .def("stmts", &procedure_wrap::p_stmts)
        .def("__str__", &backend::str<procedure_wrap>)
        .def("__repr__", &backend::repr<procedure_wrap>);
    class_<suite_wrap, std::shared_ptr<suite_wrap>, bases<node> >("Suite")
        .def("__init__", make_constructor(make_from_list<statement, suite_wrap>))
        .def("__iter__", range(&suite_wrap::p_begin, &suite_wrap::p_end))
        .def("__str__", &backend::str<backend::suite_wrap>)
        .def("__repr__", &backend::repr<backend::suite_wrap>);
    class_<structure_wrap, std::shared_ptr<structure_wrap>, bases<statement, node> >("Structure", init<std::shared_ptr<name>, std::shared_ptr<suite> >())
        .def("id", &structure_wrap::p_id)
        .def("stmts", &structure_wrap::p_stmts)
        .def("__str__", &backend::str<structure_wrap>)
        .def("__repr__", &backend::repr<structure_wrap>);

    class_<compiler, std::shared_ptr<compiler> >("Compiler", init<std::string>())
        .def("__call__", &compile);

    class_<type_t, std::shared_ptr<type_t>, boost::noncopyable >("Type", no_init)
        .def("__repr__", &backend::repr_type_apply<type_t>);
    class_<monotype_t, std::shared_ptr<monotype_t>, bases<type_t>, boost::noncopyable >("Monotype", no_init)
        .def("__repr__", &backend::repr_type<monotype_t>);
    class_<int32_mt, std::shared_ptr<int32_mt>, bases<monotype_t, type_t> >("Int32", init<>())
        .def("__repr__", &backend::repr_type<int32_mt>);
    class_<int64_mt, std::shared_ptr<int64_mt>, bases<monotype_t, type_t> >("Int64", init<>())
        .def("__repr__", &backend::repr_type<int32_mt>);
    class_<uint32_mt, std::shared_ptr<uint32_mt>, bases<monotype_t, type_t> >("Uint32", init<>())
        .def("__repr__", &backend::repr_type<int32_mt>);
    class_<uint64_mt, std::shared_ptr<uint64_mt>, bases<monotype_t, type_t> >("Uint64", init<>())
        .def("__repr__", &backend::repr_type<int32_mt>);
    class_<float32_mt, std::shared_ptr<float32_mt>, bases<monotype_t, type_t> >("Float32", init<>())
        .def("__repr__", &backend::repr_type<int32_mt>);
    class_<float64_mt, std::shared_ptr<float64_mt>, bases<monotype_t, type_t> >("Float64", init<>())
        .def("__repr__", &backend::repr_type<int32_mt>);
    class_<bool_mt, std::shared_ptr<bool_mt>, bases<monotype_t, type_t> >("Bool", init<>())
        .def("__repr__", &backend::repr_type<int32_mt>);
    class_<void_mt, std::shared_ptr<void_mt>, bases<monotype_t, type_t> >("Void", init<>())
        .def("__repr__", &backend::repr_type<int32_mt>);
    class_<sequence_t, std::shared_ptr<sequence_t>, bases<monotype_t, type_t> >("Sequence", init<std::shared_ptr<type_t> >())
        .def("__repr__", &backend::repr_type<sequence_t>);
    
    implicitly_convertible<std::shared_ptr<backend::expression>, std::shared_ptr<backend::node> >();
    implicitly_convertible<std::shared_ptr<backend::literal>, std::shared_ptr<backend::expression> >();
    implicitly_convertible<std::shared_ptr<backend::name>, std::shared_ptr<backend::node> >();
    implicitly_convertible<std::shared_ptr<backend::name>, std::shared_ptr<backend::expression> >();
    implicitly_convertible<std::shared_ptr<backend::name>, std::shared_ptr<backend::literal> >();
    implicitly_convertible<std::shared_ptr<backend::number>, std::shared_ptr<backend::node> >();
    implicitly_convertible<std::shared_ptr<backend::number>, std::shared_ptr<backend::expression> >();
    implicitly_convertible<std::shared_ptr<backend::number>, std::shared_ptr<backend::literal> >();
    implicitly_convertible<std::shared_ptr<backend::tuple>, std::shared_ptr<backend::node> >();
    implicitly_convertible<std::shared_ptr<backend::tuple>, std::shared_ptr<backend::expression> >();
    implicitly_convertible<std::shared_ptr<backend::tuple_wrap>, std::shared_ptr<backend::node> >();
    implicitly_convertible<std::shared_ptr<backend::tuple_wrap>, std::shared_ptr<backend::expression> >();
    implicitly_convertible<std::shared_ptr<backend::tuple_wrap>, std::shared_ptr<backend::tuple> >();
    implicitly_convertible<std::shared_ptr<backend::apply>, std::shared_ptr<backend::node> >();
    implicitly_convertible<std::shared_ptr<backend::apply>, std::shared_ptr<backend::expression> >();
    implicitly_convertible<std::shared_ptr<backend::apply_wrap>, std::shared_ptr<backend::node> >();
    implicitly_convertible<std::shared_ptr<backend::apply_wrap>, std::shared_ptr<backend::expression> >();
    implicitly_convertible<std::shared_ptr<backend::apply_wrap>, std::shared_ptr<backend::apply> >();
    implicitly_convertible<std::shared_ptr<backend::lambda>, std::shared_ptr<backend::node> >();
    implicitly_convertible<std::shared_ptr<backend::lambda>, std::shared_ptr<backend::expression> >();
    implicitly_convertible<std::shared_ptr<backend::lambda_wrap>, std::shared_ptr<backend::node> >();
    implicitly_convertible<std::shared_ptr<backend::lambda_wrap>, std::shared_ptr<backend::expression> >();
    implicitly_convertible<std::shared_ptr<backend::lambda_wrap>, std::shared_ptr<backend::lambda> >();
    implicitly_convertible<std::shared_ptr<backend::statement>, std::shared_ptr<backend::node> >();
    implicitly_convertible<std::shared_ptr<backend::ret>, std::shared_ptr<backend::statement> >();
    implicitly_convertible<std::shared_ptr<backend::ret>, std::shared_ptr<backend::node> >();
    implicitly_convertible<std::shared_ptr<backend::ret_wrap>, std::shared_ptr<backend::statement> >();
    implicitly_convertible<std::shared_ptr<backend::ret_wrap>, std::shared_ptr<backend::node> >();
    implicitly_convertible<std::shared_ptr<backend::ret_wrap>, std::shared_ptr<backend::ret> >();
    implicitly_convertible<std::shared_ptr<backend::bind>, std::shared_ptr<backend::statement> >();
    implicitly_convertible<std::shared_ptr<backend::bind>, std::shared_ptr<backend::node> >();
    implicitly_convertible<std::shared_ptr<backend::bind_wrap>, std::shared_ptr<backend::statement> >();
    implicitly_convertible<std::shared_ptr<backend::bind_wrap>, std::shared_ptr<backend::node> >();
    implicitly_convertible<std::shared_ptr<backend::bind_wrap>, std::shared_ptr<backend::bind> >();
    implicitly_convertible<std::shared_ptr<backend::procedure>, std::shared_ptr<backend::statement> >();
    implicitly_convertible<std::shared_ptr<backend::procedure>, std::shared_ptr<backend::node> >();
    implicitly_convertible<std::shared_ptr<backend::procedure_wrap>, std::shared_ptr<backend::statement> >();
    implicitly_convertible<std::shared_ptr<backend::procedure_wrap>, std::shared_ptr<backend::node> >();
    implicitly_convertible<std::shared_ptr<backend::procedure_wrap>, std::shared_ptr<backend::procedure> >();
    implicitly_convertible<std::shared_ptr<backend::suite>, std::shared_ptr<backend::node> >();
    implicitly_convertible<std::shared_ptr<backend::suite_wrap>, std::shared_ptr<backend::node> >();
    implicitly_convertible<std::shared_ptr<backend::suite_wrap>, std::shared_ptr<backend::suite> >();
    implicitly_convertible<std::shared_ptr<backend::structure>, std::shared_ptr<backend::node> >();
    implicitly_convertible<std::shared_ptr<backend::structure>, std::shared_ptr<backend::statement> >();
    implicitly_convertible<std::shared_ptr<backend::structure_wrap>, std::shared_ptr<backend::node> >();
    implicitly_convertible<std::shared_ptr<backend::structure_wrap>, std::shared_ptr<backend::statement> >();
    implicitly_convertible<std::shared_ptr<backend::structure_wrap>, std::shared_ptr<backend::structure> >();
    implicitly_convertible<std::shared_ptr<backend::monotype_t>, std::shared_ptr<backend::type_t> >();
    implicitly_convertible<std::shared_ptr<backend::int32_mt>, std::shared_ptr<backend::type_t> >();
    implicitly_convertible<std::shared_ptr<backend::int64_mt>, std::shared_ptr<backend::type_t> >();
    implicitly_convertible<std::shared_ptr<backend::uint32_mt>, std::shared_ptr<backend::type_t> >();
    implicitly_convertible<std::shared_ptr<backend::uint64_mt>, std::shared_ptr<backend::type_t> >();
    implicitly_convertible<std::shared_ptr<backend::float32_mt>, std::shared_ptr<backend::type_t> >();
    implicitly_convertible<std::shared_ptr<backend::float64_mt>, std::shared_ptr<backend::type_t> >();
    implicitly_convertible<std::shared_ptr<backend::bool_mt>, std::shared_ptr<backend::type_t> >();
    implicitly_convertible<std::shared_ptr<backend::void_mt>, std::shared_ptr<backend::type_t> >();
    implicitly_convertible<std::shared_ptr<backend::sequence_t>, std::shared_ptr<backend::type_t> >();
    implicitly_convertible<std::shared_ptr<backend::int32_mt>, std::shared_ptr<backend::monotype_t> >();
    implicitly_convertible<std::shared_ptr<backend::int64_mt>, std::shared_ptr<backend::monotype_t> >();
    implicitly_convertible<std::shared_ptr<backend::uint32_mt>, std::shared_ptr<backend::monotype_t> >();
    implicitly_convertible<std::shared_ptr<backend::uint64_mt>, std::shared_ptr<backend::monotype_t> >();
    implicitly_convertible<std::shared_ptr<backend::float32_mt>, std::shared_ptr<backend::monotype_t> >();
    implicitly_convertible<std::shared_ptr<backend::float64_mt>, std::shared_ptr<backend::monotype_t> >();
    implicitly_convertible<std::shared_ptr<backend::bool_mt>, std::shared_ptr<backend::monotype_t> >();
    implicitly_convertible<std::shared_ptr<backend::void_mt>, std::shared_ptr<backend::monotype_t> >();
    implicitly_convertible<std::shared_ptr<backend::sequence_t>, std::shared_ptr<backend::monotype_t> >();
}
