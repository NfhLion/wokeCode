/**
 *  Created time is 2022-02-08 1:04
 */

/**
 * UTC时间戳
 */

#ifndef MYMUDUO_BASE_TIMESTAMP_H
#define MYMUDUO_BASE_TIMESTAMP_H

#include <boost/operators.hpp>

#include "copyable.h"
#include "Types.h"

namespace muduo {
/**
 * Timestamp类继承自boost::less_than_comparable <T>模板类
 * 只要实现 <，即可自动实现>,<=,>=
 */
class Timestamp : public muduo::copyable,
                  public boost::equality_comparable1<Timestamp>,
                  public boost::less_than_comparable1<Timestamp> {
public:
    explicit Timestamp(int64_t microSecondsSinceEpoch)
        : microSecondsSinceEpoch_(microSecondsSinceEpoch) {}

    void swap(Timestamp& that) {
        std::swap(microSecondsSinceEpoch_, that.microSecondsSinceEpoch_);
    }

    string toString() const; //将时间转换为string类型
    string toFormattedString(bool showMicroseconds = true) const; //将时间转换为固定格式的string类型

    bool valid() { //判断Timestamp是否有效
        return microSecondsSinceEpoch_ > 0;
    }

    int64_t microSecondsSinceEpoch() const { return microSecondsSinceEpoch_; }

    time_t secondsSinceEpoch() const {
        return static_cast<time_t>(microSecondsSinceEpoch_ / kMicroSecondPerSecond)
    }

    static Timestamp now(); //返回当前时间的Timestamp
    static Timestamp invalid() { //返回一个无效的Timestamp
        return Timestamp();
    }

    static Timestamp fromUnixTime(time_t t){
        return fromUnixTime(t, 0);
    }
    static Timestamp fromUnixTime(time_t t, int microseconds){
        return Timestamp(static_cast<int64_t>(t) * kMicroSecondPerSecond + microseconds);
    }

    static const int kMicroSecondPerSecond = 1000*1000; //每秒所对应的微秒数

private:
    int64_t microSecondsSinceEpoch_; //表示到1970-01-01 00:00:00 UTC的微秒数
};

inline bool operator<(const Timestamp& lhs, const Timestamp& rhs) {
    return lhs.microSecondsSinceEpoch() < rhs.microSecondsSinceEpoch();
}

inline bool operator==(const Timestamp& lhs, const Timestamp& rhs) {
    return lhs.microSecondsSinceEpoch() == rhs.microSecondsSinceEpoch();
}

inline double timeDifference(const Timestamp& hign, const Timestamp& low){
    int64_t diff = hign.microSecondsSinceEpoch() - low.microSecondsSinceEpoch();
    return static_cast<double>(diff) / Timestamp::kMicroSecondPerSecond;
}

inline Timestamp addTime(const Timestamp& timestamp,double seconds){
    int64_t delta = static_cast<int64_t>(seconds * Timestamp::kMicroSecondPerSecond);
    return Timestamp(timestamp.microSecondsSinceEpoch() + delta);
}

} // namespace muduo


#endif // MYMUDUO_BASE_TIMESTAMP_H