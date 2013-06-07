#pragma once
#include <cmath>
#include <string>
#include <vector>
#include <algorithm>
#include <locale>
#include <functional>

/// Summary
///   Rounds floating point numbers to nearest integer, where ties round away from zero
///   For example, 23.5 gets rounded to 24, and -23.5 gets rounded to -24.
///   This method also treats positive and negative values symmetrically,
///   and therefore is free of overall bias if the original numbers are positive or negative with equal probability.
///   However, this rule will still introduce a positive bias for positive numbers, and a negative bias for the negative ones.
/// Argument
///   value - the value to be converted
/// Template Argument
///   FloatType - The floating point data type
/// Returns
///   The converted value
template<typename FloatType>
inline FloatType round_to_nearest_awayzero(FloatType value )
{
    using std::ceil;
    using std::floor;

    return (value < static_cast<FloatType>(0.0)) ? 
        ceil( value - static_cast<FloatType>(0.5) ) :
        floor( value + static_cast<FloatType>(0.5) );
}


/// Summary
///   Apply a filter before a transformation
/// Parameters
///   first, last
///      Input iterators to the initial and final positions of the sequence.
///      The range used is [first,last), which contains all the elements between first and last,
///      including the element pointed by first but not the element pointed by last.
///   result
///      Output iterator to the initial position of the range where function results are stored.
///      The range includes as many elements as [first,last).
///   op
///      Unary function taking one element as argument, and returning some result value.
///      This can either be a pointer to a function or an object whose class overloads operator().
///   pred
///      Unary predicate taking an element in the range as argument, and returning a value
///      indicating the falsehood (with false, or a zero value) or truth (true, or non-zero) of some condition applied to it.
///      This can either be a pointer to a function or an object whose class overloads operator().
/// Returns
///      An iterator pointing to the element that follows the last element written in the result sequence.
template <class InputIterator, class OutputIterator, class UnaryOperation, class UnaryPredicate>
inline OutputIterator transform_if(InputIterator first, InputIterator last,  OutputIterator result, UnaryOperation op, UnaryPredicate pred)
{
   while (first != last)
   {
      if (pred(*first))
            *result++ = op(*first);
      ++first;
   }
   return result;
}

/// Summary
///   Inserts an item between each pair of elements from the source sequence
///   and places the result into another sequence.
/// Parameters
///   first, last
///      Input iterators to the initial and final positions of the source sequence.
///      The range used is [first,last), which contains all the elements between first and last,
///      including the element pointed by first but not the element pointed by last.
///   result
///      Output iterator to the initial position in the destination sequence.
///      This shall not point to any element in the range [first,last).
///   insert
///      The value to insert between each pair of elements
/// Returns
///      An iterator to the end of the destination range (which points to the element following the copy of last).
template<typename InputIterator, typename InsertType, typename OutputIterator>
inline OutputIterator interlace_with(InputIterator first, InputIterator last, OutputIterator result, const InsertType& insert)
{
   if (first != last)
      *result = *first;
   if ((++first) == last)
      ++result;
   while (first != last)
   {
      *(++result) = insert;
      *(++result) = *first;
      ++first;
   }
   return result;
}


template<typename StringType>
inline bool AreEqualIgnoreCase(const StringType& first, const StringType& second, const std::locale& loc = std::locale::classic())
{
    typedef typename StringType::const_iterator iter_type;
    if(first.size() != second.size())
        return false;
    iter_type ixFirst = first.begin();
    iter_type firstEnd = first.end();
    iter_type ixSecond = second.begin();
    while(ixFirst != firstEnd)
    {
        if (std::tolower(*ixFirst, loc) != std::tolower(*ixSecond, loc))
            return false;
        ++ixFirst; ++ixSecond;
    }
    return true;
}

/// Summary:
///   Splits a string into one or more strings that are created from dividing the
///   source string into sections separated by a section divider character. 
/// Returns:
///   A vector of strings for each section without the section divider. The order of 
///   the strings in the vector is the same as the order in the source string.
template<typename StringType>
inline std::vector<StringType> Explode(const StringType& src, typename StringType::value_type sectionDivider)
{
    std::vector<StringType> sections;
    for(typename StringType::const_iterator sectionStart = src.begin(); sectionStart != src.end(); )
    {
        typename StringType::const_iterator sectionEnd = std::find(sectionStart, src.end(), sectionDivider);
        sections.push_back(std::string(sectionStart, sectionEnd));
        sectionStart = (sectionEnd != src.end()) ? ++sectionEnd : src.end();
    }
    return sections;
}

template <typename InputIterator, typename JoinType, typename StringType = InputIterator::value_type>
inline StringType JoinWith(InputIterator first, InputIterator last, const JoinType& joinWith)
{
    StringType output;
    while(first != last)
    {
        output.append(*(first++));
        if (first != last)
            output.append(joinWith).append(*(first++));
    }
    return output;
}


inline void TrimLeft(std::string& toTrim)
{
    toTrim.erase(toTrim.begin(), std::find_if_not(toTrim.begin(), toTrim.end(), [](char c) { return std::isspace(c, std::locale::classic());}));
}

inline void TrimRight(std::string& toTrim)
{
    std::string::iterator last = std::find_if_not(toTrim.rbegin(), toTrim.rend(), [](char c) { return std::isspace(c, std::locale::classic());}).base();
    toTrim.erase(last, toTrim.end());
}

inline void Trim(std::string& toTrim)
{
    TrimRight(toTrim);
    TrimLeft(toTrim);
}

inline std::string TrimLeftCopy(std::string toTrim)
{
    TrimLeft(toTrim);
    return toTrim;
}

inline std::string TrimRightCopy(std::string toTrim)
{
    TrimRight(toTrim);
    return toTrim;
}

inline std::string TrimCopy(std::string toTrim)
{
    TrimRight(toTrim);
    TrimLeft(toTrim);
    return toTrim;
}



inline int AlphaToInt(const char *pszValue)
{
   int retVal = 0;
   int mult = 1;
   size_t remainingChars = strlen(pszValue);
   const char *pcChar = pszValue + remainingChars - 1;
   while (remainingChars)
   {
      retVal += ((((*pcChar & 0xdf) - 'A') + 1) * mult);
      if (retVal < 0)
          throw std::overflow_error("The alpha encoded value cannot be converted to an integer");
      mult *= 26;
      pcChar --;
      remainingChars --;
   }
   return(retVal);
}

inline int AlphaToInt(const std::string& value)
{
    return AlphaToInt(value.c_str());
}

// Positive non-zero numbers only. A value less than or equal to zero will return an empty string.
inline std::string IntToAlpha(int value)
{
    std::string result;
    if (value <= 0)
        return result;
    int remainder = value % 26;
    if (remainder)
    {
        result.push_back( (char)(('A' - 1) + remainder) );
        value -= remainder;
    }
    else
    {
        result.push_back('Z');
        value -= 26;
    }
    int divisor = 26;
    while (value)
    {
        remainder = (value / divisor) % 26;
        result.push_back( (char)(('A' - 1) + remainder) );
        value -= (remainder * divisor);
        divisor *= 26;
    }
    std::reverse(result.begin(), result.end());
    return result;
}


// Converts a string into a string that will be interpreted as a string literal by the regular expression library.
inline std::string MakeRegExLiteral(const std::string& str)
{
    const char controlChars[] = {'^', '$', '\\', '.', '*', '+', '?', '(', ')', '[', ']', '{', '}', '|'};
    std::string result;
    result.reserve(str.length());
    for(auto ch : str)
    {
        if (std::find(std::begin(controlChars), std::end(controlChars), ch) != std::end(controlChars))
            result.push_back('\\');
        result.push_back(ch);
    }
    return result;
}


/// Summary:
/// Will encode the source string into a new string that can safely
/// be used in a URL without changing its meaning.
/// Any character that could change the meaning of a URL will be hex encoded.
/// For additional information see RFC3986
/// http://www.ietf.org/rfc/rfc3986.txt
/// Returns:
/// A new string that contains the encoded value
inline std::string UrlSafeEncode(const char* string)
{
    static const char * digits = "0123456789ABCDEF";
    std::string encoded;
    while(*string)
    {
        if (isalpha(*string) || isdigit(*string)
            || *string == '.' || *string == '-' || *string == '_' || *string == '!' || *string == '*'
            || *string == '(' || *string == ')' || *string == '\'')
        {
            encoded.push_back(*string);
        }
        else
        {
            encoded.push_back('%');
            encoded.push_back(digits[(*string >> 4) & 0xF]);
            encoded.push_back(digits[*string & 0xF]);
        }
        ++string;
    }
    return encoded;
}

inline std::string UrlSafeEncode(const std::string& string)
{
    return UrlSafeEncode(string.c_str());
}