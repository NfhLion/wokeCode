/**
 * 从Google开源代码借用的字符串参数传递类型
 * 
 * 为什么要有StringPiece这个类
 * 1. StringPiece类是google提供的一个类。在网络传输中，如果传递一个很大的char*类型的字符串，那么这个很大的内存块需要在网络上进行传输；
 *    这么一块字符串虽然小，但是对于网络上高性能传输上，小小的因素都将会带来网络负载的加重，因此，使用StringPiece类对string/char*进行封装，
 *    封装成const char* ptr_ + int length，并且ptr始终是一个指针，ptr的赋值不是通过strcpy进行拷贝，而仅仅是改变ptr的指针。
 *    这样StringPiece类的大小始终都是const char* ptr_ + int length，大小为4字节+4字节。
 * 
 * 2. StringPiece类使用的场景：目的仅仅是读取字符串的值，所以这个类的目的是传入字符串的字面值，
 *    它内部的ptr_ 这块内存不归它所有，所以不能做任何改动。
 * 
 * 3. 归根结底，是处于性能的考虑，用以实现高效的字符串传递，这里既可以用const char*，也可以用std::string类型作为参数，并且不涉及内存拷贝。
 * 
 */

#ifndef MYMUDUO_BASE_STRINGPIECE_H
#define MYMUDUO_BASE_STRINGPIECE_H

#include <string.h>
#include <iosfwd>

#include "Types.h"

namespace muduo {
// For passing C-style string argument to a function.
class StringArg {
public:
    StringArg(const char* str) 
        : str_(str) {}
    StringArg(const string& str)
        : str_(str.c_str()) {}
    
    const char* c_str() const { return str_; }
private:
    const char* str_;
};

class StringPiece {
public:
    StringPiece()
        : ptr_(NULL), length_(0) {}
    StringPiece(const char* str)
        : ptr_(str), length_(static_cast<int>(strlen(ptr_))) {}
    StringPiece(const unsigned char* str)
        : ptr_(reinterpret_cast<const char*>(str)), length_(static_cast<int>(strlen(ptr_))) {}
    StringPiece(const char* offset, int len)
        : ptr_(offset), length_(len) {}

    const char* data() const { return ptr_; }
    int size() const { return length_; }
    bool empty() const { return length_ == 0; }
    const char* begin() const { return ptr_; }
    const char* end() const { return ptr_ + length_; }

    void clear() { ptr_ = NULL; length_ = 0; }
    void set(const char* buffer, int len) { ptr_ = buffer; length_ = len; }
    void set(const char* buffer) {
        ptr_ = buffer;
        length_ = static_cast<int>(strlen(ptr_));
    }
    void set(const void* buffer, int len) {
        ptr_ = reinterpret_cast<const char*>(buffer);
        length_ = len;
    }

    char operator[](int i) { return  ptr_[i];}

    void remove_prefix(int n) {
        ptr_ += n;
        length_ -= n;
    }
    void remove_suffix(int n) {
        length_ -= n;
    }

    bool operator ==(const StringPiece& x) const {
        return ((length_ == x.length_) && 
                (memcmp(ptr_, x.ptr_, length_) == 0));
    }
    bool operator !=(const StringPiece& x) const {
        return !(*this == x);
    }

/**
 * 重载了< 、<=、 >= 、>这些运算符。这些运算符实现起来大同小异，所以通过一个宏STRINGPIECE_BINARY_PREDICATE来实现
 * STRINGPIECE_BINARY_PREDICATE有两个参数，第二个是辅助比较运算符
 * 比如”abcd” < “abcdefg”, memcp比较它们的前四个字节，得到的r的值是0，
 * 很明显”adbcd”是小于”abcdefg”但由return后面的运算返回的结果为true。
 * 又比如”abcdx” < “abcdefg”, memcp比较它们的前5个字节，r的值为大于0，
 * 显然，((r < 0) || ((r == 0) && (length_ < x.length_)))得到的结果为false.
 */

#define STRINGPIECE_BINARY_PREDICATE(cmp,anxcmp) \
    bool operator cmp(const StringPiece& x)const{\
        int r=memcmp(ptr_,x.ptr_,length_<x.length_?length_:x.length_);\
        return ((r anxcmp 0)||((r==0)&&(length_ cmp x.length_)));\
    }
    STRINGPIECE_BINARY_PREDICATE(<,  <);
    STRINGPIECE_BINARY_PREDICATE(<=, <);
    STRINGPIECE_BINARY_PREDICATE(>=, >);
    STRINGPIECE_BINARY_PREDICATE(>,  >);
// #undef这个预处理指令表示不希望下面的代码在用的这个宏，#undef就是取消一个宏的定义，之后这个宏所定义的就无效；
#undef STRINGPIECE_BINARY_PREDICATE 

    int compare(const StringPiece& x) const {
        int r = memcmp(ptr_, x.ptr_, length_ < x.length_ ? length_ : x.length_);
        if (r == 0) {
            if (length_ < x.length_) r = -1;
            else if (length_ > x.length_) r = +1;
        }
        return r;
    }

    string as_string() const {
        return string(data(), size());
    }

    /**
     * string& assign ( const char* s, size_t n );
     * 将字符数组或者字符串的首n个字符替换原字符串内容
     */
    void CopyToString(string* target) const {
        target->assign(ptr_, length_);
    }

    // Does "this" start with "x"
    bool starts_with(const StringPiece& x) const {
        return ((length_ >= x.length_) && (memcmp(ptr_, x.ptr_, x.length_) == 0));
    }

private:
    const char* ptr_;
    int length_;
};

} // namespace muduo

// allow StringPiece to be logged
std::ostream& operator<<(std::ostream& o, const muduo::StringPiece& piece);

#endif