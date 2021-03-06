%{

/*
 * asn_ref_lex.l
 * 
 * ASN lexical analysis file for typereference, objectclassreference, valuereference, ...
 *
 * Copyright (c) 2001 Institute for Information Industry 
 * (http://www.iii.org.tw/iiia/ewelcome.htm)
 *
 * The contents of this file are subject to the Mozilla Public License
 * Version 1.0 (the "License"); you may not use this file except in
 * compliance with the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS"
 * basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See
 * the License for the specific language governing rights and limitations
 * under the License.
 *
 * The Original Code is ASN Parser.
 *
 * The Initial Developer of the Original Code is Institute for Information Industry
 *
 */

#include "main.h"
#include <iostream>
#include <algorithm>

#define YY_NEVER_INTERACTIVE 1

#include "asn_ref_grammar.h"

#define FALSE 0
#define TRUE  1

#include <ctype.h>
#define yylval idlval
#define yytext idtext
extern unsigned lineNumber;

int InParameterListContext = FALSE;

extern int LexEcho;
#define YYDEBUG 1
#define YY_USER_ACTION if (LexEcho) std::cout << idtext << std::flush;

extern "C" int yywrap() { return 1; }

int braceCount, parenthesisCount;

%}

%x comment_line brace parenthesis square_bracket

%%

"::="				   { return ASSIGNMENT; }

"ABSENT"                           { return ABSENT; }              
"ABSTRACT-SYNTAX"                  { return ABSTRACT_SYNTAX; }
"ALL"                              { return ALL; }
"ANY"                              { return ANY; }
"APPLICATION"                      { return APPLICATION; }
"AUTOMATIC"                        { return AUTOMATIC; }
"BEGIN"                            { return BEGIN_t; }
"BIT"                              { return BIT; }
"BMPString"                        { return BMPString; }
"BOOLEAN"                          { return BOOLEAN_t; }
"BY"                               { return BY; }
"CHARACTER"                        { return CHARACTER; }
"CHOICE"                           { return CHOICE; }
"CLASS"                            { return CLASS; }
"COMPONENT"                        { return COMPONENT; }
"COMPONENTS"                       { return COMPONENTS; }
"CONSTRAINED"                      { return CONSTRAINED; }
"DEFAULT"                          { return DEFAULT; }
"DEFINED"                          { return DEFINED; }
"DEFINITIONS"                      { return DEFINITIONS; }
"EMBEDDED"                         { return EMBEDDED; }
"END"                              { return END; }
"ENUMERATED"                       { return ENUMERATED; }
"EXCEPT"                           { return EXCEPT; }
"EXPLICIT"                         { return EXPLICIT; }
"EXPORTS"                          { return EXPORTS; }
"EXTERNAL"                         { return EXTERNAL; }
"FALSE"                            { return FALSE_t; }
"FROM"                             { return FROM; }
"GeneralString"                    { return GeneralString; }
"GraphicString"                    { return GraphicString; }
"IA5String"                        { return IA5String; }
"TYPE-IDENTIFIER"                  { return TYPE_IDENTIFIER; } 
"IDENTIFIER"                       { return IDENTIFIER_t; }
"IMPLICIT"                         { return IMPLICIT; }
"IMPORTS"                          { return IMPORTS; }
"INCLUDES"                         { return INCLUDES; }
"INSTANCE"                         { return INSTANCE; }
"INTEGER"                          { return INTEGER_t; }
"INTERSECTION"                     { return INTERSECTION; }
"ISO646String"                     { return ISO646String; }
"MACRO"				   { return MACRO; }
"MAX"                              { return MAX; }
"MIN"                              { return MIN; }
"MINUS-INFINITY"                   { return MINUS_INFINITY; }
"NOTATION"			   { return NOTATION; }
"NULL"                             { return NULL_t; }
"NumericString"                    { return NumericString; }
"OBJECT"                           { return OBJECT; }
"OCTET"                            { return OCTET; }
"OF"                               { return OF_t; }
"OPTIONAL"                         { return OPTIONAL_t; }
"PDV"                              { return PDV; }
"PLUS-INFINITY"                    { return PLUS_INFINITY; }
"PRESENT"                          { return PRESENT; }
"PrintableString"                  { return PrintableString; }
"PRIVATE"                          { return PRIVATE; }
"REAL"                             { return REAL; }
"SEQUENCE"                         { return SEQUENCE; }
"SET"                              { return SET; }
"SIZE"                             { return SIZE_t; }
"STRING"                           { return STRING; }
"SYNTAX"                           { return SYNTAX; }
"T61String"                        { return T61String; }
"TAGS"                             { return TAGS; }
"TeletexString"                    { return TeletexString; }
"TRUE"                             { return TRUE_t; }
"TYPE"				   { return TYPE_t; }
"UNION"                            { return UNION; }
"UNIQUE"                           { return UNIQUE; }
"UNIVERSAL"                        { return UNIVERSAL; }
"UniversalString"                  { return UniversalString; }
"VALUE"				   { return VALUE; }
"VideotexString"                   { return VideotexString; }
"VisibleString"                    { return VisibleString; }
"WITH"                             { return WITH; }
            

"--" {
    BEGIN(comment_line);
  }

<comment_line>"--"  {
    BEGIN(INITIAL);
  }

<comment_line>\n  {
    lineNumber++;
    BEGIN(INITIAL);
  }

<comment_line>. {
}

<comment_line><<EOF>> {
    BEGIN(INITIAL);
}

"@"  { return '@'; }

"{"  {
	BEGIN(brace);
	braceCount++;
	return '{';
}

<brace>"{" { braceCount++; }

<brace>\n {
    lineNumber++;
}

<brace>"}"  {
    if (--braceCount == 0)
	{
		BEGIN(INITIAL); 
		return '}'; 
	}
}

<brace>. { 	/* eat anything inside brace */ }


"["  { 
	BEGIN(square_bracket); 
	return '[';
}

<square_bracket>\n {
    lineNumber++;
}

<square_bracket>"]"  { 
    BEGIN(INITIAL);
	return ']'; 
}

<square_bracket>. {}

"("  { 
    BEGIN(parenthesis); 
	parenthesisCount++;
	return '(';
}

<parenthesis>")"  { 
    if (--parenthesisCount == 0)
	{
		BEGIN(INITIAL);
		return ')';
	}
}

<parenthesis>"("  { 
    parenthesisCount++;
}

<parenthesis>\n {
    lineNumber++;
}

<parenthesis>. { /* eat anything inside parentheis */
}





"<"  { return '<'; }

">"  { return '>'; }

"|"  { return '|'; }

";"  { return ';'; }

"+"  { return '+'; }

"-"  { return '-'; }

"*"  { return '*'; }

"/"  { return '/'; }

","  { return ','; }

"."  { return '.'; }

":"  { return ':'; }

"="  { return '='; }

'|'  { return '|'; }

"^"  { return '^'; }

"'"(\ 01)*"\'B" {
   return BSTRING;
}

"'"(\ A-F0-9)*"'H" {
   return HSTRING;
}

(\"[^\"]*\")(\"[^\"]*\")*  {
    lineNumber += std::count(yytext, yytext+strlen(yytext), '\n');
    return CSTRING;
  }

[0-9][0-9]*  {
    const char * ptr = yytext;
    yylval.ival = 0;
    while (*ptr != '\0')
      yylval.ival = yylval.ival * 10 + *ptr++ - '0';
    return INTEGER;
  }

"&"([a-zA-Z][a-zA-Z0-9_-]*[a-zA-Z0-9_]) {
    yylval.sval = new std::string(yytext);
    return FIELDREFERENCE;
  }

([a-z]|([a-z][a-zA-Z0-9_-]*[a-zA-Z0-9_]))  {
    yylval.sval = new std::string(yytext);
    return IDENTIFIER;
  }


([A-Z]|([A-Z][A-Z0-9_-]*[A-Z0-9_]))  { 
    yylval.sval = new std::string(yytext);
	int result;
	if (Module) 
	  if ((result = Module->GetIdentifierType(yytext)) == OBJECTCLASSREFERENCE)
	     return OBJECTCLASSREFERENCE;
	return WOULDBE_OBJECTCLASSREFERENCE;
}

([A-Z]|([A-Z][a-zA-Z0-9_-]*[a-zA-Z0-9_]))  {
    yylval.sval = new std::string(yytext);
    return TYPEREFERENCE;
  }

[ \t\r]  { }

\n {
    lineNumber++;
  }


.  { std::cerr << StdError(Fatal) << "unknown token " << yytext << std::endl; }


%%