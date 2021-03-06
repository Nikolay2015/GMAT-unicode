//$Id: ExpressionParser.cpp 9513 2011-04-30 21:23:06Z djcinsb $
//------------------------------------------------------------------------------
//                                  ExpressionParser
//------------------------------------------------------------------------------
// GMAT: General Mission Analysis Tool
//
// Copyright (c) 2002-2011 United States Government as represented by the
// Administrator of The National Aeronautics and Space Administration.
// All Other Rights Reserved.
//
// Developed jointly by NASA/GSFC and Thinking Systems, Inc. under contract
// number S-67573-G
//
// Author: Linda Jun
// Created: 2004/09/15
//
/**
 * Implements ExpressionParser class which parses and evaluates the
 * expression. The valid expressions are composed of the following items:
 *    1. Numbers
 *    2. Operators(+,-,/,*,^)
 *    3. Parentheses
 *    4. Varialbes
 *
 * Assume this precendence of each operator:
 *    highest   + - (unary)
 *              ^ (exponentiation)
 *              * /
 *              + -
 *
 * Parser Code Reference: wxT("The Complete Reference C++") by Herbert Schildt,
 *    4th Edition, P963-997
 */
//------------------------------------------------------------------------------

#include "ExpressionParser.hpp"
#include "ParameterException.hpp"
#include "MessageInterface.hpp"
#include <iostream>

#include <cstdlib>			// Required for GCC 4.3
#include <wx/sstream.h>

//#define DEBUG_EXP_PARSER 1

//---------------------------------
// public methods
//---------------------------------


//------------------------------------------------------------------------------
// ExpressionParser()
//------------------------------------------------------------------------------
ExpressionParser::ExpressionParser()
{
   mExp = wxT("");
   mParamDb = NULL;

   for (int i=0; i<NUM_VARS; i++)
      mVars[i] = 0.0;
}


//------------------------------------------------------------------------------
// ~ExpressionParser()
//------------------------------------------------------------------------------
ExpressionParser::~ExpressionParser()
{
}


//------------------------------------------------------------------------------
// Real EvalExp(const wxString &exp)
//------------------------------------------------------------------------------
/*
 * Evaluates given expression and return results.
 *
 * @param <exp> null terminated expression
 *
 * @return expression evaluated results
 */
//------------------------------------------------------------------------------
Real ExpressionParser::EvalExp(const wxString &exp)
{
   Real result;
   mExp = exp;
   mToken = wxT("");

#if DEBUG_EXP_PARSER
   MessageInterface::ShowMessage
      (wxT("ExpressionParser::EvalExp() exp=%s\n"), exp);
#endif

   GetToken();

   if (!*mToken)
   {
      HandleSyntaxError(NO_EXPRESSION_PRESENT);
      return 0.0;
   }

   EvalTwoTerms(result);

   if (*mToken)
      HandleSyntaxError(SYNTAX_ERROR); // last token must be null


   return result;
}


//------------------------------------------------------------------------------
// void SetParameterDatabase(ParameterDatabase *pdb)
//------------------------------------------------------------------------------
/*
 * Sets ParameterDatabase pointer.
 *
 * @param <pdb> ParameterDatabase pointer to set to
 *
 */
//------------------------------------------------------------------------------
void ExpressionParser::SetParameterDatabase(ParameterDatabase *pdb)
{
   mParamDb = pdb;
}

//---------------------------------
// private methods
//---------------------------------

//------------------------------------------------------------------------------
// void EvalTwoTerms(Real &result)
//------------------------------------------------------------------------------
/*
 * Add or subtract two terms.
 */
//------------------------------------------------------------------------------
void ExpressionParser::EvalTwoTerms(Real &result)
{
   register wxChar op; // it's heavily used
   Real temp;

   EvalTwoFactors(result);

   while ((op = *mToken) == wxT('+') || op == wxT('-'))
   {
      GetToken();
      EvalTwoFactors(temp);

      switch (op)
      {
      case wxT('-'):
         result = result - temp;
         break;
      case wxT('+'):
         result = result + temp;
         break;
      default:
         break;
      }
   }
}


//------------------------------------------------------------------------------
// void EvalTwoFactors(Real &result)
//------------------------------------------------------------------------------
/*
 * Multiply or divide two factors.
 */
//------------------------------------------------------------------------------
void ExpressionParser::EvalTwoFactors(Real &result)
{
   register wxChar op; // it's heavily used
   Real temp;

   EvalExponent(result);

   while ((op = *mToken) == wxT('*') || op == wxT('/') || op == wxT('%'))
   {
      GetToken();
      EvalExponent(temp);

      switch (op)
      {
      case wxT('*'):
         result = result * temp;
         break;
      case wxT('/'):
         result = result / temp;
         break;
      default:
         break;
      }
   }
}


//------------------------------------------------------------------------------
// void EvalExponent(Real &result)
//------------------------------------------------------------------------------
/*
 * Process exponent.
 */
//------------------------------------------------------------------------------
void ExpressionParser::EvalExponent(Real &result)
{
   //register int t;
   Real temp, ex;

   EvalUnary(result);

   if (*mToken == wxT('^'))
   {
      GetToken();
      EvalExponent(temp);
      ex = result;

      if (temp == 0.0)
      {
         result = 1.0;
         return;
      }

      // This will not work for 10^.5
      //for (t=(int)temp-1; t>0; --t)
      //   result = result * (Real)ex;

      result = pow(result, temp);
      //MessageInterface::ShowMessage
      //   (wxT("===> result=%f, ex=%f, temp=%f\n"), result, ex, temp);
   }
}


//------------------------------------------------------------------------------
// void EvalUnary(Real &result)
//------------------------------------------------------------------------------
/*
 * Evaluates a unary + or -.
 */
//------------------------------------------------------------------------------
void ExpressionParser::EvalUnary(Real &result)
{
   register wxChar op;

   op = 0;

   if (((mTokenType == DELIMITER) && *mToken == wxT('+')) || *mToken == wxT('-'))
   {
      op = *mToken;
      GetToken();
   }

   EvalParenExp(result);

   if (op == wxT('-'))
      result = - result;
}


//------------------------------------------------------------------------------
//void EvalParenExp(Real &result)
//------------------------------------------------------------------------------
/*
 * Evaluates a parenthesized expression.
 */
//------------------------------------------------------------------------------
void ExpressionParser::EvalParenExp(Real &result)
{
   if (*mToken == wxT('('))
   {
      GetToken();

      EvalTwoTerms(result);

      if (*mToken != wxT(')'))
         HandleSyntaxError(UNBALANCED_PARENTHESES);

      GetToken();
   }
   else
   {
      GetValue(result);
   }
}


//------------------------------------------------------------------------------
// void GetValue(Real &result)
//------------------------------------------------------------------------------
/*
 * Get the value of a number or variable.
 */
//------------------------------------------------------------------------------
void ExpressionParser::GetValue(Real &result)
{
#if DEBUG_EXP_PARSER
   MessageInterface::ShowMessage
      (wxT("ExpressionParser::GetValue() mToken=%s\n"), mToken);
#endif

   switch (mTokenType)
   {
   case VARIABLE:
      result = EvalVariable(mToken);
      GetToken();
      return;
   case NUMBER:
      mToken.ToDouble(&result);
      GetToken();
      return;
   default:
      HandleSyntaxError(SYNTAX_ERROR);
   }
}


//------------------------------------------------------------------------------
// Real EvalVariable(wxString *var)
//------------------------------------------------------------------------------
/*
 * Get the value of a variable
 */
//------------------------------------------------------------------------------
Real ExpressionParser::EvalVariable(wxString &var)
{
   if (mParamDb == NULL)
   {
      throw ParameterException
         (wxT("ExpressionParser::EvalVariable() Associated ParameterDatabase is NULL.\n")
          wxT("Make sure to call SetParameterDatabase() in EvaluateReal() of variable.\n"));
   }

   wxString varName = wxString(var);
   Parameter *param = mParamDb->GetParameter(varName);

#if DEBUG_EXP_PARSER
   MessageInterface::ShowMessage
      (wxT("ExpressionParser::EvalVariable() varName:%s\n"), varName.c_str());
   //StringArray paramNames = mParamDb->GetNamesOfParameters();
   //for (int i = 0; i<mParamDb->GetNumParameters(); i++)
   //   MessageInterface::ShowMessage
   //      (wxT("ExpressionParser::EvalVariable() In mParamDb: %s\n"), paramNames[i].c_str());
#endif

   if (param != NULL)
   {
      return param->EvaluateReal();
   }
   else
   {
      throw ParameterException
         (wxT("ExpressionParser::EvalVariable() Requested parameter: ") + varName +
          wxT(" has NULL pointer. Make sure to call SetRefObject() of variable.\n"));
   }

}


//------------------------------------------------------------------------------
// void GetToken()
//------------------------------------------------------------------------------
/*
 * Obtain the next token.
 */
//------------------------------------------------------------------------------
void ExpressionParser::GetToken()
{
   wxString temp;

   mTokenType = NUMBER;
   temp = wxT("");

   if (mExp.IsEmpty())
      return; // end of expression

   mExp.Trim(false); //remove leading whitespace

   wxStringInputStream mExpStream(mExp);
   wxChar ch = mExpStream.GetC(); //read first character and strip from stream

   if (strchr("+-*/^=()", ch))
   {
      mTokenType = DELIMITER;
      temp += ch; 
   }
   else if (isalpha(ch))
   {
      while (!IsDelimiter(ch))
      {
         temp += ch;
         ch = mExpStream.GetC();
      }
      mTokenType = VARIABLE;
   }
   //else if (isdigit(*mExp))
   else if (isdigit(ch) || ch == wxT('.')) //loj: 7/15/05 Added check for .
   {
      while (!IsDelimiter(ch))
      {
         temp += ch;
         ch = mExpStream.GetC();
      }

      mTokenType = NUMBER;
   }
   mToken = temp;
}


//------------------------------------------------------------------------------
// bool IsDelimiter(wxChar c)
//------------------------------------------------------------------------------
/* Return true if c is a delimiter.
 */
//------------------------------------------------------------------------------
bool ExpressionParser::IsDelimiter(wxChar c)
{
   if (c == wxT(' ') || c == wxT('+') || c == wxT('-') || c == wxT('/') || c == wxT('*') 
    || c == wxT('^') || c == wxT('=') || c == wxT('(') || c == wxT(')') || c==wxT('\t') 
    || c==wxT('\r') || c==wxT('\0'))
      return true;
   else
      return false;
}


//------------------------------------------------------------------------------
// void HandleSyntaxError(int error)
//------------------------------------------------------------------------------
/*
 * Display a syntax error.
 */
//------------------------------------------------------------------------------
void ExpressionParser::HandleSyntaxError(int error)
{
   static const wxString errMsg[] =
   {
      wxT("Syntax Error\n"),
      wxT("Unbalanced Parentheses\n"),
      wxT("No expression Present\n"),
   };

   throw ParameterException(errMsg[error]);
}
