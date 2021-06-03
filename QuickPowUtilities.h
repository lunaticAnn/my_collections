#include <assert.h>
#include <iostream>
#include <string.h>

inline long long quickPow(long long x, long long n)
{
    assert(n >= 0);
    long long res = 1;
    while (n > 0)
    {
        if (n & 1)
        {
            res *= x;
        }
        x *= x;
        n >>= 1;
    }
    return res;
}

inline long long quickPowMod(long long x, long long n, long long mod = 2)
{
    assert(mod > 1);
    long long res = 1;
    while (n > 0)
    {
        if (n & 1)
        {
            res *= x;
            res %= mod;
        }
        x *= x;
        x %= mod;
        n >>= 1;
    }
    return res;
}

// @ get the MMI of a number with mod of the target prime
// The modulo multiplicative inverse ( MMI ) of a number y is z iff (z * y) % M == 1.
// And according to Fermat's little theorem, that if p is a prime number, for any integer a, if a % p != 0, then a ^ (p - 1) % p == 1
// so in this case MMI is (a ^ (p - 2)) % p
inline long long getMMI(long long a, long long p = 2)
{
    return quickPowMod(a, p - 2, p);
}

// general extension of quick pow
// The Russian Peasant Algorithm : http://lafstern.org/matt/col3.pdf
// To use this template, T must properly implement the following:
// - T::operator=() for assignment
// - T::operator*() for multiplication
// - T::unit() for the unit element that starts multiplication with.
template <typename T>
T quickPowType(T base, long long n)
{
    T res = T::unit();
    while (n > 0)
    {
        if ( n & 1 )
        {
            res = res * base;
        }
        base = base * base;
        n >>= 1;
    }
    return res;
}

typedef long long MatrixElement;
struct matrix
{
    enum {dimension = 2};
    MatrixElement m[dimension][dimension];
    matrix()
    {
        memset(m, 0, sizeof(MatrixElement) * dimension * dimension);
    }
    
    matrix(const matrix& other)
    {
        memcpy(&m, &other.m, sizeof(MatrixElement) * dimension * dimension);
    }
    
    // The followings are not strictly needed, while could invoke some opt in certain compiler
    /*
    matrix(const matrix&& other) noexcept
    {
        memcpy(m, other.m, sizeof(MatrixElement) * dimension * dimension);
    }
    
    const matrix& operator=(const matrix& other)
    {
        // Guard self assignment
        if (this == &other)
            return *this;

        memcpy(m, other.m, sizeof(MatrixElement) * dimension * dimension);
        return *this;
    }
    
    const matrix& operator=(matrix&& other)
    {
        // Guard self assignment
        if (this == &other)
            return *this;

        memcpy(m, other.m, sizeof(MatrixElement) * dimension * dimension);
        return *this;
    }
    */

    static const matrix unit()
    {
        matrix m;
        for(int i = 0; i < dimension; ++i)
        {
            m.m[i][i] = 1;
        }
        return m;
    }

    matrix operator*(const matrix& other) const
    {
        matrix result;
        for (int i = 0; i < dimension; ++i)
        {
            for(int j = 0; j < dimension; ++j)
            {
                for(int k = 0; k < dimension; ++k)
                    result.m[i][j] = result.m[i][j] + m[i][k] * other.m[k][j];
            }
        }
        return result;
    }

    friend std::ostream& operator<<(std::ostream& os, const matrix& mat)
    {
        for (int i = 0; i < dimension; ++i)
        {
            for (int j = 0; j < dimension; ++j)
            {
                os<<mat.m[i][j]<<" ";
            }
            os<<std::endl;
        }
        return os;
    }
};

// quickly getting large Finonacci
// https://en.wikipedia.org/wiki/Fibonacci_number#Matrix_form
long long Fibonacci(int a, int b, int n)
{
    matrix base;
    base.m[0][0] = base.m[0][1] = base.m[1][0] = 1;
    matrix res = quickPowType<matrix>(m, n);
    return res.m[1][0] * b + res.m[1][1] * a;
}
