#ifndef LIBCANAL_SHARED_DATA_POINTER_H
#define LIBCANAL_SHARED_DATA_POINTER_H

#include "Prereq.h"

namespace Canal {

class SharedData
{
public:
    int mReferenceCount;

public:
    SharedData() : mReferenceCount(0) {}

    /// When copying an object to a new one, reset the reference
    /// count.
    SharedData(const SharedData&) : mReferenceCount(0) {}

private:
    /// Assignment operator declaration.  Prevents accidental
    /// assignments of SharedData, which would break the reference
    /// counting.  Do not implement!
    SharedData &operator=(const SharedData&);
};

template <typename T>
class SharedDataPointer
{
    T *mPointer;

public:
    SharedDataPointer() : mPointer(0) {}

    explicit SharedDataPointer(T *ptr) : mPointer(ptr)
    {
        if (mPointer)
            ++mPointer->mReferenceCount;
    }

    SharedDataPointer(const SharedDataPointer<T> &ptr) : mPointer(ptr.mPointer)
    {
        if (mPointer)
            ++mPointer->mReferenceCount;
    }

    ~SharedDataPointer()
    {
        if (!mPointer)
            return;

        if (0 == --mPointer->mReferenceCount)
            delete mPointer;
    }

    SharedDataPointer<T> &operator=(SharedDataPointer<T> &ptr)
    {
        if (ptr.mPointer == mPointer)
            return *this;

        if (mPointer && 0 == --mPointer->mReferenceCount)
            delete mPointer;

        mPointer = ptr.mPointer;
        if (mPointer)
            ++mPointer->mReferenceCount;

        return *this;
    }

    SharedDataPointer<T> &operator=(T *ptr)
    {
        if (ptr == mPointer)
            return *this;

        if (mPointer && 0 == --mPointer->mReferenceCount)
            delete mPointer;

        mPointer = ptr;
        if (mPointer)
            ++mPointer->mReferenceCount;

        return *this;
    }

    bool operator==(const SharedDataPointer<T> &ptr) const
    {
        return mPointer == ptr.mPointer;
    }

    bool operator!=(const SharedDataPointer<T> &ptr) const
    {
        return mPointer != ptr.mPointer;
    }

    T &operator*()
    {
        considerClone();
        return *mPointer;
    }

    const T &operator*() const
    {
        return *mPointer;
    }

    T *operator->()
    {
        considerClone();
        return mPointer;
    }

    const T *operator->() const
    {
        return mPointer;
    }

    bool operator!() const
    {
        return !mPointer;
    }

    const T *data() const
    {
        return mPointer;
    }

private:
    void considerClone()
    {
        if (!mPointer || 1 == mPointer->mReferenceCount)
            return;

        // We decrease the reference count, which is larger than 1 here.
        --mPointer->mReferenceCount;
        mPointer = mPointer->clone();
        ++mPointer->mReferenceCount;
    }
 };

} // namespace Canal

#endif // LIBCANAL_SHARED_DATA_POINTER_H
