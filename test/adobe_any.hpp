#ifndef ADOBE_ANY_HPP
#define ADOBE_ANY_HPP

//#include <adobe/config.hpp>

#include <algorithm>
#include <exception>
#include <memory>
#include <typeinfo>
#include <type_traits>
#include <utility>

#ifndef NDEBUG
#include <string>

//#include <adobe/debug.hpp>
#endif

#ifdef _MSC_VER
# define ADOBE_NOEXCEPT
#else
# define ADOBE_NOEXCEPT noexcept
#endif

namespace adobe {

/**************************************************************************************************/

class bad_any_cast : public std::bad_cast {
public:
    bad_any_cast(const std::type_info& from, const std::type_info& to) : from_(&from), to_(&to) { }
    const char* what() const ADOBE_NOEXCEPT {
#ifndef NDEBUG
        //if (what_.empty()) {
        //    what_ = "adobe::bad_any_cast: ";
        //    demangle(from(), what_);
        //    what_ += " -> ";
        //    demangle(to(), what_);
        //}
        return what_.c_str();
#else
        return "adobe::bad_any_cast";
#endif
    }

    const std::type_info& from() const { return *from_; }
    const std::type_info& to() const { return *to_; }
private:
#ifndef NDEBUG
    mutable std::string what_;
#endif
    const std::type_info* from_;
    const std::type_info* to_;
};

/**************************************************************************************************/

/*!
    adobe::any is a drop in replacement for boost::any.
    <http://www.boost.org/doc/libs/1_50_0/doc/html/any.html>.
    
    The differences between this implementation and boost::any in boost 1.50.0 are:    
        C++11 rvalue constructors for move
        Small object optimization
        bad_any_cast.what() reports from and to names for debugging.


    REVISIT (sparent) : These changes have been mailed to the maintainer of the boost::any library
    for inclusion.
*/

class any {
public:
    any() { new (storage()) model_void(); }
    template <typename T>
    any(T x) { new (storage()) typename model<T>::type(std::move(x)); }
    any(const any& x) { x.object().copy(storage()); }
    any(any&& x) ADOBE_NOEXCEPT { x.object().move(storage()); }
    
    ~any() { object().~concept_t(); }
    
    any& operator=(any&& x) ADOBE_NOEXCEPT
    { object().~concept_t(); x.object().move(storage()); return *this; }
    any& operator=(const any& x)
    { object().~concept_t(); x.object().copy(storage()); return *this; }
    
    
    template <typename T>
    any& operator=(T x)
    {
      object().~concept_t();
      new (storage()) typename model<T>::type(std::move(x));
      return *this;
    }
    
    const std::type_info& type() const { return object().type(); }
    
    any& swap(any& rhs) {
        any tmp = static_cast<any&&>(rhs);
        rhs = static_cast<any&&>(*this);
        *this = static_cast<any&&>(tmp);
        return *this;
    }
    bool empty() const { return object().type() == typeid(void); }
    
    friend inline void swap(any& x, any& y) { x.swap(y); }
    
    template <typename T> friend T* any_cast(any*);
    template <typename T> friend const T* any_cast(const any*);
    template <typename T> friend T any_cast(any&);
    template <typename T> friend T any_cast(const any&);
    
    template <typename T> friend T static_cast_(any*);
    template <typename T> friend T static_cast_(const any*);
    template <typename T> friend T static_cast_(any&);
    template <typename T> friend T static_cast_(const any&);

private:

    struct concept_t {
        virtual ~concept_t() { };
        virtual void copy(void*) const = 0;
        virtual void move(void*) = 0;
        virtual const std::type_info& type() const = 0;
    };

    template <typename T>
    struct model_local : concept_t {
        model_local(T x) : data_(std::move(x)) { }
        model_local(const model_local& x) : data_(x.data_) { } // = default
        model_local(model_local&& x) ADOBE_NOEXCEPT : data_(std::move(x.data_)) { } // = default
        void copy(void* storage) const { new (storage) model_local(*this); }
        void move(void* storage) { new (storage) model_local(std::move(*this)); }
        const std::type_info& type() const { return typeid(T); }
        
        T* data() { return &data_; }
        const T* data() const { return &data_; }
        
        T data_;
    };
    
    template <typename T>
    struct model_remote : concept_t {
        model_remote(T x) : data_(new T(std::move(x))) { }
        model_remote(const model_remote& x) : data_(new T(*x.data_)) { }
        model_remote(model_remote&& x) ADOBE_NOEXCEPT : data_(std::move(x.data_)) { } // = default
        void copy(void* storage) const { new (storage) model_remote(*this); }
        void move(void* storage) { new (storage) model_remote(std::move(*this)); }
        const std::type_info& type() const { return typeid(T); }
        
        T* data() { return data_.get(); }
        const T* data() const { return data_.get(); }
        
        std::unique_ptr<T> data_;
    };
    
    struct model_void : concept_t {
        void copy(void* storage) const { new (storage) model_void(); }
        void move(void* storage) { new (storage) model_void(); }
        const std::type_info& type() const { return typeid(void); }
    };
    typedef double storage_t[2];
    
    template <bool Local, typename T> struct model_;
    template <typename T> struct model_<true, T> { typedef model_local<T> type; };
    template <typename T> struct model_<false, T> { typedef model_remote<T> type; };
    
    template <typename T> struct model {
        typedef typename model_<(sizeof(model_local<T>) <= sizeof(storage_t))
                && std::is_nothrow_move_constructible<T>::value, T>::type type;
    };
    
    concept_t& object() { return *static_cast<concept_t*>(storage()); }
    const concept_t& object() const { return *static_cast<const concept_t*>(storage()); }
    
    void* storage() { return &storage_; }
    const void* storage() const { return &storage_; }
        
    storage_t storage_;
};
    
template <typename T>
T* any_cast(any* x) {
    return (x && (x->type() == typeid(T)))
        ? static_cast<typename any::model<T>::type*>(x->storage())->data() : 0;
}

template <typename T>
const T* any_cast(const any* x) {
    return (x && (x->type() == typeid(T)))
        ? static_cast<const typename any::model<T>::type*>(x->storage())->data() : 0;
}

template <typename T>
T any_cast(any& x) {
    typedef typename std::remove_reference<T>::type type;
    type* result = any_cast<type>(&x);
    if (!result) throw bad_any_cast(x.type(), typeid(type));
    return *result;
}

// experimental added by Beman
template <typename T>
T any_cast(any&& x) {
    typedef typename std::remove_reference<T>::type type;
    type* result = any_cast<type>(&x);
    if (!result) throw bad_any_cast(x.type(), typeid(type));
    return *result;
}
template <typename T>
T any_cast(const any& x) {
    typedef typename std::remove_reference<T>::type type;
    const type* result = any_cast<type>(&x);
    if (!result) throw bad_any_cast(x.type(), typeid(type));
    return *result;
}

template <typename T>
T dynamic_cast_(any& x) {
    static_assert(std::is_reference<T>::value, "T must be a reference type");
    return any_cast<T>(x);
}

template <typename T>
T dynamic_cast_(const any& x) {
    static_assert(std::is_reference<T>::value, "T must be a reference type");
    return any_cast<T>(x);
}

template <typename T>
T dynamic_cast_(any* x) {
    static_assert(std::is_pointer<T>::value, "T must be a pointer type");
    return any_cast<typename std::remove_pointer<T>::type>(x);
}

template <typename T>
T dynamic_cast_(const any* x) {
    static_assert(std::is_pointer<T>::value, "T must be a pointer type");
    return any_cast<typename std::remove_pointer<T>::type>(x);
}

template <typename T>
T static_cast_(any* x) {
    typedef typename std::remove_pointer<T>::type type;
    typedef typename any::model<type>::type model_type;
    return static_cast<model_type*>(x->storage())->data();
}

template <typename T>
T static_cast_(const any* x) {
    typedef typename std::remove_pointer<T>::type type;
    typedef typename any::model<type>::type model_type;
    return static_cast<const model_type*>(x->storage())->data();
}

template <typename T>
T static_cast_(any& x) {
    typedef typename std::remove_reference<T>::type type;
    return *static_cast_<type*>(&x);
}

template <typename T>
T static_cast_(const any& x) {
    typedef typename std::remove_reference<T>::type type;
    return *static_cast_<type*>(&x);
}

/**************************************************************************************************/

} // namespace adobe

#endif
