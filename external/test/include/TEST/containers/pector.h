#ifndef SGE_PECTOR_H
#define SGE_PECTOR_H
#include <memory>
#include <vector>
#include <cstring>
#include <cassert>
#include <iostream>
#include <TEST/iterators/pector_iterator.h>

namespace sge {
template<typename T, size_t PAGE_BITS = 10, typename Allocator = std::allocator<T>>
class pector {
public:
    static constexpr size_t ELEMENTS_PER_PAGE =  1ULL << PAGE_BITS;
    static constexpr size_t PAGE_MASK = ELEMENTS_PER_PAGE - 1;

    using value_type    = T;
    using AllocTraits   = std::allocator_traits<Allocator>;
    using PageAlloc     = typename AllocTraits::template rebind_alloc<T>;
    using PageTraits    = std::allocator_traits<PageAlloc>;
    using PagePtrAlloc  = typename AllocTraits::template rebind_alloc<T*>;

    using iterator          = pector_iterator<T, PAGE_BITS>;
    using const_iterator    = const_pector_iterator<T, PAGE_BITS>;

    static_assert(
        AllocTraits::propagate_on_container_move_assignment::value ||
        AllocTraits::is_always_equal::value,
        "Error: [pector] requires an allocator type that propagates on move or is always equal."
    );

private:
    [[no_unique_address]] PageAlloc         m_pageAlloc;
    std::vector<T*, PagePtrAlloc>           m_pages;
    size_t                                  m_size = 0;
    size_t                                  m_capacity = 0;

    /**
     * Retrieves the page index.
     * @param index the base index
     * 
     * @return the page index
     */
    [[nodiscard]] size_t getPageIndex(size_t index) const noexcept;

    /**
     * Retrieves the offset index
     * @param index the base index
     * 
     * @return the offset index
     */
    [[nodiscard]] size_t getElementOffset(size_t index) const noexcept;

    /**
     * Calculates how many pages are required to store n elements.
     * @param elementCount the amount of elements to be held
     * 
     * @return the amount of pages required
     */
    [[nodiscard]] size_t getRequiredPages(size_t elementCount) const noexcept;

    /**
     * Retrieves the active amount of pages.
     * @return the amount of active pages being used
     */
    [[nodiscard]] size_t totalPages() const noexcept;

public:
    /**
     * Explicit parameterized constructor.
     * @param alloc the allocator instance
     */
    pector(const Allocator &alloc = Allocator());

    /**
     * Destructor.
     */
    ~pector();

    pector(const pector &) = delete;
    pector& operator=(const pector &) = delete;

    /**
     * Move constructor.
     * @param other the other pector
     */
    pector(pector&& other) noexcept;

    /**
     * Move assignment.
     * @param other the other pector
     */
    pector& operator=(pector&& other) noexcept (
        AllocTraits::propagate_on_container_move_assignment::value ||
        AllocTraits::is_always_equal::value
    );

    /**
     * Retrieves an element.
     * @param index the index to get from
     * 
     * @return a reference to the element
     */
    T& operator[](size_t index);

    /**
     * Retrieves an element.
     * @param index the index to get from
     * 
     * @return a const reference to the element
     */
    const T& operator[](size_t index) const;

    /**
     * Appends an element at the end of the pector.
     * @param value the value to push to the back
     */
    void push_back(const T &value);

    /**
     * Appends an element at the end of the pector.
     * @param value the value "move" to the back
     */
    void push_back(T&& value);

    /**
     * Appends an element at the end of the pector.
     * @tparam Args constructor arguments
     * @param args the constructor arguments of type T
     */
    template <typename... Args>
    void emplace_back(Args&&... args);

    /**
     * Removes the last element from the pector.
     */
    void pop_back() noexcept;

    /**
     * Swaps two elements.
     * @param indexA the first index
     * @param indexB the second index
     */
    void swapElements(size_t indexA, size_t indexB) noexcept;

    /**
     * Clears the pector.
     */
    void clear();

    /**
     * Reserves enough memory to match newCapacity.
     * If newCapacity <= this->capacity(), nothing happens.
     * @param newCapacity the desired capacity
     * 
     * NOTE: This allocates the page chunks so they are ready for usage.
     */
    void reserve(size_t newCapacity);

    /**
     * Resizes the pector to match newSize.
     * @param newSize the desired size
     */
    void resize(size_t newSize);

    /**
     * Resizes the pector to match newSize.
     * @param newSize the desired size
     * @param value the value to fill the empty slots with
     */
    void resize(size_t newSize, const T &value);

    /**
     * Releases all heap-allocated memory.
     */
    void release();

    /**
     * Checks if the pector is empty.
     * 
     * @return true if empty, false otherwise
     */
    bool empty() const noexcept;

    /**
     * @return the size of the pector
     */
    size_t size() const noexcept;

    /**
     * @return the capacity of the pector
     */
    size_t capacity() const noexcept;

    iterator begin() noexcept;
    iterator end() noexcept;

    const_iterator cbegin() const noexcept;
    const_iterator cend() const noexcept;
}; // class pector
} // namespace sge

// Implementation

template<typename T, size_t PAGE_BITS, typename Allocator>
inline size_t sge::pector<T, PAGE_BITS, Allocator>::getElementOffset(size_t index) const noexcept
{
    return index & PAGE_MASK;
}

template<typename T, size_t PAGE_BITS, typename Allocator>
inline size_t sge::pector<T, PAGE_BITS, Allocator>::getPageIndex(size_t index) const noexcept
{
    return (index >> PAGE_BITS);
}

template<typename T, size_t PAGE_BITS, typename Allocator>
inline size_t sge::pector<T, PAGE_BITS, Allocator>::getRequiredPages(size_t elementCount) const noexcept
{
    return (elementCount + PAGE_MASK) >> PAGE_BITS;
}

template<typename T, size_t PAGE_BITS, typename Allocator>
inline size_t sge::pector<T, PAGE_BITS, Allocator>::totalPages() const noexcept
{
    return (this->m_pages.empty()) ? 0: this->m_pages.size() - 1;
}

template<typename T, size_t PAGE_BITS, typename Allocator>
inline sge::pector<T, PAGE_BITS, Allocator>::pector(const Allocator &alloc) :
m_pageAlloc(alloc),
m_pages(PagePtrAlloc(alloc))
{}

template<typename T, size_t PAGE_BITS, typename Allocator>
inline sge::pector<T, PAGE_BITS, Allocator>::~pector()
{
    this->release();
}

template<typename T, size_t PAGE_BITS, typename Allocator>
inline sge::pector<T, PAGE_BITS, Allocator>::pector(pector&& other) noexcept :
m_pageAlloc(std::move(other.m_pageAlloc)),
m_pages(std::move(other.m_pages)),
m_capacity(other.m_capacity),
m_size(other.m_size)
{
    other.m_capacity = 0;
    other.m_size = 0;
}

template<typename T, size_t PAGE_BITS, typename Allocator>
inline sge::pector<T, PAGE_BITS, Allocator>& sge::pector<T, PAGE_BITS, Allocator>::operator=(pector&& other)
noexcept (
    AllocTraits::propagate_on_container_move_assignment::value ||
    AllocTraits::is_always_equal::value
)
{
    if (this == &other) return *this;

    constexpr bool propagate = AllocTraits::propagate_on_container_move_assignment::value;

    this->release();

    if constexpr (propagate)
    {
        this->m_pageAlloc = std::move(other.m_pageAlloc);
    }
    else
    {
        assert(this->m_pageAlloc == other.m_pageAlloc && "[pector]::operator=(pector&&) | Error. Allocators must match if propagation is disabled.");
    }

    this->m_pages = std::move(other.m_pages);
    this->m_capacity = other.m_capacity;
    this->m_size = other.m_size;

    other.m_capacity = 0;
    other.m_size = 0;
    return *this;
}

template<typename T, size_t PAGE_BITS, typename Allocator>
inline T& sge::pector<T, PAGE_BITS, Allocator>::operator[](size_t index)
{
    assert(index < this->m_size && "[pector]::operator[](size_t index) | Error. index >= size()");
    
    size_t pageIndex = this->getPageIndex(index);
    size_t offset = this->getElementOffset(index);

    return this->m_pages[pageIndex][offset];
}

template<typename T, size_t PAGE_BITS, typename Allocator>
inline const T& sge::pector<T, PAGE_BITS, Allocator>::operator[](size_t index) const
{
    assert(index < this->m_size && "[pector]::operator[](size_t index) | Error. index >= size()");
    
    size_t pageIndex = this->getPageIndex(index);
    size_t offset = this->getElementOffset(index);

    return this->m_pages[pageIndex][offset];
}

template<typename T, size_t PAGE_BITS, typename Allocator>
inline void sge::pector<T, PAGE_BITS, Allocator>::push_back(const T &value)
{
    if (this->m_size >= this->m_capacity)
    {
        size_t curPages = this->totalPages();
        size_t newPages = (curPages == 0) ? 1 : curPages * 2;
        size_t newCapacity = newPages * ELEMENTS_PER_PAGE;
        this->reserve(newCapacity);
    }

    size_t pageIndex = this->getPageIndex(this->m_size);
    size_t offset = this->getElementOffset(this->m_size);

    PageTraits::construct(this->m_pageAlloc, std::addressof(this->m_pages[pageIndex][offset]), value);

    ++this->m_size;
}

template <typename T, size_t PAGE_BITS, typename Allocator>
inline void sge::pector<T, PAGE_BITS, Allocator>::push_back(T&& value)
{
    if (this->m_size >= this->m_capacity)
    {
        size_t curPages = this->totalPages();
        size_t newPages = (curPages == 0) ? 1 : curPages * 2;
        size_t newCapacity = newPages * ELEMENTS_PER_PAGE;
        this->reserve(newCapacity);
    }
    
    size_t pageIndex = this->getPageIndex(this->m_size);
    size_t offset = this->getElementOffset(this->m_size);

    PageTraits::construct(this->m_pageAlloc, std::addressof(this->m_pages[pageIndex][offset]), std::move(value));
    ++this->m_size;
}

template <typename T, size_t PAGE_BITS, typename Allocator>
template <typename... Args>
inline void sge::pector<T, PAGE_BITS, Allocator>::emplace_back(Args&&... args)
{
    if (this->m_size >= this->m_capacity)
    {
        size_t curPages = this->totalPages();
        size_t newPages = (curPages == 0) ? 1 : curPages * 2;
        size_t newCapacity = newPages * ELEMENTS_PER_PAGE;
        this->reserve(newCapacity);
    }
    
    size_t pageIndex = this->getPageIndex(this->m_size);
    size_t offset = this->getElementOffset(this->m_size);

    PageTraits::construct(this->m_pageAlloc, std::addressof(this->m_pages[pageIndex][offset]), std::forward<Args>(args)...);

    ++this->m_size;
}

template <typename T, size_t PAGE_BITS, typename Allocator>
inline void sge::pector<T, PAGE_BITS, Allocator>::pop_back() noexcept
{
    assert(this->m_size > 0 && "[pector]::pop_back() | Error. Cannot pop from an empty pector (size <= 0).");

    --this->m_size;

    if constexpr (!std::is_trivially_destructible_v<T>)
    {
        const size_t pageIndex = this->getPageIndex(this->m_size);
        const size_t offset = this->getElementOffset(this->m_size);

        PageTraits::destroy(this->m_pageAlloc, std::addressof(this->m_pages[pageIndex][offset]));
    }
}

template <typename T, size_t PAGE_BITS, typename Allocator>
inline void sge::pector<T, PAGE_BITS, Allocator>::swapElements(size_t indexA, size_t indexB) noexcept
{
    if (indexA == indexB) return;
    
    assert(indexA < this->m_size && "[pector]::swapElements(size_t indexA, size_t indexB) | Error. indexA out-of-bounds");
    assert(indexB < this->m_size && "[pector]::swapElements(size_t indexA, size_t indexB) | Error. indexB out-of-bounds");

    size_t pageIndexA = this->getPageIndex(indexA);
    size_t offsetA = this->getElementOffset(indexA);
    
    size_t pageIndexB = this->getPageIndex(indexB);
    size_t offsetB = this->getElementOffset(indexB);

    T& elementA = this->m_pages[pageIndexA][offsetA];
    T& elementB = this->m_pages[pageIndexB][offsetB];

    std::swap(elementA, elementB);
}

template<typename T, size_t PAGE_BITS, typename Allocator>
inline void sge::pector<T, PAGE_BITS, Allocator>::clear()
{
    if (!std::is_trivially_destructible_v<T>)
    {
        if (this->m_size > 0)
        {
            const size_t startPage = this->getPageIndex(this->m_size - 1);
            const size_t endOffset = this->getElementOffset(this->m_size - 1);

            for (size_t p = startPage + 1; p > 0; --p)
            {
                const size_t pageIndex = p - 1;

                T* curPage = this->m_pages[pageIndex];

                const size_t firstElem = (pageIndex == startPage) ? endOffset : PAGE_MASK;

                for (size_t e = firstElem + 1; e > 0; --e)
                {
                    const size_t elemOffset = e - 1;
                    PageTraits::destroy(this->m_pageAlloc, std::addressof(curPage[elemOffset]));
                }
            }
        }
    }

    this->m_size = 0;
}

template<typename T, size_t PAGE_BITS, typename Allocator>
inline void sge::pector<T, PAGE_BITS, Allocator>::reserve(size_t newCapacity)
{
    // Calculate required pages.
    size_t requiredPages = this->getRequiredPages(newCapacity);
    size_t oldPageCount = this->totalPages();

    // Return if we have enough capacity.
    if (requiredPages <= oldPageCount) return;

    // Resize master directory (+1 for dummy pointer).
    this->m_pages.resize(requiredPages + 1, nullptr);

    // Allocate uninitialized memory for future use.
    for (size_t i = oldPageCount; i < requiredPages; ++i)
    {
        this->m_pages[i] = PageTraits::allocate(this->m_pageAlloc, ELEMENTS_PER_PAGE);
    }

    this->m_capacity = requiredPages * ELEMENTS_PER_PAGE;
}

template <typename T, size_t PAGE_BITS, typename Allocator>
inline void sge::pector<T, PAGE_BITS, Allocator>::resize(size_t newSize)
{
    this->resize(newSize, T());
}

template <typename T, size_t PAGE_BITS, typename Allocator>
inline void sge::pector<T, PAGE_BITS, Allocator>::resize(size_t newSize, const T &value)
{
    // Do nothing if sizes match.
    if (newSize == this->m_size) return;
    
    // Case 1. Upsizing.
    if (newSize > this->m_size)
    {
        // Reserve memory.
        this->reserve(newSize);

        // Get start stats.
        size_t startPage = this->getPageIndex(this->m_size);
        size_t startOffset = this->getElementOffset(this->m_size);

        // Get end stats.
        size_t endPage = this->getPageIndex(newSize - 1);
        size_t endOffset = this->getElementOffset(newSize - 1);

        for (size_t p = startPage; p <= endPage; ++p)
        {
            const size_t firstElem = (p == startPage) ? startOffset : 0;
            const size_t lastElem = (p == endPage) ? endOffset : PAGE_MASK;

            T* curPage = this->m_pages[p];
            
            for (size_t e = firstElem; e <= lastElem; ++e)
            {
                PageTraits::construct(this->m_pageAlloc, std::addressof(curPage[e]), value);
            }
        }
    }
    // Case 2. Downsizing
    else
    {
        // Call destructors for non-trivial types.
        if constexpr (!std::is_trivially_destructible_v<T>)
        {
            size_t startPage = this->getPageIndex(this->m_size - 1);
            size_t startOffset = this->getElementOffset(this->m_size - 1);

            size_t endPage = this->getPageIndex(newSize);
            size_t endOffset = this->getElementOffset(newSize);

            for (size_t p = startPage; p >= endPage && p < this->totalPages(); --p)
            {
                const size_t firstElem = (p == startPage) ? startOffset : PAGE_MASK;
                const size_t lastElem = (p == endPage) ? endOffset : 0;

                T* curPage = this->m_pages[p];
                
                for (size_t e = firstElem; e >= lastElem && e < ELEMENTS_PER_PAGE; --e)
                {
                    PageTraits::destroy(this->m_pageAlloc, std::addressof(curPage[e]));
                }
            }
        }
    }

    this->m_size = newSize;
}

template<typename T, size_t PAGE_BITS, typename Allocator>
inline void sge::pector<T, PAGE_BITS, Allocator>::release()
{   
    // For empty state pectors.
    if (this->m_pages.empty())
    {
        this->m_size = 0;
        this->m_capacity = 0;
        return;
    }

    // Destroy non-trivial elements.
    if (!std::is_trivially_destructible_v<T>)
    {
        if (this->m_size > 0)
        {
            const size_t startPage = this->getPageIndex(this->m_size - 1);
            const size_t endOffset = this->getElementOffset(this->m_size - 1);

            for (size_t p = startPage + 1; p > 0; --p)
            {
                const size_t pageIndex = p - 1;

                T* curPage = this->m_pages[pageIndex];

                const size_t firstElem = (pageIndex == startPage) ? endOffset : PAGE_MASK;

                for (size_t e = firstElem + 1; e > 0; --e)
                {
                    const size_t elemOffset = e - 1;
                    PageTraits::destroy(this->m_pageAlloc, std::addressof(curPage[elemOffset]));
                }
            }
        }
    }

    // Deallocate pages.
    for (size_t i = 0; i < this->totalPages(); ++i)
    {
        PageTraits::deallocate(this->m_pageAlloc, this->m_pages[i], ELEMENTS_PER_PAGE);
    }

    // Reset trackers.
    this->m_pages.clear();
    this->m_capacity = 0;
    this->m_size = 0;
}

template<typename T, size_t PAGE_BITS, typename Allocator>
inline bool sge::pector<T, PAGE_BITS, Allocator>::empty() const noexcept
{
    return this->m_size == 0;
}

template<typename T, size_t PAGE_BITS, typename Allocator>
inline size_t sge::pector<T, PAGE_BITS, Allocator>::size() const noexcept
{
    return this->m_size;
}

template<typename T, size_t PAGE_BITS, typename Allocator>
inline size_t sge::pector<T, PAGE_BITS, Allocator>::capacity() const noexcept
{
    return this->m_capacity;
}

template<typename T, size_t PAGE_BITS, typename Allocator>
inline sge::pector<T, PAGE_BITS, Allocator>::iterator sge::pector<T, PAGE_BITS, Allocator>::begin() noexcept
{
    if (this->m_size == 0)
    {
        return this->end();
    }

    T** firstPage = &this->m_pages[0];

    return iterator(firstPage, 0);
}

template<typename T, size_t PAGE_BITS, typename Allocator>
inline sge::pector<T, PAGE_BITS, Allocator>::iterator sge::pector<T, PAGE_BITS, Allocator>::end() noexcept
{
    if (this->m_size == 0)
    {
        return iterator(nullptr, 0);
    }

    size_t pageIndex = this->getPageIndex(this->m_size);
    size_t offset = this->getElementOffset(this->m_size);

    T* *endPage = &this->m_pages[pageIndex];
    
    return iterator(endPage, offset);
}

template<typename T, size_t PAGE_BITS, typename Allocator>
inline sge::pector<T, PAGE_BITS, Allocator>::const_iterator sge::pector<T, PAGE_BITS, Allocator>::cbegin() const noexcept
{
    if (this->m_size == 0)
    {
        return this->cend();
    }

    T* const* firstPage = &this->m_pages[0];

    return const_iterator(firstPage, 0);
}

template<typename T, size_t PAGE_BITS, typename Allocator>
inline sge::pector<T, PAGE_BITS, Allocator>::const_iterator sge::pector<T, PAGE_BITS, Allocator>::cend() const noexcept
{
    if (this->m_size == 0)
    {
        return iterator(nullptr, 0);
    }

    size_t pageIndex = this->getPageIndex(this->m_size);
    size_t offset = this->getElementOffset(this->m_size);

    T* const *endPage = &this->m_pages[pageIndex];
    
    return const_iterator(endPage, offset);
}

#endif // SGE_PECTOR_H