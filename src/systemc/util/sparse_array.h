/*******************************************************************************
 * Copyright 2017 MINRES Technologies GmbH
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *******************************************************************************/

#ifndef _SPARSE_ARRAY_H_
#define _SPARSE_ARRAY_H_

#include <array>
#include <cassert>

/**
 * \ingroup scc-common
 */
/**@{*/
//! @brief SCC common utilities
namespace util {

/**
 *  @brief a sparse array suitable for large sizes
 *
 *  a simple array which allocates memory in configurable chunks (size of 2^lower_width), used for
 *  large sparse arrays. Memory is allocated on demand
 */
template <typename T, int upper_width, int lower_width> class sparse_array {
public:
    const uint64_t page_addr_mask = (1 << lower_width) - 1;

    const uint64_t page_size = (1 << lower_width);

    const uint64_t SIZE = (page_size << upper_width);

    const unsigned page_count = 1 << upper_width ; // 1 + SIZE / page_size;

    const uint64_t page_addr_width = lower_width;

    using page_type = std::array<T, 1 << lower_width>;
    /**
     * the default constructor
     */
    sparse_array() { arr.fill(nullptr); }
    /**
     * the destructor
     */
    ~sparse_array() {
        for(auto i : arr)
            delete i;
    }
    /**
     * element access operator
     *
     * @param addr address to access
     * @return the data type reference
     */
    T& operator[](uint64_t addr) {
        assert(addr < SIZE);
        uint64_t nr = addr >> lower_width;
        assert(nr < page_count);
        if(arr[nr] == nullptr) {
            arr[nr] = new page_type();
        }
        return arr[nr]->at(addr & page_addr_mask);
    }
    /**
     * page fetch operator
     *
     * @param page_nr the page number ot fetch
     * @return reference to page
     */
    page_type& operator()(uint32_t page_nr) {
        assert(page_nr < page_count);
        if(arr[page_nr] == nullptr) {
            printf("sparse array is creating new page\n");
            arr.at(page_nr) = new page_type();
        }
        return *(arr[page_nr]);
    }
    /**
     * check if page for address is allocated
     *
     * @param addr the address to check
     * @return true if the page is allocated
     */
    bool is_allocated(uint64_t addr) {
        assert(addr < SIZE);
        uint64_t nr = addr >> lower_width;
        return arr.at(nr) != nullptr;
    }
    /**
     * get the size of the array
     *
     * @return the size
     */
    uint64_t size() { return SIZE; }

    void write(uint64_t addr, const uint8_t *data_ptr, size_t data_len, uint8_t *be_ptr = nullptr, size_t be_len = 0) {
        // fmt::print("\n{:10x}", addr);
        
        assert(addr < SIZE);
        const uint32_t total_page_writes = ((addr & page_addr_mask) + data_len) / page_size + 1;
        uint64_t start_addr = addr;

        for (int i = 0; i < total_page_writes; i++) {
            const uint32_t page_nr = start_addr / page_size;
            assert(page_nr < page_count);
            if (arr.at(page_nr) == nullptr) {
                arr.at(page_nr) = new page_type();
            }
            page_type& page = *(arr.at(page_nr));
            const auto offset = start_addr & page_addr_mask;
            size_t written_len = data_len;
            
            if ((offset + data_len) > page_size) {
                written_len = page_size - offset;
                data_len -= written_len;
                start_addr += written_len;
            }

            auto page_start = page.data() + offset;

            if (be_ptr == nullptr || be_len == 0) { // byte enable not used
                std::copy(data_ptr, data_ptr + written_len, page_start);
            } else {
                for (int i = 0; i < written_len; i++){
                    if(be_ptr[i % be_len] == 0xFF){ // 0x00 and 0xFF are only defined
                        *(page_start+i) = *(data_ptr+i);
                    } 
                }
            }
        }
    }

    void read(uint64_t addr, uint8_t *data_ptr, size_t len) {
        // fmt::print("\n{:10x}", addr);
        assert(addr < SIZE);
        const uint32_t page_nr = addr / page_size;
        assert(page_nr < page_count);
        if (arr.at(page_nr) == nullptr) {
            arr.at(page_nr) = new page_type();
        }
        page_type& page = *(arr.at(page_nr));
        const auto offs = addr & page_addr_mask;
        std::copy(page.data() + offs, page.data() + offs + len, data_ptr);
    }

    char* get_ptr(uint64_t addr) {
        assert(addr < SIZE);
        const uint32_t page_nr = addr / page_size;
        assert(page_nr < page_count);
        if (arr.at(page_nr) == nullptr) {
            arr.at(page_nr) = new page_type();
        }
        return (char*)arr[page_nr] + (addr & page_addr_mask);
    }

protected:
    std::array<page_type*, (1 << upper_width) + 1> arr;
};
} // namespace util
/** @}*/
#endif /* _SPARSE_ARRAY_H_ */
