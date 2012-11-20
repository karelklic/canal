#ifndef LIBCANAL_SUPERPTR_H
#define LIBCANAL_SUPERPTR_H

#include "Domain.h"
#include <utility>
#include <typeinfo>
#ifdef DEBUG
#include <iostream>
#endif

namespace Canal {
    template<typename T, typename = void>
    class SuperPtrConst
    {
    protected:
        typedef std::pair<T, unsigned> instanceType;
        instanceType* mInstance;

        inline void remove(instanceType* what) { //Removal function
#ifdef DEBUG
            std::cout << "SuperPtrConst: Remove" << std::endl;
#endif
            what->second --;
            if (!what->second) {
                delete what;
            }
        }

        inline void write() { //Someone wants write
#ifdef DEBUG
            std::cout << "SuperPtrConst: Write" << std::endl;
#endif
            if (mInstance->second == 1) return; //No need to copy if there is only one copy of current object
#ifdef DEBUG
            std::cout << "SuperPtrConst: Write - copy" << std::endl;
#endif
            instanceType* original = mInstance;
            mInstance = new instanceType(T(original->first), 1);
            remove(original);
        }

        SuperPtrConst<T>& operator= (const SuperPtrConst<T>& other); //Private

    public:
        SuperPtrConst(const T& instance) : mInstance(new instanceType(instance, 1)) {
#ifdef DEBUG
            std::cout << "SuperPtrConst: Instance constructor" << std::endl;
#endif
        }

        SuperPtrConst(const SuperPtrConst<T> &copy): mInstance(copy.mInstance) {
#ifdef DEBUG
            std::cout << "SuperPtrConst: Copy constructor" << std::endl;
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
            std::cout << "SuperPtrConst: const operator*" << std::endl;
#endif
            return mInstance->first;
        }

        const T* operator-> () const {
#ifdef DEBUG
            std::cout << "SuperPtrConst: const operator->" << std::endl;
#endif
            return &mInstance->first;
        }

        /// Comparison operator
        bool operator== (const SuperPtrConst<T>& other) const {
#ifdef DEBUG
            std::cout << "SuperPtrConst: comparison with SuperPtrConst" << std::endl;
#endif
            return mInstance->first == other.mInstance->first;
        }

        /// Comparison operator
        bool operator== (const T& other) const {
#ifdef DEBUG
            std::cout << "SuperPtrConst: comparison with T" << std::endl;
#endif
            return mInstance->first == other;
        }

        virtual ~SuperPtrConst() {
#ifdef DEBUG
            std::cout << "SuperPtrConst: Destructor" << std::endl;
#endif
            remove(mInstance);
        }

        const unsigned getCounter() const {
#ifdef DEBUG
            std::cout << "SuperPtrConst: getCounter" << std::endl;
#endif
            return mInstance->second;
        }
    };

    template<typename T, typename = void>
    class SuperPtr
    {
    protected:
        typedef std::pair<T, unsigned> instanceType;
        instanceType* mInstance;

        inline void remove(instanceType* what) { //Removal function
#ifdef DEBUG
            std::cout << "SuperPtr: Remove" << std::endl;
#endif
            what->second --;
            if (!what->second) {
                delete what;
            }
        }

        inline void write() { //Someone wants write
#ifdef DEBUG
            std::cout << "SuperPtr: Write" << std::endl;
#endif
            if (mInstance->second == 1) return; //No need to copy if there is only one copy of current object
#ifdef DEBUG
            std::cout << "SuperPtr: Write - copy" << std::endl;
#endif
            instanceType* original = mInstance;
            mInstance = new instanceType(T(original->first), 1);
            remove(original);
        }

        SuperPtr<T>& operator= (const SuperPtr<T>& other); //Private

    public:
        SuperPtr(const T& instance) : mInstance(new instanceType(instance, 1)) {
#ifdef DEBUG
            std::cout << "SuperPtr: Instance constructor" << std::endl;
#endif
        }

        SuperPtr(const SuperPtr<T> &copy): mInstance(copy.mInstance) {
#ifdef DEBUG
            std::cout << "SuperPtr: Copy constructor" << std::endl;
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
            std::cout << "SuperPtr: const operator*" << std::endl;
#endif
            return mInstance->first;
        }

        const T* operator-> () const {
#ifdef DEBUG
            std::cout << "SuperPtr: const operator->" << std::endl;
#endif
            return &mInstance->first;
        }

        /// Comparison operator
        bool operator== (const SuperPtr<T>& other) const {
#ifdef DEBUG
            std::cout << "SuperPtr: comparison with SuperPtr" << std::endl;
#endif
            return mInstance->first == other.mInstance->first;
        }

        /// Comparison operator
        bool operator== (const T& other) const {
#ifdef DEBUG
            std::cout << "SuperPtr: comparison with T" << std::endl;
#endif
            return mInstance->first == other;
        }

        virtual ~SuperPtr() {
#ifdef DEBUG
            std::cout << "SuperPtr: Destructor" << std::endl;
#endif
            remove(mInstance);
        }

        const unsigned getCounter() const {
#ifdef DEBUG
            std::cout << "SuperPtr: getCounter" << std::endl;
#endif
            return mInstance->second;
        }

        /// Get reference to associated object
        T& operator* () {
#ifdef DEBUG
            std::cout << "SuperPtr: operator *" << std::endl;
#endif
            this->write();
            return this->mInstance->first;
        }

        /// Get pointer to associated object
        T* operator-> () {
#ifdef DEBUG
            std::cout << "SuperPtr: operator ->" << std::endl;
#endif
            this->write();
            return &this->mInstance->first;
        }
    };

/// Test for base class - from http://stackoverflow.com/questions/281725/template-specialization-based-on-inherit-class/282006#282006
//namespace {
    template<bool C, typename T = void>
    struct enable_if {
      typedef T type;
    };

    template<typename T>
    struct enable_if<false, T> { };

    template<typename, typename>
    struct is_same {
        static bool const value = false;
    };

    template<typename A>
    struct is_same<A, A> {
        static bool const value = true;
    };

    template<typename B, typename D>
    struct is_base_of {
        static D * create_d();
        static char (& chk(B *))[1];
        static char (& chk(...))[2];
        static bool const value = sizeof chk(create_d()) == 1 &&
                                  !is_same<B    volatile const,
                                           void volatile const>::value;
    };

    template <bool A, bool B, typename T = void> //must be a && !b
    struct domain_check {};

    template<typename T>
    struct domain_check<true, false, T>{
        typedef T type;
    };

    template <typename T> //Contains T
    class SuperPtrDomain : public SuperPtrConst<T>, public Domain {
    public:
        /// Constructors
        SuperPtrDomain(const T &instance) : Domain(instance), SuperPtrConst<T>(instance) {}
        SuperPtrDomain(const SuperPtr<T> &copy) : Domain(copy.mInstance->first), SuperPtrConst<T>(copy) {}

        /// Conversion to const object reference
        operator const T& () const {
#ifdef DEBUG
            std::cout << "Conversion to const object reference" << std::endl;
#endif
            return this->mInstance->first;
        }

        /// Conversion to const object pointer
        operator const T* () const {
#ifdef DEBUG
            std::cout << "Conversion to const object pointer" << std::endl;
#endif
            return &this->mInstance->first;
        }

        T& modifiable() {
            this->write();
            return this->mInstance->first;
        }

#define CLONE_METHODS(type) \
        virtual type* clone() const { \
            return new type(*this); \
        } \
        virtual SuperPtrDomain<T>* cloneCleaned() const { \
            T* cloned = this->mInstance->first.cloneCleaned(); \
            type* ret = new type(*cloned); \
            delete cloned; \
            return ret; \
        }\
        static bool classof(const Domain* val) {\
            return typeid(*val) == typeid(T);\
        }

        CLONE_METHODS(SuperPtrDomain<T>)

        virtual bool operator==(const Canal::Domain& other) const {
            return this->mInstance->first == other;
        }

        virtual void merge(const Canal::Domain& other) {
            this->write();
            this->mInstance->first.merge(other);
        }

        virtual size_t memoryUsage() const {
            return this->mInstance->first.memoryUsage();
        }

        virtual std::string toString() const {
            return this->mInstance->first.toString();
        }

        virtual void setZero(const llvm::Value* v) {
            this->write();
            this->mInstance->first.setZero(v);
        }

#ifdef DEBUG
#define OP(x) virtual void x(const Domain &a, const Domain &b) { std::cout << #x << std::endl; this->write(); this->mInstance->first.x(a, b); }
#else
#define OP(x) virtual void x(const Domain &a, const Domain &b) { this->write(); this->mInstance->first.x(a, b); }
#endif
        OP(add)
        OP(fadd)
        OP(sub)
        OP(fsub)
        OP(mul)
        OP(fmul)
        OP(udiv)
        OP(sdiv)
        OP(fdiv)
        OP(urem)
        OP(srem)
        OP(frem)
        OP(shl)
        OP(ashr)
        OP(lshr)
        OP(and_)
        OP(or_)
        OP(xor_)
#undef OP
#ifdef DEBUG
#define CMP(x) virtual void x(const Domain& a, const Domain &b, llvm::CmpInst::Predicate predicate) { \
    std::cout << #x << std::endl; this->write(); this->mInstance->first.x(a, b, predicate); }
#else
#define CMP(x) virtual void x(const Domain& a, const Domain &b, llvm::CmpInst::Predicate predicate) { \
    this->write(); this->mInstance->first.x(a, b, predicate); }
#endif
        CMP(icmp)
        CMP(fcmp)
#undef CMP
#ifdef DEBUG
#define UOP(x) virtual void x(const Domain& value) { std::cout << #x << std::endl; this->write(); this->mInstance->first.x(value); }
#else
#define UOP(x) virtual void x(const Domain& value) { this->write(); this->mInstance->first.x(value); }
#endif
        UOP(trunc)
        UOP(sext)
        UOP(zext)
        UOP(fptrunc)
        UOP(fpext)
        UOP(fptoui)
        UOP(fptosi)
        UOP(uitofp)
        UOP(sitofp)
#undef UOP
    };
//}
    /// Super ptr for Domain
    template <typename T>
    class SuperPtr<T, typename domain_check< is_base_of<Domain, T>::value, //Is descendant of Domain
            is_base_of<AccuracyDomain, T>::value>::type //but NOT descedant of AccuracyDomain
            > : public SuperPtrDomain<T> {
    public:
        /// Constructors
        SuperPtr(const T &instance) : SuperPtrDomain<T>(instance) {}
        SuperPtr(const SuperPtr<T> &copy) : SuperPtrDomain<T>(copy) {}
        CLONE_METHODS(SuperPtr<T>)
    };

    /// Super ptr for Accuracy Domain
    template <typename T>
    class SuperPtr<T, typename enable_if<is_base_of<AccuracyDomain, T>::value>::type>
            : public SuperPtrDomain<T>, public AccuracyDomain {
    public:
        SuperPtr(const T &instance) : SuperPtrDomain<T>(instance) {}
        SuperPtr(const SuperPtr<T> &copy) : SuperPtrDomain<T>(copy) {}
        CLONE_METHODS(SuperPtr<T>)

        virtual float accuracy() const {
#ifdef DEBUG
            std::cout << "accuracy" << std::endl;
#endif
            return this->mInstance->first.accuracy();
        }

        /// Is it the lowest value.
        virtual bool isBottom() const {
#ifdef DEBUG
            std::cout << "isBottom" << std::endl;
#endif
            return this->mInstance->first.isBottom();
        }

        /// Set to the lowest value.
        virtual void setBottom() {
#ifdef DEBUG
            std::cout << "setBottom" << std::endl;
#endif
            this->write();
            this->mInstance->first.setBottom();
        }

        /// Is it the highest value.
        virtual bool isTop() const {
#ifdef DEBUG
            std::cout << "isTop" << std::endl;
#endif
            return this->mInstance->first.isTop();
        }

        /// Set it to the top value of lattice.
        virtual void setTop() {
#ifdef DEBUG
            std::cout << "setTop" << std::endl;
#endif
            this->write();
            this->mInstance->first.setTop();
        }
    };
#undef CLONE_METHODS
} // namespace Canal

#endif // LIBCANAL_SUPERPTR_H
