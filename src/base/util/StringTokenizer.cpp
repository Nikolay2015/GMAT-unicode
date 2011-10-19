//$Id: StringTokenizer.cpp 9513 2011-04-30 21:23:06Z djcinsb $
//-------------------------------------------------------------------------------
//                              StringTokenizer
//-------------------------------------------------------------------------------
// GMAT: General Mission Analysis Tool.
//
// Copyright (c) 2002-2011 United States Government as represented by the
// Administrator of The National Aeronautics and Space Administration.
// All Other Rights Reserved.
//
// Developed jointly by NASA/GSFC and Thinking Systems, Inc. under contract
// number S-67573-G
//
// Author:  Joey Gurganus
// Created: 2004/05/14
//
/**
 * Definition of the StringTokenizer class base
 */
//-------------------------------------------------------------------------------

#include "StringTokenizer.hpp"
#include "MessageInterface.hpp"

//#define DEBUG_STRING_TOKENIZER 1
//#define DEBUG_STRING_TOKENIZER_PARSE 1

//-------------------------------------
// public methods
//-------------------------------------

//-------------------------------------------------------------------------------
//  StringTokenizer()
//-------------------------------------------------------------------------------
/**
 * Creates default constructor.
 *
 * 
 */
//-------------------------------------------------------------------------------
StringTokenizer::StringTokenizer()
{
   delimiters = wxT(" ");
   countTokens = 0;
}


//-------------------------------------------------------------------------------
//  StringTokenizer(const wxString &str, const wxString &delim)
//-------------------------------------------------------------------------------
/**
 * Creates constructor with parameters.
 *
 * @param <str>   Given String
 * @param <delim> Given delimiters
 * 
 */
//-------------------------------------------------------------------------------
StringTokenizer::StringTokenizer(const wxString &str, const wxString &delim)
{
   #if DEBUG_STRING_TOKENIZER
   MessageInterface::ShowMessage
      (wxT("StringTokenizer() str=<%s>, delim=<%s>\n"), str.c_str(), delim.c_str());
   #endif
   
   delimiters = delim;
   Parse(str);
}


//-------------------------------------------------------------------------------
//  StringTokenizer(const wxString &str, const wxString &delim,
//                  bool insertDelim)
//-------------------------------------------------------------------------------
/**
 * Creates constructor with parameters.
 *
 * @param <str>   Given String
 * @param <delim> Given delimiters
 * @param <insertDelim> true if inserting delimiter back to tokenized string.
 *                      it will insert if delimiter size is 1
 *
 * @note:
 * I want to have constructor with
 * StringTokenizer(const wxString &str, const wxString &delim,
 *                  bool insertDelim = false)
 * but this causing some numeric differences when routine test is running
 *
 */
//-------------------------------------------------------------------------------
StringTokenizer::StringTokenizer(const wxString &str, const wxString &delim,
                                 bool insertDelim)
{
   #if DEBUG_STRING_TOKENIZER
   MessageInterface::ShowMessage
      (wxT("StringTokenizer() str='%s', delim='%s', insertDelim=%d\n"), str.c_str(),
       delim.c_str(), insertDelim);
   #endif
   
   delimiters = delim;
   Parse(str, insertDelim);
}


//-------------------------------------------------------------------------------
//  ~StringTokenizer()
//-------------------------------------------------------------------------------
/**
 * Destructor.
 * 
 */
//-------------------------------------------------------------------------------
StringTokenizer::~StringTokenizer()
{
}

//-------------------------------------------------------------------------------
//  Integer CountTokens() const
//-------------------------------------------------------------------------------
/**
 * Get the number of tokens.
 *
 * @return number of tokens. 
 * 
 */
//-------------------------------------------------------------------------------
Integer StringTokenizer::CountTokens() const
{
   return countTokens;
}

//-------------------------------------------------------------------------------
//  wxString GetToken(const Integer loc) 
//-------------------------------------------------------------------------------
/**
 * Get the string from specifiying the token number.
 *
 * @param <loc>   Specified token number
 * @return        return string in the specified token number 
 * 
 */
//-------------------------------------------------------------------------------
wxString StringTokenizer::GetToken(const Integer loc)  const
{
   if (loc <= countTokens)
      return stringTokens.at(loc);

   return wxString(wxT(""));
}

//-------------------------------------------------------------------------------
//  std::vector<string> GetAllTokens() const;
//-------------------------------------------------------------------------------
/**
 * Get all token string. 
 *
 * @return       all token strings 
 * 
 */
//-------------------------------------------------------------------------------
const StringArray& StringTokenizer::GetAllTokens() const 
{
   return stringTokens; 
}


//-------------------------------------------------------------------------------
//  void Set(const wxString &str, const wxString &delim) 
//-------------------------------------------------------------------------------
/**
 * Set the string token.
 *
 * @param <str>   given string
 * @param <delim> given delimiters 
 * 
 */
//-------------------------------------------------------------------------------
void StringTokenizer::Set(const wxString &str, const wxString &delim)
{
   #if DEBUG_STRING_TOKENIZER
   MessageInterface::ShowMessage
      (wxT("StringTokenizer::Set() str=<%s>, delim=<%s>\n"), str.c_str(), delim.c_str());
   #endif
   
   stringTokens.clear();
   delimiters = delim;
   Parse(str);
}

//-------------------------------------------------------------------------------
//  void Set(const wxString &str, const wxString &delim) 
//-------------------------------------------------------------------------------
/**
 * Set the string token.
 *
 * @param <str>   given string
 * @param <delim> given delimiters 
 * 
 */
//-------------------------------------------------------------------------------
void StringTokenizer::Set(const wxString &str, const wxString &delim,
                          bool insertDelim)
{
   #if DEBUG_STRING_TOKENIZER
   MessageInterface::ShowMessage
      (wxT("StringTokenizer::Set() str='%s', delim='%s', insertDelim=%d\n"), str.c_str(),
       delim.c_str(), insertDelim);
   #endif
   
   stringTokens.clear();
   delimiters = delim;
   Parse(str, insertDelim);
}


//-------------------------------------
// private methods
//-------------------------------------

//-------------------------------------------------------------------------------
//  void Parse(const wxString &str)
//-------------------------------------------------------------------------------
/**
 * Parse out the string for token
 *
 * @param <str>   Given string
 *
 */
//-------------------------------------------------------------------------------
void StringTokenizer::Parse(const wxString &str)
{
   #if DEBUG_STRING_TOKENIZER_PARSE
   MessageInterface::ShowMessage
      (wxT("StringTokenizer::Parse() str=%s, delimiters=<%s>, insertDelim=%d\n"),
       str.c_str(), delimiters.c_str(), insertDelim);
   #endif
   
   wxString::size_type pos, lastPos;
   wxString substr;
   
   // Skip delimiters at beginning.
   lastPos = str.find_first_not_of(delimiters, 0);
   
   // Find first delimiter.
   pos = str.find_first_of(delimiters, lastPos);
   
   #if DEBUG_STRING_TOKENIZER_PARSE > 1
   MessageInterface::ShowMessage(wxT("   lastPos=%u, pos=%u\n"), lastPos, pos);
   #endif
   
   if (pos == str.npos)
   {
      stringTokens.push_back(str);
   }
   else
   {
      while (pos != str.npos || lastPos != str.npos)
      {
         #if DEBUG_STRING_TOKENIZER_PARSE > 1
         MessageInterface::ShowMessage
            (wxT("   lastPos=%u, pos=%u\n"), lastPos, pos);
         #endif
         
         // Found a token, add it to the vector.
         substr = str.substr(lastPos, pos - lastPos);
         
         #if DEBUG_STRING_TOKENIZER_PARSE > 1
         MessageInterface::ShowMessage(wxT("   substr=<%s>\n"), substr.c_str());
         #endif

         
         stringTokens.push_back(str.substr(lastPos, pos - lastPos));
         
         // Skip delimiters.  Note the wxT("not_of")
         lastPos = str.find_first_not_of(delimiters, pos);
         
         // Find next wxT("delimiter")
         pos = str.find_first_of(delimiters, lastPos);
      }
   }
   
   // Set the number of token found
   countTokens = stringTokens.size();
   
   #if DEBUG_STRING_TOKENIZER_PARSE
   MessageInterface::ShowMessage(wxT("   countTokens=%d\n"), countTokens);
   #endif
   
}


//-------------------------------------------------------------------------------
//  void Parse(const wxString &str)
//-------------------------------------------------------------------------------
/**
 * Parse out the string for token
 *
 * @param <str>   Given string
 *
 */
//-------------------------------------------------------------------------------
void StringTokenizer::Parse(const wxString &str, bool insertDelim)
{
   #if DEBUG_STRING_TOKENIZER_PARSE
   MessageInterface::ShowMessage
      (wxT("StringTokenizer::Parse() str=%s, delimiters=<%s>, insertDelim=%d\n"),
       str.c_str(), delimiters.c_str(), insertDelim);
   #endif
   
   wxString::size_type pos, lastPos;
   wxString substr;
   
   // Skip delimiters at beginning.
   lastPos = str.find_first_not_of(delimiters, 0);
   
   // Find first delimiter.
   pos = str.find_first_of(delimiters, lastPos);
   
   #if DEBUG_STRING_TOKENIZER_PARSE > 1
   MessageInterface::ShowMessage(wxT("   lastPos=%u, pos=%u\n"), lastPos, pos);
   #endif
   
   if (pos == str.npos)
   {
      stringTokens.push_back(str);
   }
   else
   {
      while (pos != str.npos || lastPos != str.npos)
      {
         #if DEBUG_STRING_TOKENIZER_PARSE > 1
         MessageInterface::ShowMessage
            (wxT("   lastPos=%u, pos=%u\n"), lastPos, pos);
         #endif
         
         // Found a token, add it to the vector.
         substr = str.substr(lastPos, pos - lastPos);
         
         #if DEBUG_STRING_TOKENIZER_PARSE > 1
         MessageInterface::ShowMessage(wxT("   substr=<%s>\n"), substr.c_str());
         #endif

         
         ////stringTokens.push_back(str.substr(lastPos, pos - lastPos));
         stringTokens.push_back(str.substr(lastPos, pos - lastPos + 1));
         
         // Skip delimiters.  Note the wxT("not_of")
         lastPos = str.find_first_not_of(delimiters, pos);
         
         // Find next wxT("delimiter")
         pos = str.find_first_of(delimiters, lastPos);
      }
   }
   
   // Set the number of token found
   countTokens = stringTokens.size();
   
   #if DEBUG_STRING_TOKENIZER_PARSE
   MessageInterface::ShowMessage(wxT("   countTokens=%d\n"), countTokens);
   #endif
   
}
