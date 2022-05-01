#ifndef MYMUDUO_BASE_MUTEX_H
#define MYMUDUO_BASE_MUTEX_H

#include <assert.h>
#include <pthread.h>

#include "CurrentThread.h"
#include "noncopyable.h"

// Thread safety annotations {
// https://clang.llvm.org/docs/ThreadSafetyAnalysis.html

// Enable thread safety attributes only with clang.
// The attributes can be safely erased when compiling with other compilers.
#if defined(__clang__) && (!defined(SWIG))
#define THREAD_ANNOTATION_ATTRIBUTE__(x)   __attribute__((x))
#else
#define THREAD_ANNOTATION_ATTRIBUTE__(x)   // no-op
#endif

#define CAPABILITY(x) \
  THREAD_ANNOTATION_ATTRIBUTE__(capability(x))

#define SCOPED_CAPABILITY \
  THREAD_ANNOTATION_ATTRIBUTE__(scoped_lockable)

#define GUARDED_BY(x) \
  THREAD_ANNOTATION_ATTRIBUTE__(guarded_by(x))

#define PT_GUARDED_BY(x) \
  THREAD_ANNOTATION_ATTRIBUTE__(pt_guarded_by(x))

#define ACQUIRED_BEFORE(...) \
  THREAD_ANNOTATION_ATTRIBUTE__(acquired_before(__VA_ARGS__))

#define ACQUIRED_AFTER(...) \
  THREAD_ANNOTATION_ATTRIBUTE__(acquired_after(__VA_ARGS__))

#define REQUIRES(...) \
  THREAD_ANNOTATION_ATTRIBUTE__(requires_capability(__VA_ARGS__))

#define REQUIRES_SHARED(...) \
  THREAD_ANNOTATION_ATTRIBUTE__(requires_shared_capability(__VA_ARGS__))

#define ACQUIRE(...) \
  THREAD_ANNOTATION_ATTRIBUTE__(acquire_capability(__VA_ARGS__))

#define ACQUIRE_SHARED(...) \
  THREAD_ANNOTATION_ATTRIBUTE__(acquire_shared_capability(__VA_ARGS__))

#define RELEASE(...) \
  THREAD_ANNOTATION_ATTRIBUTE__(release_capability(__VA_ARGS__))

#define RELEASE_SHARED(...) \
  THREAD_ANNOTATION_ATTRIBUTE__(release_shared_capability(__VA_ARGS__))

#define TRY_ACQUIRE(...) \
  THREAD_ANNOTATION_ATTRIBUTE__(try_acquire_capability(__VA_ARGS__))

#define TRY_ACQUIRE_SHARED(...) \
  THREAD_ANNOTATION_ATTRIBUTE__(try_acquire_shared_capability(__VA_ARGS__))

#define EXCLUDES(...) \
  THREAD_ANNOTATION_ATTRIBUTE__(locks_excluded(__VA_ARGS__))

#define ASSERT_CAPABILITY(x) \
  THREAD_ANNOTATION_ATTRIBUTE__(assert_capability(x))

#define ASSERT_SHARED_CAPABILITY(x) \
  THREAD_ANNOTATION_ATTRIBUTE__(assert_shared_capability(x))

#define RETURN_CAPABILITY(x) \
  THREAD_ANNOTATION_ATTRIBUTE__(lock_returned(x))

#define NO_THREAD_SAFETY_ANALYSIS \
  THREAD_ANNOTATION_ATTRIBUTE__(no_thread_safety_analysis)

// End of thread safety annotations }

#ifdef CHECK_PTHREAD_RETURN_VALUE

#ifdef NDEBUG
__BEGIN_DECLS
extern void __assert_perror_fail (int errnum,
                                  const char *file,
                                  unsigned int line,
                                  const char *function)
    noexcept __attribute__ ((__noreturn__));
__END_DECLS
#endif

#define MCHECK(ret) ({ __typeof__ (ret) errnum = (ret);         \
                       if (__builtin_expect(errnum != 0, 0))    \
                         __assert_perror_fail (errnum, __FILE__, __LINE__, __func__);})

#else  // CHECK_PTHREAD_RETURN_VALUE

#define MCHECK(ret) ({ __typeof__ (ret) errnum = (ret);         \
                       assert(errnum == 0); (void) errnum;})

#endif // CHECK_PTHREAD_RETURN_VALUE

namespace muduo {

class MutexLock : noncopyable {
public:
    MutexLock() : holder_(0) {
        pthread_mutex_init(&mutex_, NULL);
    }
    ~MutexLock() {
        assert(0 == holder_);
        pthread_mutex_destroy(&mutex_);
    }

    bool isLockedByThisThread() const {
        return holder_ == CurrentThread::tid();
    }
    void assertLocked() const {
        assert(isLockedByThisThread());
    }

    void lock() {
        pthread_mutex_lock(&mutex_);
        assignHolder();
    }
    void unlock() {
        unassignHolder();
        pthread_mutex_unlock(&mutex_);
    }

    pthread_mutex_t* getPthreadMutex() {
        return &mutex_;
    }

private:
    void unassignHolder() {
        holder_ = 0;
    }
    void assignHolder() {
        holder_ = CurrentThread::tid();
    }

    friend class Condition;
    class UnassignGuard : noncopyable {
    public:
        UnassignGuard(MutexLock& owner) : owner_(owner) {
            owner_.unassignHolder();
        }
        ~UnassignGuard() {
            owner_.assignHolder();
        }
    private:
        MutexLock& owner_;
    };

    pthread_mutex_t mutex_;
    pid_t holder_;
};

// 利用C++的RAII机制，让锁在作用域内全自动化
class MutexLockGuard : noncopyable {
public:
    explicit MutexLockGuard(MutexLock& mutex) : mutex_(mutex) {
        mutex_.lock();
    }
    ~MutexLockGuard() {
        mutex_.unlock();
    }

private:
    MutexLock& mutex_;
};

} // namespace muduo

/**
 * 该宏作用是防止程序里出现如下错误
 * void doit(){
 *     MutexLockGuard(mutex);//遗漏变量名，产生一个临时对象又马上销毁了
 *     正确写法是 MutexLockGuard lock(mutex);
 * 
 * }
 */
#define MutexLockGuard(x) static_assert(false,"missing mutex guard var game");

#endif // MYMUDUO_BASE_MUTEX_H