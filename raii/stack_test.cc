// Kompilowanie i uruchamianie
// g++ -Wall -Wextra -O2 -std=c++20 -DTEST_NUM=... stack_test*.cc -o stack_test && ./stack_test && valgrind --error-exitcode=123 --leak-check=full --show-leak-kinds=all --errors-for-leak-kinds=all --run-cxx-freeres=yes -q ./stack_test

#include "stack.h"

#include <cassert>
#include <cstddef>
#include <cstdlib>
#include <algorithm>
#include <iterator>
#include <iostream>
#include <new>
#include <set>
#include <stdexcept>
#include <utility>
#include <vector>

using cxx::stack;

using std::align_val_t;
using std::bad_alloc;
using std::clog;
using std::forward_iterator;
using std::invalid_argument;
using std::invoke_result_t;
using std::make_unique;
using std::move;
using std::multiset;
using std::pair;
using std::set;
using std::swap;
using std::vector;

int main();

namespace {
#if TEST_NUM == 102 || TEST_NUM == 106 || TEST_NUM == 402 || TEST_NUM == 403 || \
    (TEST_NUM > 500 && TEST_NUM <= 599) || TEST_NUM == 603
  // Sprawdzamy równość stosów. To wymaga poprawnego działania większości metod
  // oraz iteratora.
  template <typename K, typename V>
  bool operator==(stack<K, V> const &s1, stack<K, V> const &s2) {
    size_t s1_size = s1.size();
    size_t s2_size = s2.size();

    if (s1_size != s2_size)
      return false;

    stack<K, V> local1 = s1;
    stack<K, V> local2 = s2;

    set<K> keys;
    multiset<pair<K, V>> removed_by_key;
    multiset<pair<K, V>> removed_from_front;
    size_t keys1_count = 0;
    size_t keys2_count = 0;

    typename stack<K, V>::const_iterator it1 = s1.cbegin(), end1 = s1.cend(),
                                         it2 = s2.cbegin(), end2 = s2.cend();
    for (; it1 != end1 && it2 != end2; ++it1, ++it2) {
      if (*it1 != *it2)
        return false;
      size_t c1 = s1.count(*it1), c2 = s2.count(*it2);
      if (c1 != c2)
        return false;
      keys.insert(*it1);
      keys1_count += c1;
      keys2_count += c2;
    }
    if (it1 != end1 || it2 != end2)
      return false;
    if (keys1_count != s1_size || keys2_count != s2_size)
      return false;

    // Sprawdzamy równość stosów za pomocą front(k) i pop(k).
    for (K const &k : keys) {
      while (local1.count(k) > 0) {
        V v1 = local1.front(k);
        V v2 = local2.front(k);
        // W testach używamy klasy V, której obiekty można porównywać.
        if (v1 != v2)
          return false;
        removed_by_key.insert({k, v1});
        local1.pop(k);
        local2.pop(k);
      }
    }
    if (local1.size() > 0 || local2.size() > 0)
      return false;

    // Jeszcze raz kopiujemy i sprawdzamy za pomocą front() lub pop().
    local1 = s1;
    local2 = s2;
    while (local1.size() > 0) {
      pair<K, V> p1 = local1.front();
      pair<K, V> p2 = local2.front();
      if (p1 != p2)
        return false;
      removed_from_front.insert(p1);
      local1.pop();
      local2.pop();
    }
    if (local1.size() > 0 || local2.size() > 0)
      return false;

    if (removed_by_key != removed_from_front)
      return false;

    return true;
  }
#endif

#if TEST_NUM == 102
  auto foo(stack<int, int> q) {
    return q;
  }
#endif

#if TEST_NUM == 103
  template <typename K, typename V>
  set<K> get_keys(stack<K, V> &s) {
    return set<K>(s.cbegin(), s.cend());
  }

  template <typename K, typename V>
  void test_push_spec(stack<K, V> &st, K const &k, V v) {
    stack<K, V> st_old = st;

    st.push(k, v);

    assert(st.size() == st_old.size() + 1);
    assert(st.count(k) == st_old.count(k) + 1);
    assert(st.front(k) == v);
    assert(st.front().second == v);

    for (K const &kk : get_keys(st)) {
      if (kk != k) {
        assert(st.count(kk) == st_old.count(kk));
        assert(st.front(kk) == st_old.front(kk));
      }
    }
  }

  template <typename K, typename V>
  void check_pop_key(stack<K, V> &st_old, stack<K, V> &st, K const &k) {
    set<K> keys = get_keys(st_old);

    assert(st.size() + 1 == st_old.size());
    assert(st.count(k) + 1 == st_old.count(k));

    for (K const &kk : get_keys(st_old)) {
      if (kk != k) {
        assert(st.count(kk) == st_old.count(kk));
        assert(st.front(kk) == st_old.front(kk));
      }
    }

    if (st_old.size() > 0 && st_old.front().first != k)
      assert(st.front() == st_old.front());
  }

  template <typename K, typename V>
  void test_pop_key_spec(stack<K, V> &st, K const &k) {
    stack<K, V> st_old = st;

    st.pop(k);

    check_pop_key(st_old, st, k);
  }

  template <typename K, typename V>
  void test_pop_spec(stack<K, V> &st) {
    stack<K, V> st_old = st;
    K k = st.front().first;

    st.pop();

    check_pop_key(st_old, st, k);
  }
#endif

#if TEST_NUM == 105 || TEST_NUM == 107 || TEST_NUM == 201 || TEST_NUM == 202 || TEST_NUM > 400
  int throw_countdown;
  bool throw_checking = false;

  void ThisCanThrow() {
    if (throw_checking && --throw_countdown <= 0)
      throw 0;
  }
#endif

#if TEST_NUM == 105 || TEST_NUM == 201 || TEST_NUM == 202 || TEST_NUM > 400
  class Key {
    friend int ::main();

  public:
    explicit Key(size_t key = 0) : key(key) {
      ThisCanThrow();
      ++instance_count;
      ++operation_count;
    }

    Key(Key const &other) {
      ThisCanThrow();
      key = other.key;
      ++instance_count;
      ++operation_count;
    }

    Key(Key &&other) {
      key = other.key;
      ++operation_count;
    }

    ~Key() {
      --instance_count;
      ++operation_count;
    }

    auto operator<=>(Key const &other) const {
      ThisCanThrow();
      ++operation_count;
      return key <=> other.key;
    }

    bool operator==(Key const &other) const {
      ThisCanThrow();
      ++operation_count;
      return key == other.key;
    }

  private:
    inline static size_t instance_count = 0;
    inline static size_t operation_count = 0;
    size_t key;
  };
#endif

#if TEST_NUM == 107 || TEST_NUM == 201 || TEST_NUM == 202 || TEST_NUM > 400
  class Value {
    friend int ::main();

  public:
    Value(Value const &other) {
      ThisCanThrow();
      value = other.value;
      ++instance_count;
      ++operation_count;
    }

    ~Value() {
      --instance_count;
      ++operation_count;
    }

  private:
    inline static size_t instance_count = 0;
    inline static size_t operation_count = 0;
    size_t value;

    explicit Value(size_t value) : value(value) {
      ++instance_count;
      ++operation_count;
    }

    Value(Value &&other) = delete;
    Value & operator=(Value const &other) = delete;
    Value & operator=(Value &&other) = delete;

#if TEST_NUM > 400 && TEST_NUM <= 599
  public:
    auto operator<=>(Value const &other) const {
      ++operation_count;
      return value <=> other.value;
    }

    bool operator==(Value const &other) const {
      ++operation_count;
      return value == other.value;
    }
#endif
  };
#endif

#if TEST_NUM > 400 && TEST_NUM <= 499
  template <typename Stack, typename Operation>
  void NoThrowCheck(Stack &s, Operation const &op, char const *name) {
    try {
      throw_countdown = 0;
      throw_checking = true;
      op(s);
      throw_checking = false;
    }
    catch (...) {
      throw_checking = false;
      clog << "Operacja " << name << " podnosi wyjątek.\n";
      assert(false);
    }
  }
#endif

#if TEST_NUM > 500 && TEST_NUM <= 599
  template <typename Stack, typename Operation>
  bool StrongCheck(Stack &s, Stack const &d, Operation const &op, char const *name) {
    bool succeeded = false;

    try {
      throw_checking = true;
      op(s);
      throw_checking = false;
      succeeded = true;
    }
    catch (...) {
      throw_checking = false;
      bool unchanged = d == s;
      if (!unchanged) {
        clog << "Operacja " << name << " nie daje silnej gwarancji\n.";
        assert(unchanged);
      }
    }

    return succeeded;
  }
#endif
} // koniec anonimowej przestrzeni nazw

// Operatory new nie mogą być deklarowane w anonimowej przestrzeni nazw.
#if TEST_NUM > 400 && TEST_NUM <= 599
void* operator new(size_t size) {
  try {
    ThisCanThrow();
    void* p = malloc(size);
    if (!p)
      throw "malloc";
    return p;
  }
  catch (...) {
    throw bad_alloc();
  }
}

void* operator new(size_t size, align_val_t al) {
  try {
    ThisCanThrow();
    void* p = aligned_alloc(static_cast<size_t>(al), size);
    if (!p)
      throw "aligned_alloc";
    return p;
  }
  catch (...) {
    throw bad_alloc();
  }
}

void* operator new[](size_t size) {
  try {
    ThisCanThrow();
    void* p = malloc(size);
    if (!p)
      throw "malloc";
    return p;
  }
  catch (...) {
    throw bad_alloc();
  }
}

void* operator new[](size_t size, align_val_t al) {
  try {
    ThisCanThrow();
    void* p = aligned_alloc(static_cast<size_t>(al), size);
    if (!p)
      throw "aligned_alloc";
    return p;
  }
  catch (...) {
    throw bad_alloc();
  }
}
#endif

int main() {
// Sprawdzamy przykład dołączony do treści zadania z pominięciem wyjątków.
#if TEST_NUM == 101
  stack<int, int> stack1;
  stack1.push(3, 10);
  stack1.push(1, 11);
  stack1.push(2, 12);

  int &ref1 = stack1.front().second;

  stack<int, int> stack2(stack1); // Wykonuje się pełna kopia.
  stack<int, int> stack3;
  stack3 = stack2;                // Nie wykonuje się kopia.

  assert(stack1.front().first == 2);
  assert(stack1.front().second == 12);
  ref1 = 20;
  assert(stack1.front().first == 2);
  assert(stack1.front().second == 20);

  // Wymuszamy wywołanie metod front w wersji const.
  assert((static_cast<const stack<int, int>>(stack2)).front().first == 2);
  assert((static_cast<const stack<int, int>>(stack2)).front().second == 12);
  assert((static_cast<const stack<int, int>>(stack3)).front().first == 2);
  assert((static_cast<const stack<int, int>>(stack3)).front().second == 12);

  stack2.pop(); // Obiekt stack2 dokonuje kopii i przestaje współdzielić dane z obiektem stack3.
  assert(stack2.size() == 2);
  assert(stack2.count(1) == 1);
  assert(stack2.count(2) == 0);
  assert(stack2.count(3) == 1);
  assert(stack3.size() == 3);
  assert(stack3.count(1) == 1);
  assert(stack3.count(2) == 1);
  assert(stack3.count(3) == 1);

  stack3.pop(1);
  assert(stack3.size() == 2);
  assert(stack3.count(1) == 0);
  assert(stack3.count(2) == 1);
  assert(stack3.count(3) == 1);

  stack1.clear();
  assert(stack1.size() == 0);
  assert(stack1.count(3) == 0);

  stack1.push(7, 1);
  stack1.push(7, 2);
  stack1.push(8, 3);
  stack1.push(8, 4);
  int k = 7;
  for (stack<int, int>::const_iterator it = stack1.cbegin(); it != stack1.cend(); ++it)
    assert(*it == k++);

  int &ref2 = stack1.front(7);
  assert(stack1.front(7) == 2);
  ref2 = 33;
  assert(stack1.front(7) == 33);
  stack1.pop(7);
  assert(stack1.front(7) == 1);

  // Wymuszamy wywołanie metod front w wersji const.
  stack<int, int> const stack4 = stack2;
  pair<int const &, int const &> p = stack4.front();
  int const &r = stack4.front(3);
  assert(p.first == 1 && p.second == 11 && r == 10);

  swap(stack1, stack3); // Działa w czasie stałym.

  vector<stack<int, int>> vec;
  for (int i = 0; i < 100000; i++)
    stack1.push(i, i);
  for (int i = 0; i < 1000000; i++)
    vec.push_back(stack1);  // Wszystkie obiekty w vec współdzielą dane.
#endif

// Testujemy konstruktor bezparametrowy, konstruktor kopiujący, konstruktor
// przenoszący, operator przypisania.
#if TEST_NUM == 102
  stack<int, int> q;

  q.push(3, 33);
  q.push(4, 44);
  q.push(4, 45);
  q.push(5, 55);

  stack<int, int> r = q;
  assert(r == q);

  stack<int, int> s(foo(q));
  assert(s == q);

  stack<int, int> t;
  t = q;
  assert(t == q);

  stack<int, int> u(r);
  assert(u == r);

  stack<int, int> v = stack<int, int>(foo(r));
  assert(v == r);

  t = r;
  assert(t == r);

  stack<int, int> w;
  w = move(r);
  assert(t == w);

  r = foo(w);
  assert(t == r);
#endif

// Testujemy metody size, count, push(k, v), pop(), pop(k), front(), front(k),
// clear.
#if TEST_NUM == 103
  stack<int, int> st;
  stack<int, int> const &const_st = st;
  assert(st.size() == 0);

  for (int i = 0; i < 1000; i++)
    test_push_spec(st, i, i);
  assert(const_st.front().first == 999 && const_st.front().second == 999);
  assert(st.size() == 1000);

  test_pop_spec(st);
  assert(const_st.front().first == 998 && const_st.front().second == 998);
  assert(st.size() == 999);

  test_pop_key_spec(st, 0);
  assert(const_st.front().first == 998 && const_st.front().second == 998);
  assert(st.size() == 998);

  for (int i = 998; i >= 500; i--) {
    assert(const_st.front().first == i && const_st.front().second == i);
    test_pop_spec(st);
    test_pop_key_spec(st, 999 - i);
  }
  assert(st.size() == 0);

  for (int j = 0; j <= 99; j++)
    for (int i = 0; i <= 9; i++)
      test_push_spec(st, i, j);

  assert(st.size() == 1000);
  assert(const_st.front().first == 9 && const_st.front().second == 99);

  test_pop_spec(st);
  assert(st.size() == 999);
  assert(const_st.front().first == 8 && const_st.front().second == 99);

  test_pop_key_spec(st, 8);
  assert(st.size() == 998);
  assert(const_st.front().first == 7 && const_st.front().second == 99);

  for (int j = 98; j > 0 ; j--) {
    for (int i = 0; i <= 7; i++) {
      test_pop_key_spec(st, i);
      assert(const_st.front(i) == j);
    }
    test_pop_spec(st);
  }
  assert(st.size() == 116);
  st.clear();
  assert(st.size() == 0);

  for (int i = 0; i <= 99; i++)
    assert(st.count(i) == 0);
#endif

// Jeszcze testujemy metody front i count.
#if TEST_NUM == 104
  stack<int, int> st;

  for (int i = 0; i <= 100; i++) {
    assert(st.count(i) == 0);
    st.push(i, i);
    assert(st.count(i) == 1);
    assert(st.front(i) == i);
  }

  for (int i = 0; i <= 100; i++) {
    st.push(i, i + 1);
    assert(st.count(i) == 2);
    assert(st.front(i) == i + 1);
  }

  for (int i = 0; i <= 99; i++) {
    st.pop(i);
    assert(st.count(i) == 1);
    assert(st.front(i + 1));
  }

  for (int i = 0; i <= 99; i++) {
    st.push(i, i - 1);
    assert(st.count(i) == 2);
    assert(st.front(i) == i - 1);
  }

  for (int i = 2; i <= 99; i += 3) {
    st.pop(i);
    assert(st.count(i) == 1);
    assert(st.front(i) == i);
  }
#endif

// Testujemy metody cbegin, cend i iterator.
#if TEST_NUM == 105
  stack<int, int> st;

  for (int i = 0; i <= 1000; i++)
    for (int j = 0; j <= 10; j++)
      st.push(j, i);

  int i = 0;
  for (stack<int, int>::const_iterator it = st.cbegin(); it != st.cend(); ++it, ++i)
    assert(*it == i);
  assert(i == 11);

  for (int i = 0; i <= 1000; i++) {
    for (int j = 0; j <= 10; j++) {
      st.push(j, i);
    }
  }

  i = 0;
  for (stack<int, int>::const_iterator it = st.cbegin(); it != st.cend(); it++, ++i)
    assert(*it == i);
  assert(i == 11);

  stack<Key, size_t> su;

  for (size_t i = 0; i <= 77; i++)
    su.push(Key{i}, 100 + i);

  size_t j = 0;
  for (stack<Key, size_t>::const_iterator it = su.cbegin(); it != su.cend(); ++it, ++j)
    assert(it->key == j);
  assert(j == 78);
#endif

// Testujemy operację swap.
#if TEST_NUM == 106
  stack<int, int> st;
  for (int i = 0; i < 1000; i++)
    for (int j = 0; j < 10; j++)
      st.push(j, i);
  stack<int, int> su = stack<int, int>(st);

  stack<int, int> sv;
  for (int i = 0; i > -1000; i--)
    for (int j = 0; j < 10; j++)
      sv.push(-j, i);
  stack<int, int> sw = stack<int, int>(sv);

  swap(st, sv);
  assert(st == sw && sv == su);

  swap(st, sv);
  assert(st == su && sv == sw);

  swap(sv, st);
  assert(st == sw && sv == su);

  swap(st, sv);
  assert(st == su && sv == sw);

  swap(st, st);
  assert(st == su);

  stack<int, int> sx, sy;
  swap(sx, sy);
  assert(sx.size() == 0 && sx == sx);
#endif

// Testujemy, czy metody front rzeczywiście zwracają poprawną referencję.
#if TEST_NUM == 107
  stack<size_t, Value> st;

  for (size_t i = 1; i <= 666; i++)
    st.push(i, Value(i));

  for (size_t i = 1; i <= 666; i++)
    assert(st.front(i).value == i);

  assert(st.front().first == 666 && st.front().second.value == 666);

  for (size_t i = 1; i <= 42; i++)
    st.push(i, Value(i + 1000));

  for (size_t i = 1; i <= 42; i++)
    assert(st.front(i).value == i + 1000);
  for (size_t i = 43; i <= 666; i++)
    assert(st.front(i).value == i);
#endif

// Testujemy, czy metoda clear pozostawia obiekt w spójnym stanie.
#if TEST_NUM == 108
  stack<int, int> st;

  st.push(1, 1);
  st.push(2, 2);
  st.push(1, 11);
  st.push(2, 12);
  st.push(6, 6);
  st.push(6, 16);
  st.clear();

  assert(st.size() == 0);
  assert(st.count(1) == 0);
  assert(st.count(2) == 0);
  assert(st.count(6) == 0);

  st.push(1, 1);
  st.push(2, 2);
  st.push(1, 11);
  st.push(2, 12);
  st.push(6, 6);
  st.push(6, 16);
  st.clear();

  stack<int, int> su(st);

  assert(su.size() == 0);
  assert(su.count(1) == 0);
  assert(su.count(2) == 0);
  assert(su.count(6) == 0);

  stack<int, int> sv;

  st.push(1, 1);
  st.push(2, 2);
  st.push(1, 11);
  st.push(2, 12);
  st.push(6, 6);
  st.push(6, 16);
  st.clear();

  sv = st;

  assert(sv.size() == 0);
  assert(sv.count(1) == 0);
  assert(sv.count(2) == 0);
  assert(sv.count(6) == 0);

  st.push(7, 7);
  st.push(7, 17);
  st.push(8, 8);
  st.push(8, 9);
  st.push(7, 18);
  st.clear();

  st.push(8, 8);
  st.push(8, 8);
  st.push(8, 8);

  assert(st.size() == 3);
  assert(st.count(7) == 0);
  assert(st.count(8) == 3);

  st.clear();

  assert(st.count(8) == 0);
  assert(st.size() == 0);

  st.push(9, 9);
  st.push(9, 19);

  st.clear();

  int sum = 0;
  for (auto it = st.cbegin(); it != st.cend(); ++it)
    ++sum;

  assert(sum == 0);

  sum = 0;
  for (auto it = st.cbegin(); it != st.cend(); it++)
    ++sum;

  assert(sum == 0);

  st.push(1, 1);
  st.push(2, 2);
  st.push(1, 11);
  st.push(2, 12);
  st.push(6, 6);
  st.push(6, 16);
  st.clear();

  stack<int, int> sw(move(st));

  assert(sw.size() == 0);
  assert(sw.count(1) == 0);
  assert(sw.count(2) == 0);
  assert(sw.count(6) == 0);
#endif

// Czy iterator spełnia wymagany koncept?
#if TEST_NUM == 109
  int keys[] = {5, 1, 3};
  int values[] = {11, 12, 13};

  stack<int, int> st;
  for (int i = 0; i < 3; ++i)
    st.push(keys[i], values[i]);

  static_assert(forward_iterator<stack<int, int>::const_iterator>);

  auto i1 = st.cbegin();
  static_assert(forward_iterator<decltype(i1)>);
  auto i2 = st.cend();
  static_assert(forward_iterator<decltype(i2)>);
  auto i3 = ++i1;
  static_assert(forward_iterator<decltype(i3)>);
  auto i4 = i3++;
  static_assert(forward_iterator<decltype(i4)>);
#endif

// Test 110 jest w pliku stack_test_external.cc.
#if TEST_NUM == 110
#endif

// Testowanie złóżoności pamięciowej.
// Czy rozwiązanie nie przechowuje zbyt dużo kopii kluczy lub wartości?
#if TEST_NUM == 201
  stack<Key, Value> st;

  for (size_t i = 0; i < 1000; i++)
    for (size_t j = 0; j < 10; j++)
      st.push(Key(i), Value(i));

  clog << "Liczba przechowywanych kluczy: " << Key::instance_count << '\n';
  clog << "Liczba przechowywanych wartości: " << Value::instance_count << '\n';

  assert(st.size() == 10000);
  assert(Value::instance_count == 10000);
  clog << Key::instance_count << std::endl;
  assert(Key::instance_count == 1000);

  for (size_t i = 0; i < 1000; i++)
    assert(st.count(Key(i)) == 10);

  st.push(Key(0), Value(0));

  assert(st.size() == 10001);
  assert(Value::instance_count == 10001);
  assert(Key::instance_count == 1000);
  assert(st.count(Key(0)) == 11);

  st.push(Key(777777), Value(0));

  assert(st.size() == 10002);
  assert(Value::instance_count == 10002);
  assert(Key::instance_count == 1001);
  assert(st.count(Key(777777)) == 1);

  st.pop(Key(777));

  assert(st.size() == 10001);
  assert(Value::instance_count == 10001);
  assert(Key::instance_count == 1001);
  assert(st.count(Key(777)) == 9);

  st.pop();

  assert(st.size() == 10000);
  assert(Value::instance_count == 10000);
  assert(Key::instance_count == 1000);
  assert(st.count(Key(777777)) == 0);
#endif

// Testowanie złóżoności czasowej.
// Czy rozwiązanie nie wykonuje zbyt wielu operacji?
#if TEST_NUM == 202
  do {
    stack<Key, Value> st;

    for (size_t i = 0; i < 1000; i++)
      for (size_t j = 0; j < 10; j++)
        st.push(Key(i), Value(i));

    Key::operation_count = 0;
    Value::operation_count = 0;

    stack<Key, Value> st_copy(st);

    clog << "Liczba operacji wykonanych przez konstruktor kopiujący: "
         << Key::operation_count << ", " << Value::operation_count << ".\n";

    assert(Key::operation_count == 0 && Value::operation_count == 0);
    assert(st_copy.size() == 10000);
  } while (0);

  do {
    stack<Key, Value> st;

    for (size_t i = 0; i < 1000; i++)
      for (size_t j = 0; j < 10; j++)
        st.push(Key(i), Value(i));

    Key::operation_count = 0;
    Value::operation_count = 0;

    stack<Key, Value> st_copy(move(st));

    clog << "Liczba operacji wykonanych przez konstruktor przenoszący: "
         << Key::operation_count << ", " << Value::operation_count << ".\n";

    assert(Key::operation_count == 0 && Value::operation_count == 0);
    assert(st_copy.size() == 10000);
  } while (0);

  do {
    stack<Key, Value> st;

    for (size_t i = 0; i < 1000; i++)
      for (size_t j = 0; j < 10; j++)
        st.push(Key(i), Value(i));

    Key::operation_count = 0;
    Value::operation_count = 0;

    stack<Key, Value> st_copy;

    st_copy = st;

    clog << "Liczba operacji wykonanych przez operator przypisania: "
         << Key::operation_count << ", " << Value::operation_count << ".\n";

    assert(Key::operation_count == 0 && Value::operation_count == 0);
    assert(st_copy.size() == 10000);
  } while (0);

  Key::operation_count = 0;
  Value::operation_count = 0;

  stack<Key, Value> st;

  clog << "Liczba operacji wykonanych przez konstruktor bezparametrowy: "
       << Key::operation_count << ", " << Value::operation_count << ".\n";

  assert(Key::operation_count == 0 && Value::operation_count == 0);

  for (size_t i = 0; i < 1000; i++)
    for (size_t j = 0; j < 10; j++)
      st.push(Key(i), Value(i));

  assert(st.size() == 10000);

  Key::operation_count = 0;
  Value::operation_count = 0;

  st.push(Key(0), Value(0));

  clog << "Liczba operacji wykonanych przez push: "
       << Key::operation_count << ", " << Value::operation_count << ".\n";

  assert(Key::operation_count <= 45 && Value::operation_count <= 15);

  Key::operation_count = 0;
  Value::operation_count = 0;

  st.push(Key(777777), Value(0));

  clog << "Liczba operacji wykonanych przez push: "
       << Key::operation_count << ", " << Value::operation_count << ".\n";

  assert(Key::operation_count <= 45 && Value::operation_count <= 15);

  Key::operation_count = 0;
  Value::operation_count = 0;

  st.pop(Key(777));

  clog << "Liczba operacji wykonanych przez pop(k): "
       << Key::operation_count << ", " << Value::operation_count << ".\n";

  assert(Key::operation_count <= 45 && Value::operation_count <= 15);

  Key::operation_count = 0;
  Value::operation_count = 0;

  st.pop();

  clog << "Liczba operacji wykonanych przez pop: "
       << Key::operation_count << ", " << Value::operation_count << ".\n";

  assert(Key::operation_count <= 45 && Value::operation_count <= 15);

  Key::operation_count = 0;
  Value::operation_count = 0;

  [[maybe_unused]] auto v1 = st.front().first;

  clog << "Liczba operacji wykonanych przez front: "
       << Key::operation_count << ", " << Value::operation_count << ".\n";

  assert(Key::operation_count <= 45 && Value::operation_count <= 15);

  do {
    stack<Key, Value> const st_copy(st);

    Key::operation_count = 0;
    Value::operation_count = 0;

    [[maybe_unused]] auto v = st_copy.front().first;

    clog << "Liczba operacji wykonanych przez front const: "
         << Key::operation_count << ", " << Value::operation_count << ".\n";

    assert(Key::operation_count <= 45 && Value::operation_count <= 15);
  } while (0);

  Key::operation_count = 0;
  Value::operation_count = 0;

  [[maybe_unused]] auto v2 = st.front(Key(555));

  clog << "Liczba operacji wykonanych przez front(k): "
       << Key::operation_count << ", " << Value::operation_count << ".\n";

  assert(Key::operation_count <= 45 && Value::operation_count <= 15);

  do {
    stack<Key, Value> const st_copy(st);

    Key::operation_count = 0;
    Value::operation_count = 0;

    [[maybe_unused]] auto v = st_copy.front(Key(444));

    clog << "Liczba operacji wykonanych przez front(k) const: "
         << Key::operation_count << ", " << Value::operation_count << ".\n";

    assert(Key::operation_count <= 45 && Value::operation_count <= 15);
  } while (0);


  Key::operation_count = 0;
  Value::operation_count = 0;

  [[maybe_unused]] auto size = st.size();

  clog << "Liczba operacji wykonanych przez size: "
       << Key::operation_count << ", " << Value::operation_count << ".\n";

  assert(Key::operation_count == 0 && Value::operation_count == 0);

  Key::operation_count = 0;
  Value::operation_count = 0;

  [[maybe_unused]] auto count = st.count(Key(333));

  clog << "Liczba operacji wykonanych przez count(k): "
       << Key::operation_count << ", " << Value::operation_count << ".\n";

  assert(Key::operation_count <= 45 && Value::operation_count <= 15);
#endif

// Czy podnoszone są wyjątki zgodnie ze specyfikacją?
// pop
#if TEST_NUM == 301
  stack<int, int> s;

  s.push(1, 1);
  s.clear();

  bool exception_catched = false;
  try {
    s.pop();
  }
  catch (invalid_argument const &) {
    exception_catched = true;
  }
  catch (...) {
  }
  assert(exception_catched);
#endif

// pop(k)
#if TEST_NUM == 302
  stack<int, int> s;

  s.push(1, 1);

  bool exception_catched = false;
  try {
    s.pop(2);
  }
  catch (invalid_argument const &) {
    exception_catched = true;
  }
  catch (...) {
  }
  assert(exception_catched);
#endif

// front
#if TEST_NUM == 303
  stack<int, int> s;

  s.push(1, 1);
  s.clear();

  bool exception_catched = false;
  try {
    [[maybe_unused]] auto p = s.front();
  }
  catch (invalid_argument const &) {
    exception_catched = true;
  }
  catch (...) {
  }
  assert(exception_catched);
#endif

// front(k)
#if TEST_NUM == 304
  stack<int, int> s;

  s.push(1, 1);

  bool exception_catched = false;
  try {
    [[maybe_unused]] auto v = s.front(2);
  }
  catch (invalid_argument const &) {
    exception_catched = true;
  }
  catch (...) {
  }
  assert(exception_catched);
#endif

// Czy metoda size nie podnosi wyjątku?
#if TEST_NUM == 401
  stack<Key, Value> s;

  s.push(Key(1), Value(11));
  s.push(Key(2), Value(12));
  s.push(Key(3), Value(13));
  s.push(Key(1), Value(21));
  s.push(Key(2), Value(22));
  s.push(Key(3), Value(23));

  size_t size;

  NoThrowCheck(s, [&](auto const &s) {size = s.size();}, "size");
  assert(size == 6);
#endif

// Czy konstruktor przenoszący nie podnosi wyjątku?
#if TEST_NUM == 402
  stack<Key, Value> s1, s2;

  for (size_t i = 1; i <= 7; i++)
    for (size_t j = 1; j <= 11; j++)
      s1.push(Key(j), Value(i));

  size_t size = s1.size();

  stack<Key, Value> s1_copy(s1);

  NoThrowCheck(s1, [&s2](auto &s) {s2 = move(s);}, "move");
  assert(s1_copy == s2);
  assert(s2.size() == size);
#endif

// Czy operacja swap nie podnosi wyjątku?
#if TEST_NUM == 403
  stack<Key, Value> s1, s2;

  for (size_t i = 1; i <= 8; i++)
    for (size_t j = 1; j <= 9; j++)
      s1.push(Key(j), Value(i));

  size_t size = s1.size();

  stack<Key, Value> s1_copy(s1);
  stack<Key, Value> s2_copy(s2);

  auto f = [&s2](auto &s) {swap(s, s2);};

  NoThrowCheck(s1, f, "swap");
  assert(s1_copy == s2);
  assert(s2_copy == s1);
  assert(s2.size() == size);

  NoThrowCheck(s1, f, "swap");
  assert(s1_copy == s1);
  assert(s2_copy == s2);
  assert(s1.size() == size);
#endif

// Czy destruktor nie podnosi wyjątku?
#if TEST_NUM == 404
  char buf[sizeof (stack<Key, Value>)];
  stack<Key, Value> *s = new (buf) stack<Key, Value>();

  for (int i = 1; i <= 9; i++)
    for (int j = 1; j <= 9; j++)
      s->push(Key(j), Value(i));

  NoThrowCheck(*s, [](auto &s) {s.~stack<Key, Value>();}, "destruktor");
#endif

// Czy iterator nie podnosi wyjątku?
#if TEST_NUM == 405
  stack<Key, Value> s;

  for (int i = 1; i <= 4; i++)
    for (int j = 1; j <= 15; j++)
      s.push(Key(j), Value(i));

  size_t sum1 = 0;
  auto f1 = [&](auto &s) {for (auto it = s.cbegin(); it != s.cend(); ++it) sum1 += it->key;};
  NoThrowCheck(s, f1, "iterator");
  assert(sum1 == 120);

  size_t sum2 = 0;
  auto f2 = [&](auto &s) {for (auto it = s.cbegin(); it != s.cend(); it++) sum2 += it->key;};
  NoThrowCheck(s, f2, "iterator");
  assert(sum2 == 120);
#endif

// Czy metoda clear nie podnosi wyjątku?
#if TEST_NUM == 406
  stack<Key, Value> s;

  s.push(Key(1), Value(11));
  s.push(Key(2), Value(12));
  s.push(Key(3), Value(13));
  s.push(Key(1), Value(21));
  s.push(Key(2), Value(22));
  s.push(Key(3), Value(23));

  NoThrowCheck(s, [](auto &s) {s.clear();}, "clear");
  assert(s.size() == 0);
#endif

// Czy metoda clear nie podnosi wyjątku?
#if TEST_NUM == 407
  stack<Key, Value> s1;

  s1.push(Key(1), Value(11));
  s1.push(Key(2), Value(12));
  s1.push(Key(3), Value(13));
  s1.push(Key(1), Value(21));
  s1.push(Key(2), Value(22));
  s1.push(Key(3), Value(23));

  stack<Key, Value> s2(s1);

  NoThrowCheck(s1, [](auto &s) {s.clear();}, "clear");
  assert(s1.size() == 0);
  assert(s2.size() == 6);
#endif

// Testujemy silne gwarancje. Te testy sprawdzają też przeźroczystość na wyjątki.
// push
#if TEST_NUM == 501
  bool success = false;
  int trials;

  for (trials = 1; !success; trials++) {
    success = true;

    stack<Key, Value> s, d;

    Key k1(999), k2(0);
    Value v1(9999), v2(100);
    s.push(k1, v1);
    d.push(k1, v1);

    throw_countdown = trials;
    bool result = StrongCheck(s, d, [&](auto &s) {s.push(k2, v2);}, "push");
    if (result) {
      assert(s.front().first == k2 &&  s.front().second == v2);
      d.push(k2, v2);
    }
    success &= result;

    for (size_t i = 100; i < 110; i++) {
      Key k3(i);
      Value v3(100);

      throw_countdown = trials;
      result = StrongCheck(s, d, [&](auto &s) {s.push(k3, v3);}, "push");
      if (result) {
        assert(s.front().first == k3 &&  s.front().second == v3);
        d.push(k3, v3);
      }
      success &= result;
    }

    for (size_t i = 0; i < 10; i++) {
      Key k3((48271 * i) % 31);
      Value v3(100);
      s.push(k3, v3);
      d.push(k3, v3);
    }

    for (size_t i = 0; i < 10; i++) {
      Key k3((16807 * i) % 31);
      Value v3(100);

      throw_countdown = trials;
      result = StrongCheck(s, d, [&](auto &s) {s.push(k3, v3);}, "push");
      if (result) {
        assert(s.front().first == k3 &&  s.front().second == v3);
        d.push(k3, v3);
      }
      success &= result;
    }

    assert(s == d);
  }

  clog << "Liczba prób wykonanych dla push: " << trials << ".\n";
#endif

// pop
#if TEST_NUM == 502
  bool success = false, result;
  int trials;

  for (trials = 1; !success; trials++) {
    success = true;

    stack<Key, Value> s, d;

    Key k1(1), k2(2);
    Value v1(41), v2(42);

    s.push(k1, v1);
    s.push(k2, v1);
    s.push(k1, v2);
    s.push(k2, v2);

    d.push(k1, v1);
    d.push(k2, v1);
    d.push(k1, v2);
    d.push(k2, v2);

    throw_countdown = trials;

    for (int i = 0; i < 4; i++) {
      result = StrongCheck(s, d, [](auto &s) {s.pop();}, "pop");
      if (result)
        d.pop();
      success &= result;
    }

    assert(s == d);
  }

  clog << "Liczba prób wykonanych dla pop: " << trials << ".\n";
#endif

// pop(k)
#if TEST_NUM == 503
  bool success = false, result;
  int trials;

  for (trials = 1; !success; trials++) {
    success = true;

    stack<Key, Value> s, d;

    Key k1(1), k2(2);
    Value v1(41), v2(42);

    s.push(k1, v1);
    s.push(k1, v1);
    s.push(k2, v1);
    s.push(k2, v1);
    s.push(k1, v2);
    s.push(k1, v2);
    s.push(k2, v2);
    s.push(k2, v2);

    d.push(k1, v1);
    d.push(k1, v1);
    d.push(k2, v1);
    d.push(k2, v1);
    d.push(k1, v2);
    d.push(k1, v2);
    d.push(k2, v2);
    d.push(k2, v2);

    throw_countdown = trials;

    for (int i = 0; i < 2; i++) {
      result = StrongCheck(s, d, [&](auto &s) {s.pop(k1);}, "pop(k)");
      if (result)
        d.pop(k1);
      success &= result;
    }

    assert(s == d);

    for (int i = 0; i < 2; i++) {
      result = StrongCheck(s, d, [&](auto &s) {s.pop(k2);}, "pop(k)");
      if (result)
        d.pop(k2);
      success &= result;
    }

    assert(s == d);

    for (int i = 0; i < 2; i++) {
      result = StrongCheck(s, d, [&](auto &s) {s.pop(k1);}, "pop(k)");
      if (result)
        d.pop(k1);
      success &= result;
    }

    assert(s == d);

    for (int i = 0; i < 2; i++) {
      result = StrongCheck(s, d, [&](auto &s) {s.pop(k2);}, "pop(k)");
      if (result)
        d.pop(k2);
      success &= result;
    }

    assert(s == d);
  }

  clog << "Liczba prób wykonanych dla pop(k): " << trials << ".\n";
#endif

// front
#if TEST_NUM == 504
  bool success = false, result, op_result;
  int trials;

  for (trials = 1; !success; trials++) {
    success = true;

    stack<Key, Value> s, d;

    Key k1(1), k2(2);
    Value v1(41), v2(42);

    s.push(k1, v1);
    s.push(k2, v1);
    s.push(k1, v2);
    s.push(k2, v2);

    d.push(k1, v1);
    d.push(k2, v1);
    d.push(k1, v2);
    d.push(k2, v2);

    throw_countdown = trials;

    result = StrongCheck(s, d, [&](auto &s) {op_result = s.front().first == k2;}, "front");
    if (result)
      assert(op_result);
    success &= result;

    result = StrongCheck(s, d, [&](auto &s) {op_result = s.front().second == v2;}, "front");
    if (result)
      assert(op_result);
    success &= result;

    result = StrongCheck(s, d, [&](auto const &s) {op_result = s.front().first == k2;}, "front");
    if (result)
      assert(op_result);
    success &= result;

    result = StrongCheck(s, d, [&](auto const &s) {op_result = s.front().second == v2;}, "front");
    if (result)
      assert(op_result);
    success &= result;

    assert(s == d);
  }

  clog << "Liczba prób wykonanych dla front: " << trials << ".\n";
#endif

// front(k)
#if TEST_NUM == 505
  bool success = false, result, op_result;
  int trials;

  for (trials = 1; !success; trials++) {
    success = true;

    stack<Key, Value> s, d;

    Key k1(1), k2(2);
    Value v1(41), v2(42);

    s.push(k1, v1);
    s.push(k2, v2);
    s.push(k1, v1);
    s.push(k2, v2);

    d.push(k1, v1);
    d.push(k2, v2);
    d.push(k1, v1);
    d.push(k2, v2);

    throw_countdown = trials;

    result = StrongCheck(s, d, [&](auto &s) {op_result = s.front(k1) == v1;}, "front(k)");
    if (result)
      assert(op_result);
    success &= result;

    result = StrongCheck(s, d, [&](auto &s) {op_result = s.front(k2) == v2;}, "front(k)");
    if (result)
      assert(op_result);
    success &= result;

    result = StrongCheck(s, d, [&](auto const &s) {op_result = s.front(k1) == v1;}, "front(k)");
    if (result)
      assert(op_result);
    success &= result;

    result = StrongCheck(s, d, [&](auto const &s) {op_result = s.front(k2) == v2;}, "front(k)");
    if (result)
      assert(op_result);
    success &= result;

    assert(s == d);
  }

  clog << "Liczba prób wykonanych dla front(k): " << trials << ".\n";
#endif

// count(k)
#if TEST_NUM == 506
  bool success = false, result, op_result;
  int trials;

  for (trials = 1; !success; trials++) {
    success = true;

    stack<Key, Value> s, d;

    Key k1(1), k2(2);
    Value v1(41), v2(42);

    s.push(k1, v1);
    s.push(k2, v2);
    s.push(k1, v1);
    s.push(k2, v2);
    s.push(k2, v2);

    d.push(k1, v1);
    d.push(k2, v2);
    d.push(k1, v1);
    d.push(k2, v2);
    d.push(k2, v2);

    throw_countdown = trials;

    result = StrongCheck(s, d, [&](auto const &s) {op_result = s.count(k1) == 2;}, "count(k)");
    if (result)
      assert(op_result);
    success &= result;

    result = StrongCheck(s, d, [&](auto const &s) {op_result = s.count(k2) == 3;}, "count(k)");
    if (result)
      assert(op_result);
    success &= result;

    assert(s == d);
  }

  clog << "Liczba prób wykonanych dla count(k): " << trials << ".\n";
#endif

// clear
#if TEST_NUM == 507
  bool success = false, result;
  int trials;

  for (trials = 1; !success; trials++) {
    success = true;

    stack<Key, Value> s, d;

    Key k1(1), k2(2);
    Value v1(41), v2(42);

    s.push(k1, v1);
    s.push(k2, v2);
    s.push(k1, v1);
    s.push(k2, v2);
    s.push(k2, v2);

    throw_countdown = trials;

    result = StrongCheck(s, d, [](auto &s) {s.clear();}, "clear");
    if (result)
      assert(s == d);
    success &= result;
  }

  clog << "Liczba prób wykonanych dla clear: " << trials << ".\n";
#endif

// przypisanie
#if TEST_NUM == 508
  bool success = false, result;
  int trials;

  for (trials = 1; !success; trials++) {
    success = true;

    stack<Key, Value> s, d, u;

    Key k1(1), k2(2);
    Value v1(41), v2(42);

    s.push(k1, v1);
    s.push(k2, v2);
    s.push(k1, v1);
    s.push(k2, v2);
    s.push(k2, v2);
    [[maybe_unused]] auto p = s.front(); // Wymuszamy wykonanie kopii.

    d.push(k1, v1);
    d.push(k2, v2);
    d.push(k1, v1);
    d.push(k2, v2);
    d.push(k2, v2);

    throw_countdown = trials;

    result = StrongCheck(s, d, [&](auto &s) {u = s;}, "operator przypisania");
    if (result)
      assert(u == d);
    success &= result;
  }

  clog << "Liczba prób wykonanych dla operatora przypisania: " << trials << ".\n";
#endif

// konstruktor kopiujący
#if TEST_NUM == 509
  bool success = false;
  int trials;

  for (trials = 1; !success; trials++) {
    stack<Key, Value> s, d;

    Key k1(1), k2(2);
    Value v1(41), v2(42);

    s.push(k1, v1);
    s.push(k2, v2);
    s.push(k1, v1);
    s.push(k2, v2);
    s.push(k2, v2);
    [[maybe_unused]] auto p = s.front(); // Wymuszamy wykonanie kopii.

    d.push(k1, v1);
    d.push(k2, v2);
    d.push(k1, v1);
    d.push(k2, v2);
    d.push(k2, v2);

    throw_countdown = trials;
    try {
      throw_checking = true;
      stack<Key, Value> u(s);
      throw_checking = false;
      success = true;
      assert(u == d);
    }
    catch (...) {
      throw_checking = false;
      bool unchanged = d == s;
      if (!unchanged) {
        clog << "Konstruktor kopiujący nie daje silnej gwarancji\n.";
        assert(unchanged);
      }
    }
  }

  clog << "Liczba prób wykonanych dla konstruktora kopiującego: " << trials << ".\n";
#endif

// Testujemy ważność iteratora po zgłoszeniu wyjątku.
#if TEST_NUM == 510
  int trials;
  bool success = false;

  for (trials = 1; !success; trials++) {
    stack<Key, Value> s1;

    s1.push(Key(0), Value(10));
    s1.push(Key(1), Value(11));
    s1.push(Key(2), Value(12));

    auto s2 = make_unique<stack<Key, Value>>(s1);

    s2->push(Key(3), Value(13));

    stack<Key, Value> s3 = *s2;

    auto it = s3.cbegin();
    auto end = s3.cend();
    Key key(0);
    Value value(100);

    throw_countdown = trials;
    try {
      throw_checking = true;
      s3.push(key, value);
      throw_checking = false;
      break; // Jeśli się udało, to poniżej będą problemy.
    }
    catch (...) {
      throw_checking = false;
    }
    s2.reset();
    for (int i = 0; it != end; ++it, ++i)
      assert(*it == Key(i));
  }

  clog << "Liczba wykonanych prób: " << trials << ".\n";
#endif

// Testujemy przywracanie stanu po zgłoszeniu wyjątku.
#if TEST_NUM == 511
  bool success = false;
  int trials;

  stack<Key, Value> s1;
  s1.push(Key(1), Value(1));
  stack<Key, Value> s2(s1);

  assert(Key::instance_count == 1);
  assert(Value::instance_count == 1);

  for (trials = 1; !success; trials++) {
    throw_countdown = trials;

    try {
      throw_checking = true;
      s2.push(Key(2), Value(2));
      throw_checking = false;
      success = true;
    }
    catch (...) {
      throw_checking = false;
      success = false;
    }

    if (success) {
      assert(Key::instance_count == 3);
      assert(Value::instance_count == 3);
    }
    else {
      assert(Key::instance_count == 1);
      assert(Value::instance_count == 1);
    }
  }

  clog << "Liczba wykonanych prób: " << trials << ".\n";
#endif

// Testujemy przywracanie stanu po zgłoszeniu wyjątku.
#if TEST_NUM == 512
  bool success = false;
  int trials;

  stack<Key, Value> s1;
  s1.push(Key(1), Value(1));
  s1.push(Key(2), Value(2));
  stack<Key, Value> s2(s1);

  assert(Key::instance_count == 2);
  assert(Value::instance_count == 2);

  for (trials = 1; !success; trials++) {
    throw_countdown = trials;

    try {
      throw_checking = true;
      s2.pop();
      throw_checking = false;
      success = true;
    }
    catch (...) {
      throw_checking = false;
      success = false;
    }

    if (success) {
      assert(Key::instance_count == 3);
      assert(Value::instance_count == 3);
    }
    else {
      assert(Key::instance_count == 2);
      assert(Value::instance_count == 2);
    }
  }

  clog << "Liczba wykonanych prób: " << trials << ".\n";
#endif

// Testujemy przywracanie stanu po zgłoszeniu wyjątku.
#if TEST_NUM == 513
  bool success = false;
  int trials;

  stack<Key, Value> s1;
  s1.push(Key(1), Value(1));
  s1.push(Key(2), Value(2));
  stack<Key, Value> s2(s1);

  assert(Key::instance_count == 2);
  assert(Value::instance_count == 2);

  for (trials = 1; !success; trials++) {
    throw_countdown = trials;

    try {
      throw_checking = true;
      s2.pop(Key(1));
      throw_checking = false;
      success = true;
    }
    catch (...) {
      throw_checking = false;
      success = false;
    }

    if (success) {
      assert(Key::instance_count == 3);
      assert(Value::instance_count == 3);
    }
    else {
      assert(Key::instance_count == 2);
      assert(Value::instance_count == 2);
    }
  }

  clog << "Liczba wykonanych prób: " << trials << ".\n";
#endif

// Testujemy kopiowanie przy modyfikowaniu.
#if TEST_NUM == 601
  stack<Key, Value> s1;

  for (int i = 1; i <= 7; i++) {
    s1.push(Key(i), Value(0));
    for (int j = 1; j <= 9; j++) {
      s1.push(Key(i), Value(j));
      s1.push(Key(j), Value(i));
    }
  }

  size_t k_count = Key::instance_count;

  clog << "Liczba przechowywanych kluczy: " << Key::instance_count << '\n';
  clog << "Liczba przechowywanych wartości: " << Value::instance_count << '\n';

  assert(Value::instance_count == 133);

  stack<Key, Value> s2(s1);
  stack<Key, Value> s3;
  stack<Key, Value> s4;
  s3 = s1;
  s4 = move(s2);

  assert(s1.size() == 133);
  assert(s3.size() == 133);
  assert(s4.size() == 133);

  clog << "Liczba przechowywanych kluczy: " << Key::instance_count << '\n';
  clog << "Liczba przechowywanych wartości: " << Value::instance_count << '\n';

  assert(Key::instance_count == k_count);
  assert(Value::instance_count == 133);

  assert(s3.front().second.value == 7);

  clog << "Liczba przechowywanych kluczy: " << Key::instance_count << '\n';
  clog << "Liczba przechowywanych wartości: " << Value::instance_count << '\n';

  assert(Key::instance_count == 2 * k_count);
  assert(Value::instance_count == 2 * 133);

  assert(s4.front(Key(7)).value == 9);

  clog << "Liczba przechowywanych kluczy: " << Key::instance_count << '\n';
  clog << "Liczba przechowywanych wartości: " << Value::instance_count << '\n';

  assert(Key::instance_count == 3 * k_count);
  assert(Value::instance_count == 3 * 133);

  s1.pop();
  s1.push(Key(13), Value(17));
  s1.pop(Key(9));

  clog << "Liczba przechowywanych kluczy: " << Key::instance_count << '\n';
  clog << "Liczba przechowywanych wartości: " << Value::instance_count << '\n';

  assert(Key::instance_count == 3 * k_count + 1);
  assert(Value::instance_count == 3 * 133 - 1);

  stack<Key, Value> s5(s1);
  stack<Key, Value> s6;
  stack<Key, Value> s7;
  s6 = s1;
  s7 = move(s5);

  clog << "Liczba przechowywanych kluczy: " << Key::instance_count << '\n';
  clog << "Liczba przechowywanych wartości: " << Value::instance_count << '\n';

  assert(Key::instance_count == 3 * k_count + 1);
  assert(Value::instance_count == 3 * 133 - 1);

  s1.pop();
  s1.push(Key(15), Value(18));
  s1.pop(Key(9));

  clog << "Liczba przechowywanych kluczy: " << Key::instance_count << '\n';
  clog << "Liczba przechowywanych wartości: " << Value::instance_count << '\n';

  assert(Key::instance_count == 4 * k_count + 2);
  assert(Value::instance_count == 4 * 133 - 3);

  s7.clear();

  clog << "Liczba przechowywanych kluczy: " << Key::instance_count << '\n';
  clog << "Liczba przechowywanych wartości: " << Value::instance_count << '\n';

  assert(Key::instance_count == 4 * k_count + 2);
  assert(Value::instance_count == 4 * 133 - 3);

  s1.clear();

  clog << "Liczba przechowywanych kluczy: " << Key::instance_count << '\n';
  clog << "Liczba przechowywanych wartości: " << Value::instance_count << '\n';

  assert(Key::instance_count == 3 * k_count + 1);
  assert(Value::instance_count == 3 * 133 - 1);
#endif

// Testujemy unieważnianie udzielonych referencji.
#if TEST_NUM == 602
  stack<Key, Value> s1;

  for (int i = 1; i <= 3; i++) {
    s1.push(Key(i), Value(0));
    for (int j = 1; j <= 3; j++) {
      s1.push(Key(j), Value(i));
      s1.push(Key(i), Value(j));
    }
  }

  size_t k_count = Key::instance_count;

  clog << "Liczba przechowywanych kluczy: " << Key::instance_count << '\n';
  clog << "Liczba przechowywanych wartości: " << Value::instance_count << '\n';

  assert(Value::instance_count == 21);
  assert(s1.front().second.value == 3);

  s1.front().second.value = 333;

  assert(s1.front().second.value == 333);

  s1.pop();

  clog << "Liczba przechowywanych kluczy: " << Key::instance_count << '\n';
  clog << "Liczba przechowywanych wartości: " << Value::instance_count << '\n';

  assert(Key::instance_count == k_count);
  assert(Value::instance_count == 20);

  stack<Key, Value> s2(s1);
  stack<Key, Value> s3;
  stack<Key, Value> s4;
  s3 = s1;
  s4 = move(s2);

  assert(s1.size() == 20);
  assert(s3.size() == 20);
  assert(s4.size() == 20);

  clog << "Liczba przechowywanych kluczy: " << Key::instance_count << '\n';
  clog << "Liczba przechowywanych wartości: " << Value::instance_count << '\n';

  assert(Key::instance_count == k_count);
  assert(Value::instance_count == 20);

  assert(s1.front(Key(2)).value == 3);

  s1.front(Key(2)).value = 33;

  assert(s1.front(Key(2)).value == 33);
  assert(Key::instance_count == 2 * k_count);
  assert(Value::instance_count == 40);

  s1.pop(Key(2));

  assert(s1.size() == 19);

  clog << "Liczba przechowywanych kluczy: " << Key::instance_count << '\n';
  clog << "Liczba przechowywanych wartości: " << Value::instance_count << '\n';

  assert(Key::instance_count == 2 * k_count);
  assert(Value::instance_count == 39);

  stack<Key, Value> s5(s1);
  stack<Key, Value> s6;
  stack<Key, Value> s7;
  s6 = s1;
  s7 = move(s5);

  assert(s6.size() == 19);
  assert(s7.size() == 19);

  clog << "Liczba przechowywanych kluczy: " << Key::instance_count << '\n';
  clog << "Liczba przechowywanych wartości: " << Value::instance_count << '\n';

  assert(Key::instance_count == 2 * k_count);
  assert(Value::instance_count == 39);
#endif

#if TEST_NUM == 603
  auto st = make_unique<stack<int, int>>();
  vector<stack<int, int>> vec;
  for (int i = 0; i < 77777; i++)
    st->push(i, i);
  for (int i = 0; i < 88888; i++)
    vec.push_back(*st);
  st.reset();
  for (int i = 0; i < 10; i++)
    vec[i].push(i, i);
  for (int i = 0; i < 10; i++) {
    for (int j = 0; j < 11; j++) {
      if (i != j) {
        assert(!(vec[i] == vec[j]));
      }
    }
  }
  assert(!(vec[0] == vec[88887]));
#endif

// Testujemy rozdzielanie stosów przy modyfikowaniu wartości.
#if TEST_NUM == 604
  size_t k_count;

  do {
    stack<Key, long> s1;

    s1.push(Key(101), 111);
    s1.push(Key(102), 122);
    s1.push(Key(101), 121);
    s1.push(Key(103), 113);
    s1.push(Key(103), 123);
    s1.push(Key(102), 112);
    s1.push(Key(103), 133);

    k_count = Key::instance_count;

    clog << "Liczba przechowywanych kluczy: " << Key::instance_count << '\n';

    stack<Key, long> s2(s1);

    clog << "Liczba przechowywanych kluczy: " << Key::instance_count << '\n';

    assert(Key::instance_count == k_count);

    long &v1 = s1.front().second;
    v1 = 233;

    clog << "Liczba przechowywanych kluczy: " << Key::instance_count << '\n';

    clog << 2 * k_count << '\n';
    assert(Key::instance_count == 2 * k_count);

    assert(s1.front().second == 233);
    assert(s2.front().second == 133);
  } while (0);

  clog << "Liczba przechowywanych kluczy: " << Key::instance_count << '\n';

  assert(Key::instance_count == 0);

  do {
    stack<Key, long> s1;

    s1.push(Key(101), 111);
    s1.push(Key(102), 122);
    s1.push(Key(101), 121);
    s1.push(Key(103), 113);
    s1.push(Key(103), 123);
    s1.push(Key(102), 112);
    s1.push(Key(103), 133);

    clog << "Liczba przechowywanych kluczy: " << Key::instance_count << '\n';

    assert(Key::instance_count == k_count);

    stack<Key, long> s2(s1);

    clog << "Liczba przechowywanych kluczy: " << Key::instance_count << '\n';

    assert(Key::instance_count == k_count);

    long &v2 = s1.front(Key(101));
    v2 = 221;

    clog << "Liczba przechowywanych kluczy: " << Key::instance_count << '\n';

    assert(Key::instance_count == 2 * k_count);

    assert(s1.front(Key(101)) == 221);
    assert(s2.front(Key(101)) == 121);
  } while (0);
#endif
}
