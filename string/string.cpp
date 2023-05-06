#pragma once

#include <iostream>
#include <cstring>

class String {
public:
    String() {
        begin_ = new char[capacity_];
        begin_[size_] = '\0';
    }

    String(size_t size, char symbol = '\0') :
            size_(size), capacity_(near_capacity(size_)), begin_(new char[capacity_]) {
        memset(begin_, symbol, size_);
        begin_[size_] = '\0';
    }

    String(char symbol) : size_(1), capacity_(near_capacity(size_)),
                          begin_(new char[capacity_]) {

        memset(begin_, symbol, size_);
        begin_[size_] = '\0';
    }

    String(const String &s) {
        size_ = s.size_;
        capacity_ = s.capacity_;
        begin_ = new char[capacity_];
        memcpy(begin_, s.begin_, size_);
    }

    String(const char *ch) {
        size_ = size_search(ch);
        capacity_ = near_capacity(size_);
        begin_ = new char[capacity_];
        memcpy(begin_, ch, size_ + 1);

    }

    ~String() {
        delete[] begin_;
    }

    String &operator=(String s) {
        swap(s);
        return *(this);
    }

    void swap(String &s) {
        std::swap(size_, s.size_);
        std::swap(capacity_, s.capacity_);
        std::swap(begin_, s.begin_);
    }

    const char &operator[](size_t i) const {
        return begin_[i];
    }

    char &operator[](size_t i) {
        return begin_[i];

    }

    String &operator+=(const char &ch) {
        push_back(ch);
        return *(this);
    }

    String &operator+=(const String &s) {
        size_ += s.size_;
        correct();
        memcpy(begin_ + size_ - s.size_, s.begin_, s.size_ + 1);
        return *(this);
    }

    bool operator==(const String &s) const {
        if (size_ != s.size_) {
            return false;
        }
        for (size_t i = 0; i < s.length(); ++i) {
            if (begin_[i] != s[i]) {
                return false;
            }
        }
        return true;
    }

    const char &front() const {
        return begin_[0];
    }

    char &front() {
        return begin_[0];
    }

    const char &back() const {
        return begin_[size_ - 1];
    }

    char &back() {
        return begin_[size_ - 1];
    }

    size_t length() const {
        return size_;
    }

    void clear() {
        size_ = 0;
        capacity_ = 1;
        delete[] begin_;
        begin_ = new char[capacity_];
        begin_[size_] = '\0';
    }

    bool empty() const {
        return size_ == 0;
    }

    void push_back(const char &symbol) {
        begin_[size_] = symbol;
        ++size_;
        correct();
        begin_[size_] = '\0';
    }

    void pop_back() {
        if (size_ != 0) {
            --size_;
            begin_[size_] = '\0';
            correct();
        }
    }

    String substr(size_t start, size_t count) const {
        String substr(count);
        memcpy(substr.begin_, begin_ + start, count + 1);
        return substr;
    }

    size_t find(const String &substring) const {
        return find(substring,  /*compare substr from begin to end=*/ true);
    }

    size_t rfind(const String &substring) const {
        return find(substring,/*compare substr from end to begin=*/ false);
    }

private:
    size_t size_ = 0;
    size_t capacity_ = 1;
    char *begin_ = nullptr;
private:
    size_t size_search(const char *ch) const {
        size_t size_ch = 0;
        for (size_t i = 0; ch[i] != '\0'; ++i) {
            ++size_ch;
        }
        return size_ch;
    }

    size_t near_capacity(const size_t &size) const {
        size_t capacity = 1;
        while (capacity <= size + 1) {
            capacity += capacity;
        }
        return capacity;
    }

    void correct() {
        if (4 * size_ <= capacity_ && capacity_ > 2) {
            capacity_ = near_capacity(size_) * 2;
            auto pointer = new char[capacity_];
            memcpy(pointer, begin_, size_ + 1);
            delete[] begin_;
            begin_ = pointer;
        }
        if (size_ + 1 > capacity_) {
            capacity_ = near_capacity(size_);
            auto pointer = new char[capacity_];
            memcpy(pointer, begin_, size_ + 1);
            delete[] begin_;
            begin_ = pointer;
        }
    }

    bool compare_substr(const size_t &i, const String &another) const {
        for (size_t j = 0; j < another.size_; ++j) {
            if (begin_[j + i] != another[j]) {
                return false;
            }
        }
        return true;
    }

    size_t find(const String &substring, bool left) const {
        for (size_t j = 0; j < size_ - substring.size_; ++j) {
            if (compare_substr((left ? j : size_ - substring.size_ - j - 1), substring)) {
                return (left ? j : size_ - substring.size_ - j - 1);
            }
        }
        return size_;
    }
};

bool operator==(const char *ch, const String &s) {
    size_t ch_size = 0;
    for (size_t i = 0; ch[i] != '\0'; ++i) {
        ++ch_size;
    }
    if (s.length() != ch_size) {
        return false;
    }
    for (size_t i = 0; i < ch_size; ++i) {
        if (s[i] != ch[i]) {
            return false;
        }
    }
    return true;
}

String operator+(String s1, const String &s2) {
    return s1 += s2;
}

std::istream &operator>>(std::istream &in, String &s) {
    char symbol;
    s.clear();
    in >> std::noskipws;
    while (in >> symbol) {
        if (isspace(symbol) != 0) {
            in >> std::skipws;
            return in;
        }
        s.push_back(symbol);
    }
    return in;
}

std::ostream &operator<<(std::ostream &out, const String &s) {
    for (size_t i = 0; i < s.length(); ++i) {
        out << s[i];
    }
    return out;
}


