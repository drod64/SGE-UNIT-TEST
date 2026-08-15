#ifndef SGE_PECTOR_ITERATOR_H
#define SGE_PECTOR_ITERATOR_H
#include <type_traits>
#include <iterator>
#include <cassert>

namespace sge {
template <typename T, size_t PAGE_BITS>
class const_pector_iterator;

template <typename T, size_t PAGE_BITS>
class pector_iterator {
private:
    template <typename, size_t>
    friend class pector_iterator;
    template <typename, size_t>
    friend class const_pector_iterator;

public:
    static constexpr size_t ELEMENTS_PER_PAGE = 1ULL << PAGE_BITS;
    static constexpr size_t PAGE_MASK = ELEMENTS_PER_PAGE - 1;

    using iterator_category = std::random_access_iterator_tag;
    using iterator_concept = std::random_access_iterator_tag;
    using value_type = T;
    using pointer = T*;
    using reference = T&;
    using difference_type = std::ptrdiff_t;

private:
    pointer*        m_page      = nullptr;
    pointer         m_cur       = nullptr;
    pointer         m_first     = nullptr;
    pointer         m_end       = nullptr;

public:
    pector_iterator() = default;

    pector_iterator(pointer* currentPage, size_t pageOffset) :
    m_page(currentPage)
    {
        if (this->m_page)
        {
            this->m_first = *this->m_page;
            this->m_end = this->m_first + ELEMENTS_PER_PAGE;
            this->m_cur = this->m_first + pageOffset;
        }
    }

    reference operator*() const
    {
        assert(this->m_cur && "[pector_iterator]::operator*() | Error. Attempt to dereference nullptr.");
        return *this->m_cur;
    }
    
    pointer operator->() const
    {
        assert(this->m_cur && "[pector_iterator]::operator*() | Error. Attempt to dereference nullptr.");
        return this->m_cur;
    }

    reference operator[](difference_type n) const
    {
        return *(*this + n);
    }

    pector_iterator& operator++()
    {
        ++this->m_cur;

        if (this->m_cur == this->m_end)
        {
            ++this->m_page;
            this->m_first = *this->m_page;
            this->m_end = this->m_first + ELEMENTS_PER_PAGE;
            this->m_cur = this->m_first;
        }

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
        if (this->m_cur == this->m_first)
        {
            --this->m_page;
            this->m_first = *this->m_page;
            this->m_end = this->m_first + ELEMENTS_PER_PAGE;
            this->m_cur = this->m_end - 1;
        }
        else
        {
            --this->m_cur;
        }

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
        if (n == 0) return *this;
        if (n < 0) return *this -= (-n);

        difference_type curOffset = this->m_cur - this->m_first;
        difference_type targetOffset = curOffset + n;
        
        if (targetOffset >= 0 && targetOffset < static_cast<difference_type>(ELEMENTS_PER_PAGE))
        {
            this->m_cur += n;
        }
        else
        {
            difference_type pagesToJump = targetOffset >> PAGE_BITS;
            
            this->m_page += pagesToJump;
            
            this->m_first = *this->m_page;
            this->m_end = this->m_first + ELEMENTS_PER_PAGE;
            this->m_cur = this->m_first + (targetOffset & PAGE_MASK);
        }

        return *this;
    }

    pector_iterator& operator-=(difference_type n)
    {
        if (n == 0) return *this;
        if (n < 0) return *this += (-n);

        difference_type curOffset = this->m_cur - this->m_first;

        if (n > curOffset)
        {
            difference_type targetOffset = curOffset - n;
            difference_type pagesToBacktrack = (-targetOffset + ELEMENTS_PER_PAGE - 1) >> PAGE_BITS;

            this->m_page -= pagesToBacktrack;
            this->m_first = *this->m_page;
            this->m_end = this->m_first + ELEMENTS_PER_PAGE;
            this->m_cur = this->m_end + (targetOffset % static_cast<difference_type>(ELEMENTS_PER_PAGE));
        }
        else
        {
            this->m_cur -= n;
        }

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
        if (!lhs.m_page && !rhs.m_page) return 0;

        difference_type pageDist    = lhs.m_page - rhs.m_page;
        difference_type lhsOffset   = lhs.m_cur - lhs.m_first;
        difference_type rhsOffset   = rhs.m_cur - rhs.m_first;

        return (pageDist << PAGE_BITS) + lhsOffset - rhsOffset;
    }

    friend bool operator==(const pector_iterator &lhs, const pector_iterator &rhs)
    {
        return lhs.m_cur == rhs.m_cur;
    }

    friend auto operator<=>(const pector_iterator &lhs, const pector_iterator &rhs)
    {
        if (lhs.m_page != rhs.m_page)
        {
            return lhs.m_page <=> rhs.m_page;
        }
        
        return lhs.m_cur <=> rhs.m_cur;
    }
}; // class pector_iterator

template <typename T, size_t PAGE_BITS>
class const_pector_iterator {
private:
    template <typename, size_t>
    friend class const_pector_iterator;

public:
    static constexpr size_t ELEMENTS_PER_PAGE = 1ULL << PAGE_BITS;
    static constexpr size_t PAGE_MASK = ELEMENTS_PER_PAGE - 1;

    using iterator_category = std::random_access_iterator_tag;
    using iterator_concept = std::random_access_iterator_tag;
    using value_type = T;
    using pointer = const T*;
    using reference = const T&;
    using difference_type = std::ptrdiff_t;

private:
    pointer*        m_page      = nullptr;
    pointer         m_cur       = nullptr;
    pointer         m_first     = nullptr;
    pointer         m_end       = nullptr;

public:
    const_pector_iterator() = default;

    const_pector_iterator(const pector_iterator<T, PAGE_BITS> &other) noexcept :
    m_page(other.m_page),
    m_cur(other.m_cur),
    m_first(other.m_first),
    m_end(other.m_end)
    {}

    const_pector_iterator(pointer* currentPage, size_t pageOffset) noexcept :
    m_page(currentPage)
    {
        if (this->m_page)
        {
            this->m_first = *this->m_page;
            this->m_end = this->m_first + ELEMENTS_PER_PAGE;
            this->m_cur = this->m_first + pageOffset;
        }
    }

    reference operator*() const
    {
        assert(this->m_cur && "[const_pector_iterator]::operator*() | Error. Attempt to dereference nullptr.");
        return *this->m_cur;
    }
    
    pointer operator->() const
    {
        assert(this->m_cur && "[const_pector_iterator]::operator*() | Error. Attempt to dereference nullptr.");
        return this->m_cur;
    }

    reference operator[](difference_type n) const
    {
        return *(*this + n);
    }

    const_pector_iterator& operator++()
    {
        ++this->m_cur;

        if (this->m_cur == this->m_end)
        {
            ++this->m_page;
            this->m_first = *this->m_page;
            this->m_end = this->m_first + ELEMENTS_PER_PAGE;
            this->m_cur = this->m_first;
        }

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
        if (this->m_cur == this->m_first)
        {
            --this->m_page;
            this->m_first = *this->m_page;
            this->m_end = this->m_first + ELEMENTS_PER_PAGE;
            this->m_cur = this->m_end - 1;
        }
        else
        {
            --this->m_cur;
        }

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
        if (n == 0) return *this;
        if (n < 0) return *this -= (-n);

        difference_type curOffset = this->m_cur - this->m_first;
        difference_type targetOffset = curOffset + n;
        
        if (targetOffset >= 0 && targetOffset < static_cast<difference_type>(ELEMENTS_PER_PAGE))
        {
            this->m_cur += n;
        }
        else
        {
            difference_type pagesToJump = targetOffset >> PAGE_BITS;
            
            this->m_page += pagesToJump;
            
            this->m_first = *this->m_page;
            this->m_end = this->m_first + ELEMENTS_PER_PAGE;
            this->m_cur = this->m_first + (targetOffset & PAGE_MASK);
        }

        return *this;
    }

    const_pector_iterator& operator-=(difference_type n)
    {
        if (n == 0) return *this;
        if (n < 0) return *this += (-n);

        difference_type curOffset = this->m_cur - this->m_first;

        if (n > curOffset)
        {
            difference_type targetOffset = curOffset - n;
            difference_type pagesToBacktrack = (-targetOffset + ELEMENTS_PER_PAGE - 1) >> PAGE_BITS;

            this->m_page -= pagesToBacktrack;
            this->m_first = *this->m_page;
            this->m_end = this->m_first + ELEMENTS_PER_PAGE;
            this->m_cur = this->m_end + (targetOffset % static_cast<difference_type>(ELEMENTS_PER_PAGE));
        }
        else
        {
            this->m_cur -= n;
        }

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
        if (!lhs.m_page && !rhs.m_page) return 0;

        difference_type pageDist    = lhs.m_page - rhs.m_page;
        difference_type lhsOffset   = lhs.m_cur - lhs.m_first;
        difference_type rhsOffset   = rhs.m_cur - rhs.m_first;

        return (pageDist << PAGE_BITS) + lhsOffset - rhsOffset;
    }

    friend bool operator==(const const_pector_iterator &lhs, const const_pector_iterator &rhs)
    {
        return lhs.m_cur == rhs.m_cur;
    }

    friend auto operator<=>(const const_pector_iterator &lhs, const const_pector_iterator &rhs)
    {
        if (lhs.m_page != rhs.m_page)
        {
            return lhs.m_page <=> rhs.m_page;
        }
        
        return lhs.m_cur <=> rhs.m_cur;
    }
}; // class const_pector_iterator
} // namespace sge

#endif // SGE_PECTOR_ITERATOR_H