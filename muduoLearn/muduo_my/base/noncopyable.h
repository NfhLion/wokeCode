/**
 *  Created time is 2022-02-08
 */

#ifndef MYMUDUO_BASE_NONCOPYABLE_H
#define MYMUDUO_BASE_NONCOPYABLE_H

namespace muduo {

class noncopyable {
public:
    noncopyable(const noncopyable&) = delete;
    void operator=(const noncopyable&) = delete;
protected:
    noncopyable() = default;
    ~noncopyable() = default;
};

} // namespace muduo

#endif // MYMUDUO_BASE_NONCOPYABLE_H