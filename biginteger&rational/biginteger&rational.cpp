#pragma once

#include <iostream>
#include <vector>

class BigInteger {
 public:

  BigInteger() {
    number.push_back(0);
  }

  BigInteger(int a) {
    if (a < 0) {
      sign = false;
      a = -a;
    }
    number.push_back(a % base);
    a /= base;
    while (a) {
      number.push_back(a % base);
      a /= base;
    }
  }

  BigInteger(const BigInteger &another) {
    number = another.number;
    sign = another.sign;
  }

  ~BigInteger() = default;

  BigInteger &operator=(const BigInteger &another) {
    number = another.number;
    sign = another.sign;
    return *this;
  }

  BigInteger operator-() const {
    BigInteger result = *this;
    result.flip_sign();
    return result;
  }

  BigInteger &operator++() {
    return *this += 1;
  }

  BigInteger operator++(int) {
    BigInteger result = *this;
    *this += 1;
    return result;
  }

  bool operator==(const BigInteger &another) const {
    if (number.size() != another.number.size()) return false;
    for (size_t i = 0; i < number.size(); ++i) {
      if (number[i] != another.number[i]) return false;
    }
    return true;
  }

  bool operator==(int x) const {
    return *this == BigInteger(x);
  }

  bool operator!=(const BigInteger &another) const {
    return !(*this == another);
  }

  bool operator!=(int x) const {
    return *this != BigInteger(x);
  }

  bool operator<(const BigInteger &another) const {
    if (sign != another.sign) return another.sign;
    if (number.size() != another.number.size()) return (number.size() < another.number.size()) ^ !sign;
    for (int64_t i = number.size() - 1; i > -1; --i) {
      if (number[i] != another.number[i]) return (number[i] < another.number[i]) ^ !sign;
    }
    return false;
  }

  bool operator<(int x) const {
    return *this < BigInteger(x);
  }

  bool operator>(const BigInteger &another) const {
    return another < *this;
  }

  bool operator>(int x) const {
    return *this > BigInteger(x);
  }

  bool operator>=(const BigInteger &another) const {
    return !(*this < another);
  }

  bool operator>=(int x) const {
    return *this >= BigInteger(x);
  }

  bool operator<=(const BigInteger &another) const {
    return !(another < *this);
  }

  bool operator<=(int x) const {
    return *this <= BigInteger(x);
  }

  BigInteger &operator+=(const BigInteger &another) {
    if (sign == another.sign) {
      size_t max_size = std::max(number.size(), another.number.size());
      while (number.size() <= max_size + 1) {
        number.push_back(0);
      }
      for (size_t i = 0; i < another.number.size() || number[i] >= base; ++i) {
        if (i < another.number.size()) {
          number[i] += another.number[i];
        }
        if (number[i] >= base) {
          number[i] %= base;
          ++number[i + 1];
        }
      }
      fix_this();
    } else {
      sign = !sign;
      *this -= another;
      sign = !sign;
    }
    return *this;
  }

  BigInteger &operator+=(int x) {
    return *this += BigInteger(x);
  }

  BigInteger &operator-=(const BigInteger &another) {
    if (sign == another.sign) {
      size_t max_size = std::max(number.size(), another.number.size());
      while (number.size() <= max_size + 1) {
        number.push_back(0);
      }
      for (size_t i = 0; (i < another.number.size() || number[i] < 0) && i < number.size() - 1; ++i) {
        if (i < another.number.size()) {
          number[i] -= another.number[i];
        }
        if (number[i] < 0) {
          number[i] += base;
          --number[i + 1];
        }
      }
      fix_this();
      if (number.back() < 0) {
        sign = !sign;
        for (size_t i = 0; i < number.size(); ++i) {
          number[i] = -number[i];
        }
        for (size_t i = 0; i < number.size() - 1; ++i) {
          if (number[i] < 0) {
            number[i] += base;
            --number[i + 1];
          }
        }
      }
      fix_this();
    } else {
      sign = !sign;
      *this += another;
      sign = !sign;
    }
    return *this;
  }

  BigInteger &operator-=(int x) {
    return *this -= BigInteger(x);
  }

  BigInteger operator+(const BigInteger &another) const {
    BigInteger result = *this;
    return result += another;
  }

  BigInteger operator+(int x) const {
    BigInteger result = *this;
    return result += x;
  }

  BigInteger operator-(const BigInteger &another) const {
    BigInteger result = *this;
    return result -= another;
  }

  BigInteger operator-(int x) const {
    BigInteger result = *this;
    return result -= x;
  }

  BigInteger &operator*=(int x) {
    number.push_back(0);
    if (x < 0) {
      sign = !sign;
      x = -x;
    }
    int add = 0;
    for (size_t i = 0; i < number.size(); ++i) {
      long long curr = add + 1ll * number[i] * x;
      number[i] = curr % base;
      add = curr / base;
    }
    fix_this();
    return *this;
  }

  BigInteger operator*(int x) const {
    BigInteger result = *this;
    return (result *= x);
  }

  BigInteger operator*(const BigInteger &another) const {
    BigInteger result;
    result.number.resize(number.size() + another.number.size());
    for (size_t i = 0; i < number.size(); ++i) {
      int add = 0;
      for (size_t j = 0; j < another.number.size() || add; ++j) {
        long long curr = result.number[i + j] + add;
        if (j < another.number.size()) {
          curr += 1ll * number[i] * another.number[j];
        }
        result.number[i + j] = curr % base;
        add = curr / base;
      }
    }
    result.sign = sign == another.sign;
    result.fix_this();
    return result;
  }

  BigInteger &operator*=(const BigInteger &another) {
    return *this = *this * another;
  }

  BigInteger operator/(const BigInteger &another) const {
    BigInteger mod = *this;
    mod.sign = another.sign;
    BigInteger result = 0;
    while ((another.sign ? mod >= another : mod <= another)) {
      size_t sub_length = mod.length() - another.length();
      if (sub_length == 0) {
        result += 1;
        mod -= another;
      } else {
        BigInteger curr = 1;
        curr = curr.addition_pow(sub_length - 1);
        BigInteger curr_add_another = another.addition_pow(sub_length - 1);
        int mod_curr = (mod.front_digit() <= another.front_digit() ? 1 : 10);
        curr *= mod_curr;
        curr_add_another *= mod_curr;
        mod -= curr_add_another;
        result += curr;
      }
    }
    result.sign = (sign == another.sign);
    result.fix_this();
    return result;
  }

  BigInteger operator/(int x) const {
    return (*this) / BigInteger(x);
  }

  BigInteger &operator/=(const BigInteger &another) {
    return *this = *this / another;
  }

  BigInteger &operator/=(int x) {
    return *this /= BigInteger(x);
  }

  BigInteger &operator%=(const BigInteger &another) {
    return *this -= *this / another * another;
  }

  BigInteger operator%(const BigInteger &another) const {
    BigInteger result = *this;
    result -= *this / another * another;
    return result;
  }

  std::string toString() const {
    std::string result;
    if (!sign) {
      result.push_back('-');
    }
    result += tostring_num(number.back());
    for (size_t i = number.size() - 1; i > 0; --i) {
      int cnt_null = 9 - length_num(number[i - 1]);
      for (int j = 0; j < cnt_null; ++j) {
        result.push_back('0');
      }
      result += tostring_num(number[i - 1]);
    }
    return result;
  }

  friend std::istream &operator>>(std::istream &in, BigInteger &other) {
    std::string s;
    in >> s;
    other.build_string(s);
    return in;
  }

  operator bool() const {
    return (number.size() != 1 || number.front() != 0);
  }

  operator int() const {
    if (number.size() == 1) {
      return (sign ? number.front() : -number.front());
    }
    if (sign) {
      if (*this < BigInteger(INT32_MAX)) {
        return (number[1] * base + number.front());
      } else {
        return INT32_MAX;
      }
    } else {
      if (*this > BigInteger(INT32_MIN)) {
        return -(number[1] * base + number.front());
      } else {
        return INT32_MIN;
      }
    }
  }

  void flip_sign() {
    if (number.size() != 1 || number.front() != 0) sign = !sign;
  }

  BigInteger addition_pow(size_t x) const {
    if (x == 0) return *this;
    if (*this == 0) return *this;
    BigInteger result;
    result.sign = sign;
    result.number.resize(x / 9);
    for (size_t i = 0; i < number.size(); ++i) {
      result.number.push_back(number[i]);
    }
    x %= 9;
    int mod_pow = 1;
    for (size_t i = 0; i < x; ++i) {
      mod_pow *= 10;
    }
    result *= mod_pow;
    return result;
  }

  void build_string(const std::string &s) {
    number.clear();
    sign = s.front() != '-';
    size_t i = s.size() - 1;
    while (i + 1 > 0 && s[i] != '-') {
      number.push_back(0);
      int pow = 1;
      for (size_t j = 0; j < 9 && i + 1 > 0 && s[i] != '-'; ++j, --i) {
        number.back() += pow * (s[i] - '0');
        pow *= 10;
      }
    }
  }

  size_t length() const {
    return (number.size() - 1) * 9 + length_num(number.back());
  }

 private:
  int base = 1000000000;
  std::vector<int> number;
  bool sign = true;
 private:
  size_t length_num(long long a) const {
    size_t length = 1;
    while (a > 9) {
      a /= 10;
      ++length;
    }
    return length;
  }

  void fix_this() {
    while (!number.back() && number.size() > 1) {
      number.pop_back();
    }
    if (number.size() == 1 && number[0] == 0 && !sign) sign = true;
  }

  std::string tostring_num(int x) const {
    std::string result;
    while (x > 9) {
      result.push_back(x % 10 + '0');
      x /= 10;
    }
    result.push_back(x + '0');
    int p1 = 0;
    int p2 = result.size();
    while (p1 < p2) {
      std::swap(result[p1++], result[--p2]);
    }
    return result;
  }

  int front_digit() const {
    int result = number.back();
    while (result > 9) {
      result /= 10;
    }
    return result;
  }

};

std::ostream &operator<<(std::ostream &out, const BigInteger &other) {
  return out << other.toString();
}

BigInteger operator+(int x, const BigInteger &other) {
  return BigInteger(x) + other;
}

BigInteger operator-(int x, const BigInteger &other) {
  return BigInteger(x) - other;
}

BigInteger operator*(int x, const BigInteger &other) {
  return BigInteger(x) * other;
}

BigInteger operator/(int x, const BigInteger &other) {
  return BigInteger(x) / other;
}

BigInteger operator%(int x, const BigInteger &other) {
  return BigInteger(x) % other;
}

class Rational {
 public:
  Rational(const BigInteger &x) : P(x), Q(1) {}

  Rational(int x) : P(x), Q(1) {}

  Rational(const Rational &another) : P(another.P), Q(another.Q) {}

  Rational() = default;

  ~Rational() = default;

  Rational &operator=(const Rational &another) {
    P = another.P;
    Q = another.Q;
    return *this;
  }

  Rational &operator=(const BigInteger &another) {
    P = another;
    Q = 1;
    return *this;
  }

  Rational &operator=(int x) {
    P = BigInteger(x);
    Q = 1;
    return *this;
  }

  Rational operator-() const {
    Rational result = *this;
    result.P.flip_sign();
    return result;
  }

  Rational &operator+=(const Rational &another) {
    BigInteger another_P = (another.P * Q);
    P *= another.Q;
    Q *= another.Q;
    P += another_P;
    fix();
    return *this;
  }

  Rational operator+(const Rational &another) const {
    Rational result = *this;
    return result += another;
  }

  Rational &operator-=(const Rational &another) {
    return (*this += (-another));
  }

  Rational operator-(const Rational &another) const {
    Rational result = *this;
    return result -= another;
  }

  Rational &operator*=(const Rational &another) {
    P *= another.P;
    Q *= another.Q;
    fix();
    return *this;
  }

  Rational operator*(const Rational &another) const {
    Rational result = *this;
    return result *= another;
  }

  Rational &operator/=(const Rational &another) {
    P *= another.Q;
    Q *= another.P;
    if (Q < 0) {
      P.flip_sign();
      Q.flip_sign();
    }
    fix();
    return *this;
  }

  Rational operator/(const Rational &another) const {
    Rational result = *this;
    return result /= another;
  }

  bool operator==(const Rational &another) const {
    return (Q == another.Q) && (P == another.P);
  }

  bool operator!=(const Rational &another) const {
    return !(*this == another);
  }

  bool operator<(const Rational &another) const {
    return (P * another.Q < another.P * Q);
  }

  bool operator>(const Rational &another) const {
    return (another < *this);
  }

  bool operator>=(const Rational &another) const {
    return !(*this < another);
  }

  bool operator<=(const Rational &another) const {
    return !(another < *this);
  }

  Rational &operator+=(const BigInteger &x) {
    return *this += Rational(x);
  }

  Rational operator+(const BigInteger &x) const {
    return *this + Rational(x);
  }
  Rational &operator-=(const BigInteger &x) {
    return *this -= Rational(x);
  }

  Rational operator-(const BigInteger &x) const {
    return *this - Rational(x);
  }
  Rational &operator*=(const BigInteger &x) {
    return *this *= Rational(x);
  }
  Rational operator*(const BigInteger &x) const {
    return *this * Rational(x);
  }

  Rational &operator/=(const BigInteger &x) {
    return *this /= Rational(x);
  }
  Rational operator/(const BigInteger &x) const {
    Rational result = *this;
    return result /= x;
  }

  bool operator==(const BigInteger &x) const {
    return (*this == Rational(x));
  }

  bool operator!=(const BigInteger &x) const {
    return !(*this == x);
  }

  bool operator<(const BigInteger &x) const {
    return *this < Rational(x);
  }

  bool operator>(const BigInteger &x) const {
    return Rational(x) < *this;
  }

  bool operator<=(const BigInteger &x) const {
    return !(Rational(x) < *this);
  }

  bool operator>=(const BigInteger &x) const {
    return !(*this < Rational(x));
  }

  Rational &operator+=(int x) {
    return *this += Rational(x);
  }

  Rational operator+(int x) const {
    return *this + Rational(x);
  }
  Rational &operator-=(int x) {
    return *this -= Rational(x);
  }

  Rational operator-(int x) const {
    return *this - Rational(x);
  }
  Rational &operator*=(int x) {
    return *this *= Rational(x);
  }
  Rational operator*(int x) const {
    return *this * Rational(x);
  }

  Rational &operator/=(int x) {
    return *this /= Rational(x);
  }
  Rational operator/(int x) const {
    Rational result = *this;
    return result /= x;
  }

  bool operator==(int x) const {
    return (*this == Rational(x));
  }

  bool operator!=(int x) const {
    return !(*this == x);
  }

  bool operator<(int x) const {
    return *this < Rational(x);
  }

  bool operator>(int x) const {
    return Rational(x) < *this;
  }

  bool operator<=(int x) const {
    return !(Rational(x) < *this);
  }

  bool operator>=(int x) const {
    return !(*this < Rational(x));
  }
  std::string toString() const {
    if (Q == 1) return P.toString();
    return P.toString() + '/' + Q.toString();
  }

  std::string asDecimal(size_t precision = 0) const {
    std::string result;
    if (P < 0 && -P <= Q) {
      result.push_back('-');
    }
    result += (P / Q).toString();
    if (precision == 0) return result;
    result += '.';
    BigInteger after_point = (P.addition_pow(precision) / Q - (P / Q).addition_pow(precision));
    if (P < 0) {
      after_point.flip_sign();
    }
    int num_null = precision - after_point.length();
    for (int i = 0; i < num_null; ++i) {
      result.push_back('0');
    }
    result += after_point.toString();
    return result;
  }

  operator double() const {
    return std::stod(asDecimal(20));
  }

 private:
  BigInteger P;
  BigInteger Q;
 private:
  void fix() {
    BigInteger GCD = gcd(P, Q);
    P /= GCD;
    Q /= GCD;
  }

  BigInteger gcd(BigInteger N, BigInteger M) const {
    while (M != 0) {
      BigInteger tmp = N % M;
      N = M;
      M = tmp;
    }
    return (N < 0 ? -N : N);
  }

};

Rational operator+(int x, const Rational &other) {
  return Rational(x) + other;
}
Rational operator-(int x, const Rational &other) {
  return Rational(x) - other;
}
Rational operator*(int x, const Rational &other) {
  return Rational(x) * other;
}
Rational operator/(int x, const Rational &other) {
  return Rational(x) / other;
}
bool operator<(int x, const Rational &other) {
  return Rational(x) < other;
}
bool operator>(int x, const Rational &other) {
  return Rational(x) > other;
}
bool operator>=(int x, const Rational &other) {
  return Rational(x) >= other;
}
bool operator<=(int x, const Rational &other) {
  return Rational(x) <= other;
}
bool operator==(int x, const Rational &other) {
  return Rational(x) == other;
}
bool operator!=(int x, const Rational &other) {
  return Rational(x) != other;
}
Rational operator+(const BigInteger &x, const Rational &other) {
  return Rational(x) + other;
}
Rational operator-(const BigInteger &x, const Rational &other) {
  return Rational(x) - other;
}
Rational operator*(const BigInteger &x, const Rational &other) {
  return Rational(x) * other;
}
Rational operator/(const BigInteger &x, const Rational &other) {
  return Rational(x) / other;
}
bool operator<(const BigInteger &x, const Rational &other) {
  return Rational(x) < other;
}
bool operator>(const BigInteger &x, const Rational &other) {
  return Rational(x) > other;
}
bool operator>=(const BigInteger &x, const Rational &other) {
  return Rational(x) >= other;
}
bool operator<=(const BigInteger &x, const Rational &other) {
  return Rational(x) <= other;
}
bool operator==(const BigInteger &x, const Rational &other) {
  return Rational(x) == other;
}
bool operator!=(const BigInteger &x, const Rational &other) {
  return Rational(x) != other;
}
