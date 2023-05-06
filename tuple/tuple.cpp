#include <iostream>
#include <type_traits>



template<typename T>
using remove_all_t = std::remove_reference_t<std::remove_const_t<T>>;

template<size_t N, typename ...Args>
struct IdTuple {
    void swap(IdTuple &) {}
};


template<size_t N, typename T>
struct Head {
    T value;

    Head() : value() {}

    template<typename Arg>
    Head(Arg && arg) : value(std::forward<Arg>(arg)) {}




protected:
    static auto &get(Head &other) {
        return other.value;
    }

    static const auto &get(const Head &other) {
        return other.value;
    }

    static auto&&get(Head &&other) {
        return std::move(other.value);
    }

    static const auto &&get(const Head &&other) {
        return std::move(other.value);
    }
};

template <typename T>
struct TupleSize {
    static constexpr size_t value = 0;
};

template <typename Tup>
constexpr size_t TupleSize_v = TupleSize<Tup>::value;



template<size_t N, typename T, typename ...Tail>
struct IdTuple<N, T, Tail...> : public IdTuple<N + 1, Tail...>, private Head<N, T> {

    IdTuple() {}


    template<typename U, typename ...Args, typename = std::enable_if_t <sizeof...(Args) != 0 || TupleSize_v<remove_all_t<U>> != sizeof...(Tail) + 1>>
    IdTuple(U &&head, Args &&... tail) :
            IdTuple<N + 1, Tail...>(std::forward<Args>(tail)...), Head<N, T>(std::forward<U>(head)) {}

    template<typename T_, typename ...Args>
    IdTuple(IdTuple<N, T_, Args...> &other) : IdTuple <N + 1, Tail...>(other), Head<N, T>(IdTuple<N, T_, Args...>::get(other)) {}

    template<typename T_, typename ...Args>
    IdTuple(const IdTuple<N, T_, Args...> &other) : IdTuple <N + 1, Tail...>(other), Head<N, T>(IdTuple<N, T_, Args...>::get(other)) {}


    template<typename T_, typename ...Args>
    IdTuple(IdTuple<N, T_, Args...> &&other) : IdTuple <N + 1, Tail...>(std::move(other)), Head<N, T>(IdTuple<N, T_, Args...>::get(std::move(other))) {}

    IdTuple(IdTuple&&) = default;

    IdTuple(const IdTuple&) = default;

    IdTuple &operator=(const IdTuple &other) {
        IdTuple<N + 1, Tail...>::operator=(other);
        get(*this) = get(other);
        return *this;
    }

    IdTuple &operator=(IdTuple &&other) {
        IdTuple<N + 1, Tail...>::operator=(std::move(other));
        get(*this) = get(std::move(other));
        return *this;
    }

    void swap(IdTuple &other) &{
        std::swap(get(*this), get(other));
        IdTuple<N + 1, Tail...>::swap(other);
    }

    static auto &get(IdTuple &other) {
        return Head<N, T>::get(other);
    }

    static const auto &get(const IdTuple &other) {
        return Head<N, T>::get(other);
    }

    static auto &&get(IdTuple &&other) {
        return Head<N, T>::get(std::move(other));
    }

    static const auto &&get(const IdTuple &&other) {
        return Head<N, T>::get(std::move(other));
    }

};




template<typename ...Args>
class Tuple : public IdTuple<0, Args...> {
public:
    template<typename ...Args_>
    Tuple(Args_ &&... args) : IdTuple<0, Args...>(std::forward<Args_>(args)...) {}

    Tuple() {}

    Tuple(const Tuple &other) = default;

    Tuple(Tuple &other) = default;

    Tuple(Tuple &&other) = default;

    void swap(Tuple &other) &{
        IdTuple<0, Args...>::swap(other);
    }

    Tuple &operator=(const Tuple &other) & {
        IdTuple<0, Args...>::operator=(other);
        return *this;
    }

    Tuple &operator=(Tuple &&other) {
        IdTuple<0, Args...>::operator=(std::move(other));
        return *this;
    }


};


template<size_t I, typename Head, typename ...Tail>
auto &get_from_id(IdTuple<I, Head, Tail...> &id_tuple) {
    return IdTuple<I, Head, Tail...>::get(id_tuple);
}

template<size_t I, typename Head, typename ...Tail>
auto &get_from_id(const IdTuple<I, Head, Tail...> &id_tuple) {
    return IdTuple<I, Head, Tail...>::get(id_tuple);
}

template<size_t I, typename Head, typename ...Tail>
auto &&get_from_id(IdTuple<I, Head, Tail...> &&id_tuple) {
    return IdTuple<I, Head, Tail...>::get(std::move(id_tuple));
}

template<size_t I, typename Head, typename ...Tail>
auto &&get_from_id(const IdTuple<I, Head, Tail...> &&id_tuple) {
    return IdTuple<I, Head, Tail...>::get(std::move(id_tuple));
}

template<size_t I, typename ...Args>
auto &get(Tuple<Args...> &tuple) {
    return get_from_id<I>(tuple);
}

template<size_t I, typename ...Args>
auto &get(const Tuple<Args...> &tuple) {
    return get_from_id<I>(tuple);
}

template<size_t I, typename ...Args>
auto &&get(Tuple<Args...> &&tuple) {
    return get_from_id<I>(std::move(tuple));
}

template<size_t I, typename ...Args>
auto &&get(const Tuple<Args...> &&tuple) {
    return get_from_id<I>(std::move(tuple));
}

template<typename Head, size_t I, typename ...Tail>
auto &get_from_id(IdTuple<I, Head, Tail...> &id_tuple) {
    return IdTuple<I, Head, Tail...>::get(id_tuple);
}

template<typename Head, size_t I, typename ...Tail>
auto &get_from_id(const IdTuple<I, Head, Tail...> &id_tuple) {
    return IdTuple<I, Head, Tail...>::get(id_tuple);
}

template<typename Head, size_t I, typename ...Tail>
auto &&get_from_id(IdTuple<I, Head, Tail...> &&id_tuple) {
    return IdTuple<I, Head, Tail...>::get(std::move(id_tuple));
}

template<typename Head, size_t I, typename ...Tail>
auto &&get_from_id(const IdTuple<I, Head, Tail...> &&id_tuple) {
    return IdTuple<I, Head, Tail...>::get(std::move(id_tuple));
}

template<typename T, typename ...Args>
auto &get(Tuple<Args...> &tuple) {
    return get_from_id<T>(tuple);
}

template<typename T, typename ...Args>
auto &get(const Tuple<Args...> &tuple) {
    return get_from_id<T>(tuple);
}

template<typename T, typename ...Args>
auto &&get(Tuple<Args...> &&tuple) {
    return get_from_id<T>(std::move(tuple));
}

template<typename T, typename ...Args>
auto &&get(const Tuple<Args...> &&tuple) {
    return get_from_id<T>(std::move(tuple));
}

template<typename T, typename U, size_t I, size_t Size>
struct TupleCompare {
    static constexpr bool less(const T &t, const U &u) {
        return (get<I>(t) < get<I>(u)) || (!(get<I>(u) < get<I>(t)) && TupleCompare<T, U, I + 1, Size>::less(t, u));
    }

    static constexpr bool equal(const T &t, const U &u) {
        return (get<I>(t) == get<I>(u)) && TupleCompare<T, U, I + 1, Size>::equal(t, u);
    }
};

template<typename T, typename U, size_t Size>
struct TupleCompare<T, U, Size, Size> {
    static constexpr bool less(const T &, const U &) {
        return false;
    }

    static constexpr bool equal(const T &, const U &) {
        return true;
    }
};

template<typename ...Args>
constexpr bool operator<(const Tuple<Args...> &tup_f, const Tuple<Args...> &tup_s) {
    return TupleCompare<Tuple<Args...>, Tuple<Args...>, 0, sizeof...(Args)>::less(tup_f, tup_s);
}

template<typename ...Args>
constexpr bool operator==(const Tuple<Args...> &tup_f, const Tuple<Args...> &tup_s) {
    return TupleCompare<Tuple<Args...>, Tuple<Args...>, 0, sizeof...(Args)>::equal(tup_f, tup_s);
}

template<typename ...Args>
constexpr bool operator!=(const Tuple<Args...> &tup_f, const Tuple<Args...> &tup_s) {
    return !(tup_f == tup_s);
}

template<typename ...Args>
constexpr bool operator>(const Tuple<Args...> &tup_f, const Tuple<Args...> &tup_s) {
    return tup_s < tup_f;
}

template<typename ...Args>
constexpr bool operator<=(const Tuple<Args...> &tup_f, const Tuple<Args...> &tup_s) {
    return !(tup_s < tup_f);
}

template<typename ...Args>
constexpr bool operator>=(const Tuple<Args...> &tup_f, const Tuple<Args...> &tup_s) {
    return !(tup_f < tup_s);
}

template<typename ...Args>
auto makeTuple(Args &&...args) {
    return Tuple<std::decay_t<Args>...>(std::forward<Args>(args)...);
}

template<size_t ...>
struct Index {};

template<size_t N>
struct BuildIndex {
    template<size_t ...I>
    using typeId = typename BuildIndex<N - 1>::template typeId<N - 1, I...>;
    using type = typeId<>;
};
template<>
struct BuildIndex<0> {
    template<size_t ...Id>
    using typeId = Index<Id...>;
    using type = typeId<>;
};

template<typename ...Args>
struct TupleSize<Tuple<Args...>> {
    static constexpr size_t value = sizeof...(Args);
};
template <size_t N, typename ...Args>
struct TupleSize <IdTuple <N, Args...>> {
    static constexpr size_t value = sizeof...(Args);
};


template<typename ...>
struct BuildTupleIndex {
    using type = BuildIndex<0>::type;
};

template<typename T, typename ...Args>
struct BuildTupleIndex<T, Args...> {
    using type = typename BuildIndex<TupleSize_v<T>>::type;
};

template<typename ...>
struct TupleConcater {
};

template<typename Res, size_t ...Id, typename T, typename ...Args>
struct TupleConcater<Res, Index<Id...>, T, Args...> {
    template<typename ...Units>
    static const auto concatenate(T &&t, Args &&... args, Units &&... units) {
        using next = TupleConcater<Res, typename BuildTupleIndex<remove_all_t<Args>...>::type, Args...>;
        return next::concatenate(std::forward<Args>(args)..., std::forward<Units>(units)...,
                                 get<Id>(std::forward<T>(t))...);
    }
};

template<typename Res>
struct TupleConcater<Res, Index<>> {
    template<typename ...Units>
    static constexpr auto concatenate(Units &&... units) {
        return Res(std::forward<Units>(units)...);
    }
};

template<typename ...Args>
struct Concatenation {
};

template<typename ...Elements, typename ...Tuples>
struct Concatenation<Tuple<Elements...>, Tuples...> {
    template<typename ...Elements_>
    using typeId = typename Concatenation<Tuples...>::template typeId<Elements_..., Elements...>;
    using type = typeId<>;
};

template<>
struct Concatenation<> {
    template<typename ...Args>
    using typeId = Tuple<Args...>;
    using type = typeId<>;
};

template<typename ...Args>
auto tupleCat(Args &&... args) {
    return TupleConcater<typename Concatenation<remove_all_t<Args>...>::type, typename BuildTupleIndex<remove_all_t<Args>...>::type, Args...>::concatenate(
            std::forward<Args>(args)...);
}

