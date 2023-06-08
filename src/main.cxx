#include <map>
#include <array>
#include <iostream>

class vector
{
#pragma region aliases {

public:
  using key_t = std::uint64_t;
  using value_t = long double;
  using container_t = std::map<key_t, value_t>;

#pragma endregion } aliases
#pragma region nested definitions {

#pragma region uval* definitions {
private:
  class uval_base
  {
  public:
    using type = key_t;

  protected:
    type m_max;

  public:
    uval_base(const type &max) : m_max(max) {}
    operator type() const { return m_max; }
    uval_base &operator=(const type &val)
    {
      this->m_max = val;
      return *this;
    }

    friend class vector;
    // friend class uval;
  };

public:
  class uval : public uval_base
  {
    uval_base m_val;

  public:
    uval() : uval_base(0), m_val(0) {}
    uval(const type &val) : uval_base(val), m_val(val) {}
    uval(const type &val, const type &max) : uval_base(max), m_val(val)
    {
      if (val > max)
        throw std::out_of_range("value out of range");
    }
    operator type() const { return m_val; }

    uval &operator++()
    {
      ++this->m_val.m_max;
      return *this;
    }
    uval operator++(int)
    {
      uval tmp = *this;
      this->m_val.m_max++;
      return tmp;
    }
    uval &operator--()
    {
      --this->m_val.m_max;
      return *this;
    }
    uval operator--(int)
    {
      uval tmp = *this;
      this->m_val.m_max--;
      return tmp;
    }

    uval &operator=(const type &val)
    {
      if (val > this->m_max)
        throw std::out_of_range("value out of range");
      this->m_val = val;
      return *this;
    }
  };
#pragma endregion } uval *definitions
#pragma region entry* definitions {
private:
  template <class V>
  class entry_base_t
  {
  protected:
    V &m_vector;
    // uval m_key;
  public:
    class key_t
    {
    private:
      uval m_val;

    public:
      key_t(const uval &key) : m_val(key) {}
      // key_t(const key_t &other) = delete;
      // key_t(key_t &&other) = delete;

      // operator uval&()
      // {
      //   return this->m_key;
      // }
      operator uval() const
      {
        return this->m_val;
      }
      const key_t &operator=(const uval &key)
      {
        this->m_val = key;
        return *this;
      }
      // const key_t& operator=(std::uint64_t &id) {
      //   this->m_key = id;
      //   return *this;
      // }

      friend class entry_t;
      friend class const_entry_t;

      friend class iterator_t;
      friend class const_iterator_t;

      friend class vector;
    };
    key_t key;

  public:
    entry_base_t(const uval &_key, V &vec) : key(_key), m_vector(vec) {}
    operator value_t() const
    {
      // if (this->m_key > m_vector->size)
      //   throw std::out_of_range("key out of range");

      auto it = this->m_vector.m_data.find(static_cast<uval>(this->key.m_val));
      return (it != this->m_vector.m_data.end() ? it->second : this->m_vector.default_value);
    }
  };

public:
  class entry_t : public entry_base_t<vector>
  {
  public:
    using entry_base_t::entry_base_t;

    entry_t &operator=(const value_t &val)
    {
      // if (m_key > m_vector->size)
      //   throw std::out_of_range("key out of range");

      if (val != this->m_vector.default_value)
        this->m_vector.m_data.insert_or_assign(this->key.m_val, val);
      else
      {
        this->m_vector.m_data.erase(this->key.m_val);
      }

      return *this;
    }
  };
  class const_entry_t : public entry_base_t<const vector>
  {
    using entry_base_t::entry_base_t;
  };

#pragma endregion } entry *definitions
#pragma region iterator* definitions {
private:
  template <class V>
  class iterator_base_t : public std::iterator<std::random_access_iterator_tag, entry_t>
  {
  public:
    using entry_base_t = entry_base_t<V>;

  protected:
    entry_base_t *m_be;

  public:
    iterator_base_t(entry_base_t *be) : m_be(be) {}
    iterator_base_t(const iterator_base_t &other) = delete;
    iterator_base_t(iterator_base_t &&other) = delete;

    iterator_base_t &operator++()
    {
      ++this->m_be->key.m_val;
      return *this;
    }
    iterator_base_t &operator--()
    {
      --this->m_be->key.m_val;
      return *this;
    }

    bool operator!=(const iterator_base_t &other) const
    {
      return this->m_be->key.m_val != other.m_be->key.m_val;
    }
    bool operator==(const iterator_base_t &other) const
    {
      return this->m_be->key.m_val == other.m_be->key.m_val;
    }

    virtual entry_base_t &operator*() = 0;
    virtual entry_base_t *operator->() = 0;
  };

public:
  class iterator_t : public iterator_base_t<vector>
  {
  private:
    entry_t m_e;

  public:
    // using iterator_base_t::iterator_base_t;
    using iterator_base_t::operator++;
    using iterator_base_t::operator--;

    iterator_t(const uval &key, vector &vec) : iterator_base_t(&this->m_e), m_e(key, vec) {}
    iterator_t(const iterator_t &other) : iterator_base_t(&this->m_e), m_e(other.m_e) {}
    iterator_t(iterator_t &&other) : iterator_base_t(&this->m_e), m_e(other.m_e) {}

    iterator_t operator++(int) const
    {
      iterator_t tmp = *this;

      this->m_be->key.m_val++;
      return tmp;
    }
    iterator_t operator--(int) const
    {
      iterator_t tmp = *this;

      this->m_be->key.m_val--;
      return tmp;
    }

    entry_t &operator*() override
    {
      return this->m_e;
    }
    entry_t *operator->() override
    {
      return &this->m_e;
    }

    friend class vector;
  };
  class const_iterator_t : public iterator_base_t<const vector>
  {
  private:
    const_entry_t m_ce;

  public:
    // using iterator_base_t::iterator_base_t;
    using iterator_base_t::operator++;
    using iterator_base_t::operator--;

    const_iterator_t(const uval &key, const vector &vec) : iterator_base_t(&this->m_ce), m_ce(key, vec) {}
    const_iterator_t(const const_iterator_t &other) : iterator_base_t(&this->m_ce), m_ce(other.m_ce) {}
    const_iterator_t(const_iterator_t &&other) : iterator_base_t(&this->m_ce), m_ce(other.m_ce) {}

    const_iterator_t operator++(int) const
    {
      const_iterator_t tmp = *this;

      this->m_be->key.m_val++;
      return tmp;
    }
    const_iterator_t operator--(int) const
    {
      const_iterator_t tmp = *this;

      this->m_be->key.m_val--;
      return tmp;
    }
    const_entry_t &operator*() override
    {
      return this->m_ce;
    };
    const_entry_t *operator->() override
    {
      return &this->m_ce;
    };

    friend class vector;
  };
#pragma endregion } iterator *definitions

#pragma endregion } nested definitions

private:
  container_t m_data;

public:
  class size_t
  {
  private:
    uval m_size;

  public:
    size_t() : m_size(0) {}
    size_t(const uval::type &val) : m_size(val) {}
    operator uval::type() const { return this->m_size; }
  } size;
  class default_value_t
  {
  private:
    value_t m_default_value;

  public:
    default_value_t() : m_default_value(0) {}
    default_value_t(const value_t &val) : m_default_value(val) {}
    operator value_t() const { return this->m_default_value; }
  } default_value;

  vector() : m_data(), size(0), default_value(0) {}
  vector(const uval::type &_size, const value_t &_val) : m_data(), size(_size), default_value(_val) {}

#pragma region iterator methods {
  iterator_t begin()
  {
    iterator_t it(static_cast<uval>(this->size), *this);
    it.m_e.key.m_val = 0;

    return it;
  }
  const_iterator_t begin() const
  {
    const_iterator_t it(static_cast<uval>(this->size), *this);
    it.m_ce.key.m_val = 0;

    return it;
  }
  const_iterator_t cbegin() const
  {
    return this->begin();
  }

  iterator_t end()
  {
    iterator_t it(static_cast<uval>(this->size), *this);
    return it;
  }
  const_iterator_t end() const
  {
    const_iterator_t it(static_cast<uval>(this->size), *this);
    return it;
  }
  const_iterator_t cend() const
  {
    return this->end();
  }

#pragma endregion } iterator methods

  entry_t operator[](const key_t &key)
  {
    uval ukey(key, this->size - 1);
    return entry_t(ukey, *this);
  }
};

std::ostream &operator<<(std::ostream &os, const vector &vec)
{
  os << "{\n";
  if (vec.size != 0)
  {
    for (auto it = vec.begin(); it != --vec.end(); ++it)
    {
      os << "  " << static_cast<vector::uval>(it->key) << ": " << *it << ",\n";
    }
    os << "  " << static_cast<vector::uval>((--vec.end())->key) << ": " << *(--vec.end()) << "\n";
  }
  os << "}";
  return os;
}

int main(int argc, char *argv[])
{
  // std::cout << "STD vector size: " << sizeof(std::vector<double>) << std::endl;
  // std::cout << "STL vector size: " << sizeof(vector) << std::endl;

  vector vec(10, 3);

  for (auto i = 0; i < vec.size; i++)
    vec[i] = i;

  std::cout << "vec: " << vec << std::endl;

  return 0;
}