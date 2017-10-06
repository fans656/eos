#ifndef ITERATOR_H
#define ITERATOR_H

namespace std {

template<class C> auto begin(C& c) -> decltype(c.begin()) { return c.begin(); }
template<class C> auto end(C& c) -> decltype(c.end()) { return c.end(); }
template<class C> auto rbegin(C& c) -> decltype(c.rbegin()) { return c.rbegin(); }
template<class C> auto rend(C& c) -> decltype(c.rend()) { return c.rend(); }

}

#endif
