#ifndef _PAIR_H
#define _PAIR_H

template<typename T, typename S>
struct Pair {
    Pair(T first, S second) : first(first), second(second) {}
    
    T first;
    S second;
};

template<typename T, typename S>
Pair<T, S> make_pair(T first, S second) { return Pair<T, S>(first, second); }

#endif
