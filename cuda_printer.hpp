#pragma once
#include <cassert>
#include <sstream>

#include "py_printer.hpp"
#include "environment.hpp"
#include "type_printer.hpp"

namespace backend
{


class cuda_printer
    : public py_printer
{
private:
    const std::string& entry;
    environment<std::string> declared;
    ctype::ctype_printer tp;
public:
    inline cuda_printer(const std::string &entry_point,
                        const registry& globals,
                        std::ostream &os)
        : py_printer(os), entry(entry_point), tp(os) {
        const std::map<ident, fn_info>& fns = globals.fns();
        for(auto i = fns.cbegin();
            i != fns.cend();
            i++) {
            declared.insert(i->first());
        }
    }
    
    using backend::py_printer::operator();

    //XXX Why do I need to qualify backend::name?
    //Am I hiding backend::name somewhere?
    //Or a compiler bug?
    inline void operator()(const backend::name &n) {
        if (!declared.exists(n.id())) {
            boost::apply_visitor(tp, n.ctype());
            m_os << " ";
            declared.insert(n.id());
        }
        m_os << n.id();
    }

    inline void operator()(const templated_name &n) {
        m_os << n.id();
        m_os << "<";
        detail::list(tp, n.template_types());
        m_os << ">";
    }
    
    inline void operator()(const number &n) {
        m_os << n.val();
    }

    inline void operator()(const tuple &n) {
        open();
        detail::list(*this, n);
        close();
    }

    inline void operator()(const apply &n) {
        boost::apply_visitor(*this, n.fn());
        (*this)(n.args());
    }
    inline void operator()(const closure &n) {
    }
    inline void operator()(const conditional &n) {
    }
    inline void operator()(const ret &n) {
        m_os << "return ";
        boost::apply_visitor(*this, n.val());
        m_os << ";";
    }
    inline void operator()(const bind &n) {
        boost::apply_visitor(*this, n.lhs());
        m_os << " = ";
        boost::apply_visitor(*this, n.rhs());
        m_os << ";";
    }
    inline void operator()(const call &n) {
        boost::apply_visitor(*this, n.sub());
        m_os << ";";
    }
    inline void operator()(const procedure &n) {
        const std::string& proc_id = n.id().id();
        declared.insert(proc_id);
        declared.begin_scope();
        bool is_entry = proc_id == entry;
        if (!is_entry) {
            m_os << "__device__ ";
        }
        const ctype::type_t &n_t = n.ctype();
        //If this procedure has a type, print the return type
        //Procedures with no types are only generated by the compiler
        //For things like the BOOST_PYTHON_MODULE declaration
        if (!detail::isinstance(ctype::void_mt, ctype::type_t)) {
            const ctype::fn_t &n_f = boost::get<const ctype::fn_t&>(n_t);
            const ctype::type_t& ret_t = n_f.result();
            boost::apply_visitor(tp, ret_t);
            m_os << " ";
        }
        (*this)(n.id());
        (*this)(n.args());
        m_os << " {" << std::endl;
        indent();
        (*this)(n.stmts());
        dedent();
        indentation();
        m_os << "}" << std::endl;
        declared.end_scope();
    }
    inline void operator()(const suite &n) {
        for(auto i = n.begin();
            i != n.end();
            i++) {
            indentation();
            boost::apply_visitor(*this, *i);
            m_os << std::endl;
        }
    }

    inline void operator()(const structure &n) {
        indentation();
        m_os << "struct ";
        (*this)(n.id());
        m_os << " {" << std::endl;
        indent();
        (*this)(n.stmts());
        dedent();
        m_os << "};" << std::endl;
    }
    inline void operator()(const std::string &s) {
        m_os << s;
    }
    template<typename T>
        inline void operator()(const std::vector<T> &v) {
        detail::list(*this, v);
    }
};


}
