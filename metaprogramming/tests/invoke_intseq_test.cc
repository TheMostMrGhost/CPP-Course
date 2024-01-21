#include "invoke_intseq.h"
#include <algorithm>
#include <array>
#include <cassert>
#include <iostream>
#include <ranges>
#include <sstream>
#include <string>
#include <tuple>
#include <utility>

namespace {

std::stringstream ss;

template <class... Args>
void Print(const Args&... args)
{
    ((ss << args << ' '), ...);
    ss << std::endl;
}

template <class T, T... t>
struct Printer
{
    static void Print()
    {
        ((ss << t << ' '), ...);
        ss << std::endl;
    }
};

struct Foo
{
    static size_t instances_count;
    int id;

    void Init()
    {
        id = instances_count++;
    }

    Foo()
    {
        Init();
        ss << "Foo(): id = " << id << std::endl;
    }

    Foo(const Foo& o)
    {
        Init();
        ss << "Foo(const Foo&) id = " << o.id << " -> id = " << id << std::endl;
    }

    Foo(Foo&& o)
    {
        id = o.id;
        ss << "Foo(Foo&&) id = " << id << std::endl;
    }

    void Print()
    {
        ss << "Foo::Print() id = " << id << std::endl;
    }
};

size_t Foo::instances_count = 0;

struct Caller
{
    Foo foo;

    template <class... Args>
    void operator()(const Args&... args) const
    {
        Print(args...);
    }
};

#if TEST_NUM == 101 || TEST_NUM == 201
std::ostream& operator<<(std::ostream& s, const Foo& a)
{
    s << "[Foo: " << a.id << "]";
    return s;
}
#endif

}

int main() {

#if TEST_NUM == 101
    // std::invoke
    Foo foo;
    constexpr auto make_number = [](auto x, auto y, auto z) {return 100 * x + 10 * y + z;};

    static_assert(std::is_same_v<decltype(invoke_intseq([](auto...) {}, 1, 2, 3)), void>);
    static_assert(std::is_same_v<decltype(invoke_intseq([](auto...) {return 0;}, 1, 2, 3)), int>);
    invoke_intseq([]() {ss << "nothing" << std::endl;});
    invoke_intseq([](auto... a) {Print(a...);}, 1, 2, 3);
    invoke_intseq(Print<int, int, int>, 4, 5, 6);
    invoke_intseq(&Foo::Print, foo);
    assert(invoke_intseq(make_number, 9, 8, 7) == 987);
    invoke_intseq([](const auto&... a) {Print(a...);}, Foo(), foo);
    invoke_intseq([](auto... a) {Print(a...);}, Foo(), foo);
    invoke_intseq([](auto&... a) {Print(a...);}, foo, foo);

    assert(ss.view() ==
        "Foo(): id = 0\n"
        "nothing\n"
        "1 2 3 \n"
        "4 5 6 \n"
        "Foo::Print() id = 0\n"
        "Foo(): id = 1\n"
        "[Foo: 1] [Foo: 0] \n"
        "Foo(): id = 2\n"
        "Foo(Foo&&) id = 2\n"
        "Foo(const Foo&) id = 0 -> id = 3\n"
        "[Foo: 2] [Foo: 3] \n"
        "[Foo: 0] [Foo: 0] \n"
    );
#endif

#if TEST_NUM == 201
    // void result, perfect forwarding args
    Foo foo;
    static_assert(std::is_same_v<decltype(invoke_intseq([](auto...) {}, std::integer_sequence<int, 1, 2, 3>(), 4, 5)), void>);
    static_assert(std::ranges::range<decltype(invoke_intseq([](auto...) {return 0;}, std::integer_sequence<int, 1, 2, 3>(), 4, 5))>);
    static_assert(std::is_same_v<std::ranges::range_value_t<decltype(invoke_intseq([](auto...) {return 0;}, std::integer_sequence<int, 1, 2, 3>(), 4, 5))>, int>);
    invoke_intseq([](auto... a) {Print(a...);}, std::integer_sequence<int, 1, 2, 3>(), size_t(10), std::integer_sequence<int, 7, 8>());
    invoke_intseq([](auto... a) {Print(a...);}, std::integer_sequence<int, 1, 2, 3>(), std::integral_constant<size_t, 11>(), std::integer_sequence<int, 7, 8>());
    invoke_intseq([](auto... a) {Print(a...);}, "ala", size_t(10), std::integer_sequence<int, 7, 8>());

    invoke_intseq([](const auto&... a) {Print(a...);}, std::make_index_sequence<2>(), Foo(), foo);
    invoke_intseq([](auto... a) {Print(a...);}, std::make_index_sequence<2>(), Foo(), foo);
    invoke_intseq([]<class... Args>(Args&&... a) {Print(std::forward<Args>(a)...);}, std::make_index_sequence<2>(), Foo(), foo);
    assert(ss.view() ==
        "Foo(): id = 0\n"
        "1 10 7 \n"
        "1 10 8 \n"
        "2 10 7 \n"
        "2 10 8 \n"
        "3 10 7 \n"
        "3 10 8 \n"
        "1 11 7 \n"
        "1 11 8 \n"
        "2 11 7 \n"
        "2 11 8 \n"
        "3 11 7 \n"
        "3 11 8 \n"
        "ala 10 7 \n"
        "ala 10 8 \n"
        "Foo(): id = 1\n"
        "0 [Foo: 1] [Foo: 0] \n"
        "1 [Foo: 1] [Foo: 0] \n"
        "Foo(): id = 2\n"
        "Foo(Foo&&) id = 2\n"
        "Foo(const Foo&) id = 0 -> id = 3\n"
        "0 [Foo: 2] [Foo: 3] \n"
        "Foo(Foo&&) id = 2\n"
        "Foo(const Foo&) id = 0 -> id = 4\n"
        "1 [Foo: 2] [Foo: 4] \n"
        "Foo(): id = 5\n"
        "0 [Foo: 5] [Foo: 0] \n"
        "1 [Foo: 5] [Foo: 0] \n"
    );
#endif

#if TEST_NUM == 202
    // Ten test jest w pliku invoke_intseq_extern.cc.
#endif

#if TEST_NUM == 203
    Foo foo;
    invoke_intseq([](auto&...) {}, foo);
#endif

// Testy 204 i 205 nie powinny się kompilować: rvalue nie może być przekazywane jako auto&.
#if TEST_NUM == 204
    invoke_intseq([](auto&...) {}, Foo());
#endif

#if TEST_NUM == 205
    invoke_intseq([](auto&...) {}, std::make_index_sequence<3>());
#endif

#if TEST_NUM == 301
    // void result, perfect forwarding of f
    Caller caller;
    invoke_intseq(caller, std::integer_sequence<int, 3, 2, 1>(), "caller");
    invoke_intseq(Caller{}, std::integer_sequence<int, 3, 2, 1>(), "caller");
    invoke_intseq(invoke_intseq([](auto&& f) -> decltype(auto) {return f;}, caller), std::integer_sequence<int, 4, 5>{});
    assert(ss.view() ==
        "Foo(): id = 0\n"
        "3 caller \n"
        "2 caller \n"
        "1 caller \n"
        "Foo(): id = 1\n"
        "3 caller \n"
        "2 caller \n"
        "1 caller \n"
        "4 \n5 \n"
    );
#endif

#if TEST_NUM == 401
    // non-void result
    constexpr auto make_number = [](auto x, auto y, auto z) {return 100 * x + 10 * y + z;};
    for (auto i : invoke_intseq(make_number, std::integer_sequence<int, 6>(), 5, 4))
        ss << i << std::endl;
    for (auto i : invoke_intseq(make_number, std::integer_sequence<int>(), 5, 4))
        ss << i << std::endl;
    for (auto i : invoke_intseq(make_number, 1, std::integer_sequence<int, 2, 3>(), std::integer_sequence<int, 4, 5>()))
        ss << i << std::endl;
    for (const auto& t : invoke_intseq([](auto a, auto b, int c, auto d) {return std::make_tuple(a, b, c, d);}, "ala", "ma", std::integer_sequence<int, 5, 7, 9>(), "kotów"))
        std::apply([](auto... a) {Print(a...);}, t);
    assert(ss.view() ==
        "654\n"
        "124\n"
        "125\n"
        "134\n"
        "135\n"
        "ala ma 5 kotów \n"
        "ala ma 7 kotów \n"
        "ala ma 9 kotów \n"
    );
#endif

#if TEST_NUM == 501
    // constexpr
    constexpr auto make_number = [](auto x, auto y, auto z) {return 100 * x + 10 * y + z;};
    static_assert(invoke_intseq(make_number, 9, 8, 7) == 987);
    auto template_print = [=](auto... a) {Printer<int, make_number(a...)>::Print();};
    invoke_intseq(template_print, std::integer_sequence<int, 1, 2>(), std::integral_constant<int, 3>(), std::integer_sequence<int, 7, 8>());
    invoke_intseq([](auto... a) {Printer<int, a...>::Print();}, std::integer_sequence<int, 1, 2>(), std::integral_constant<int, 3>(), std::integer_sequence<int, 7, 8>());
    static_assert(std::ranges::equal(invoke_intseq(make_number, std::integer_sequence<int, 6>(), 5, 4), std::array{654}));
    static_assert(std::ranges::equal(invoke_intseq(make_number, std::integer_sequence<int>(), 5, 4), std::array<int, 0>{}));
    static_assert(std::ranges::equal(invoke_intseq(make_number, 1, std::integer_sequence<int, 2, 3>(), std::integer_sequence<int, 4, 5>()), std::array{124, 125, 134, 135}));
    invoke_intseq([]<int I>(std::integral_constant<int, I>) {Printer<int, I>::Print();},
                  std::integer_sequence<int, 3, 1, 4>{});
    assert(ss.view() ==
        "137 \n"
        "138 \n"
        "237 \n"
        "238 \n"
        "1 3 7 \n"
        "1 3 8 \n"
        "2 3 7 \n"
        "2 3 8 \n"
        "3 \n1 \n4 \n"
    );
#endif

#if TEST_NUM == 601
    // reference result
    size_t s = 0;
    auto ref_seq = invoke_intseq([](size_t a, size_t& r) -> size_t& {r += a; return r;}, std::make_index_sequence<5>(), s);
    assert(s == 10);
    for (size_t& r : ref_seq)
        r++;
    assert(s == 15);
    invoke_intseq([](size_t a, size_t& r) -> size_t& {r += a; return r;}, 200, s) += 3000;
    assert(s == 3215);
    std::array<std::array<int, 3>, 2> a{};
    for (int& i : invoke_intseq([&](int i, int j) -> int& {return a[i][j];}, std::integer_sequence<int, 0, 1, 0>{}, std::integer_sequence<int, 0, 1, 1, 1, 2, 2>{}))
        i++;
    assert(std::ranges::equal(a, std::array{std::array{2, 6, 4}, std::array{1, 3, 2}}));
#endif
}
