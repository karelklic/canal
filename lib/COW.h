#ifndef LIBCANAL_COW_H
#define LIBCANAL_COW_H

#include "Domain.h"
#include "ArrayInterface.h"
#include <utility>
#include <typeinfo>
#ifdef DEBUG
#include <iostream>
#endif

namespace Canal {
    template<typename T, typename = void>
    class COWConst
    {
    protected:
        typedef std::pair<T, unsigned> instanceType;
        instanceType* mInstance;

        inline void remove(instanceType* what) { //Removal function
#ifdef DEBUG
            std::cout << "COWConst: Remove" << std::endl;
#endif
            what->second --;
            if (!what->second) {
                delete what;
            }
        }

        inline void write() { //Someone wants write
#ifdef DEBUG
            std::cout << "COWConst: Write" << std::endl;
#endif
            if (mInstance->second == 1) return; //No need to copy if there is only one copy of current object
#ifdef DEBUG
            std::cout << "COWConst: Write - copy" << std::endl;
#endif
            instanceType* original = mInstance;
            mInstance = new instanceType(T(original->first), 1);
            remove(original);
        }

        COWConst<T>& operator= (const COWConst<T>& other); //Private

    public:
        COWConst(const T& instance) : mInstance(new instanceType(instance, 1)) {
#ifdef DEBUG
            std::cout << "COWConst: Instance constructor" << std::endl;
#endif
        }

        COWConst(const COWConst<T> &copy): mInstance(copy.mInstance) {
#ifdef DEBUG
            std::cout << "COWConst: Copy constructor" << std::endl;
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

#if 0
        /// Get const reference to associated object
        const T& operator* () const {
#ifdef DEBUG
            std::cout << "COWConst: const operator*" << std::endl;
#endif
            return mInstance->first;
        }

        const T* operator-> () const {
#ifdef DEBUG
            std::cout << "COWConst: const operator->" << std::endl;
#endif
            return &mInstance->first;
        }
#endif

        /// Comparison operator
        bool operator== (const COWConst<T>& other) const {
#ifdef DEBUG
            std::cout << "COWConst: comparison with COWConst" << std::endl;
#endif
            //Check if pointer to instance is same, if not, check instance
            return mInstance == other.mInstance || mInstance->first == other.mInstance->first;
        }

        /// Comparison operator
        bool operator== (const T& other) const {
#ifdef DEBUG
            std::cout << "COWConst: comparison with T" << std::endl;
#endif
            return mInstance->first == other;
        }

        virtual ~COWConst() {
#ifdef DEBUG
            std::cout << "COWConst: Destructor" << std::endl;
#endif
            remove(mInstance);
        }

        const unsigned getCounter() const {
#ifdef DEBUG
            std::cout << "COWConst: getCounter" << std::endl;
#endif
            return mInstance->second;
        }
    };

    template<typename T, typename = void>
    class COW
    {
    protected:
        typedef std::pair<T, unsigned> instanceType;
        instanceType* mInstance;

        inline void remove(instanceType* what) { //Removal function
#ifdef DEBUG
            std::cout << "COW: Remove" << std::endl;
#endif
            what->second --;
            if (!what->second) {
                delete what;
            }
        }

        inline void write() { //Someone wants write
#ifdef DEBUG
            std::cout << "COW: Write" << std::endl;
#endif
            if (mInstance->second == 1) return; //No need to copy if there is only one copy of current object
#ifdef DEBUG
            std::cout << "COW: Write - copy" << std::endl;
#endif
            instanceType* original = mInstance;
            mInstance = new instanceType(T(original->first), 1);
            remove(original);
        }

        COW<T>& operator= (const COW<T>& other); //Private

    public:
        COW(const T& instance) : mInstance(new instanceType(instance, 1)) {
#ifdef DEBUG
            std::cout << "COW: Instance constructor" << std::endl;
#endif
        }

        COW(const COW<T> &copy): mInstance(copy.mInstance) {
#ifdef DEBUG
            std::cout << "COW: Copy constructor" << std::endl;
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
            std::cout << "COW: const operator*" << std::endl;
#endif
            return mInstance->first;
        }

        const T* operator-> () const {
#ifdef DEBUG
            std::cout << "COW: const operator->" << std::endl;
#endif
            return &mInstance->first;
        }

        /// Comparison operator
        bool operator== (const COW<T>& other) const {
#ifdef DEBUG
            std::cout << "COW: comparison with COW" << std::endl;
#endif
            //Check if pointer to instance is same, if not, check instance
            return mInstance == other.mInstance || mInstance->first == other.mInstance->first;
        }

        /// Comparison operator
        bool operator== (const T& other) const {
#ifdef DEBUG
            std::cout << "COW: comparison with T" << std::endl;
#endif
            return mInstance->first == other;
        }

        virtual ~COW() {
#ifdef DEBUG
            std::cout << "COW: Destructor" << std::endl;
#endif
            remove(mInstance);
        }

        const unsigned getCounter() const {
#ifdef DEBUG
            std::cout << "COW: getCounter" << std::endl;
#endif
            return mInstance->second;
        }

        /// Get reference to associated object
        T& operator* () {
#ifdef DEBUG
            std::cout << "COW: operator *" << std::endl;
#endif
            this->write();
            return this->mInstance->first;
        }

        /// Get pointer to associated object
        T* operator-> () {
#ifdef DEBUG
            std::cout << "COW: operator ->" << std::endl;
#endif
            this->write();
            return &this->mInstance->first;
        }
    };

/// Test for base class - from http://stackoverflow.com/questions/281725/template-specialization-based-on-inherit-class/282006#282006
    template<bool C, typename T = void>
    struct enable_if {
      typedef T type;
    };

    template<typename T>
    struct enable_if<false, T> { };

    template<bool C, typename T = void>
    struct enable_if_not {
      typedef T type;
    };

    template<typename T>
    struct enable_if_not<true, T> { };

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

    template <bool>
    struct static_not {
        static const bool value = true;
    };
    template <>
    struct static_not<true> {
        static const bool value = false;
    };

    template <bool, bool>
    struct static_and {
        static const bool value = false;
    };
    template<>
    struct static_and<true, true> {
        static const bool value = true;
    };

    template <typename T>
    class COWDomain : public COWConst<T>, public Domain {
    public:
        /// Constructors
        COWDomain(const T &instance) : Domain(instance), COWConst<T>(instance) {}
        COWDomain(const COWDomain<T> &copy) : Domain(copy.mInstance->first), COWConst<T>(copy) {}

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
#ifdef DEBUG
            std::cout << "Conversion to modifiable reference" << std::endl;
#endif
            this->write();
            return this->mInstance->first;
        }

#ifdef DEBUG
#define CLONE(TYPE) \
        virtual TYPE* clone() const { \
            std::cout << "clone for " #TYPE << std::endl; \
            return new TYPE(*this); \
        }
#else
#define CLONE(TYPE) \
        virtual TYPE* clone() const { \
            return new TYPE(*this); \
        }
#endif
        CLONE(COWDomain<T>)

        virtual bool operator==(const Canal::Domain& other) const {
#ifdef DEBUG
            std::cout << "operator== with Domain" << std::endl;
#endif
            return this->mInstance->first == other;
        }

        virtual void merge(const Canal::Domain& other) {
#ifdef DEBUG
            std::cout << "merge" << std::endl;
#endif
            this->write();
            this->mInstance->first.merge(other);
        }

        virtual size_t memoryUsage() const {
#ifdef DEBUG
            std::cout << "memoryUsage" << std::endl;
#endif
            return this->mInstance->first.memoryUsage();
        }

        virtual std::string toString() const {
#ifdef DEBUG
            std::cout << "toString" << std::endl;
#endif
            return this->mInstance->first.toString();
        }

        virtual void setZero(const llvm::Value* v) {
#ifdef DEBUG
            std::cout << "setZero" << std::endl;
#endif
            this->write();
            this->mInstance->first.setZero(v);
        }

        // OP(x) macro - defines binary operator with name x, which takes 2 domains
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
        //CMP(x) macro - defines compare operator with name x, which takes 2 domains and LLVM predicate
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
        //UOP(x) macro - defines unary operator with name x, which takes 1 domain
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

        // Former AccuracyDomain methods.

        /// Get accuracy of domain
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

    /// COW for Domain, which is not Array::Interface
    template <typename T>
    class COW<T, typename enable_if<
            static_and<
                is_base_of<Domain, T>::value, //T is descendant of Domain
                static_not<is_base_of<Array::Interface, T>::value>::value //T is NOT descendat of Array::Interface
            >::value
        >::type > : public COWDomain<T> {
    public:
        COW(const T &instance) : COWDomain<T>(instance) {}
        COW(const COW<T> &copy) : COWDomain<T>(copy) {}
        CLONE(COW<T>)
    };

    /// COW for Domain, which is Array::Interface
    template <typename T>
    class COW<T, typename enable_if<
            static_and<
                is_base_of<Domain, T>::value, //T is descendant of Domain
                is_base_of<Array::Interface, T>::value //T is also descendat of Array::Interface
            >::value
        >::type > : public COWDomain<T>, public Array::Interface {
    public:
        COW(const T &instance) : COWDomain<T>(instance) {}
        COW(const COW<T> &copy) : COWDomain<T>(copy) {}
        CLONE(COW<T>)

        // Implement Array::Interface

        Domain* getValue(const Domain& offset) const {
#ifdef DEBUG
            std::cout << "getValue(Domain)" << std::endl;
#endif
            return this->mInstance->first.getValue(offset);
        }

        Domain* getValue(uint64_t offset) const {
#ifdef DEBUG
            std::cout << "getValue(uint64_t)" << std::endl;
#endif
            return this->mInstance->first.getValue(offset);
        }

        virtual std::vector<Domain*> getItem(const Domain &offset) const {
#ifdef DEBUG
            std::cout << "getItem(Domain)" << std::endl;
#endif
            return this->mInstance->first.getItem(offset);
        }

        virtual Domain* getItem(uint64_t offset) const {
#ifdef DEBUG
            std::cout << "getItem(uint64_t)" << std::endl;
#endif
            return this->mInstance->first.getItem(offset);
        }

        virtual void setItem(const Domain &offset, const Domain &value) {
#ifdef DEBUG
            std::cout << "setItem(Domain)" << std::endl;
#endif
            this->write();
            return this->mInstance->first.setItem(offset, value);
        }

        virtual void setItem(uint64_t offset, const Domain &value) {
#ifdef DEBUG
            std::cout << "setItem(uint64_t)" << std::endl;
#endif
            this->write();
            return this->mInstance->first.setItem(offset, value);
        }
    };
#undef CLONE
} // namespace Canal
//Allow domain class to use COW
//Defines T_type as COW<T>
#define USE_COW(T) typedef COW<T> T##_type
//DONT_USE_COW is in Domain.h
#endif // LIBCANAL_COW_H
