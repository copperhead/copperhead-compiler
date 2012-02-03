#include "cudata.h"
#include <cuda_runtime.h>
#include <string.h>
#include <cassert>

typedef std::pair<void*, ssize_t> seq;
using std::make_pair;
using std::auto_ptr;


cuarray::cuarray() {
    l_d = NULL;
    r_d = NULL;
    s = 0;
    e = 0;
    n = 0;
    clean_local = true;
    clean_remote = true;
}

#include <iostream>
cuarray::~cuarray() {
    std::cout << "destroying cuarray " << l_d << " " << r_d << std::endl;
    if (l_d != NULL) {
        free(l_d);
        l_d = NULL;
    }
    if (r_d != NULL) {
        cudaFree(r_d);
        r_d = NULL;
    }
    std::cout << "  Successfully destroyed cuarray" << std::endl;
}

cuarray::cuarray(ssize_t _n, CUTYPE _t, bool host) {
    clean_local = host;
    clean_remote = !host;
    n = _n;
    t = _t;
    if (t == CUBOOL) {
        e = sizeof(bool);
    } else if (t == CUINT32) {
        e = sizeof(int);
    } else if (t == CUINT64) {
        e = sizeof(long);
    } else if (t == CUFLOAT32) {
        e = sizeof(float);
    } else if (t == CUFLOAT64) {
        e = sizeof(double);
    } else {
        e = 0;
    }
        
    s = e * n;
    if (clean_local) {
        l_d = malloc(s);
        r_d = NULL;
    } else {
        cudaMalloc(&r_d, s);
        l_d = NULL;
    }
    std::cout << "creating cuarray " << l_d << " " << r_d << std::endl;
    std::cout << "  " << "n: " << n << " e: " << e << " s: " << s << std::endl;
}

cuarray::cuarray(ssize_t _n, bool* l) {
    clean_local = true;
    clean_remote = false;
    n = _n;
    e = sizeof(bool);
    s = e * n;
    l_d = malloc(s);
    r_d = NULL;
    memcpy(l_d, l, s);
    t = CUBOOL;
}

cuarray::cuarray(ssize_t _n, int* l) {
    clean_local = true;
    clean_remote = false;
    n = _n;
    e = sizeof(int);
    s = e * n;
    l_d = malloc(s);
    r_d = NULL;
    memcpy(l_d, l, s);
    t = CUINT32;
}

cuarray::cuarray(ssize_t _n, long* l) {
    clean_local = true;
    clean_remote = false;
    n = _n;
    e = sizeof(long);
    s = e * n;
    l_d = malloc(s);
    r_d = NULL;
    memcpy(l_d, l, s);
    t = CUINT64;
}

cuarray::cuarray(ssize_t _n, float* l) {
    clean_local = true;
    clean_remote = false;
    n = _n;
    e = sizeof(float);
    s = e * n;
    l_d = malloc(s);
    r_d = NULL;
    memcpy(l_d, l, s);
    t = CUFLOAT32;
    std::cout << "creating cuarray " << l_d << " " << r_d << std::endl;
    std::cout << "  " << "n: " << n << " e: " << e << " s: " << s << std::endl;
}

cuarray::cuarray(ssize_t _n, double* l) {
    clean_local = true;
    clean_remote = false;
    n = _n;
    e = sizeof(float);
    s = e * n;
    l_d = malloc(s);
    r_d = NULL;
    memcpy(l_d, l, s);
    t = CUFLOAT64;
}


void cuarray::retrieve() {
    //Lazy data movement
    if (!clean_local) {
        assert(r_d != NULL);
        //Lazy allocation
        if (l_d == NULL) {
            l_d = malloc(s);
            std::cout << "allocating local space: cuarray " << l_d << " " << r_d << std::endl;     
            std::cout << "  " << "n: " << n << " e: " << e << " s: " << s << std::endl;
        }
        cudaMemcpy(l_d, r_d, s, cudaMemcpyDeviceToHost);
        clean_local = true;
    }
}

void cuarray::exile() {
    //Lazy data movement
    if (!clean_remote) {
        assert(l_d != NULL);
        //Lazy allocation
        if (r_d == NULL) {
            cudaMalloc(&r_d, s);
            std::cout << "allocating remote space: cuarray " << l_d << " " << r_d << std::endl;
            
            std::cout << "  " << "n: " << n << " e: " << e << " s: " << s << std::endl;
        }
        cudaMemcpy(r_d, l_d, s, cudaMemcpyHostToDevice);
        clean_remote = true;
    }
}

seq cuarray::get_local_r() {
    retrieve();
    return make_pair(l_d, n);
}

seq cuarray::get_local_w() {
    retrieve();
    clean_remote = false;
    return make_pair(l_d, n);
}

seq cuarray::get_remote_r() {
    exile();
    return make_pair(r_d, n);
}

seq cuarray::get_remote_w() {
    exile();
    clean_local = false;
    return make_pair(r_d, n);
}

template<typename T>
boost::shared_ptr<cuarray> make_remote(ssize_t in) {
    return boost::shared_ptr<cuarray>(new cuarray(0, CUVOID, false));
}

template<>
boost::shared_ptr<cuarray> make_remote<bool>(ssize_t in) {
    return boost::shared_ptr<cuarray>(new cuarray(in, CUBOOL, false));
}

template<>
boost::shared_ptr<cuarray> make_remote<int>(ssize_t in) {
    return boost::shared_ptr<cuarray>(new cuarray(in, CUINT32, false));
}

template<>
boost::shared_ptr<cuarray> make_remote<long>(ssize_t in) {
    return boost::shared_ptr<cuarray>(new cuarray(in, CUINT64, false));
}

template<>
boost::shared_ptr<cuarray> make_remote<float>(ssize_t in) {
    return boost::shared_ptr<cuarray>(new cuarray(in, CUFLOAT32, false));
}

template<>
boost::shared_ptr<cuarray> make_remote<double>(ssize_t in) {
    return boost::shared_ptr<cuarray>(new cuarray(in, CUFLOAT64, false));
}