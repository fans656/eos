#ifndef _ALGORITHM_H
#define _ALGORITHM_H

template<typename T>
void swap(T& x, T& y) {
    T tmp(x);
    x = y;
    y = tmp;
}

#endif
