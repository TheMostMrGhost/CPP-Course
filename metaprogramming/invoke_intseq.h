#ifndef INVOKE_INTSEQ
#define INVOKE_INTSEQ

#include <functional>
#include <iostream>
#include <optional>
#include <tuple>
#include <type_traits>
#include <utility>
#include <vector>

template <class F, class... Args>
constexpr auto invoke_intseq(F &&f, Args &&...args) -> decltype(auto);

namespace invoke_intseq_details {

template <typename T> struct is_integer_sequence : std::false_type {
    using type = T;
};

template <typename T, T... vals>
struct is_integer_sequence<std::integer_sequence<T, vals...>> : std::true_type {
    using type = std::integral_constant<T, 0>;
};

template <typename T, T First, T... rest>
struct is_integer_sequence<std::integer_sequence<T, First, rest...>>
    : std::true_type {
    using type = std::integral_constant<T, First>;
};

/* Checks if any of 'others' is of type std::integer_sequence. */
template <class... others>
struct any_match : std::disjunction<is_integer_sequence<others>...> {};

template <typename T> struct is_empty_integer_sequence : std::false_type {};

template <typename T>
struct is_empty_integer_sequence<std::integer_sequence<T>> : std::true_type {};

/* Checks if any of 'others' is an empty integer sequence. */
template <class... others>
struct any_empty_integer_sequence
    : std::disjunction<is_empty_integer_sequence<others>...> {};

// Get the result type of F.
template <typename F, typename... Args> struct get_f_result_type {
    using type =
        std::invoke_result_t<F, typename is_integer_sequence<Args>::type...>;
};

// T1 is not a tuple so the use of the template is incorrect, if this ever gets
// called. Exists to use for specialization.
template <typename T1, typename T2, typename... Remaining_args>
struct generate_struct {
    constexpr static auto generate() { return; }
};

/* Recursion - argument current_arg to the function is not an integer sequence.
 * Function inserts reference current_arg into previous_args copying the tuple,
 * then it calls generate for remaining args.*/
template <typename... Previous_args, typename Current_arg,
          typename... Remaining_args>
struct generate_struct<std::tuple<Previous_args...>, Current_arg,
                       Remaining_args...> {
    constexpr static auto generate(std::tuple<Previous_args...> &&previous_args,
                                   Current_arg &&current_arg,
                                   Remaining_args &&...remaining_args) {
        std::tuple<Current_arg &&> tup(std::forward<Current_arg>(current_arg));
        return generate_struct<std::tuple<Previous_args..., Current_arg &&>,
                               Remaining_args...>::
            generate(
                std::tuple_cat(
                    std::forward<std::tuple<Previous_args...>>(previous_args),
                    std::forward<std::tuple<Current_arg &&>>(tup)),
                std::forward<Remaining_args>(remaining_args)...);
    }
};

/* Recursion - current argument beeing processed is an integer sequence.
 * The function merges each of the ints into previous_args seperately,
 * creating a new tuple for each int. Then calls generate
 * on each tuple. Then it merges the results.*/
template <typename Int_type, Int_type... ints, typename... Remaining_args,
          typename... Previous_args>
struct generate_struct<std::tuple<Previous_args...>,
                       std::integer_sequence<Int_type, ints...>,
                       Remaining_args...> {
    constexpr static auto generate(std::tuple<Previous_args...> &&previous_args,
                                   std::integer_sequence<Int_type, ints...>,
                                   Remaining_args &&...remaining_args) {
        return std::apply(
            [](auto &&...args) {
                return std::tuple_cat(std::forward<decltype(args)>(args)...);
            },
            std::make_tuple( 
                generate_struct<std::tuple<Previous_args &&..., 
                                            std::integral_constant<Int_type, ints> &&>,
                                Remaining_args...>::
                generate(
                         std::tuple_cat(
                                std::forward<std::tuple<Previous_args &&...>>(previous_args),
                                std::tuple<std::integral_constant<Int_type, ints>>(
                                std::integral_constant<Int_type, ints>())
                         ),
                    std::forward<Remaining_args>(remaining_args)...
                )...
            )
        );
    }
};

/* A case when the last argument is an integer sequence.
 * Instead of calling generate impl it returns a tuple of tuples
 * containing each being a copy of previous_args with an added integer.*/
template <typename Int_type, Int_type... ints, typename... Previous_args>
struct generate_struct<std::tuple<Previous_args...>,
                       std::integer_sequence<Int_type, ints...>> {
    constexpr static auto generate(std::tuple<Previous_args...> &&previous_args,
                                   std::integer_sequence<Int_type, ints...>) {
        return std::make_tuple(std::tuple_cat(
            std::forward<std::tuple<Previous_args...>>(previous_args),
            std::make_tuple(std::integral_constant<Int_type, ints>()))...);
    }
};

/* A case when the last element is not an integer sequence.
 * Merges last into remaining_args and returns the new tuple.*/
template <typename Last, typename... Previous_args>
struct generate_struct<std::tuple<Previous_args...>, Last> {
    constexpr static auto generate(std::tuple<Previous_args...> &&previous_args,
                                   Last &&last) {
        std::tuple<Last &&> tup(std::forward<Last>(last));
        return std::make_tuple(std::tuple_cat(
            std::forward<std::tuple<Previous_args...>>(previous_args),
            std::forward<std::tuple<Last &&>>(tup)));
    }
};

/* Generates all possible combinations of integer sequences, invokes
 * function 'f' and deals with its return type.
 */
template <class F, typename... Args>
constexpr static auto apply_to_comb(F &&f, Args &&...args) -> decltype(auto) {
    using f_res_t =
        typename invoke_intseq_details::
            get_f_result_type<decltype(f), Args...>::type;

    using result_t = std::conditional_t<
        std::is_reference_v<f_res_t>,
        std::reference_wrapper<std::remove_reference_t<f_res_t>>, 
        f_res_t>;

    auto invoke_args = generate_struct<std::tuple<>, Args...>::generate(
        std::forward<std::tuple<>>(std::tuple<>()),
        std::forward<Args>(args)...);

    if constexpr (std::is_same_v<void, f_res_t>) {
        // f returns nothing (void).
        std::apply(
            [&f](auto &&...x) {
                (..., std::apply(std::forward<F>(f),
                                 std::forward<decltype(x)>(x)));
            },
            std::forward<decltype(invoke_args)>(invoke_args));
    } else {
        std::vector<result_t> result{};

        std::apply(
            [&](auto &&...x) {
                (..., result.push_back(std::forward<f_res_t>(std::apply(
                          std::forward<F>(f), std::forward<decltype(x)>(x)))));
            },
            std::forward<decltype(invoke_args)>(invoke_args));
        return result;
    }
}

} // namespace invoke_intseq_details

template <class F, class... Args>
constexpr auto invoke_intseq(F &&f, Args &&...args) -> decltype(auto) {
    if constexpr (!invoke_intseq_details::any_match<Args...>::value ||
                  sizeof...(args) == 0) {
        // No argument of type std::integer_sequence - no special processing.
        return std::invoke(std::forward<F>(f), std::forward<Args>(args)...);
    } else if constexpr (!invoke_intseq_details::
                             any_empty_integer_sequence<Args...>::value) {
        // Process one or more empty arguments of type std::integer_sequence.
        return invoke_intseq_details::apply_to_comb(
            std::forward<F>(f), std::forward<Args>(args)...);
    } else {
        using result_type =
            typename invoke_intseq_details::get_f_result_type<decltype(f),
                                                              Args...>::type;
        // Process arguments of type std::integer_sequence<T>, so
        // integer_sequences with no integers.
        return std::vector<result_type>{};
    }
}

#endif // !INVOKE_INTSEQ
