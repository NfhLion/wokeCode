#ifndef MYMUDUO_TYPES_H
#define MYMUDUO_TYPES_H

#include <stdint.h>
#include <string.h> // memset
#include <string>

#ifndef NDEBUG
#   include <assert.h>
#endif

namespace muduo {

using std::string;

inline void memZero(void* p, size_t n) {
    memset(p, 0, n);
}
/**
 * 用于在继承关系中， 子类指针转化为父类指针；隐式转换
 * inferred 推断，因为 from type 可以被推断出来，所以使用方法和 static_cast 相同
 * 在up_cast时应该使用implicit_cast替换static_cast,因为前者比后者要安全。
 * 以一个例子说明,在菱形继承中，static_cast把最底层的对象可以转换为中层对象，这样编译可以通过，但是在运行时可能崩溃
 * implicit_cast就不会有这个问题，在编译时就会给出错误信息
 */
template<typename To,typename From>
inline To implicit_cast(From const &f) {
    return f;
}

template<typename To,typename From>
inline To down_cast(From *f) {// so we only accept pointers
    /**
     * down_cast在debug模式下内部使用了dynamic_cast进行验证，在release下使用static_cast替换dynamic_cast。
     * 为什么使用down_cast而不直接使用dynamic_cast?
     * 1.因为但凡程序设计正确，dynamic_cast就可以用static_cast来替换，而后者比前者更有效率。
     * 2.dynamic_cast可能失败(在运行时crash)，运行时RTTI不是好的设计，不应该在运行时RTTI或者需要RTTI时一般都有更好的选择。
     */
#if !defined(NDEBUG) && !defined(GOOGLE_PROTOBUF_NO_RTTI)
    assert(f == NULL || dynamic_cast<To>(f) != NULL);  // RTTI: debug mode only!
#endif
    return static_cast<To>(f);
}

} // namespace muduo

#endif // MYMUDUO_TYPES_H