#ifndef SGE_SPARSE_SET_H
#define SGE_SPARSE_SET_H
#include <cstdint>
#include <numeric>
#include <vector>
#include <stdexcept>
#include <type_traits>
#include <TEST/containers/SparsePageTable.h>

namespace sge {
using size_type = size_t;

template <typename T>
concept unsigned_integral = std::unsigned_integral<T>;

template <typename T>
concept unsigned_enum = std::is_enum_v<T> && std::is_unsigned_v<std::underlying_type_t<T>>;

template <typename T>
concept unsigned_int_or_enum = unsigned_integral<T> || unsigned_enum<T>;

template <unsigned_int_or_enum KeyType>
struct DefaultIndexExtractor {
    size_type operator()(const KeyType &key) const
    {
        return static_cast<size_type>(key);
    }
}; // DefaultIndexExtractor

template <unsigned_int_or_enum KeyType = uint32_t, typename IndexExtractor = DefaultIndexExtractor<KeyType>>
class SparseSet {
private:
    SparsePageTable<size_type>  m_sparse;
    std::vector<KeyType>        m_dense;
    IndexExtractor              m_extractor{};

public:
    SparseSet() = default;

    /**
     * Checks if a key is present.
     * @param key the key to check
     * @return true if it contains the key, false otherwise
     */
    bool contains(KeyType key) const;

    /**
     * Accessor into the SparseSet.
     * @param key the key to query
     * @return the mapped dense index of the key
     */
    size_type index(KeyType key) const;

    /**
     * @param denseIndex the dense index to access
     * @return the Key value stored at the dense index.
     */
    KeyType getKeyAt(size_type denseIndex) const;

    /**
     * Inserts a key into the SparseSet.
     * @param key the key to insert
     * @return the mapped dense index of the key
     */
    size_type insert(KeyType key);

    /**
     * Removes a key from the SparseSet.
     * @param key the key to remove
     */
    template <typename SwapFunc>
    void erase(KeyType key, SwapFunc&& swapCallback);

    /**
     * Swaps the placement of two indices in the dense storage.
     * @param idxA the first index
     * @param idxB the second storage
     */
    void swapIndices(size_type idxA, size_type idxB);

    /**
     * Swaps the placement of two keys.
     * @param keyA the first key
     * @param keyB the second key
     */
    void swapKeys(KeyType keyA, KeyType keyB);

    /**
     * Clears the SparseSet.
     */
    void clear();

    /**
     * @return the total amount of registered keys
     */
    size_type size() const;

    /**
     * @return the dense keys stored in the set
     */
    const std::vector<KeyType>& getKeys() const;

}; // class SparseSet
} // namespace sge

// Implementation
template <sge::unsigned_int_or_enum KeyType, typename IndexExtractor>
inline bool sge::SparseSet<KeyType, IndexExtractor>::contains(KeyType key) const
{
    const size_type sparseIndex = this->m_extractor(key);
    
    if (!this->m_sparse.contains(sparseIndex)) return false;
    
    const size_type denseIndex = this->m_sparse.get(sparseIndex);
    
    return (denseIndex < this->m_dense.size() && this->m_dense[denseIndex] == key);
}

template <sge::unsigned_int_or_enum KeyType, typename IndexExtractor>
inline sge::size_type sge::SparseSet<KeyType, IndexExtractor>::index(KeyType key) const
{
    assert(this->contains(key) && "[SparseSet]::index | Error. Key does not exist in this set.");
    
    return this->m_sparse.get(this->m_extractor(key));
}

template <sge::unsigned_int_or_enum KeyType, typename IndexExtractor>
inline KeyType sge::SparseSet<KeyType, IndexExtractor>::getKeyAt(size_type denseIndex) const
{
    assert(denseIndex < this->m_dense.size() && "[SparseSet]::getKeyAt(size_type denseIndex) | Error. denseIndex out-of-bounds.");

    return this->m_dense[denseIndex];
}

template <sge::unsigned_int_or_enum KeyType, typename IndexExtractor>
inline sge::size_type sge::SparseSet<KeyType, IndexExtractor>::insert(KeyType key)
{
    size_type sparseIndex = this->m_extractor(key);

    if (this->contains(key))
    {
        return this->m_sparse.get(sparseIndex);
    }
    
    size_type denseIndex = this->m_dense.size();
    this->m_sparse.set(sparseIndex, denseIndex);
    this->m_dense.push_back(key);
    
    return denseIndex;
}

template <sge::unsigned_int_or_enum KeyType, typename IndexExtractor>
template <typename SwapFunc>
inline void sge::SparseSet<KeyType, IndexExtractor>::erase(KeyType key, SwapFunc&& swapCallback)
{
    if (!this->contains(key)) return;
    
    const size_type indexToRemove = this->m_sparse.get(this->m_extractor(key));
    const size_type lastIndex = this->m_dense.size() - 1;

    if (indexToRemove != lastIndex)
    {
        KeyType lastKey = this->m_dense[lastIndex];
        swapCallback(indexToRemove, lastIndex);
        this->m_dense[indexToRemove] = lastKey;
        this->m_sparse.set(this->m_extractor(lastKey), indexToRemove);
    }

    this->m_sparse.erase(indexToRemove);
    this->m_dense.pop_back();
}

template <sge::unsigned_int_or_enum KeyType, typename IndexExtractor>
inline void sge::SparseSet<KeyType, IndexExtractor>::swapIndices(size_type idxA, size_type idxB)
{
    if (idxA == idxB) return;
    
    assert(idxA < this->m_dense.size() && "[SparseSet]::swapIndices(size_type idxA, size_type idxB) | Error. Index A out-of-bounds.");
    assert(idxB < this->m_dense.size() && "[SparseSet]::swapIndices(size_type idxA, size_type idxB) | Error. Index B out-of-bounds.");
    
    KeyType keyA = this->m_dense[idxA];
    KeyType keyB = this->m_dense[idxB];
    
    std::swap(this->m_dense[idxA], this->m_dense[idxB]);
    
    this->m_sparse.set(this->m_extractor(keyA), idxB);
    this->m_sparse.set(this->m_extractor(keyB), idxA);
}

template <sge::unsigned_int_or_enum KeyType, typename IndexExtractor>
inline void sge::SparseSet<KeyType, IndexExtractor>::swapKeys(KeyType keyA, KeyType keyB)
{
    if (keyA == keyB) return;
    
    this->swapIndices(this->m_sparse.get(this->m_extractor(keyA)), this->m_sparse.get(this->m_extractor(keyB)));
}

template <sge::unsigned_int_or_enum KeyType, typename IndexExtractor>
inline void sge::SparseSet<KeyType, IndexExtractor>::clear()
{
    this->m_dense.clear();
    this->m_sparse.clear();
}

template <sge::unsigned_int_or_enum KeyType, typename IndexExtractor>
inline sge::size_type sge::SparseSet<KeyType, IndexExtractor>::size() const
{
    return this->m_dense.size();
}

template <sge::unsigned_int_or_enum KeyType, typename IndexExtractor>
inline const std::vector<KeyType>& sge::SparseSet<KeyType, IndexExtractor>::getKeys() const
{
    return this->m_dense;
}

#endif // SGE_SPARSE_SET_H