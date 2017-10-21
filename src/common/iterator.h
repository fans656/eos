#ifndef _ITERATOR_H
#define _ITERATOR_H

///////////////////////////////////////////////////////////////////

// https://stackoverflow.com/a/28139075/3881776
template<class C> struct Reversed { C& c; };
template<class C> auto begin(Reversed<C> c) { return c.c.rbegin(); }
template<class C> auto end(Reversed<C> c) { return c.c.rend(); }
template<class C> Reversed<C> reversed(C& c) { return {c}; }

#endif
