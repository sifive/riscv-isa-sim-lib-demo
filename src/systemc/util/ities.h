/*******************************************************************************
 * Copyright 2017-2022 MINRES Technologies GmbH
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

#ifndef _UTIL_ITIES_H_
#define _UTIL_ITIES_H_

#include <algorithm>
#include <array>
#include <assert.h>
#include <bitset>
#include <cctype>
#include <iterator>
#include <memory>
#include <sstream>
#include <sys/stat.h>
#include <type_traits>
#include <vector>

/**
 * \ingroup scc-common
 */
/**@{*/
// some helper functions
/**
 * @brief extract bit ranges from plain data types
 *
 * @tparam bit start bit
 * @tparam width size of the bit field to extract
 * @tparam T data type carrying the bits
 * @param v value from which the bytes are to be extracted
 * @return the extracted bit. It is of the same data type as the passed value
 */
template <unsigned int bit, unsigned int width, typename T>
inline constexpr typename std::enable_if<std::is_unsigned<T>::value, T>::type bit_sub(T v) {
    static_assert((bit+width)<=8*sizeof(T), "out of range access");
    T res = (v >> bit) & ((T(1) << width) - 1);
    return res;
}

template <unsigned int bit, unsigned int width, typename T>
inline constexpr typename std::enable_if<std::is_signed<T>::value, T>::type bit_sub(T v) {
    static_assert((bit+width)<=8*sizeof(T), "out of range access");
    static_assert(width>0, "negative width");
    auto field = v>>bit;
    auto amount = (field & ~(~T(1) << (width - 1) << 1)) -
                  (field & (T(1) << (width - 1)) << 1);
    return amount;
}
/**
 * @brief sign-extend a given value
 *
 * @tparam T the datatype
 * @tparam B the numer of signed bits
 * @param x the actualö value
 * @return the sign-extended value of type T
 */
#if __cplusplus < 201402L
template <typename T, unsigned B> inline T signextend(const T x) {
#else
template <typename T, unsigned B> inline constexpr T signextend(const typename std::make_unsigned<T>::type x) {
#endif
    struct X {
        T x : B;
        X(T x_)
        : x(x_) {}
    } s(x);
    return s.x;
}

// according to http://graphics.stanford.edu/~seander/bithacks.html#FixedSignExtend
/**
 * @brief a function that converts from B bits to T in one operation
 *
 * @tparam bit
 * @tparam width
 * @tparam T
 * @param v
 * @return
 */
template <unsigned int bit, unsigned int width, typename T>
inline constexpr typename std::make_signed<T>::type signed_bit_sub(T v) {
#if __cplusplus < 201402L
    return ((v << (sizeof(T) * 8 - bit - width)) >> (sizeof(T) * 8 - width));
#else
    typename std::make_signed<T>::type r = v << (sizeof(T) * 8 - bit - width);
    typename std::make_signed<T>::type ret = (r >> (sizeof(T) * 8 - width));
    return ret;
#endif
}

//! @brief SCC common utilities
namespace util {
template <typename T, typename... Args> std::unique_ptr<T> make_unique(Args&&... args) {
#if __cplusplus < 201402L
    return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
#else
    return std::make_unique<T>(std::forward<Args>(args)...);
#endif
}

// according to
// http://graphics.stanford.edu/~seander/bithacks.html#ZerosOnRightMultLookup
static std::array<const int, 32> MultiplyDeBruijnBitPosition = {{0,  1,  28, 2,  29, 14, 24, 3,  30, 22, 20,
                                                                 15, 25, 17, 4,  8,  31, 27, 13, 23, 21, 19,
                                                                 16, 7,  26, 12, 18, 6,  11, 5,  10, 9}};
template <size_t N> constexpr size_t find_first(std::bitset<N>& bits) {
    static_assert(N <= 32, "find_first only supports bitsets smaller than 33");
    return MultiplyDeBruijnBitPosition[static_cast<uint32_t>((bits.to_ulong() & -bits.to_ulong()) * 0x077CB531U) >> 27];
}
template <typename T> T leftmost_one(T n) {
    for(T mask = 1; mask < sizeof(T) * 8; mask <<= 1)
        n |= (n >> mask);
    return n - (n >> 1);
}

// according to
// https://stackoverflow.com/questions/8871204/count-number-of-1s-in-binary-representation
#if defined(__GNUG__)
constexpr inline size_t bit_count(uint32_t u) { return __builtin_popcount(u); }
constexpr inline size_t bit_count(uint64_t u) { return __builtin_popcountl(u); }
#elif __cplusplus < 201402L
constexpr size_t uCount(uint32_t u) { return u - ((u >> 1) & 033333333333) - ((u >> 2) & 011111111111); }
constexpr size_t bit_count(uint32_t u) { return ((uCount(u) + (uCount(u) >> 3)) & 030707070707) % 63; }
#else
constexpr size_t bit_count(uint32_t u) {
    size_t uCount = u - ((u >> 1) & 033333333333) - ((u >> 2) & 011111111111);
    return ((uCount + (uCount >> 3)) & 030707070707) % 63;
}
#endif

/**
 * get the log2 value fo an integer
 *
 * @param val the value
 * @return the number of bit needed to hold the value val
 */
#if __cplusplus < 201402L
inline unsigned ilog2(uint32_t val) {
#else
inline constexpr unsigned ilog2(uint32_t val) {
#endif
#ifdef __GNUG__
    return sizeof(uint32_t) * 8 - 1 - __builtin_clz(static_cast<unsigned>(val));
#else

    if(val == 0)
        return std::numeric_limits<uint32_t>::max();
    if(val == 1)
        return 0;
    auto ret = 0U;
    while(val > 1) {
        val >>= 1;
        ++ret;
    }
    return ret;
#endif
} // namespace util

/**
 * get the log2 value fo an first bigger pow od two integer
 *
 * @param val the value
 * @return the number of bit needed to hold the next (or equal) power of two val
 */
#if __cplusplus < 201402L
inline unsigned ilog2plus(uint32_t val) {
#else
inline constexpr unsigned ilog2plus(uint32_t val) {
#endif
    unsigned pos = ilog2(val);
    if ((val & (val - 1)) != 0) {
        pos++; // Increment pos if val is not a power of 2
    }
    return pos;
} // namespace util

constexpr bool hasOddParity(uint32_t u) { return bit_count(u) % 2; }
/**
 * split a given string using specified separator
 *
 * @param s the string to split
 * @param separator the separator char
 * @return vector of splitted strings
 */
inline std::vector<std::string> split(const std::string& s, char separator) {
    std::vector<std::string> output;
    std::string::size_type prev_pos = 0;
    std::string::size_type pos = 0;
    while((pos = s.find(separator, pos)) != std::string::npos) {
        std::string substring(s.substr(prev_pos, pos - prev_pos));
        output.push_back(substring);
        prev_pos = ++pos;
    }
    output.push_back(s.substr(prev_pos, pos - prev_pos)); // Last word
    return output;
    /* could also be done similar to this
        // construct a stream from the string
        std::stringstream ss(str);
        // use stream iterators to copy the stream to the vector as whitespace separated strings
        std::istream_iterator<std::string> it(ss);
        std::istream_iterator<std::string> end;
        std::vector<std::string> results(it, end);
        return results;
    */
}

/*! note: delimiter cannot contain NUL characters
 */
template <typename Range, typename Value = typename Range::value_type>
std::string join(Range const& elements, char const* const delimiter) {
    std::ostringstream os;
    auto b = std::begin(elements);
    auto e = std::end(elements);
    if(b != e) {
        std::copy(b, std::prev(e), std::ostream_iterator<Value>(os, delimiter));
        b = std::prev(e);
    }
    if(b != e)
        os << *b;
    return os.str();
}

/*! note: imput is assumed to not contain NUL characters
 */
template <typename Input, typename Output, typename Value = typename Output::value_type>
void split(char delimiter, Output& output, Input const& input) {
    for(auto cur = std::begin(input), beg = cur;; ++cur) {
        if(cur == std::end(input) || *cur == delimiter || !*cur) {
            output.insert(std::end(output), Value(beg, cur));
            if(cur == std::end(input) || !*cur)
                break;
            beg = std::next(cur);
        }
    }
}
/**
 * trim the left side of a given string
 * @param str the string to trim
 * @param chars set of chars to trim away
 * @return
 */
inline std::string& ltrim(std::string& str, const std::string& chars = "\t\n\v\f\r ") {
    str.erase(0, str.find_first_not_of(chars));
    return str;
}
/**
 * trim the right side of a given string
 * @param str the string to trim
 * @param chars set of chars to trim away
 * @return
 */
inline std::string& rtrim(std::string& str, const std::string& chars = "\t\n\v\f\r ") {
    str.erase(str.find_last_not_of(chars) + 1);
    return str;
}
/**
 * trim the both sides of a given string
 * @param str the string to trim
 * @param chars set of chars to trim away
 * @return
 */
inline std::string& trim(std::string& str, const std::string& chars = "\t\n\v\f\r ") {
    return ltrim(rtrim(str, chars), chars);
}
/**
 * convert string to lower case
 * @param str the string to convert
 * @return
 */
inline std::string str_tolower(std::string str) {
    std::transform(str.begin(), str.end(), str.begin(), [](unsigned char c) { return std::tolower(c); });
    return str;
}
/**
 * convert string to upper case
 * @param str the string to convert
 * @return
 */
inline std::string str_toupper(std::string str) {
    std::transform(str.begin(), str.end(), str.begin(), [](unsigned char c) { return std::toupper(c); });
    return str;
}
/**
 * @fn bool iequals(const std::string&, const std::string&)
 * @brief compare two string ignoring case
 *
 * @param a string a to compare
 * @param b string b to compare
 * @result true if the are equal otherwise false
 */
inline bool iequals(const std::string& a, const std::string& b) {
#if __cplusplus < 201402L
    auto sz = a.size();
    if(b.size() != sz)
        return false;
    for(auto i = 0U; i < sz; ++i)
        if(tolower(static_cast<unsigned>(a[i])) != tolower(static_cast<unsigned>(b[i])))
            return false;
    return true;
#else
    return std::equal(a.begin(), a.end(), b.begin(), b.end(),
                      [](unsigned char a, unsigned char b) { return tolower(a) == tolower(b); });
#endif
}

inline bool ends_with(std::string const & value, std::string const & ending){
//    if (ending.size() > value.size()) return false;
//    return std::equal(ending.rbegin(), ending.rend(), value.rbegin());
    return value.length() >= ending.length()?
    		!value.compare (value.length() - ending.length(), ending.length(), ending):
			false;
}
/**
 * @fn std::string padded(std::string, size_t, bool=true)
 * @brief pad a string to a given length by either cutting of the overflow or inserting an ellipsis
 *
 * @param str string to adjust
 * @param width of the targeted field
 * @param show_ellipsis use ellipsis (...) when shortening
 * @return string with the given length
 */
inline std::string padded(std::string str, size_t width, bool show_ellipsis = true) {
    if(width < 7)
        return str;
    if(str.length() > width) {
        if(show_ellipsis) {
            auto pos = str.size() - (width - 6);
            return str.substr(0, 3) + "..." + str.substr(pos, str.size() - pos);
        } else
            return str.substr(0, width);
    } else {
        return str + std::string(width - str.size(), ' ');
    }
}
/**
 * checks if a file exists
 * @param name the file name
 * @return true if file exists and can be opened
 */
inline bool file_exists(const std::string& name) {
    struct stat buffer {};
    return (stat(name.c_str(), &buffer) == 0);
}
/**
 * return directory name portion of a given path (as string)
 * @param path
 * @param delims the path delimiters to use
 * @return
 */
template <class T> inline T dir_name(T const& path, T const& delims = "/\\") {
    auto pos = path.find_last_of(delims);
    return pos > path.length() ? "." : path.substr(0, pos);
}
/**
 * return file name portion of a given path (as string)
 * @param path
 * @param delims the path delimiters to use
 * @return
 */
template <class T> inline T base_name(T const& path, T const& delims = "/\\") {
    auto pos = path.find_last_of(delims);
    return pos > path.length() ? path : path.substr(pos + 1);
}
/**
 * return the base name (without extension) of a file name (as string)
 * @param filename
 * @return
 */
template <class T> inline T remove_ext(T const& filename) {
    typename T::size_type const p(filename.find_last_of('.'));
    return p > 0 && p != T::npos ? filename.substr(0, p) : filename;
}
/**
 * converts a globbing string into a regular expression
 *
 * The globbing supports ?,*,**, and character classes ([a-z] as well as [!a-z]). '.' acts as
 * hierarchy delimiter and is only matched with **
 * Regular expression must start with a carret ('^') so that it can be identified as regex.
 *
 * @param filename
 * @return
 */
inline
std::string  glob_to_regex(std::string val) {
	const struct  {
#ifdef MTI_SYSTEMC
        const char *question_mark="[^/]";
        const char *star= "[^/]*";
#else
        const char *question_mark="[^.]";
		const char *star= "[^.]*";
#endif
		const char *double_star = ".*";
	} subst_table;
	auto is_regex_meta = [](char c)->bool {
		switch(c) {
		default: return false;
		case '.':
		case '(':
		case ')':
		case '{':
		case '}':
		case '+':
		case '^':
		case '$':
		case '|':
			return true;
		}
	};
	util::trim(val);
	std::ostringstream oss;
	oss<<"^";
	bool in_character_class = false, in_quote = false;
	for (auto idx=0U; idx<val.length(); ++idx) {
		auto c = val[idx];
		if (in_character_class) {
			in_character_class = ((c != ']') || (val[idx-1] == '\\'));
			oss << c;
			continue;
		}
		if (in_quote) {
			in_quote = false;
			oss << c;
			continue;
		}
		if (c == '\\') {
			in_quote = true;
			oss << c;
			continue;
		} else if (c == '[') {
			oss << c;
			in_character_class = true;
			if (val[idx+1] == '!') {
				oss << '^';
				idx++;
			}
		} else if (is_regex_meta(c)) {
			oss << '\\' << c;
		} else if (c == '?') {
			oss << subst_table.question_mark;
		} else if (c == '*') {
			if ((idx+1)<val.length() && val[idx+1] == '*') {
				idx++;
				oss << subst_table.double_star;
			} else
				oss << subst_table.star;
		} else {
			oss << c;
		}
	}
	oss<<"$";
	return oss.str();
}

} // namespace util
/** @} */
#endif /* _UTIL_ITIES_H_ */
