// Copyright 2025 SiFive
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef DBG_C
#define DBG_C

#include <iostream>
#include <string>
#include <sstream>

class debug_component {
    public:
    debug_component(const std::string& nm) 
        : dbg_name(nm)
        , isDebugEn(false) {}
    ~debug_component() = default;

    void log_debug(const std::string& message, bool print_name = true) {
        if(isDebugEn && print_name) {
            std::cout << "[" << dbg_name << "] " << message << std::endl;
        } else if(isDebugEn) {
            std::cout << message << std::endl;
        }
    }
    
    // Add methods to enable/disable debugging
    void enableDebug(bool enable = true) {
        isDebugEn = enable;
    }
    
    void disableDebug() {
        isDebugEn = false;
    }
    
    template<typename... Args>
    void log_dbg(std::ostringstream&& os) {
        if (isDebugEnabled()) {
            log_debug(os.str());
        }
    }

    template<typename T>
    void log_dbg(T&& msg) {
        std::ostringstream os;
        os << std::forward<T>(msg);
        log_dbg(std::move(os));
    }

#define LOG_DBG(...) \
    do { \
        if (isDebugEnabled()) { \
            std::ostringstream os; \
            os << __VA_ARGS__; \
            log_debug(os.str()); \
        } \
    } while(0)
    
    inline bool isDebugEnabled() const {
        return isDebugEn;
    }
    
    private:
    const std::string dbg_name;
    bool isDebugEn{false}; // Changed from const to allow runtime modification
};


#endif
