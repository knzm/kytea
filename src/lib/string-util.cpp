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

#include <kytea/string-util.h>
#include <iostream>

using namespace kytea;
using namespace std;

// map a string to a character
KyteaChar StringUtilUtf8::mapChar(const string & str, bool add) {
    StringCharMap::iterator it = charIds_.find(str);
    KyteaChar ret = 0;
    if(it != charIds_.end())
        ret = it->second;
    else if (add) {
        ret = charTypes_.size();
        charIds_.insert(pair<string, KyteaChar>(str,ret));
        charTypes_.push_back(findType(str));
        charNames_.push_back(str);
    }
    return ret;
}

string StringUtilUtf8::showChar(KyteaChar c) {
#ifdef KYTEA_SAFE
    if(c >= charNames_.size())
        THROW_ERROR("FATAL: Index out of bounds in showChar");
#endif 
    return charNames_[c];
}

StringUtil::CharType StringUtilUtf8::findType(KyteaChar c) {
    return charTypes_[c];
}

KyteaString StringUtilUtf8::mapString(const string & str) {
    unsigned pos = 0, len = str.length();
    vector<KyteaChar> ret;
    while(pos < len) {
        // single character unicode values
        if(!(maskl1 & str[pos]))
            ret.push_back(mapChar(str.substr(pos++, 1)));
        else if((maskl5 & str[pos]) == maskl5) {
            THROW_ERROR("Expected UTF8 file but found non-UTF8 string (specify the proper encoding with -encode utf8/euc/sjis): "<<str);
        }
        else if((maskl4 & str[pos]) == maskl4) {
            if(pos + 3 >= len || badu(str[pos+1]) || badu(str[pos+2]) || badu(str[pos+3]))
                THROW_ERROR("Expected UTF8 file but found non-UTF8 string (specify the proper encoding with -encode utf8/euc/sjis): "<<str);
            ret.push_back(mapChar(str.substr(pos, 4)));
            pos += 4;
        }
        else if((maskl3 & str[pos]) == maskl3) {
            if(pos + 2 >= len || badu(str[pos+1]) || badu(str[pos+2]))
                THROW_ERROR("Expected UTF8 file but found non-UTF8 string (specify the proper encoding with -encode utf8/euc/sjis): "<<str);
            ret.push_back(mapChar(str.substr(pos, 3)));
            pos += 3;
        }
        else {
            if(pos + 1 >= len || badu(str[pos+1]))
                THROW_ERROR("Expected UTF8 file but found non-UTF8 string (specify the proper encoding with -encode utf8/euc/sjis): "<<str);
            ret.push_back(mapChar(str.substr(pos, 2)));
            pos += 2;
        }
    }
    KyteaString retstr(ret.size());
    for(unsigned i = 0; i < ret.size(); i++)
        retstr[i] = ret[i];
    return retstr;
}

// find the type of a unicode character
StringUtil::CharType StringUtilUtf8::findType(const string & str) {
    // find the type of a unicode character
    if(str.length() == 0)
        return OTHER;
    if(str.length()>4)
        THROW_ERROR("Malformed utf8 character in findType");
    // parse into unicode integer values
    unsigned val = 0;
    if(str.length() == 1) val = str[0];
    else if(str.length() == 2) val = ((str[0]&maskr5)<<6) | (maskr6&str[1]);
    else if(str.length() == 3) val = ((str[0]&maskr4)<<12) | ((maskr6&str[1])<<6) | (maskr6&str[2]);
    else val = ((str[0]&maskr3)<<18) | ((maskr6&str[1])<<12) | ((maskr6&str[2])<<18) | (maskr6&str[3]);
    
    // Basic latin uppercase, basic latin lowercase
    // Full width uppercase, full width lowercase
    if((val >= 0x41 && val <= 0x5A) || (val >= 0x61 && val <= 0x7A)
        || (val >= 0xFF21 && val <= 0xFF3A) || (val >= 0xFF41 && val <= 0xFF5A)) {
        return ROMAJI;
    }
    // hiragana (exclude repetition characters)
    else if((val >= 0x3040 && val <= 0x3096)) {
        return HIRAGANA;
    }
    // full width (exclude center dot), half width
    else if((val >= 0x30A0 && val <= 0x30FF && val != 0x30FB) || (val >= 0xFF66 && val <= 0xFF9F)) {
        return KATAKANA;
    }
    // basic latin digits
    else if((val >= 0x30 && val <= 0x39) || (val >= 0xFF10 && val <= 0xFF19)) {
        return DIGIT;
    }
    // CJK Unified Ideographs
    else if((val >= 0x4E00 && val <= 0x9FFF) || (val >= 0xEFA480 && val <= 0xEFAB99)) {
        return KANJI;
    }
    return OTHER;
}


void StringUtilUtf8::unserialize(const string & str) {
    charIds_.clear(); charNames_.clear(); charTypes_.clear();
    mapChar("");
    KyteaString ret = mapString(str);
}

string StringUtilUtf8::serialize() const {
    ostringstream buff;
    for(unsigned i = 1; i < charNames_.size(); i++)
        buff << charNames_[i];
    return buff.str();
}

inline KyteaChar eucm(char a, char b) {
    KyteaChar ret = a & 0xFF;
    ret = ret << 8;
    ret = ret | (b&0xFF);
    return ret;
}
inline unsigned char euc1(KyteaChar a) {
    return (a & 0xFF00) >> 8;
}
inline unsigned char euc2(KyteaChar a) {
    return (a & 0xFF);
}

KyteaChar StringUtilEuc::mapChar(const string & str, bool add) {
    unsigned len = str.length();
    KyteaChar ret;
    if(len == 1) {
#ifdef KYTEA_SAFE
        if(str[0] & maskl1)
            THROW_ERROR("Expected EUC file but found non-EUC string (specify the proper encoding with -encode utf8/euc/sjis): "<<str);
#endif
        ret = eucm(0,str[0]);
    }
    else if(len == 2) {
#ifdef KYTEA_SAFE
        if(!(maskl1 & str[0] & str[1]))
            THROW_ERROR("Expected EUC file but found non-EUC string (specify the proper encoding with -encode utf8/euc/sjis): "<<str);
#endif
        ret = eucm(str[0],str[1]);
    } 
    else
        THROW_ERROR("Expected EUC file but found non-EUC string (specify the proper encoding with -encode utf8/euc/sjis): "<<str);
    return ret;
}

string StringUtilEuc::showChar(KyteaChar c) {
    if(c < 0x8E) {
        char arr[2] = { c, 0 };
        string ret(arr);
        return ret;
    }
    else {
        char arr[3] = { euc1(c), euc2(c), 0 }; 
        string ret(arr);
        return ret;
    }
}

// map an unparsed string to a KyteaString
KyteaString StringUtilEuc::mapString(const string & str) {
    unsigned pos = 0, len = str.length();
    vector<KyteaChar> ret;
    while(pos < len) {
        // single character unicode values
        if(!(maskl1 & str[pos]))
            ret.push_back(mapChar(str.substr(pos++, 1)));
        else {
            ret.push_back(mapChar(str.substr(pos,2)));
            pos += 2;
        }
    }
    KyteaString retstr(ret.size());
    for(unsigned i = 0; i < ret.size(); i++)
        retstr[i] = ret[i];
    return retstr;
}

// get the type of a character
StringUtil::CharType StringUtilEuc::findType(const string & str) {
    return findType(mapChar(str));
}
StringUtil::CharType StringUtilEuc::findType(KyteaChar c) {
    unsigned char c1 = euc1(c), c2 = euc2(c);
    // digits (hankaku/zenkaku)
    if((c2 >= 0x30 && c2 <= 0x39) || (c1 == 0xA3 && c2 >= 0xB0 && c2 <= 0xB9))
        return DIGIT;
    // romaji (lower/upper for hankaku/zenkaku)
    else if((c2 >= 0x41 && c2 <= 0x5A) || (c2 >= 0x61 && c2 <= 0x7A)
        || (c1 == 0xA3 && ((c2 >= 0xC1 && c2 <= 0xDA) || (c2 >= 0xE1 && c2 <= 0xFA)))) {
        return ROMAJI;
    }
    // hiragana
    else if(c1 == 0xA4 && c2 >= 0xA1 && c2 <= 0xF3) {
        return HIRAGANA;
    }
    // katakana
    else if((c1 == 0xA5 && c2 >= 0xA1 && c2 <= 0xF6) || // full-width
        (c1 == 0xA1 && c2 == 0xBC) || // horizontal bar
        (c1 == 0x8E) // half-width
        ) {
        return KATAKANA;
    }
    // kanji
    else if(c1 >= 0xB0 && c1 <= 0xF4) {
        return KANJI;
    }
    return OTHER;
}

// return the encoding provided by this util
StringUtil::Encoding StringUtilEuc::getEncoding() { return StringUtil::ENCODING_EUC; } 
const char* StringUtilEuc::getEncodingString() { return "euc"; }

// transform to or from a character string
void StringUtilEuc::unserialize(const string & str) {  }
string StringUtilEuc::serialize() const { string ret; return ret; } 


inline KyteaChar sjism(char a, char b) {
    KyteaChar ret = a & 0xFF;
    ret = ret << 8;
    ret = ret | (b&0xFF);
    return ret;
}
inline unsigned char sjis1(KyteaChar a) {
    return (a & 0xFF00) >> 8;
}
inline unsigned char sjis2(KyteaChar a) {
    return (a & 0xFF);
}

KyteaChar StringUtilSjis::mapChar(const string & str, bool add) {
    unsigned len = str.length();
    KyteaChar ret;
    if(len == 1) {
#ifdef KYTEA_SAFE
        const unsigned char first = (unsigned char)str[0];
        if((first & maskl1) && !(first >= 0xA0 && first <= 0xDF))
            THROW_ERROR("Expected SJIS file but found non-SJIS string (specify the proper encoding with -encode utf8/euc/sjis): "<<str);
#endif
        ret = sjism(0,str[0]);
    }
    else if(len == 2) {
#ifdef KYTEA_SAFE
        const unsigned char first = (unsigned char)str[0];
        if(!(first & maskl1) || (first >= 0xA0 && first <= 0xDF))
            THROW_ERROR("Expected SJIS file but found non-SJIS string (specify the proper encoding with -encode utf8/euc/sjis): "<<str);
#endif
        ret = sjism(str[0],str[1]);
    } 
    else
        THROW_ERROR("Expected SJIS file but found non-SJIS string (specify the proper encoding with -encode utf8/euc/sjis): "<<str);
    return ret;
}

string StringUtilSjis::showChar(KyteaChar c) {
    if(c < 0xFF) {
        char arr[2] = { c, 0 };
        string ret(arr);
        return ret;
    }
    else {
        char arr[3] = { sjis1(c), sjis2(c), 0 }; 
        string ret(arr);
        return ret;
    }
}

// map an unparsed string to a KyteaString
KyteaString StringUtilSjis::mapString(const string & str) {
    unsigned pos = 0, len = str.length();
    vector<KyteaChar> ret;
    while(pos < len) {
        // single character unicode values
        const unsigned char first = (unsigned char)str[pos];
        if(!(first & maskl1) || (first >= 0xA0 && first <= 0xDF))
            ret.push_back(mapChar(str.substr(pos++, 1)));
        else {
            ret.push_back(mapChar(str.substr(pos,2)));
            pos += 2;
        }
    }
    KyteaString retstr(ret.size());
    for(unsigned i = 0; i < ret.size(); i++)
        retstr[i] = ret[i];
    return retstr;
}

// get the type of a character
StringUtil::CharType StringUtilSjis::findType(const string & str) {
    return findType(mapChar(str));
}
StringUtil::CharType StringUtilSjis::findType(KyteaChar c) {
    unsigned char c1 = sjis1(c), c2 = sjis2(c);
    // digits (hankaku/zenkaku)
    if((c1 == 0 && c2 >= 0x30 && c2 <= 0x39) || (c1 == 0x82 && c2 >= 0x4F && c2 <= 0x58))
        return DIGIT;
    // romaji (lower/upper for hankaku/zenkaku)
    else if((c1 == 0 && ((c2 >= 0x41 && c2 <= 0x5A) || (c2 >= 0x61 && c2 <= 0x7A)))
        || (c1 == 0x82 && ((c2 >= 0x60 && c2 <= 0x79) || (c2 >= 0x81 && c2 <= 0x9A)))) {
        return ROMAJI;
    }
    // hiragana
    else if(c1 == 0x82 && c2 >= 0x9F && c2 <= 0xF1) {
        return HIRAGANA;
    }
    // katakana
    else if((c1 == 0x83 && c2 >= 0x40 && c2 <= 0x96) || // full-width
        (c1 == 0x81 && c2 == 0x5B) || // horizontal bar
        (c1 == 0 && c2 >= 0xA6 && c2 <= 0xDF) // half-width
        ) {
        return KATAKANA;
    }
    // kanji
    else if(
        (c1 >= 0x88 && c1 <= 0x9F) ||
        (c1 >= 0xE0 && c1 <= 0xEA)
    ) {
        return KANJI;
    }
    return OTHER;
}

// return the encoding provided by this util
StringUtil::Encoding StringUtilSjis::getEncoding() { return StringUtil::ENCODING_SJIS; } 
const char* StringUtilSjis::getEncodingString() { return "sjis"; }

// transform to or from a character string
void StringUtilSjis::unserialize(const string & str) {  }
string StringUtilSjis::serialize() const { string ret; return ret; } 
