#pragma once

#include <vector>

template<typename T>
class Deque {
public:
    Deque() {}

    Deque(int64_t count, const T &item) {
        begin_ = (count + size_bucket_ - 1) / size_bucket_ * size_bucket_;
        end_ = begin_ + count;
        array_bucket_.resize((int64_t(end_) + size_bucket_ - 1) / size_bucket_, nullptr);
        MemoryAllocation();
        size_t id = begin_;
        try {
            for (; id < end_; ++id) {
                new(Pointer(id)) T(item);
            }
        } catch (...) {
            for (size_t i = begin_; i < id; ++i) {
                (Pointer(i))->~T();
            }
            MemoryDelete();
            begin_ = end_ = 0;
            throw;
        }
    }

    Deque(int64_t count) {
        begin_ = (count + size_bucket_ - 1) / size_bucket_ * size_bucket_;
        end_ = begin_ + count;
        array_bucket_.resize((int64_t(end_) + size_bucket_ - 1) / size_bucket_, nullptr);
        MemoryAllocation();
        size_t id = begin_;
        try {
            for (; id < end_; ++id) {
                new(Pointer(id)) T();
            }
        } catch (...) {
            for (size_t i = begin_; i < id; ++i) {
                (Pointer(i))->~T();
            }
            MemoryDelete();
            begin_ = end_ = 0;
            throw;
        }
    }

    Deque(const Deque<T> &other) {
        begin_ = other.begin_;
        end_ = other.end_;
        array_bucket_.resize((end_ - 1) / size_bucket_ + 1, nullptr);
        MemoryAllocation();
        size_t id = begin_;
        try {
            for (; id < end_; ++id) {
                new(Pointer(id)) T(*(other.Pointer(id)));
            }
        } catch (...) {
            for (size_t i = begin_; i < id; ++i) {
                (Pointer(i))->~T();
            }
            MemoryDelete();
            begin_ = end_ = 0;
            throw;
        }
    }

    Deque &operator=(const Deque<T> &other) {
        Deque<T> copy_deque(other);
        Swap(copy_deque);
        return *this;
    }

    ~Deque() {
        for (size_t i = begin_; i < end_; ++i) {
            Pointer(i)->~T();
        }
        MemoryDelete();
    }

    T &operator[](int64_t index) {
        index += begin_;
        return *(Pointer(index));
    }

    const T &operator[](int64_t index) const {
        index += begin_;
        return *(Pointer(index));
    }

    T &at(int64_t index) {
        RangeCheck(index);
        return (*this)[index];
    }

    const T &at(int64_t index) const {
        RangeCheck(index);
        return (*this)[index];
    }

    size_t size() const {
        return end_ - begin_;
    }

    void push_back(const T &item) {
        if (end_ % size_bucket_ == 0) {
            array_bucket_.push_back(reinterpret_cast<T *>(new int8_t[size_bucket_ * sizeof(T)]));
        }
        try {
            new(Pointer(end_)) T(item);
        } catch (...) {
            if (end_ % size_bucket_ == 0) {
                delete[] reinterpret_cast<int8_t *>(array_bucket_.back());
                array_bucket_.pop_back();
            }
            throw;
        }
        ++end_;
    }

    void pop_back() {
        (Pointer(end_ - 1))->~T();
        --end_;
        if (end_ % size_bucket_ == 0) {
            delete[] reinterpret_cast<int8_t *>(array_bucket_.back());
            array_bucket_.pop_back();
        }
    }

    void pop_front() {
        (Pointer(begin_))->~T();
        if ((begin_ + 1) % size_bucket_ == 0) {
            delete[] reinterpret_cast <int8_t *>(array_bucket_[begin_ / size_bucket_]);
            array_bucket_[begin_ / size_bucket_] = nullptr;
        }
        ++begin_;
    }

    void push_front(const T &item) {
        if (begin_ == 0) {
            Resize();
        }
        if (begin_ % size_bucket_ == 0) {
            array_bucket_[begin_ / size_bucket_ - 1] = reinterpret_cast <T *> (new int8_t[size_bucket_ * sizeof(T)]);
        }
        --begin_;
        try {
            new(Pointer(begin_)) T(item);
        } catch (...) {
            if ((begin_ + 1) % size_bucket_ == 0) {
                delete[] reinterpret_cast <int8_t *>(array_bucket_[begin_ / size_bucket_]);
            }
            ++begin_;
            throw;
        }
    }

private:
    template<bool IsConst>
    struct common_iterator {
    private:
        using T_ = std::conditional_t<IsConst, const T, T>;
    public:
        common_iterator() = default;

        common_iterator(const common_iterator<IsConst> &other)
                : begin_array_(other.begin_array_), index_(other.index_), size_bucket_(other.size_bucket_) {}


        common_iterator<IsConst> &operator=(common_iterator<IsConst> other) {
            begin_array_ = other.begin_array_;
            index_ = other.index_;
            size_bucket_ = other.size_bucket_;
        }

        common_iterator<IsConst> &operator++() {
            ++index_;
            return *this;
        }

        common_iterator<IsConst> operator++(int) {
            common_iterator<IsConst> copy_iterator(*this);
            ++index_;
            return copy_iterator;
        }

        common_iterator<IsConst> &operator--() {
            --index_;
            return *this;
        }

        common_iterator<IsConst> operator--(int) {
            common_iterator<IsConst> copy_iterator(*this);
            --index_;
            return copy_iterator;
        }

        common_iterator<IsConst> &operator+=(int64_t advance) {
            index_ += advance;
            return *this;
        }

        common_iterator<IsConst> &operator-=(int64_t advance) {
            index_ -= advance;
            return *this;
        }

        common_iterator<IsConst> operator+(int64_t advance) const {
            common_iterator<IsConst> copy_iterator(*this);
            return copy_iterator += advance;
        }

        common_iterator<IsConst> operator-(int64_t advance) const {
            common_iterator<IsConst> copy_iterator(*this);
            return copy_iterator -= advance;
        }

        int64_t operator-(common_iterator<IsConst> other) const {
            return index_ - other.index_;
        }

        bool operator<(common_iterator<IsConst> other) const {
            return index_ < other.index_;
        }

        bool operator>(common_iterator<IsConst> other) const {
            return index_ > other.index_;
        }

        bool operator<=(common_iterator<IsConst> other) const {
            return index_ <= other.index_;
        }

        bool operator>=(common_iterator<IsConst> other) const {
            return index_ >= other.index_;
        }

        bool operator==(common_iterator<IsConst> other) const {
            return (index_ == other.index_) && (begin_array_ == other.begin_array_);
        }

        bool operator!=(common_iterator<IsConst> other) const {
            return !operator==(other);
        }

        T_ &operator*() {
            return *(*(begin_array_ + index_ / size_bucket_) + index_ % size_bucket_);
        }

        T_ *operator->() {
            return (*(begin_array_ + index_ / size_bucket_) + index_ % size_bucket_);
        }

        operator common_iterator<true>() const {
            return common_iterator<true>(begin_array_, index_, size_bucket_);
        }


    public:
        using iterator_category = std::random_access_iterator_tag;
        using value_type = T_;
        using difference_type = ptrdiff_t;
        using pointer = T_ *;
        using reference = T_ &;

    private:
        friend common_iterator<false>;
        friend Deque<T>;

        common_iterator(
                std::conditional_t<IsConst, typename std::vector<T *>::const_iterator, typename std::vector<T *>::iterator> begin_array,
                size_t index, size_t size_bucket)
                : begin_array_(begin_array), index_(index), size_bucket_(size_bucket) {}

        std::conditional_t<IsConst, typename std::vector<T *>::const_iterator, typename std::vector<T *>::iterator> begin_array_;
        size_t index_;
        const size_t size_bucket_ = 16;

    };

public:
    using iterator = common_iterator<false>;
    using const_iterator = common_iterator<true>;
    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;

    iterator begin() {
        return iterator(array_bucket_.begin(), begin_, size_bucket_);
    }

    const_iterator begin() const {
        return const_iterator(array_bucket_.begin(), begin_, size_bucket_);
    }

    iterator end() {
        return iterator(array_bucket_.begin(), end_, size_bucket_);
    }

    const_iterator end() const {
        return const_iterator(array_bucket_.begin(), end_, size_bucket_);
    }

    const_iterator cbegin() const {
        return const_iterator(array_bucket_.begin(), begin_, size_bucket_);
    }

    const_iterator cend() const {
        return const_iterator(array_bucket_.begin(), end_, size_bucket_);
    }

    reverse_iterator rbegin() {
        return std::reverse_iterator<iterator>(end());
    }

    reverse_iterator rend() {
        return std::reverse_iterator<iterator>(begin());
    }

    const_reverse_iterator rbegin() const {
        return std::reverse_iterator<const_iterator>(cend());
    }

    const_reverse_iterator rend() const {
        return std::reverse_iterator<const_iterator>(cbegin());
    }

    const_reverse_iterator crbegin() const {
        return std::reverse_iterator<const_iterator>(cend());
    }

    const_reverse_iterator crend() const {
        return std::reverse_iterator<const_iterator>(cbegin());
    }


    void insert(iterator iter, const T &item) {
        Deque<T> copy_deque;
        copy_deque.size_bucket_ = size_bucket_;
        copy_deque.begin_ = begin_;
        copy_deque.end_ = begin_;
        copy_deque.array_bucket_.resize((begin_ + size_bucket_ - 1) / size_bucket_, nullptr);

        copy_deque.MemoryAllocation();
        for (size_t i = begin_; i < iter.index_; ++i) {
            copy_deque.push_back(*Pointer(i));
        }
        copy_deque.push_back(item);
        for (size_t i = iter.index_; i < end_; ++i) {
            copy_deque.push_back(*Pointer(i));
        }
        Swap(copy_deque);
    }

    void erase(iterator iter) {
        Deque<T> copy_deque;
        copy_deque.size_bucket_ = size_bucket_;
        copy_deque.begin_ = begin_;
        copy_deque.end_ = begin_;
        copy_deque.array_bucket_.resize((begin_ + size_bucket_ - 1) / size_bucket_, nullptr);

        copy_deque.MemoryAllocation();
        for (size_t i = begin_; i < iter.index_; ++i) {
            copy_deque.push_back(*Pointer(i));
        }
        for (size_t i = iter.index_ + 1; i < end_; ++i) {
            copy_deque.push_back(*Pointer(i));
        }
        Swap(copy_deque);
    }

private:

    std::vector<T *> array_bucket_;
    size_t begin_ = 0;
    size_t end_ = 0;
    size_t size_bucket_ = 12;

    void RangeCheck(int64_t index) const {
        index += begin_;
        if (index >= int64_t(end_) || index < 0) {
            throw std::out_of_range("index > deque.size()");
        }
    }

    void Resize() {
        std::vector<T *> new_array_bucket_(array_bucket_.size() * 2 + 2, nullptr);
        for (size_t i = 0; i < array_bucket_.size(); ++i) {
            new_array_bucket_[i + array_bucket_.size() + 2] = array_bucket_[i];
        }
        begin_ += (array_bucket_.size() + 2) * size_bucket_;
        end_ += (array_bucket_.size() + 2) * size_bucket_;
        array_bucket_ = new_array_bucket_;
    }

    T *Pointer(size_t index) const {
        return array_bucket_[index / size_bucket_] + index % size_bucket_;
    }

    void MemoryAllocation() {
        for (size_t i = begin_ / size_bucket_; i * size_bucket_ < end_; ++i) {
            array_bucket_[i] = reinterpret_cast <T *> (new int8_t[size_bucket_ * sizeof(T)]);
        }
    }

    void MemoryDelete() {
        for (size_t i = begin_ / size_bucket_; i * size_bucket_ < end_; ++i) {
            delete[] reinterpret_cast<int8_t *> (array_bucket_[i]);
        }
    }

    void Swap(Deque<T> &other) {
        std::swap(array_bucket_, other.array_bucket_);
        std::swap(begin_, other.begin_);
        std::swap(end_, other.end_);
        std::swap(size_bucket_, other.size_bucket_);
    }


};
