#ifndef SGE_PECTOR_ITERATOR_H
#define SGE_PECTOR_ITERATOR_H
#include <type_traits>
#include <iterator>
#include <cassert>

namespace sge {
template <typename T, typename Container>
class const_pector_iterator;

template <typename T, typename Container>
class pector_iterator {
private:
    template <typename, typename>
    friend class pector_iterator;
    template <typename, typename>
    friend class const_pector_iterator;

public:
    using iterator_category = std::random_access_iterator_tag;
    using iterator_concept = std::random_access_iterator_tag;
    using value_type = T;
    using pointer = T*;
    using reference = T&;
    using difference_type = std::ptrdiff_t;

private:
    Container *m_parent = nullptr;
    size_t m_index = 0;

public:
    pector_iterator(Container *parent, size_t index) :
    m_parent(parent),
    m_index(index)
    {}

    reference operator*() const
    {
        assert(this->m_parent && "[pector_iterator]::operator*() | Attempt to dereference .end().");
        return this->m_parent->operator[](this->m_index);
    }
    
    pointer operator->() const
    {
        assert(this->m_parent && "[pector_iterator]::operator*() | Attempt to dereference .end().");
        return &this->m_parent->operator[](this->m_index);
    }

    reference operator[](difference_type n) const
    {
        assert(this->m_index + n <= this->m_parent->size() && "[pector_iterator]::operator(difference_type n) | Attempt to exceed .end().");
        return *(*this + n);
    }

    pector_iterator& operator++()
    {
        assert(this->m_index + 1 <= this->m_parent->size() && "[pector_iterator]::operator++() | Attempt to exceed .end().");
        ++this->m_index;
        return *this;
    }

    pector_iterator operator++(int)
    {
        pector_iterator temp = *this;
        ++(*this);
        return temp;
    }

    pector_iterator& operator--()
    {
        assert(this->m_index - 1 >= 0 && "[pector_iterator]::operator--() | Attempt to decrement before valid index (0).");
        --this->m_index;
        return *this;
    }

    pector_iterator operator--(int)
    {
        pector_iterator temp = *this;
        --(*this);
        return temp;
    }

    pector_iterator& operator+=(difference_type n)
    {
        assert(this->m_index + n <= this->m_parent->size() && "[pector_iterator]::operator+=(difference_type n) | Attempt to exceed .end().");
        this->m_index += n;
        return *this;
    }

    pector_iterator& operator-=(difference_type n)
    {
        assert(this->m_index - n >= 0 && "[pector_iterator]::operator-=(difference_type n) | Attempt to decrement before valid index (0).");
        this->m_index -= n;
        return *this;
    }

    friend pector_iterator operator+(pector_iterator it, difference_type n)
    {
        it += n;
        return it;
    }

    friend pector_iterator operator+(difference_type n, pector_iterator it)
    {
        it += n;
        return it;
    }

    friend pector_iterator operator-(pector_iterator it, difference_type n)
    {
        it -= n;
        return it;
    }

    friend difference_type operator-(const pector_iterator &lhs, const pector_iterator &rhs)
    {
        return lhs.m_index - rhs.m_index;
    }

    friend bool operator==(const pector_iterator &lhs, const pector_iterator &rhs)
    {
        return lhs.m_parent == rhs.m_parent && lhs.m_index == rhs.m_index;
    }

    friend auto operator<=>(const pector_iterator &lhs, const pector_iterator &rhs)
    {
        if (lhs.m_parent != rhs.m_parent)
        {
            return lhs.m_parent <=> rhs.m_parent;
        }
        
        return lhs.m_index <=> rhs.m_index;
    }
}; // class pector_iterator

template <typename T, typename Container>
class const_pector_iterator {
private:
    template <typename, typename>
    friend class const_pector_iterator;

public:
    using iterator_category = std::random_access_iterator_tag;
    using iterator_concept = std::random_access_iterator_tag;
    using value_type = T;
    using pointer = T*;
    using reference = T&;
    using difference_type = std::ptrdiff_t;

private:
    Container *m_parent = nullptr;
    size_t m_index = 0;

public:
    const_pector_iterator(const pector_iterator<T, Container> &other) :
    m_parent(other.m_parent),
    m_index(other.m_index)
    {}

    const_pector_iterator(Container *parent, size_t index) :
    m_parent(parent),
    m_index(index)
    {}

    reference operator*() const
    {
        assert(this->m_parent && "[const_pector_iterator]::operator*() | Attempt to dereference .end().");
        return this->m_parent->operator[](this->m_index);
    }
    
    pointer operator->() const
    {
        assert(this->m_parent && "[const_pector_iterator]::operator*() | Attempt to dereference .end().");
        return &this->m_parent->operator[](this->m_index);
    }

    reference operator[](difference_type n) const
    {
        assert(this->m_index + n <= this->m_parent->size() && "[const_pector_iterator]::operator(difference_type n) | Attempt to exceed .end().");
        return *(*this + n);
    }

    const_pector_iterator& operator++()
    {
        assert(this->m_index + 1 <= this->m_parent->size() && "[const_pector_iterator]::operator++() | Attempt to exceed .end().");
        ++this->m_index;
        return *this;
    }

    const_pector_iterator operator++(int)
    {
        const_pector_iterator temp = *this;
        ++(*this);
        return temp;
    }

    const_pector_iterator& operator--()
    {
        assert(this->m_index - 1 >= 0 && "[const_pector_iterator]::operator--() | Attempt to decrement before valid index (0).");
        --this->m_index;
        return *this;
    }

    const_pector_iterator operator--(int)
    {
        const_pector_iterator temp = *this;
        --(*this);
        return temp;
    }

    const_pector_iterator& operator+=(difference_type n)
    {
        assert(this->m_index + n <= this->m_parent->size() && "[const_pector_iterator]::operator+=(difference_type n) | Attempt to exceed .end().");
        this->m_index += n;
        return *this;
    }

    const_pector_iterator& operator-=(difference_type n)
    {
        assert(this->m_index - n >= 0 && "[const_pector_iterator]::operator-=(difference_type n) | Attempt to decrement before valid index (0).");
        this->m_index -= n;
        return *this;
    }

    friend const_pector_iterator operator+(const_pector_iterator it, difference_type n)
    {
        it += n;
        return it;
    }

    friend const_pector_iterator operator+(difference_type n, const_pector_iterator it)
    {
        it += n;
        return it;
    }

    friend const_pector_iterator operator-(const_pector_iterator it, difference_type n)
    {
        it -= n;
        return it;
    }

    friend difference_type operator-(const const_pector_iterator &lhs, const const_pector_iterator &rhs)
    {
        return lhs.m_index - rhs.m_index;
    }

    friend bool operator==(const const_pector_iterator &lhs, const const_pector_iterator &rhs)
    {
        return lhs.m_parent == rhs.m_parent && lhs.m_index == rhs.m_index;
    }

    friend auto operator<=>(const const_pector_iterator &lhs, const const_pector_iterator &rhs)
    {
        if (lhs.m_parent != rhs.m_parent)
        {
            return lhs.m_parent <=> rhs.m_parent;
        }
        
        return lhs.m_index <=> rhs.m_index;
    }
}; // class const_pector_iterator

} // namespace sge

#endif // SGE_PECTOR_ITERATOR_H