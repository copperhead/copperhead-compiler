#pragma once

#include <boost/variant.hpp>
#include <functional>

namespace backend
{

class number;
class name;
class apply;
class lambda;
class closure;
class conditional;
class tuple;
class ret;
class bind;
class procedure;


namespace detail
{

typedef boost::variant<
    number &,
    name &,
    apply &,
    lambda &,
    closure &,
    conditional &,
    tuple &,
    ret &,
    bind &,
    procedure &
    > node_base;

struct make_node_base_visitor
    : boost::static_visitor<node_base>
{
    make_node_base_visitor(void *p)
        : ptr(p)
        {}

    template<typename Derived>
    node_base operator()(const Derived &) const {
        // use of std::ref disambiguates variant's copy constructor dispatch
        return node_base(std::ref(*reinterpret_cast<Derived*>(ptr)));
    }

    void* ptr;
};

node_base make_node_base(void *ptr, const node_base &other) {
    return boost::apply_visitor(make_node_base_visitor(ptr), other);
}

} // end detail

class node
    : public detail::node_base
{
protected:
    typedef detail::node_base super_t;

    template<typename Derived>
    node(Derived &self)
        : super_t(std::ref(self)) // use of std::ref disambiguates variant's copy constructor dispatch
        {}

    //copy constructor requires special handling
    node(const node &other)
        : super_t(detail::make_node_base(this, other))
        {}

};

}