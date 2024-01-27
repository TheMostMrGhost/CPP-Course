#ifndef STACK_H
#define STACK_H

#include <list>
#include <map>
#include <memory>
#include <stack>

// Pattern for handling copy-on-write together with strong guarantee for use
// inside stack functions.
// Note: A stack is safe if no valid non-const references were shared.
#define COPY_ON_WRITE_CORE(expr, safety)                                       \
    if (!st) {                                                                 \
        st = std::make_shared<impl>();                                         \
        expr;                                                                  \
    }                                                                          \
    else {                                                                     \
        auto _old_impl = st;                                                   \
        try {                                                                  \
            if (st.use_count() > 2) {                                          \
                st = std::make_shared<impl>(impl(*st));                        \
            }                                                                  \
            st->is_safe = safety;                                              \
            expr;                                                              \
        } catch (...) {                                                        \
            st = _old_impl;                                                    \
            throw;                                                             \
        }                                                                      \
    }                                                                         

#define COPY_SAFE(expr) COPY_ON_WRITE_CORE(expr, true);
#define COPY_UNSAFE(expr) COPY_ON_WRITE_CORE(expr, false);

namespace cxx {

// Comparison functor for std::shared_ptr<T> objects.
struct deref_compare {
    template <typename T>
    bool operator()(const std::shared_ptr<T> &lhs,
                    const std::shared_ptr<T> &rhs) const {
        return *lhs < *rhs;
    }
};

template <typename K, typename V> class stack {

    using key_val_list = typename std::list<std::pair<std::shared_ptr<K>, V>>;
    using l_iter = typename key_val_list::iterator;
    using key_val_map = typename std::map<std::shared_ptr<K>,
                                          std::stack<l_iter>, deref_compare>;
    using key_val_map_cit =
        typename std::map<std::shared_ptr<K>, std::stack<l_iter>,
                          deref_compare>::const_iterator;

  public:
    class const_iterator;

    stack();
    stack(stack const &);
    stack(stack &&) noexcept;

    void push(K const &, V const &);
    void pop();
    void pop(K const &);
    std::pair<K const &, V &> front();
    std::pair<K const &, V const &> front() const;
    V &front(K const &);
    V const &front(K const &) const;
    size_t size() const noexcept;
    size_t count(K const &) const;
    void clear() noexcept;
    stack &operator=(stack) noexcept;
    const_iterator cbegin() const noexcept;
    const_iterator cend() const noexcept;

  private:
    /*
     * Realization of the stack using PIMPL idiom.
     * Allows to implement copy-on-write.
     */
    class impl {
      public:
        impl();
        impl(const impl &);
        impl(impl &&) noexcept;

        void push(K const &, V const &);
        void pop();
        void pop(K const &);
        std::pair<K const &, V &> front();
        std::pair<K const &, V const &> front() const;
        V &front(K const &);
        V const &front(K const &) const;
        size_t size() const noexcept;
        size_t count(K const &) const;
        void clear() noexcept;
        stack &operator=(stack) noexcept;

      private:
        // Indicates whether the object returned a non-const reference.
        bool is_safe = true;
        key_val_map key_order;
        key_val_list pos_order;
        size_t sz;

        // Performs deep copy on the provided object and saves it in this.
        void deep_copy(const impl &other);

        friend class stack;
        friend class const_iterator;
    };

    std::shared_ptr<impl> st;
};
} // namespace cxx

template <typename K, typename V>
cxx::stack<K, V>::stack() : st(std::make_shared<impl>()) {}

template <typename K, typename V>
cxx::stack<K, V>::stack(cxx::stack<K, V> &&other) noexcept
    : st(std::move(other.st)) {}

template <typename K, typename V>
cxx::stack<K, V>::stack(cxx::stack<K, V> const &other) : st(other.st) {
    // If the object has published a non-const reference, a deep copy is done.
    if (!st->is_safe) {
        st = std::make_shared<impl>(impl(*st));
    }
}

template <typename K, typename V>
void cxx::stack<K, V>::push(K const &key, V const &value) {
    COPY_SAFE(st->push(key, value))
}

template <typename K, typename V> void cxx::stack<K, V>::pop() {
    COPY_SAFE(st->pop());
}

template <typename K, typename V> void cxx::stack<K, V>::pop(K const &key) {
    COPY_SAFE(st->pop(key));
}

template <typename K, typename V>
std::pair<K const &, V &> cxx::stack<K, V>::front() {
    COPY_UNSAFE(auto res = st->front(); return res;)
}

template <typename K, typename V>
std::pair<K const &, V const &> cxx::stack<K, V>::front() const {
    return st->front();
}

template <typename K, typename V> V &cxx::stack<K, V>::front(K const &key) {
    COPY_UNSAFE(V &res = st->front(key); return res;)
}

template <typename K, typename V>
V const &cxx::stack<K, V>::front(K const &key) const {
    return st->key_order.find(std::make_shared<K>(key))->second.top()->second;
}

template <typename K, typename V>
size_t cxx::stack<K, V>::size() const noexcept {
    if (!st) {
        return 0;
    }

    return st->size();
}

template <typename K, typename V>
size_t cxx::stack<K, V>::count(K const &key) const {
    return st->count(key);
}

template <typename K, typename V> void cxx::stack<K, V>::clear() noexcept {
    if (st.use_count() > 1) {
        st.reset();
    }
    else {
        st->clear();
    }
}

template <typename K, typename V>
cxx::stack<K, V> &cxx::stack<K, V>::operator=(cxx::stack<K, V> other) noexcept {
    std::swap(st, other.st);
    return *this;
}

template <typename K, typename V>
cxx::stack<K, V>::impl::impl() : is_safe(true), sz(0) {}

template <typename K, typename V>
cxx::stack<K, V>::impl::impl(const impl &other) : is_safe(true), sz(other.sz) {
    this->deep_copy(other);
}

template <typename K, typename V>
cxx::stack<K, V>::impl::impl(impl &&other) noexcept
    : is_safe(true), key_order(std::move(other.key_order)),
      pos_order(std::move(other.pos_order)), sz(std::move(other.sz)) {}

template <typename K, typename V>
void cxx::stack<K, V>::impl::push(K const &key, V const &value) {
    std::shared_ptr<K> new_key = std::make_shared<K>(key);
    auto it = key_order.find(new_key);
    if (it != key_order.end())
      new_key = it->first;
    pos_order.emplace_back(std::shared_ptr<K>(new_key), value);

    try {
        if (it == key_order.end()) {
            std::stack<l_iter> s;
            s.push(--pos_order.end());
            key_order.insert(std::make_pair(new_key, std::move(s)));
        } else {
            it->second.push(--pos_order.end());
        }
    } catch (...) {
        pos_order.pop_back();
        throw;
    }

    sz++;
}

template <typename K, typename V> void cxx::stack<K, V>::impl::pop() {
    if (sz == 0) {
        throw std::invalid_argument("The stack is empty.");
    }

    auto last = pos_order.back();
    auto it = key_order.find(last.first);

    if (it->second.size() == 1) {
        key_order.erase(it);
    } else {
        it->second.pop();
    }

    pos_order.pop_back(); // Note: This is noexcept.
    sz--;
}

template <typename K, typename V>
void cxx::stack<K, V>::impl::pop(K const &key) {
    if (sz == 0) {
        throw std::invalid_argument("The stack is empty.");
    }

    auto search_key = std::make_shared<K>(key);
    auto it = key_order.find(search_key);

    if (it == key_order.end()) {
        throw std::invalid_argument("The stack does not contain provided key.");
    }

    l_iter removed_element_iter = it->second.top();

    if (it->second.size() == 1) {
        key_order.erase(it);
    } else {
        it->second.pop();
    }

    pos_order.erase(removed_element_iter); // Note: This is noexcept.
    sz--;
}

template <typename K, typename V>
std::pair<K const &, V &> cxx::stack<K, V>::impl::front() {
    if (sz == 0) {
        throw std::invalid_argument("The stack is empty.");
    }

    auto &last = pos_order.back();
    return std::make_pair(std::ref(*last.first), std::ref(last.second));
}

template <typename K, typename V>
std::pair<K const &, V const &> cxx::stack<K, V>::impl::front() const {
    if (sz == 0) {
        throw std::invalid_argument("The stack is empty.");
    }

    return pos_order.back();
}

template <typename K, typename V>
V &cxx::stack<K, V>::impl::front(K const &key) {
    if (sz == 0) {
        throw std::invalid_argument("The stack is empty.");
    }

    auto new_key = std::make_shared<K>(key);
    auto it = key_order.find(new_key);

    if (it == key_order.end()) {
        throw std::invalid_argument("The stack does not contain provided key.");
    }

    return std::ref(it->second.top()->second);
}

template <typename K, typename V>
V const &cxx::stack<K, V>::impl::front(K const &key) const {
    return front(std::forward(key));
}

template <typename K, typename V>
size_t cxx::stack<K, V>::impl::size() const noexcept {
    return sz;
}

template <typename K, typename V>
size_t cxx::stack<K, V>::impl::count(K const &key) const {
    auto key_ptr = std::make_shared<K>(key);
    auto key_iter = key_order.find(key_ptr);

    if (key_iter == key_order.end()) {
        return 0;
    }

    return key_iter->second.size();
}

template <typename K, typename V>
void cxx::stack<K, V>::impl::clear() noexcept {
    key_order.clear();
    pos_order.clear();
    sz = 0;
    is_safe = true;
}

template <typename K, typename V>
cxx::stack<K, V> &
cxx::stack<K, V>::impl::operator=(cxx::stack<K, V> other) noexcept {
    std::swap(key_order, other.key_order);
    std::swap(pos_order, other.pos_order);
    std::swap(sz, other.sz);
    return *this;
}

template <typename K, typename V>
void cxx::stack<K, V>::impl::deep_copy(const impl &other) {
    key_val_map new_map;
    key_val_list new_list;

    // Copy pos_order from other.
    for (const auto &pair : other.pos_order) {
        auto new_key = std::make_shared<K>(*pair.first);

        // Insert or update in the map
        auto key_it = new_map.find(new_key);
        if (key_it == new_map.end()) {
            new_list.push_back(std::make_pair(new_key, pair.second));
            std::stack<l_iter> stack;
            stack.push(--new_list.end());
            new_map[new_key] = std::move(stack);
        } else {
            new_list.push_back(std::make_pair(key_it->first, pair.second));
            key_it->second.push(--new_list.end());
        }
    }

    this->key_order = std::move(new_map);
    this->pos_order = std::move(new_list);
    this->sz = other.sz;
    this->is_safe = true;
}

template <typename K, typename V> class cxx::stack<K, V>::const_iterator {
  public:
    using value_type = K;
    using difference_type = typename key_val_map_cit::difference_type;

    const_iterator() = default;
    const_iterator(const const_iterator &other) = default;
    const_iterator(const_iterator &&other) noexcept = default;
    const_iterator &operator=(const const_iterator &other) = default;
    const_iterator &operator=(const_iterator &&other) noexcept = default;

    const_iterator &operator++() {
        ++it;
        return *this;
    }

    const_iterator operator++(int) {
        const_iterator temp = *this;
        ++(*this);
        return temp;
    }

    const K &operator*() const { return *it->first; }

    const K *operator->() const { return it->first.get(); }

    bool operator==(const const_iterator &other) const {
        return it == other.it;
    }

    bool operator!=(const const_iterator &other) const {
        return it != other.it;
    }

  private:
    const_iterator(key_val_map_cit it) : it(it) {}

    key_val_map_cit it;

    friend class stack;
};

template <typename K, typename V>
typename cxx::stack<K, V>::const_iterator
cxx::stack<K, V>::cbegin() const noexcept {
    return const_iterator(st->key_order.cbegin());
}

template <typename K, typename V>
typename cxx::stack<K, V>::const_iterator
cxx::stack<K, V>::cend() const noexcept {
    return const_iterator(st->key_order.cend());
}

#endif // !STACK_H
