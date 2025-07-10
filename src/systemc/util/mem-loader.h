#include <util/sparse_array.h>
#include <util/dbg_component.h>
#include <queue>
#include <memory>
#include <util/elfloader.h>

class MemoryLoader{
    private:
      util::sparse_array<uint8_t, 18, 30>* mem_ptr; // pointer to memory array
  
    public:
    MemoryLoader() = delete;
    MemoryLoader(util::sparse_array<uint8_t, 18, 30>* m_ptr)
    : mem_ptr(m_ptr)
    {
  
    }

    uint64_t loadElf(std::string elfFName) {
      uint64_t start_address;
      if (FILE *file = fopen(elfFName.c_str(), "r")) {
        fclose(file);
        auto x = load_elf(elfFName.c_str(), mem_ptr, &start_address); // TODO: fix this
        std::cout << "start address of elf " << elfFName << " is : 0x" << hex << start_address << std::endl;
      } else {
        std::cerr << "File name " << elfFName << " cannot be read " << std::endl;
        std::cerr << "Simulation is aborted\n" << std::endl;
        assert(0);
      }
  
      return start_address;
    }
  
    void loadVector(std::vector<uint32_t>& vec, uint64_t start_addr) {
        std::cout << "Loading vector of size " << vec.size() << " at address 0x" << hex << start_addr << std::endl;
      uint8_t* tmp_mem = new uint8_t[vec.size() * sizeof(uint32_t)];
      for(int i = 0; i < vec.size(); i++) {
        memcpy(tmp_mem + i * sizeof(uint32_t), &vec[i], sizeof(uint32_t));
      }
      mem_ptr->write(start_addr, tmp_mem, vec.size() * sizeof(uint32_t));
      // mem_ptr->write(start_addr, vec.data(), vec.size() * sizeof(uint32_t));
    }
  };