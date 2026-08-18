#ifndef SGE_SPARSE_PAGE_TABLE_H
#define SGE_SPARSE_PAGE_TABLE_H
#include <memory>
#include <vector>
#include <cstring>
#include <cstdint>

namespace sge {
template <std::unsigned_integral T, size_t PAGE_BITS = 10, typename Allocator = std::allocator<T>>
class SparsePageTable {
public:
    static constexpr size_t ELEMENTS_PER_PAGE =  1ULL << PAGE_BITS;
    static constexpr size_t PAGE_MASK = ELEMENTS_PER_PAGE - 1;
    static constexpr T TOMBSTONE = std::numeric_limits<T>::max();

    using value_type    = T;
    using AllocTraits   = std::allocator_traits<Allocator>;
    using PageAlloc     = typename AllocTraits::template rebind_alloc<T>;
    using PageTraits    = std::allocator_traits<PageAlloc>;
    using PagePtrAlloc  = typename AllocTraits::template rebind_alloc<T*>;

    static_assert(
        AllocTraits::propagate_on_container_move_assignment::value ||
        AllocTraits::is_always_equal::value,
        "Error: [SparsePageTable] requires an allocator type that propagates on move or is always equal."
    );
    
private:
    [[no_unique_address]] PageAlloc         m_pageAlloc;
    std::vector<T*, PagePtrAlloc>           m_pages;
    size_t                                  m_activePageCount = 0;

    /**
     * Retrieves the page index.
     * @param index the base index
     * @return the page index
     */
    [[nodiscard]] size_t getPageIndex(size_t index) const noexcept;

    /**
     * Retrieves the offset index.
     * @param index the base index
     * @return the offset index
     */
    [[nodiscard]] size_t getElementOffset(size_t index) const noexcept;

    /**
     * Calculates how many pages are necessary to store n elements.
     * @param elementCount the desired amount of elements to be held
     * @return the required amount of pages
     */
    [[nodiscard]] size_t getRequiredPages(size_t elementCount) const noexcept;

    /**
     * Retrieves (or allocates a nonexistent) page.
     * @param pageIndex the page to get
     * @return a pointer to the page
     */
    [[nodiscard]] T* getOrCreatePage(size_t pageIndex);

public:
    /**
     * Explicit parameterized constructor.
     * @param alloc the alloc instance
     */
    explicit SparsePageTable(const Allocator &alloc = Allocator());

    /**
     * Destructor.
     */
    ~SparsePageTable();

    SparsePageTable(const SparsePageTable &other) = delete;
    SparsePageTable& operator=(const SparsePageTable &other) = delete;

    /**
     * Move constructor.
     * @param other the other SparsePageTable
     */
    SparsePageTable(SparsePageTable&& other) noexcept;

    /**
     * Move assignment.
     * @param other the other SparsePageTable
     */
    SparsePageTable& operator=(SparsePageTable&& other) noexcept (
        AllocTraits::propagate_on_container_move_assignment::value ||
        AllocTraits::is_always_equal::value
    );

    /**
     * Retrieves an element.
     * @param index the index to get from
     * @return a reference to the element
     */
    T& get(size_t index);

    /**
     * Retrieves an element.
     * @param index the index to get from
     * @return a const reference to the element
     */
    const T& get(size_t index) const;

    /**
     * Sets an element.
     * @param index the index to set
     * @param value the value
     */
    void set(size_t index, const T &value);

    /**
     * Erases an element (essentially making the slot value == TOMBSTONE).
     * @param index the index to erase
     */
    void erase(size_t index);

    /**
     * Clears container.
     */
    void clear() noexcept;

    /**
     * Lazily reserves pages.
     */
    void reservePages(size_t pageCount);

    /**
     * Shrinks the master page directory to fit only until the last active page.
     */
    void shrinkToFit();

    /**
     * Releases all heap-allocated memory chunks.
     */
    void release();

    /**
     * Checks if an index has a valid value (!= TOMBSTONE).
     * @param index the index to check
     * 
     * @return true if value != TOMBSTONE, false otherwise
     */
    bool contains(size_t index) const noexcept;

    /**
     * @return the amount of actively allocated pages
     */
    size_t allocatedPageCount() const noexcept;

    /**
     * @return the amount of pages (allocated or nullptr)
     */
    size_t pageCount() const noexcept;
}; // class SparsePageTable
} // namespace sge

template<std::unsigned_integral T, size_t PAGE_BITS, typename Allocator>
inline size_t sge::SparsePageTable<T, PAGE_BITS, Allocator>::getElementOffset(size_t index) const noexcept
{
    return index & PAGE_MASK;
}

template<std::unsigned_integral T, size_t PAGE_BITS, typename Allocator>
inline size_t sge::SparsePageTable<T, PAGE_BITS, Allocator>::getPageIndex(size_t index) const noexcept
{
    return index >> PAGE_BITS;
}

template<std::unsigned_integral T, size_t PAGE_BITS, typename Allocator>
inline size_t sge::SparsePageTable<T, PAGE_BITS, Allocator>::getRequiredPages(size_t elementCount) const noexcept
{
    return (elementCount + PAGE_MASK) >> PAGE_BITS;
}

template<std::unsigned_integral T, size_t PAGE_BITS, typename Allocator>
inline T* sge::SparsePageTable<T, PAGE_BITS, Allocator>::getOrCreatePage(size_t pageIndex)
{
    // Resize master directory.
    if(pageIndex >= this->m_pages.size())
    {
        size_t newSize = this->m_pages.size() * 2;
        if (pageIndex >= newSize) newSize = pageIndex + 1;

        this->m_pages.resize(newSize, nullptr);
    }

    // Get page.
    T* page = this->m_pages[pageIndex];

    // Allocate page if it doesn't exist.
    if (!page)
    {
        page = PageTraits::allocate(this->m_pageAlloc, ELEMENTS_PER_PAGE);

        ++this->m_activePageCount;

        // Fill page with default TOMBSTONE values.
        std::fill(page, page + ELEMENTS_PER_PAGE, TOMBSTONE);
        
        this->m_pages[pageIndex] = page;
    }

    // Return page pointer.
    return page;
}

template<std::unsigned_integral T, size_t PAGE_BITS, typename Allocator>
inline sge::SparsePageTable<T, PAGE_BITS, Allocator>::SparsePageTable(const Allocator &alloc) :
m_pages(PagePtrAlloc(alloc))
{}

template<std::unsigned_integral T, size_t PAGE_BITS, typename Allocator>
inline sge::SparsePageTable<T, PAGE_BITS, Allocator>::~SparsePageTable()
{
    this->release();
}

template<std::unsigned_integral T, size_t PAGE_BITS, typename Allocator>
inline sge::SparsePageTable<T, PAGE_BITS, Allocator>::SparsePageTable(SparsePageTable&& other) noexcept :
m_pageAlloc(std::move(other.m_pageAlloc)),
m_pages(std::move(other.m_pages)),
m_activePageCount(other.m_activePageCount)
{
    other.m_activePageCount = 0;
}

template<std::unsigned_integral T, size_t PAGE_BITS, typename Allocator>
inline sge::SparsePageTable<T, PAGE_BITS, Allocator>& sge::SparsePageTable<T, PAGE_BITS, Allocator>::operator=(SparsePageTable &&other)
noexcept (
    AllocTraits::propagate_on_container_move_assignment::value ||
    AllocTraits::is_always_equal::value
)
{
    if (this == &other) return *this;

    this->release();

    constexpr bool propagate = AllocTraits::propagate_on_container_move_assignment::value;

    if constexpr (propagate)
    {
        this->m_pageAlloc = std::move(other.m_pageAlloc);
    }
    else
    {
        assert(this->m_pageAlloc == other.m_pageAlloc && "[SparsePageTable]::operator=(SparsePageTable&&) | Error. Allocators must match if propagation is disabled.");
    }

    this->m_pages = std::move(other.m_pages);
    this->m_activePageCount = other.m_activePageCount;

    other.m_activePageCount = 0;
    return *this;
}

template <std::unsigned_integral T, size_t PAGE_BITS, typename Allocator>
inline T& sge::SparsePageTable<T, PAGE_BITS, Allocator>::get(size_t index)
{
    size_t pageIndex = this->getPageIndex(index);
    size_t offset = this->getElementOffset(index);
    
    assert(pageIndex < this->m_pages.size() && "[SparsePageTable]::get(size_t index) | Error. index out of bounds.");

    T *page = this->m_pages[pageIndex];

    assert(page != nullptr && "[SparsePageTable]::get(size_t index) | Error. Attempt to get from unallocated page.");

    return page[offset];
}

template <std::unsigned_integral T, size_t PAGE_BITS, typename Allocator>
inline const T& sge::SparsePageTable<T, PAGE_BITS, Allocator>::get(size_t index) const
{
    size_t pageIndex = this->getPageIndex(index);
    size_t offset = this->getElementOffset(index);
    
    assert(pageIndex < this->m_pages.size() && "[SparsePageTable]::get(size_t index) | Error. index out of bounds.");

    T *page = this->m_pages[pageIndex];

    assert(page != nullptr && "[SparsePageTable]::get(size_t index) | Error. Attempt to get from unallocated page.");

    return page[offset];
}

template <std::unsigned_integral T, size_t PAGE_BITS, typename Allocator>
inline void sge::SparsePageTable<T, PAGE_BITS, Allocator>::set(size_t index, const T &value)
{
    size_t pageIndex = this->getPageIndex(index);
    size_t offset = this->getElementOffset(index);
    
    T* page = this->getOrCreatePage(pageIndex);

    page[offset] = value;
}

template <std::unsigned_integral T, size_t PAGE_BITS, typename Allocator>
inline void sge::SparsePageTable<T, PAGE_BITS, Allocator>::erase(size_t index)
{
    size_t pageIndex = this->getPageIndex(index);
    
    if (pageIndex >= this->m_pages.size() || !this->m_pages[pageIndex]) return;

    T* page = this->m_pages[pageIndex];

    size_t offset = this->getElementOffset(index);

    page[offset] = TOMBSTONE;
}


template<std::unsigned_integral T, size_t PAGE_BITS, typename Allocator>
inline void sge::SparsePageTable<T, PAGE_BITS, Allocator>::clear() noexcept
{
    for (T* page : this->m_pages)
    {
        if (page)
        {
            std::fill(page, page + ELEMENTS_PER_PAGE, TOMBSTONE);
        }
    }
}

template<std::unsigned_integral T, size_t PAGE_BITS, typename Allocator>
inline void sge::SparsePageTable<T, PAGE_BITS, Allocator>::reservePages(size_t pageCount)
{
    this->m_pages.reserve(pageCount);
}

template<std::unsigned_integral T, size_t PAGE_BITS, typename Allocator>
inline void sge::SparsePageTable<T, PAGE_BITS, Allocator>::shrinkToFit()
{
    size_t activePages = this->m_pages.size();

    while (activePages > 0 && this->m_pages[activePages - 1] == nullptr)
    {
        --activePages;
    }

    this->m_pages.resize(activePages);
    this->m_pages.shrink_to_fit();
}

template<std::unsigned_integral T, size_t PAGE_BITS, typename Allocator>
inline void sge::SparsePageTable<T, PAGE_BITS, Allocator>::release()
{
    for (T* page : this->m_pages)
    {
        if (page)
        {
            PageTraits::deallocate(this->m_pageAlloc, page, ELEMENTS_PER_PAGE);
        }
    }

    this->m_activePageCount = 0;
    this->m_pages.clear();
}

template<std::unsigned_integral T, size_t PAGE_BITS, typename Allocator>
inline bool sge::SparsePageTable<T, PAGE_BITS, Allocator>::contains(size_t index) const noexcept
{
    size_t pageIndex = this->getPageIndex(index);

    if (pageIndex >= this->m_pages.size() || !this->m_pages[pageIndex]) return false;

    T* page = this->m_pages[pageIndex];

    size_t offset = this->getElementOffset(index);

    return page[offset] != TOMBSTONE;
}

template<std::unsigned_integral T, size_t PAGE_BITS, typename Allocator>
inline size_t sge::SparsePageTable<T, PAGE_BITS, Allocator>::allocatedPageCount() const noexcept
{
    return this->m_activePageCount;
}

template<std::unsigned_integral T, size_t PAGE_BITS, typename Allocator>
inline size_t sge::SparsePageTable<T, PAGE_BITS, Allocator>::pageCount() const noexcept
{
    return this->m_pages.size();
}

#endif // SGE_SPARSE_PAGE_TABLE_H