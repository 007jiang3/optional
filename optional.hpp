#pragma once

#include <exception>
#include <type_traits>
#include <initializer_list>

namespace nonstd {
struct bad_optional_access : std::exception {
    bad_optional_access() noexcept = default;
    virtual ~bad_optional_access() = default;

    const char* what() const noexcept override {
        return "bad optional access";
    }
};

struct nullopt_t {
    constexpr explicit nullopt_t(int) {};
};

inline constexpr nullopt_t nullopt{ 0 };

struct in_place_t {
    constexpr explicit in_place_t() = default;
};

inline constexpr in_place_t in_place{};

template <class T>
class optional {
public:
    constexpr optional() noexcept : has_value_(false), nullopt_(0) {}
    constexpr optional(nullopt_t) noexcept : has_value_(false), nullopt_(0) {}

    template <class... Args>
    constexpr explicit optional(in_place_t, Args&&... args) : has_value_(true), value_(std::forward<Args>(args)...) {}

    template <class U, class... Args>
    constexpr explicit optional(in_place_t, std::initializer_list<U> ilist, Args&&... args)
        : has_value_(true), value_(ilist, std::forward<Args>(args)...) {}

    constexpr optional(const optional& other) : has_value_(other.has_value_) {
        if (has_value_) {
            new (&value_) T(other.value_);
        }
    }

    constexpr optional(optional&& other)
        noexcept(std::is_nothrow_move_constructible<T>::value)
        : has_value_(other.has_value_) {
        if (has_value_) {
            new (&value_) T(std::move(other.value_));
        }
    }

    template <class U>
    optional(const optional<U>& other) : has_value_(other.has_value_) {
        if (has_value_) {
            new (&value_) T(other.value_);
        }
    }

    template <class U>
    optional(optional<U>&& other) : has_value_(other.has_value_) {
        if (has_value_) {
            new (&value_) T(std::move(other.value_));
        }
    }

    template <class U>
    constexpr optional(U&& value) : has_value_(true), value_(std::forward<U>(value)) {}

    ~optional() noexcept {
        if (has_value_) {
            value_.~T();
        }
    }

    optional& operator=(nullopt_t) noexcept {
        if (has_value_) {
            value_.~T();
        }

        has_value_ = false;

        return *this;
    }

    optional& operator=(const optional& other) {
        if (this == &other) {
            return *this;
        }

        if (has_value_) {
            value_.~T();
        }

        has_value_ = other.has_value_;
        if (has_value_) {
            new (&value_) T(other.value_);
        }

        return *this;
    }

    optional& operator=(optional&& other) {
        if (this == &other) {
            return *this;
        }

        if (has_value_) {
            value_.~T();
        }

        has_value_ = other.has_value_;
        if (has_value_) {
            new (&value_) T(std::move(other.value_));
        }

        return *this;
    }

    template <class U>
    optional& operator=(const optional<U>& other) {
        if (has_value_) {
            value_.~T();
        }

        has_value_ = other.has_value_;
        if (has_value_) {
            new (&value_) T(other.value_);
        }

        return *this;
    }

    template <class U>
    optional& operator=(optional<U>&& other) {
        if (has_value_) {
            value_.~T();
        }

        has_value_ = other.has_value_;
        if (has_value_) {
            new (&value_) T(std::move(other.value_));
        }

        return *this;
    }

    template <class U>
    optional& operator=(U&& value) {
        if (has_value_) {
            value_ = std::forward<U>(value);
        } else {
            new (&value_) T(std::forward<U>(value));
            has_value_ = true;
        }

        return *this;
    }

    template <class... Args>
    T& emplace(Args&&... args) {
        if (has_value_) {
            value_.~T();
        }

        new (&value_) T(std::forward<Args>(args)...);
        has_value_ = true;

        return value_;
    }

    template <class U, class... Args>
    T& emplace(std::initializer_list<U> ilist, Args&&... args) {
        if (has_value_) {
            value_.~T();
        }

        new (&value_) T(ilist, std::forward<Args>(args)...);
        has_value_ = true;

        return value_;
    }

    void reset() noexcept {
        if (has_value_) {
            value_.~T();
        }

        has_value_ = false;
    }

    constexpr bool has_value() const noexcept { return has_value_; }

    constexpr T& value()& {
        if (!has_value_) {
            throw bad_optional_access();
        }
        return value_;
    }

    constexpr T&& value()&& {
        if (!has_value_) {
            throw bad_optional_access();
        }
        return std::move(value_);
    }

    constexpr const T& value() const& {
        if (!has_value_) {
            throw bad_optional_access();
        }
        return value_;
    }

    constexpr const T&& value() const&& {
        if (!has_value_) {
            throw bad_optional_access();
        }
        return std::move(value_);
    }

    template <class U>
    constexpr T value_or(U&& default_value) const& {
        return has_value_ ? value_ : static_cast<T>(std::forward<U>(default_value));
    }

    template <class U>
    constexpr T value_or(U&& default_value)&& {
        return has_value_ ? std::move(value_) : static_cast<T>(std::forward<U>(default_value));
    }

    constexpr T& operator*() & noexcept { return value_; }
    constexpr T&& operator*() && noexcept { return std::move(value_); }
    constexpr const T& operator*() const& noexcept { return value_; }
    constexpr const T&& operator*() const&& noexcept { return std::move(value_); }

    constexpr T* operator->() noexcept { return &value_; }
    constexpr const T* operator->() const noexcept { return &value_; }

    constexpr explicit operator bool() const noexcept { return has_value_; }

    void swap(optional& other)
        noexcept(std::is_nothrow_move_constructible<T>::value
            && std::is_nothrow_swappable<T>::value) {
        if (has_value_ && other.has_value_) {
            using std::swap;
            swap(value_, other.value_);
        } else if (!has_value_ && other.has_value_) {
            new (&value_) T(std::move(other.value_));
            other.value_.~T();
        } else if (has_value_ && !other.has_value_) {
            new (&other.value_) T(std::move(value_));
            value_.~T();
        }
        std::swap(has_value_, other.has_value_);
    }

private:
    bool has_value_;
    union {
        T value_;
        nullopt_t nullopt_;
    };
};

template <class T>
using __optional_relop_t = std::enable_if_t<std::is_convertible<T, bool>::value, bool>;

template <class T, class U>
using __optional_eq_t = __optional_relop_t<decltype(std::declval<const T&>() == std::declval<const U&>())>;

template <class T, class U>
using __optional_ne_t = __optional_relop_t<decltype(std::declval<const T&>() != std::declval<const U&>())>;

template <class T, class U>
using __optional_lt_t = __optional_relop_t<decltype(std::declval<const T&>() < std::declval<const U&>())>;

template <class T, class U>
using __optional_le_t = __optional_relop_t<decltype(std::declval<const T&>() <= std::declval<const U&>())>;

template <class T, class U>
using __optional_gt_t = __optional_relop_t<decltype(std::declval<const T&>() > std::declval<const U&>())>;

template <class T, class U>
using __optional_ge_t = __optional_relop_t<decltype(std::declval<const T&>() >= std::declval<const U&>())>;

template <class T, class U>
constexpr __optional_eq_t<T, U> operator==(const optional<T>& lhs, const optional<U>& rhs) {
    return lhs.has_value() == rhs.has_value() && (!lhs.has_value() || *lhs == *rhs);
}

template <class T, class U>
constexpr __optional_ne_t<T, U> operator!=(const optional<T>& lhs, const optional<U>& rhs) {
    return !(lhs == rhs);
}

template <class T, class U>
constexpr __optional_lt_t<T, U> operator<(const optional<T>& lhs, const optional<U>& rhs) {
    if (!lhs.has_value() && rhs.has_value()) {
        return true;
    } else if (lhs.has_value() && !rhs.has_value()) {
        return false;
    } else if (!lhs.has_value() && !rhs.has_value()) {
        return false;
    } else {
        return *lhs < *rhs;
    }
}

template <class T, class U>
constexpr __optional_le_t<T, U> operator<=(const optional<T>& lhs, const optional<U>& rhs) {
    if (lhs.has_value() && !rhs.has_value()) {
        return false;
    } else if (!lhs.has_value() && rhs.has_value()) {
        return true;
    } else if (!lhs.has_value() && !rhs.has_value()) {
        return true;
    } else {
        return *lhs <= *rhs;
    }
}

template <class T, class U>
constexpr __optional_gt_t<T, U> operator>(const optional<T>& lhs, const optional<U>& rhs) {
    if (lhs.has_value() && !rhs.has_value()) {
        return true;
    } else if (!lhs.has_value() && rhs.has_value()) {
        return false;
    } else if (!lhs.has_value() && !rhs.has_value()) {
        return false;
    } else {
        return *lhs > *rhs;
    }
}

template <class T, class U>
constexpr __optional_ge_t<T, U> operator>=(const optional<T>& lhs, const optional<U>& rhs) {
    if (!lhs.has_value() && rhs.has_value()) {
        return false;
    } else if (lhs.has_value() && !rhs.has_value()) {
        return true;
    } else if (!lhs.has_value() && !rhs.has_value()) {
        return true;
    } else {
        return *lhs >= *rhs;
    }
}

template <class T>
constexpr bool operator==(const optional<T>& opt, nullopt_t) noexcept {
    return !opt.has_value();
}

template <class T>
constexpr bool operator==(nullopt_t, const optional<T>& opt) noexcept {
    return !opt.has_value();
}

template <class T>
constexpr bool operator!=(const optional<T>& opt, nullopt_t) noexcept {
    return opt.has_value();
}

template <class T>
constexpr bool operator!=(nullopt_t, const optional<T>& opt) noexcept {
    return opt.has_value();
}

template <class T>
constexpr bool operator<(const optional<T>& opt, nullopt_t) noexcept {
    return false;
}

template <class T>
constexpr bool operator<(nullopt_t, const optional<T>& opt) noexcept {
    return opt.has_value();
}

template <class T>
constexpr bool operator<=(const optional<T>& opt, nullopt_t) noexcept {
    return !opt.has_value();
}

template <class T>
constexpr bool operator<=(nullopt_t, const optional<T>& opt) noexcept {
    return true;
}

template <class T>
constexpr bool operator>(const optional<T>& opt, nullopt_t) noexcept {
    return opt.has_value();
}

template <class T>
constexpr bool operator>(nullopt_t, const optional<T>& opt) noexcept {
    return false;
}

template <class T>
constexpr bool operator>=(const optional<T>& opt, nullopt_t) noexcept {
    return true;
}

template <class T>
constexpr bool operator>=(nullopt_t, const optional<T>& opt) noexcept {
    return !opt.has_value();
}

template <class T, class U>
constexpr __optional_eq_t<T, U> operator==(const optional<T>& opt, const U& value) {
    return opt.has_value() ? *opt == value : false;
}

template <class T, class U>
constexpr __optional_eq_t<U, T> operator==(const U& value, const optional<T>& opt) {
    return opt.has_value() ? value == *opt : false;
}

template <class T, class U>
constexpr __optional_ne_t<T, U> operator!=(const optional<T>& opt, const U& value) {
    return opt.has_value() ? *opt != value : true;
}

template <class T, class U>
constexpr __optional_ne_t<U, T> operator!=(const U& value, const optional<T>& opt) {
    return opt.has_value() ? value != *opt : true;
}

template <class T, class U>
constexpr __optional_lt_t<T, U> operator<(const optional<T>& opt, const U& value) {
    return opt.has_value() ? *opt < value : true;
}

template <class T, class U>
constexpr __optional_lt_t<U, T> operator<(const U& value, const optional<T>& opt) {
    return opt.has_value() ? value < *opt : false;
}

template <class T, class U>
constexpr __optional_le_t<T, U> operator<=(const optional<T>& opt, const U& value) {
    return opt.has_value() ? *opt <= value : true;
}

template <class T, class U>
constexpr __optional_le_t<U, T> operator<=(const U& value, const optional<T>& opt) {
    return opt.has_value() ? value <= *opt : false;
}

template <class T, class U>
constexpr __optional_gt_t<T, U> operator>(const optional<T>& opt, const U& value) {
    return opt.has_value() ? *opt > value : false;
}

template <class T, class U>
constexpr __optional_gt_t<U, T> operator>(const U& value, const optional<T>& opt) {
    return opt.has_value() ? value > *opt : true;
}

template <class T, class U>
constexpr __optional_ge_t<T, U> operator>=(const optional<T>& opt, const U& value) {
    return opt.has_value() ? *opt >= value : false;
}

template <class T, class U>
constexpr __optional_ge_t<U, T> operator>=(const U& value, const optional<T>& opt) {
    return opt.has_value() ? value >= *opt : true;
}

template <class T>
void swap(optional<T>& lhs, optional<T>& rhs)
noexcept(noexcept(lhs.swap(rhs))) {
    lhs.swap(rhs);
}

#ifdef __cpp_deduction_guides
template <class T>
optional(T) -> optional<T>;
#endif

template <class T>
constexpr optional<::std::decay_t<T>> make_optional(T&& value) {
    return optional<::std::decay_t<T>>(std::forward<T>(value));
}

template <class T, class... Args>
constexpr optional<T> make_optional(Args&&... args) {
    return optional<T>(in_place, std::forward<Args>(args)...);
}

template <class T, class U, class... Args>
constexpr optional<T> make_optional(std::initializer_list<U> ilist, Args&&... args) {
    return optional<T>(in_place, ilist, std::forward<Args>(args)...);
}

} // namespace nonstd