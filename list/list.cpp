#include <memory>
#include  <iostream>

template<size_t chunkSize>
class FixedAllocator {
public:

    FixedAllocator() : chunk_(new Chunk()) {}

    FixedAllocator(const FixedAllocator &other) : chunk_(other.chunk_) {}

    FixedAllocator &operator=(const FixedAllocator &other) {
        chunk_ = other.chunk_;
        return *this;
    }

    bool operator==(const FixedAllocator &other) const {
        return chunk_ == other.chunk_;
    }

    bool operator!=(const FixedAllocator &other) const {
        return chunk_ != other.chunk_;
    }

    void *allocate(size_t n) {
        if (chunk_->pointer - chunk_->pool + n > chunkSize) {
            std::shared_ptr<Chunk> new_chunk(new Chunk());
            new_chunk->prev = chunk_;
            chunk_ = new_chunk;
        }
        chunk_->pointer += n;
        return chunk_->pointer - n;
    }

    void deallocate(void *, size_t) {}

private:

    struct Chunk {
        char pool[chunkSize];
        char *pointer = pool;
        std::shared_ptr<Chunk> prev = nullptr;
    };
    std::shared_ptr<Chunk> chunk_;

};

template<typename T, size_t chunkSize = 4096>
class FastAllocator {
public:

    using value_type = T;

    template<class U>
    struct rebind {
        using other = FastAllocator<U, chunkSize>;
    };

public:
    FastAllocator() = default;

    FastAllocator(const FastAllocator &other) : pool_(other.pool_) {}

    FastAllocator(const FixedAllocator<chunkSize> &pool) : pool_(pool) {}

    FastAllocator &operator=(const FastAllocator &other) {
        pool_ = other.pool_;
        return *this;
    }

    bool operator==(const FastAllocator &other) const {
        return pool_ == other.pool_;
    }

    bool operator!=(const FastAllocator &other) const {
        return pool_ != other.pool_;
    }

    template<class U>
    operator FastAllocator<U, chunkSize>() const {
        return FastAllocator<U, chunkSize>(pool_);
    }

    T *allocate(size_t n) {
        size_t num_bytes = n * sizeof(T);
        if (num_bytes > (chunkSize / 2)) {
            return reinterpret_cast <T *>(::operator new(num_bytes));
        } else {
            return reinterpret_cast <T *> (pool_.allocate(num_bytes));
        }
    }

    void deallocate(T *ptr, size_t n) {
        if (n * sizeof(T) > (chunkSize / 2)) {
            operator delete(ptr);
        } else {
            pool_.deallocate(reinterpret_cast <void *>(ptr), n * sizeof(T));
        }
    }

    template<typename... Args>
    void construct(T *ptr, const Args &...args) {
        new(ptr) T(args...);
    }

    void destroy(T *ptr) {
        ptr->~T();
    }

private:

    FixedAllocator<chunkSize> pool_;
};


template<typename T, typename Allocator = std::allocator<T>>
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


        common_iterator operator=(const common_iterator &other) {
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

    ~List() {
        while (size_ != 0) {
            pop_back();
        }
        AllocTraits::deallocate(allocator_, fake_, 1);
    }

    void push_back(const T &value) {
        InsertPtr(fake_, value);
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

    void InsertPtr(Node *ptr) {
        Node *new_node = AllocTraits::allocate(allocator_, 1);
        AllocTraits::construct(allocator_, new_node);
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
