#ifndef SGE_PECTOR_ITERATOR_H
#define SGE_PECTOR_ITERATOR_H
#include <type_traits>
#include <iterator>

namespace sge {

template <typename T, size_t PAGE_SIZE>
class const_pector_iterator;

template <typename T, size_t PAGE_SIZE>
class pector_iterator {
private:
    // template <typename, size_t>
    // friend class pector_iterator;
    // template <typename, size_t>
    // friend class const_pector_iterator<T, PAGE_SIZE>;

public:
    using iterator_category = std::random_access_iterator_tag;
    using iterator_concept = std::random_access_iterator_tag;
    using value_type = T;
    using pointer = T*;
    using reference = T&;
    using difference_type = std::ptrdiff_t;

private:
    pointer* m_curPage;
    pointer m_curElement;
    pointer m_pageBegin;

public:
    pector_iterator() = default;

    pector_iterator(pointer* pagePtr, pointer element, pointer pageBegin) :
    m_curPage(pagePtr),
    m_curElement(element),
    m_pageBegin(pageBegin)
    {}

    reference operator*() const
    {
        return *this->m_curElement;
    }

    pointer operator->() const
    {
        return this->m_curElement;
    }

    reference operator[](difference_type n) const
    {
        return *(*this + n);
    }

    pector_iterator& operator++()
    {
        ++this->m_curElement;

        if (this->m_curElement == this->m_pageBegin + PAGE_SIZE)
        {
            // Advance page.
            ++this->m_curPage;
            // Update begin pointer.
            this->m_pageBegin = *this->m_curPage;
            // Update element tracking pointer.
            this->m_curElement = this->m_pageBegin;
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
        if (this->m_curElement == this->m_pageBegin)
        {
            --this->m_curPage;
            this->m_pageBegin = *this->m_curPage;
            this->m_curElement = this->m_pageBegin + PAGE_SIZE;
        }

        --this->m_curElement;
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

        difference_type curOffset = this->m_curElement - this->m_pageBegin;
        difference_type targetOffset = curOffset + n;

        if (targetOffset >= 0 && targetOffset < static_cast<difference_type>(PAGE_SIZE))
        {
            this->m_curElement += n;
        }
        else
        {
            difference_type pageOffset = (targetOffset >= 0) ? (targetOffset / PAGE_SIZE) :
                                                                -static_cast<difference_type>((-targetOffset - 1) / PAGE_SIZE + 1);
            this->m_curPage += pageOffset;
            this->m_pageBegin = *this->m_curPage;

            difference_type elementOffset = targetOffset - (pageOffset * PAGE_SIZE);
            this->m_curElement = this->m_pageBegin + elementOffset;
        }

        return *this;
    }

    pector_iterator& operator-=(difference_type n)
    {
        return *this += -n;
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
        if (!lhs.m_curElement && !rhs.m_curElement) return 0;

        difference_type pageDist    = lhs.m_curPage - rhs.m_curPage;
        difference_type lhsOffset   = lhs.m_curElement - lhs.m_pageBegin;
        difference_type rhsOffset   = rhs.m_curElement - rhs.m_pageBegin;

        return (pageDist * PAGE_SIZE) + lhsOffset - rhsOffset;
    }

    friend bool operator==(const pector_iterator &lhs, const pector_iterator &rhs)
    {
        return lhs.m_curElement == rhs.m_curElement;
    }

    friend bool operator!=(const pector_iterator &lhs, const pector_iterator &rhs)
    {
        return !(lhs == rhs);
    }

    friend bool operator<(const pector_iterator &lhs, const pector_iterator &rhs)
    {
        return (lhs - rhs) < 0;
    }

    friend bool operator>(const pector_iterator &lhs, const pector_iterator &rhs)
    {
        return rhs < lhs;
    }

    friend bool operator<=(const pector_iterator &lhs, const pector_iterator &rhs)
    {
        return !(rhs < lhs);
    }
    friend bool operator>=(const pector_iterator &lhs, const pector_iterator &rhs)
    {
        return !(lhs < rhs);
    }
}; // class pector_iterator

template <typename T, size_t PAGE_SIZE>
class const_pector_iterator {
private:
    template <typename>
    friend class const_pector_iterator;

public:
    using iterator_category = std::random_access_iterator_tag;
    using iterator_concept = std::random_access_iterator_tag;
    using value_type = T;
    using pointer = const T*;
    using reference = const T&;
    using difference_type = std::ptrdiff_t;

private:
    pointer* m_curPage;
    pointer m_curElement;
    pointer m_pageBegin;

public:
    const_pector_iterator() = default;

    const_pector_iterator(pointer* pagePtr, pointer element, pointer pageBegin) :
    m_curPage(pagePtr),
    m_curElement(element),
    m_pageBegin(pageBegin)
    {}

    const_pector_iterator(const pector_iterator<T, PAGE_SIZE> &other) :
    m_curElement(other.m_curElement),
    m_curPage(other.m_curPage),
    m_pageBegin(other.m_pageBegin)
    {}

    reference operator*() const
    {
        return *this->m_curElement;
    }

    pointer operator->() const
    {
        return this->m_curElement;
    }

    reference operator[](difference_type n) const
    {
        return *(*this + n);
    }

    const_pector_iterator& operator++()
    {
        ++this->m_curElement;

        if (this->m_curElement == this->m_pageBegin + PAGE_SIZE)
        {
            // Advance page.
            ++this->m_curPage;
            // Update begin pointer.
            this->m_pageBegin = *this->m_curPage;
            // Update element tracking pointer.
            this->m_curElement = this->m_pageBegin;
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
        if (this->m_curElement == this->m_pageBegin)
        {
            --this->m_curPage;
            this->m_pageBegin = *this->m_curPage;
            this->m_curElement = this->m_pageBegin + PAGE_SIZE;
        }

        --this->m_curElement;
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

        difference_type curOffset = this->m_curElement - this->m_pageBegin;
        difference_type targetOffset = curOffset + n;

        if (targetOffset >= 0 && targetOffset < static_cast<difference_type>(PAGE_SIZE))
        {
            this->m_curElement += n;
        }
        else
        {
            difference_type pageOffset = (targetOffset >= 0) ? (targetOffset / PAGE_SIZE) :
                                                                -static_cast<difference_type>((-targetOffset - 1) / PAGE_SIZE + 1);
            this->m_curPage += pageOffset;
            this->m_pageBegin = *this->m_curPage;

            difference_type elementOffset = targetOffset - (pageOffset * PAGE_SIZE);
            this->m_curElement = this->m_pageBegin + elementOffset;
        }

        return *this;
    }

    const_pector_iterator& operator-=(difference_type n)
    {
        return *this += -n;
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
        if (!lhs.m_curElement && !rhs.m_curElement) return 0;

        difference_type pageDist    = lhs.m_curPage - rhs.m_curPage;
        difference_type lhsOffset   = lhs.m_curElement - lhs.m_pageBegin;
        difference_type rhsOffset   = rhs.m_curElement - rhs.m_pageBegin;

        return (pageDist * PAGE_SIZE) + lhsOffset - rhsOffset;
    }

    friend bool operator==(const const_pector_iterator &lhs, const const_pector_iterator &rhs)
    {
        return lhs.m_curElement == rhs.m_curElement;
    }

    friend bool operator!=(const const_pector_iterator &lhs, const const_pector_iterator &rhs)
    {
        return !(lhs == rhs);
    }

    friend bool operator<(const const_pector_iterator &lhs, const const_pector_iterator &rhs)
    {
        return (lhs - rhs) < 0;
    }

    friend bool operator>(const const_pector_iterator &lhs, const const_pector_iterator &rhs)
    {
        return rhs < lhs;
    }

    friend bool operator<=(const const_pector_iterator &lhs, const const_pector_iterator &rhs)
    {
        return !(rhs < lhs);
    }
    friend bool operator>=(const const_pector_iterator &lhs, const const_pector_iterator &rhs)
    {
        return !(lhs < rhs);
    }
}; // class const_pector_iterator
} // namespace sge
#endif // SGE_PECTOR_ITERATOR_H