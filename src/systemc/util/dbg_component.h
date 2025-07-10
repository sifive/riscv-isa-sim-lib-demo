
// Copyright (c) 2023 SiFive, Inc. -- Proprietary and Confidential
// All Rights Reserved.
//
// NOTICE: All information contained herein is, and remains the
// property of SiFive, Inc. The intellectual and technical concepts
// contained herein are proprietary to SiFive, Inc. and may be covered
// by U.S. and Foreign Patents, patents in process, and are protected by
// trade secret or copyright law.
//
// This work may not be copied, modified, re-published, uploaded,
// executed, or distributed in any way, in any medium, whether in whole
// or in part, without prior written permission from SiFive, Inc.
//
// The copyright notice above does not evidence any actual or intended
// publication or disclosure of this source code, which includes
// information that is confidential and/or proprietary, and is a trade
// secret, of SiFive, Inc.

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
