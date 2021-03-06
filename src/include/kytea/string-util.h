/*
* Copyright 2009, KyTea Development Team
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
*/

#ifndef STRING_UTIL_H__
#define STRING_UTIL_H__

#include "kytea-struct.h"
#include "kytea-string.h"
#include <iostream>
#include <sstream>
#include <vector>
#include <cstdlib>

namespace kytea {

// a class for turning std::strings into internal representation
class StringUtil {

public:

    // types of characters (set in the constructor)
    typedef char CharType;
    const static CharType KANJI    = 'K';
    const static CharType KATAKANA = 'T';
    const static CharType HIRAGANA = 'H';
    const static CharType ROMAJI   = 'R';
    const static CharType DIGIT    = 'D';
    const static CharType OTHER    = 'O';

    // types of encodings
    typedef char Encoding;
    const static Encoding ENCODING_UTF8    = 'W';
    const static Encoding ENCODING_EUC     = 'E';
    const static Encoding ENCODING_SJIS    = 'S';

public:

    StringUtil() { }

    virtual ~StringUtil() { }

    // map a std::string to a character
    virtual KyteaChar mapChar(const std::string & str, bool add = true) = 0;
    virtual std::string showChar(KyteaChar c) = 0;

    std::string showString(const KyteaString & c) {
        std::ostringstream buff;
        for(unsigned i = 0; i < c.length(); i++)
            buff << showChar(c[i]);
        return buff.str();
    }

    // map an unparsed std::string to a KyteaString
    virtual KyteaString mapString(const std::string & str) = 0;

    // get the type of a character
    virtual CharType findType(const std::string & str) = 0;
    virtual CharType findType(KyteaChar c) = 0;

    // return the encoding provided by this util
    virtual Encoding getEncoding() = 0;
    virtual const char* getEncodingString() = 0;
    
    // transform to or from a character std::string
    virtual void unserialize(const std::string & str) = 0;
    virtual std::string serialize() const = 0;

    // Check that these are equal by serializing them
    void checkEqual(const StringUtil & rhs) const {
        std::string me = serialize();
        std::string you = rhs.serialize();
        if(me != you) {
            THROW_ERROR("String utils don't match" << std::endl 
                        << " --- lhs --- " << std::endl << me << std::endl
                        << " --- rhs --- " << std::endl << you);
        }
    }

    // parse an integer or float
    int parseInt(const char* str) {
        char* endP;
        int ret = strtol(str, &endP, 10);
        if(endP == str)
            THROW_ERROR("Bad integer value '" << str << "'");
        return ret;
    }
    double parseFloat(const char* str) {
        char* endP;
        double ret = strtod(str, &endP);
        if(endP == str)
            THROW_ERROR("Bad floating-point value '" << str << "'");
        return ret;
    }

    // get a std::string of character types
    std::string getTypeString(const KyteaString& str) {
        std::ostringstream buff;
        for(unsigned i = 0; i < str.length(); i++)
            buff << findType(str[i]);
        return buff.str();
    }


};

// a class for parsing UTF8
class StringUtilUtf8 : public StringUtil {

private:
    
    const static char maskr6 = 63, maskr5 = 31, maskr4 = 15, maskr3 = 7,
                      maskl1 = 1 << 7, maskl2 = 3 << 6, maskl3 = 7 << 5, 
                      maskl4 = 15 << 4, maskl5 = 31 << 3;

    // variables
    StringCharMap charIds_;
    std::vector<std::string> charNames_;
    std::vector<CharType> charTypes_;

public:

    StringUtilUtf8() : charIds_(), charNames_(), charTypes_() {
        const char * initial[7] = { "", "K", "T", "H", "R", "D", "O" };
        for(unsigned i = 0; i < 7; i++) {
            charIds_.insert(std::pair<std::string,KyteaChar>(initial[i], i));
            charTypes_.push_back(i==0?6:4); // first is other, rest romaji
            charNames_.push_back(initial[i]);
        }
    }

    ~StringUtilUtf8() { }
    
    // map a std::string to a character
    KyteaChar mapChar(const std::string & str, bool add = true);
    std::string showChar(KyteaChar c);

    CharType findType(KyteaChar c);

    bool badu(char val) { return ((val ^ maskl1) & maskl2); }
    KyteaString mapString(const std::string & str);

    // find the type of a unicode character
    CharType findType(const std::string & str);

    Encoding getEncoding() { return ENCODING_UTF8; }
    const char* getEncodingString() { return "utf8"; }

    const std::vector<std::string> & getCharNames() { return charNames_; }

    // transform to or from a character std::string
    void unserialize(const std::string & str);
    std::string serialize() const;

};

class StringUtilEuc : public StringUtil {

const static char maskl1 = 1 << 7;
const static KyteaChar mask3len = 1 << 14;
    

public:
    StringUtilEuc() { }
    ~StringUtilEuc() { }

    KyteaChar mapChar(const std::string & str, bool add = true);

    std::string showChar(KyteaChar c);
    
    // map an unparsed std::string to a KyteaString
    KyteaString mapString(const std::string & str);

    // get the type of a character
    CharType findType(const std::string & str);
    CharType findType(KyteaChar c);

    // return the encoding provided by this util
    Encoding getEncoding();
    const char* getEncodingString();
    
    // transform to or from a character std::string
    void unserialize(const std::string & str);
    std::string serialize() const;

};

class StringUtilSjis : public StringUtil {

const static char maskl1 = 1 << 7;
const static KyteaChar mask3len = 1 << 14;
    

public:
    StringUtilSjis() { }
    ~StringUtilSjis() { }

    KyteaChar mapChar(const std::string & str, bool add = true);

    std::string showChar(KyteaChar c);
    
    // map an unparsed std::string to a KyteaString
    KyteaString mapString(const std::string & str);

    // get the type of a character
    CharType findType(const std::string & str);
    CharType findType(KyteaChar c);

    // return the encoding provided by this util
    Encoding getEncoding();
    const char* getEncodingString();
    
    // transform to or from a character std::string
    void unserialize(const std::string & str);
    std::string serialize() const;

};



}

#endif
