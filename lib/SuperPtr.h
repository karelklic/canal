#ifndef LIBCANAL_SUPERPTR_H
#define LIBCANAL_SUPERPTR_H

#include "Domain.h"
#include <utility>
#ifdef DEBUG
#include <iostream>
#endif

namespace Canal {
    template<typename T>
    class SuperPtr
    {
        typedef std::pair<T, unsigned> instanceType;
        instanceType* mInstance;

        inline void remove(instanceType* what) { //Removal function
#ifdef DEBUG
            std::cout << "Remove" << std::endl;
#endif
            what->second --;
            if (!what->second) {
                delete what;
            }
        }

        inline void write() { //Someone wants write
#ifdef DEBUG
            std::cout << "Write" << std::endl;
#endif
            if (mInstance->second == 1) return; //No need to copy if there is only one copy of current object
#ifdef DEBUG
            std::cout << "Write - copy" << std::endl;
#endif
            instanceType* original = mInstance;
            mInstance = new instanceType(T(original->first), 1);
            remove(original);
        }

        SuperPtr<T>& operator= (const SuperPtr<T>& other); //Private

    public:
        SuperPtr(const T& instance) : mInstance(new instanceType(instance, 1)) {}

        SuperPtr(const SuperPtr<T> &copy): mInstance(copy.mInstance) {
#ifdef DEBUG
            std::cout << "Copy constructor" << std::endl;
#endif
            mInstance->second ++;
        }

#if 0
        /// Cast to const T*
        operator const T*() const {
            return mInstance->first;
        }

        /// Cast to T*
        operator T*() {
            write();
            return mInstance->first;
        }
#endif

        /// Get const reference to associated object
        const T& operator* () const {
#ifdef DEBUG
            std::cout << "const operator*" << std::endl;
#endif
            return mInstance->first;
        }

        const T& operator-> () const {
#ifdef DEBUG
            std::cout << "const operator->" << std::endl;
#endif
            return mInstance->first;
        }

        /// Get reference to associated object
        T& operator* () {
#ifdef DEBUG
            std::cout << "operator *" << std::endl;
#endif
            write();
            return mInstance->first;
        }

        T& operator-> () {
#ifdef DEBUG
            std::cout << "operator ->" << std::endl;
#endif
            write();
            return mInstance->first;
        }
    public:
        /// Comparison operator
        bool operator== (const SuperPtr<T>& other) const {
            return mInstance->first == other.mInstance->first;
        }

        /// Comparison operator
        bool operator== (const T& other) const {
            return mInstance->first == other;
        }

        virtual ~SuperPtr() {
#ifdef DEBUG
            std::cout << "Destructor" << std::endl;
#endif
            remove(mInstance);
        }

        const unsigned getCounter() const {
            return mInstance->second;
        }
    };
} // namespace Canal

#endif // LIBCANAL_SUPERPTR_H
