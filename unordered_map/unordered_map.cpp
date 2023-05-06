#include <vector>
#include <functional>
#include <list>
#include <iostream>
#include <cmath>


template<typename T, typename Alloc = std::allocator<T>>
class List;


template<typename Key, typename Value, typename NodePtr, typename Hash, typename Equal, typename Alloc>
class _HashTablePtr { // this container keep iterators of List<Node*> like keys

public:
    using NodeType = std::pair<const Key, Value>;

private:
    template<typename T>
    using CommonAlloc = typename std::allocator_traits<Alloc>::template rebind_alloc<T>;

    struct Node {
        Node(NodePtr pair_ptr, Node *next, size_t cached) : pair_ptr(pair_ptr), next(next), cached(cached) {}

        NodePtr pair_ptr;
        Node *next = nullptr;
        size_t cached;
    };

    using AllocTraits = typename std::allocator_traits<CommonAlloc<Node>>;

public:


    _HashTablePtr(_HashTablePtr &&other) : table_(std::move(other.table_)) {
        other.table_.resize(other.begin_size_, nullptr);
    }

    _HashTablePtr(const Alloc &alloc) : alloc_(alloc), table_(begin_size_, nullptr, alloc_) {}

    _HashTablePtr &operator=(_HashTablePtr &&other) {
        ClearTable();
        alloc_ = other.alloc_;
        table_ = std::move(other.table_);
        return *this;
    }

    NodeType *find(const Key &key, size_t cached) {
        std::pair<NodePtr, bool> iter = FindIterator(key, cached);
        if (iter.second) {
            return *iter.first;
        }
        return nullptr;
    }

    std::pair<NodePtr, bool> FindIterator(const Key &key, size_t cached) {
        size_t cached_mod = cached % table_.size();
        Node *ptr = table_[cached_mod];
        while (ptr != nullptr) {
            if (equal_((*ptr->pair_ptr)->first, key)) {
                return {ptr->pair_ptr, true};
            }
            ptr = ptr->next;
        }
        return {NodePtr(), false};
    }

    void insert(NodePtr new_ptr, size_t cached) {
        size_t cached_mod = cached % table_.size();
        Node *new_node = AllocTraits::allocate(alloc_, 1);
        AllocTraits::construct(alloc_, new_node, new_ptr, table_[cached_mod], cached);
        table_[cached_mod] = new_node;
    }

    void erase(const Key &key, size_t cached) {

        size_t cached_mod = cached % table_.size();
        Node *ptr = table_[cached_mod];
        if (equal(key, ptr->pair_ptr)) {
            table_[cached_mod] = ptr->next;
            AllocTraits::destroy(alloc_, ptr);
            AllocTraits::deallocate(alloc_, ptr, 1);
            return;
        }

        while (ptr->next != nullptr) {
            if (equal(key, ptr->next->pair_ptr)) {
                Node *erase_ptr = ptr->next;
                ptr->next = erase_ptr->next;
                AllocTraits::destroy(alloc_, erase_ptr);
                AllocTraits::deallocate(alloc_, erase_ptr, 1);
                return;
            }
            ptr = ptr->next;
        }
    }

    void ClearTable() {
        for (auto &ptr : table_) {
            auto copy_ptr = ptr;
            while (copy_ptr != nullptr) {
                auto prev_ptr = copy_ptr;
                copy_ptr = copy_ptr->next;
                AllocTraits::destroy(alloc_, prev_ptr);
                AllocTraits::deallocate(alloc_, prev_ptr, 1);
            }
        }
        table_.resize(begin_size_, nullptr);
    }

    void NewAlloc(const Alloc &new_alloc) {
        alloc_ = new_alloc;
        table_ = std::vector<Node *, CommonAlloc<Node *>>(begin_size_, nullptr, new_alloc);
    }

    void rehash(size_t n) {
        std::vector<Node *, CommonAlloc<Node *>> old_table_ = std::move(table_);
        table_.resize(n, nullptr);
        for (auto bucket: old_table_) {
            auto ptr = bucket;
            while (ptr != nullptr) {
                auto prev_ptr = ptr;
                ptr = ptr->next;
                InsertNode(prev_ptr);
            }
        }
    }


    size_t bucket_count() const {
        return table_.size();
    }

private:

    bool equal(const Key &key, const NodePtr &ptr) {
        return equal_(key, (*ptr)->first);
    }

    bool equal(const NodePtr &ptr, const Key &key) {
        return equal_(key, (*ptr)->first);
    }

    void InsertNode(Node *ptr) {
        size_t cached_mod = ptr->cached % table_.size();
        ptr->next = table_[cached_mod];
        table_[cached_mod] = ptr;
    }

private:
    const size_t begin_size_ = 16;
    Hash hash_;
    Equal equal_;
    CommonAlloc<Node> alloc_;
    std::vector<Node *, CommonAlloc<Node *>> table_;
};


template<typename Key, typename Value, typename Hash = std::hash<Key>, typename Equal = std::equal_to<Key>, typename Alloc = std::allocator<std::pair<const Key, Value>>>
class UnorderedMap {

public:
    using NodeType = std::pair<const Key, Value>;

private:
    template<typename T>
    using CommonAlloc = typename std::allocator_traits<Alloc>::template rebind_alloc<T>;
    using AllocTraits = typename std::allocator_traits<CommonAlloc<NodeType>>;
    using IterableContainer = List<NodeType *, CommonAlloc<NodeType *>>; //= List <NodeType*, CommonAlloc<NodeType*>>;
    using NodePtr = typename IterableContainer::iterator;
    using ConstNodePtr = typename IterableContainer::const_iterator;

    template<bool IsConst>
    struct common_iterator {
    private:
        using T_ = std::conditional_t<IsConst, const NodeType, NodeType>;
        using NodePtr_ = std::conditional_t<IsConst, ConstNodePtr, NodePtr>;
    public:
        using iterator_category = std::forward_iterator_tag;
        using value_type = T_;
        using difference_type = ptrdiff_t;
        using pointer = T_ *;
        using reference = T_ &;

    public:
        common_iterator() = default;

        common_iterator(const common_iterator &other) : iter_(other.iter_) {}

        common_iterator &operator=(const common_iterator &other) {
            iter_ = other.iter_;
            return *this;
        }

        common_iterator &operator++() {
            ++iter_;
            return *this;
        }

        common_iterator operator++(int) {
            common_iterator copy = *this;
            ++iter_;
            return copy;
        }

        bool operator==(const common_iterator &other) const {
            return iter_ == other.iter_;
        }

        bool operator!=(const common_iterator &other) const {
            return iter_ != other.iter_;
        }

        T_ &operator*() const {
            return *(*iter_);
        }

        T_ *operator->() const {
            return *iter_;
        }

        operator common_iterator<true>() const {
            return common_iterator<true>(iter_);
        }

    private:
        friend common_iterator<false>;
        friend UnorderedMap;

        common_iterator(NodePtr_ iter) : iter_(iter) {}

        NodePtr_ iter_;
    };

public:
    using iterator = common_iterator<false>;
    using const_iterator = common_iterator<true>;

    UnorderedMap() : hash_table_(alloc_), container_(alloc_) {}

    UnorderedMap(const Alloc &alloc) : alloc_(alloc), hash_table_(alloc_), container_(alloc_) {}

    UnorderedMap(const UnorderedMap &other) : alloc_(AllocTraits::select_on_container_copy_construction(other.alloc_)),
                                              hash_table_(alloc_), container_(alloc_) {
        for (auto &x: other.container_) {
            NodeType *new_pair = AllocTraits::allocate(alloc_, 1);
            AllocTraits::construct(alloc_, new_pair, *x);
            InsertPtr(new_pair, hash_(x->first));
        }
    }

    UnorderedMap(UnorderedMap &&other) : alloc_(other.alloc_), hash_table_(std::move(other.hash_table_)),
                                         container_(std::move(other.container_)) {}

    ~UnorderedMap() {
        Destroy();
    }

    UnorderedMap &operator=(const UnorderedMap &other) {
        if (this == &other) return *this;
        Destroy();
        if (AllocTraits::propagate_on_container_copy_assignment::value) {
            alloc_ = other.alloc_;
            hash_table_.NewAlloc(alloc_);
            container_ = IterableContainer(alloc_);
        }
        for (auto &x: other.container_) {
            NodeType *new_pair = AllocTraits::allocate(alloc_, 1);
            AllocTraits::construct(alloc_, new_pair, *x);
            InsertPtr(new_pair, hash_(x->first));
        }
        return *this;
    }

    UnorderedMap &operator=(UnorderedMap &&other) {
        if (this == &other) return *this;

        alloc_ = other.alloc_;
        hash_table_ = std::move(other.hash_table_);
        container_ = std::move(other.container_);
        return *this;
    }

    Value &operator[](const Key &key) {
        size_t cached = hash_(key);
        NodeType *ptr = hash_table_.find(key, cached);
        if (ptr == nullptr) {
            ptr = AllocTraits::allocate(alloc_, 1);
            AllocTraits::construct(alloc_, ptr, key, Value());
            InsertPtr(ptr, cached);
        }
        Fix();
        return ptr->second;
    }

    Value &at(const Key &key) {
        size_t cached = hash_(key);
        NodeType *ptr = hash_table_.find(key, cached);
        if (ptr == nullptr) {
            throw std::out_of_range("key isn't correct");
        }
        Fix();
        return ptr->second;
    }

    std::pair<iterator, bool> insert(const NodeType &node) {
        return CommonInsert(node);
    }

    template<typename T>
    std::pair<iterator, bool> insert(T &&node) {
        return CommonInsert(std::forward<T>(node));
    }

    template<typename InputIterator>
    void insert(const InputIterator &begin_iter, const InputIterator &end_iter) {
        for (auto it = begin_iter; it != end_iter; ++it) {
            size_t cached = hash_(it->first);
            if (hash_table_.find(it->first, cached) == nullptr) {
                NodeType *new_node = AllocTraits::allocate(alloc_, 1);
                AllocTraits::construct(alloc_, new_node, *it);
                InsertPtr(new_node, cached);
            }
        }
        Fix();
    }

    template<typename ...Args>
    std::pair<iterator, bool> emplace(Args &&... args) {
        NodeType *new_pair = AllocTraits::allocate(alloc_, 1);
        AllocTraits::construct(alloc_, new_pair, std::forward<Args>(args)...);
        size_t cached = hash_(new_pair->first);
        auto iter = hash_table_.FindIterator(new_pair->first, cached);
        if (!iter.second) {
            InsertPtr(new_pair, cached);
            Fix();
            return {iterator(--container_.end()), true};
        }
        AllocTraits::destroy(alloc_, new_pair);
        AllocTraits::deallocate(alloc_, new_pair, 1);
        return {iterator(iter.first), false};
    }

    void erase(const iterator &iter) {
        Erase(iter->first, hash_(iter->first));
    }


    void erase(const iterator &begin_iter, const iterator &end_iter) {
        for (auto it = begin_iter; it != end_iter;) {
            auto copy_it = it;
            ++it;
            erase(copy_it);
        }
    }


    iterator find(const Key &key) {
        auto iter = hash_table_.FindIterator(key, hash_(key));
        if (iter.second) {
            return iterator(iter.first);
        } else {
            return end();
        }
    }

    void rehash(size_t n) {
        if (max_load_factor_ * bucket_count() > n) {
            return;
        }
        hash_table_.rehash(n);
    }

    size_t size() {
        return container_.size();
    }

    iterator begin() {
        return iterator(container_.begin());
    }

    iterator end() {
        return iterator(container_.end());
    }

    const_iterator begin() const {
        return const_iterator(container_.begin());
    }

    const_iterator end() const {
        return const_iterator(container_.end());
    }

    const_iterator cbegin() const {
        return const_iterator(container_.cbegin());
    }

    const_iterator cend() const {
        return const_iterator(container_.cend());
    }

    float max_load_factor() const {
        return max_load_factor_;
    }

    void max_load_factor(float ml) {
        max_load_factor_ = ml;
    }

    float load_factor() const {
        return float(container_.size()) / float(hash_table_.bucket_count());
    }

    size_t bucket_count() const {
        return hash_table_.bucket_count();
    }

    void reserve(size_t count) {
        rehash(std::ceil(count / max_load_factor_));
    }

private:

    void Fix() {
        if (max_load_factor_ < load_factor()) {
            hash_table_.rehash(coefficient_ * container_.size() / max_load_factor_);
        } else {
            return;
        }
    }


    template<typename T>
    std::pair<iterator, bool> CommonInsert(T &&node) {
        size_t cached = hash_(node.first);
        auto ptr = hash_table_.FindIterator(node.first, cached);
        if (!ptr.second) {
            NodeType *new_node = AllocTraits::allocate(alloc_, 1);
            AllocTraits::construct(alloc_, new_node, std::forward<T>(node));
            InsertPtr(new_node, cached);
            Fix();
            return {iterator(--container_.end()), true};
        }
        return {iterator(ptr.first), false};
    }

    void Destroy() {
        for (auto &ptr:container_) {
            AllocTraits::destroy(alloc_, ptr);
            AllocTraits::deallocate(alloc_, ptr, 1);
        }

        hash_table_.ClearTable();

        container_.resize(0);
    }

    void InsertPtr(NodeType *ptr, size_t cached) {
        container_.push_back(ptr);
        hash_table_.insert(--container_.end(), cached);
    }

    void Erase(const Key &key, size_t cached) {
        auto ptr = hash_table_.FindIterator(key, cached);
        if (!ptr.second) {
            return;
        }
        hash_table_.erase(key, cached);
        container_.erase(ptr.first);
    }


private:
    Hash hash_;
    Equal equal_;
    CommonAlloc<NodeType> alloc_;
    _HashTablePtr<Key, Value, NodePtr, Hash, Equal, Alloc> hash_table_;//<Key, Value, NodePtr, Hash, Equal, Alloc> hash_table_;
    IterableContainer container_;
    float max_load_factor_ = 0.8;
    float coefficient_ = 2;
};


template<typename T, typename Allocator>
class List {

private:

    struct Node {
        Node(const T &item) : item(item) {
            prev = this;
            next = this;
        }

        Node() {
            prev = this;
            next = this;
        }

        T item;
        Node *prev;
        Node *next;
    };

    using AllocNode = typename std::allocator_traits<Allocator>::template rebind_alloc<Node>;
    using AllocTraits = std::allocator_traits<AllocNode>;


private:
    template<bool IsConst>
    struct common_iterator {
    private:
        using T_ = std::conditional_t<IsConst, const T, T>;
    public:
        common_iterator() = default;

        common_iterator(const common_iterator<IsConst> &other)
                : node_ptr_(other.node_ptr_) {}


        common_iterator &operator=(const common_iterator &other) {
            node_ptr_ = other.node_ptr_;
            return *this;
        }

        common_iterator<IsConst> &operator++() {
            node_ptr_ = node_ptr_->next;
            return *this;
        }

        common_iterator<IsConst> operator++(int) {
            common_iterator<IsConst> copy_iterator(*this);
            node_ptr_ = node_ptr_->next;
            return copy_iterator;
        }

        common_iterator<IsConst> &operator--() {
            node_ptr_ = node_ptr_->prev;
            return *this;
        }

        common_iterator<IsConst> operator--(int) {
            common_iterator<IsConst> copy_iterator(*this);
            node_ptr_ = node_ptr_->prev;
            return copy_iterator;
        }


        bool operator==(common_iterator<IsConst> other) const {
            return node_ptr_ == other.node_ptr_;
        }

        bool operator!=(common_iterator<IsConst> other) const {
            return !operator==(other);
        }

        T_ &operator*() const {
            return node_ptr_->item;
        }

        T_ *operator->() const {
            return &(node_ptr_->item);
        }

        operator common_iterator<true>() const {
            return common_iterator<true>(node_ptr_);
        }


    public:
        using iterator_category = std::bidirectional_iterator_tag;
        using value_type = T_;
        using difference_type = ptrdiff_t;
        using pointer = T_ *;
        using reference = T_ &;

    private:
        friend common_iterator<false>;
        friend List<T, Allocator>;

        common_iterator(Node *node_ptr) : node_ptr_(node_ptr) {}

        Node *node_ptr_;

    };

public:
    using iterator = common_iterator<false>;
    using const_iterator = common_iterator<true>;
    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;

    List() {
        fake_->prev = fake_;
        fake_->next = fake_;
    }

    explicit List(const Allocator &allocator) : allocator_(allocator) {
        fake_->prev = fake_;
        fake_->next = fake_;
    }

    List(size_t count) : List() {
        for (size_t i = 0; i < count; ++i) {
            InsertPtr(fake_);
        }
    }

    List(size_t count, const Allocator &allocator) : List(allocator) {
        for (size_t i = 0; i < count; ++i) {
            InsertPtr(fake_);
        }
    }

    List(size_t count, const T &value) : List() {
        for (size_t i = 0; i < count; ++i) {
            InsertPtr(fake_, value);
        }
    }

    List(size_t count, const T &value, const Allocator &allocator) : List(allocator) {
        for (size_t i = 0; i < count; ++i) {
            InsertPtr(fake_, value);
        }
    }

    List(const List &other) : allocator_(AllocTraits::select_on_container_copy_construction(other.allocator_)) {
        fake_->prev = fake_;
        fake_->next = fake_;
        for (const T &item : other) {
            InsertPtr(fake_, item);
        }
    }

    List(List &&other) : allocator_(other.allocator_) {
        size_ = other.size_;
        fake_ = other.fake_;
        other.size_ = 0;
        other.fake_ = AllocTraits::allocate(other.allocator_, 1);
        other.fake_->prev = other.fake_;
        other.fake_->next = other.fake_;
    }

    List(const List &other, Allocator allocator) : List(allocator) {
        for (const T &item : other) {
            InsertPtr(fake_, item);
        }
    }


    List &operator=(const List &other) {
        if (this == &other) {
            return *this;
        }
        while (size_ != 0) {
            pop_back();
        }
        if (AllocTraits::propagate_on_container_copy_assignment::value) {
            AllocTraits::deallocate(allocator_, fake_, 1);
            allocator_ = other.allocator_;
            fake_ = AllocTraits::allocate(allocator_, 1);
        }
        fake_->prev = fake_;
        fake_->next = fake_;
        for (const T &item : other) {
            InsertPtr(fake_, item);
        }
        return *this;
    }

    List &operator=(List &&other) {

        if (this == &other) {
            return *this;
        }
        while (size_ != 0) {
            pop_back();
        }
        allocator_ = other.allocator_;
        size_ = other.size_;
        fake_ = other.fake_;
        other.size_ = 0;
        other.fake_ = AllocTraits::allocate(other.allocator_, 1);
        other.fake_->prev = other.fake_;
        other.fake_->next = other.fake_;
        return *this;
    }

    ~List() {
        while (size_ != 0) {
            pop_back();
        }
        AllocTraits::deallocate(allocator_, fake_, 1);
    }

    void push_back(const T &value) {
        InsertPtr(fake_, value);
    }

    template<typename ...Args>
    void emplace_back(Args &&... args) {
        InsertPtr(fake_, std::forward<Args>(args)...);
    }

    void pop_back() {
        ErasePtr(fake_->prev);
    }

    void push_front(const T &value) {
        InsertPtr(fake_->next, value);
    }

    void pop_front() {
        ErasePtr(fake_->next);
    }

    template<bool IsConst>
    common_iterator<IsConst> insert(common_iterator<IsConst> iter, const T &value) {
        InsertPtr(iter.node_ptr_, value);
        return --iter;
    }

    void erase(const const_iterator &iter) {
        ErasePtr(iter.node_ptr_);
    }


    void resize(size_t n) {
        while (n < size_) {
            pop_back();
        }
        while (n > size_) {
            emplace_back();
        }
    }

    const AllocNode &get_allocator() const {
        return allocator_;
    }

    size_t size() const {
        return size_;
    }

    iterator begin() {
        return iterator(fake_->next);
    }

    iterator end() {
        return iterator(fake_);
    }

    const_iterator begin() const {
        return const_iterator(fake_->next);
    }

    const_iterator end() const {
        return const_iterator(fake_);
    }

    const_iterator cbegin() const {
        return const_iterator(fake_->next);
    }

    const_iterator cend() const {
        return const_iterator(fake_);
    }

    reverse_iterator rbegin() {
        return reverse_iterator(end());
    }

    reverse_iterator rend() {
        return reverse_iterator(begin());
    }

    const_reverse_iterator rbegin() const {
        return const_reverse_iterator(cend());
    }

    const_reverse_iterator rend() const {
        return const_reverse_iterator(cbegin());
    }

    const_reverse_iterator crbegin() const {
        return const_reverse_iterator(cend());
    }

    const_reverse_iterator crend() const {
        return const_reverse_iterator(cbegin());
    }

private:

    AllocNode allocator_ = AllocNode();
    size_t size_ = 0;
    Node *fake_ = AllocTraits::allocate(allocator_, 1);

    void InsertPtr(Node *ptr, const T &value) {
        Node *new_node = AllocTraits::allocate(allocator_, 1);
        AllocTraits::construct(allocator_, new_node, value);
        Binding(ptr->prev, new_node);
        Binding(new_node, ptr);
        ++size_;
    }

    template<typename ...Args>
    void InsertPtr(Node *ptr, Args &&...args) {
        Node *new_node = AllocTraits::allocate(allocator_, 1);
        AllocTraits::construct(allocator_, new_node, std::forward<Args>(args)...);
        Binding(ptr->prev, new_node);
        Binding(new_node, ptr);
        ++size_;
    }

    void ErasePtr(Node *ptr) {
        Binding(ptr->prev, ptr->next);
        AllocTraits::destroy(allocator_, ptr);
        AllocTraits::deallocate(allocator_, ptr, 1);
        --size_;
    }

    void Binding(Node *&first, Node *&second) {
        first->next = second;
        second->prev = first;
    }
};
