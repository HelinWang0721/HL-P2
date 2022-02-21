/*
 * Copyright (C) Rida Bazzi, 2016
 *
 * Do not share this file with anyone
 */
#include <iostream>
#include <istream>
#include <vector>
#include <string>
#include <cctype>

#include "lexer.h"
#include "format.h"
#include "LineAnalysis.h"
#include "Evaluation.h"

using namespace std;

static string reserved[] = { "END_OF_FILE",
    "IF", "WHILE", "DO", "THEN", "PRINT","PUBLIC","PRIVATE",
    "PLUS", "MINUS", "DIV", "MULT",
    "EQUAL", "COLON", "COMMA", "SEMICOLON",
    "LBRAC", "RBRAC", "LPAREN", "RPAREN",
    "NOTEQUAL", "GREATER", "LESS", "LTEQ", "GTEQ",
    "DOT", "NUM", "ID", "ERROR","REALNUM","BASE08NUM","BASE16NUM","COMMENT","LBRACE","RBRACE" // TODO: Add labels for new token types here (as string)
};

#define KEYWORDS_COUNT 7
string keyword[] = { "IF", "WHILE", "DO", "THEN", "PRINT","public","private"};



string pdigit16 = "123456789ABCDEF";
string digit16 = "0123456789ABCDEF";


void Token::Print()
{
    cout << "{" << this->lexeme << " , "
         << reserved[(int) this->token_type] << " , "
         << this->line_no << "}\n";
}

LexicalAnalyzer::LexicalAnalyzer()
{
    this->line_no = 1;
    tmp.lexeme = "";
    tmp.line_no = 1;
    tmp.token_type = ERROR;
}

Token LexicalAnalyzer::EOFToken() {
    Token endOfFile_token;
    endOfFile_token.lexeme = "";
    endOfFile_token.line_no = this->line_no;
    endOfFile_token.token_type = END_OF_FILE;
    return endOfFile_token;
}

bool LexicalAnalyzer::SkipSpace() // return whather encountered space
{
    char c;
    bool space_encountered = false;

    // input.GetChar(c);
    // line_no += (c == '\n');

    // while (!input.EndOfInput() && isspace(c)) {
    //     space_encountered = true;
    //     input.GetChar(c);
    //     line_no += (c == '\n');
    // }

    // if (!input.EndOfInput()) {
    //     input.UngetChar(c);
    // }
    // 
    
    while (true)
    {
        if (input.EndOfInput()) {
            tmp.lexeme = "";
            tmp.line_no = line_no;
            tmp.token_type = END_OF_FILE;
            break;
        }
        input.GetChar(c);
        if (isspace(c)) {
            space_encountered = true;
            line_no += (c == '\n');
        }
        else {
            input.UngetChar(c);
            break;
        }
    }
    return space_encountered;
}

bool LexicalAnalyzer::IsKeyword(string s)
{
    for (int i = 0; i < KEYWORDS_COUNT; i++) {
        if (s == keyword[i]) {
            return true;
        }
    }
    return false;
}

TokenType LexicalAnalyzer::FindKeywordIndex(string s)
{
    for (int i = 0; i < KEYWORDS_COUNT; i++) {
        if (s == keyword[i]) {
            return (TokenType) (i + 1);
        }
    }
    return ERROR;
}

Token LexicalAnalyzer::ScanNumber()
{
    char c;

    input.GetChar(c);
    if (isdigit(c)) {
        if (c == '0') {
            tmp.lexeme = "0";
        } else {
            tmp.lexeme = "";
            while (!input.EndOfInput() && isdigit(c)) {
                tmp.lexeme += c;
                input.GetChar(c);
            }
            if (!input.EndOfInput()) {
                input.UngetChar(c);
            }
        }
        // TODO: You can check for REALNUM, BASE08NUM and BASE16NUM here!
        tmp.token_type = NUM;
        tmp.line_no = line_no;

        ScanBASE08NUM();
        if (tmp.token_type == BASE08NUM) 
            return tmp;

        ScanRealNumber();
        return tmp;

    } else {
        if (!input.EndOfInput()) {
            input.UngetChar(c);
        }
        tmp.lexeme = "";
        tmp.token_type = ERROR;
        tmp.line_no = line_no;
        return tmp;
    }
}

Token LexicalAnalyzer::ScanIdOrKeyword()
{
    if (input.EndOfInput()) {
        return EOFToken();
    }

    char c;
    input.GetChar(c);

    if (isalpha(c)) {
        tmp.lexeme = "";
        tmp.lexeme += c;
        while (!input.EndOfInput()) {
            input.GetChar(c);
            if (!isalnum(c)) {
                input.UngetChar(c);
                break;
            }
            tmp.lexeme += c;
        }
        tmp.line_no = line_no;

        if (IsKeyword(tmp.lexeme))
            tmp.token_type = FindKeywordIndex(tmp.lexeme);
        else
            tmp.token_type = ID;
    }
    else {
        input.UngetChar(c);
        tmp.lexeme = "";
        tmp.line_no = line_no;
        tmp.token_type = ERROR;
    }
    return tmp;
}

Token LexicalAnalyzer::ScanRealNumber() {
    char c;
    input.GetChar(c);
    
    
    if (c == '.'){

        input.GetChar(c);

        if (isdigit(c)) {
            tmp.lexeme = tmp.lexeme + ".";

            while (!input.EndOfInput() && isdigit(c)) {
                tmp.lexeme += c;
                input.GetChar(c);

            }if (!input.EndOfInput()) {
                input.UngetChar(c);
            }

            tmp.token_type = REALNUM;
            tmp.line_no = line_no;
            return tmp;
        }
        else {
            input.UngetChar(c);
            input.UngetChar('.');
            return tmp;
        }
    }
    else {
        input.UngetChar(c);
        tmp.token_type = NUM;
        tmp.line_no = line_no;
        return tmp;
    }
}

Token LexicalAnalyzer::ScanBASE08NUM(){

    if (tmp.lexeme.find("8") != tmp.lexeme.npos || tmp.lexeme.find("9") != tmp.lexeme.npos) {
        return tmp;
    }
    char x;
    input.GetChar(x);
    if (x == 'x') {
        char zero;
        input.GetChar(zero);
        if (zero == '0') {
            char eight;
            input.GetChar(eight);
            if (eight == '8') {
                tmp.token_type = BASE08NUM;
                tmp.lexeme += "x08";
                tmp.line_no = line_no;
                return tmp;
            }
            input.UngetChar(eight);
        }
        input.UngetChar(zero);
    }
    input.UngetChar(x);
    tmp.token_type = NUM;
    tmp.line_no = line_no;
    return tmp;
}


Token LexicalAnalyzer::ScanBASE16NUM() {
    char c;
    input.GetChar(c);

    if (pdigit16.find(c) == digit16.npos) {
        tmp.token_type = ERROR;
        input.UngetChar(c);
        return tmp;
    }
    else
    {
        tmp.lexeme = "";
        tmp.lexeme += c;
    }
    
     while (true) {
        input.GetChar(c);
        tmp.lexeme += c;
        
        if (digit16.find(c) == digit16.npos || input.EndOfInput()) {
            if (c == 'x') {
                break;
            }
            tmp.token_type = ERROR;
            input.UngetString(tmp.lexeme);
            return tmp;
        }
     }
    input.GetChar(c);
    tmp.lexeme += c;
    if (c == '1') {
        input.GetChar(c);
        tmp.lexeme += c;
        if(c == '6') {
            tmp.token_type = BASE16NUM;
            tmp.line_no = line_no;
            return tmp;
        }
    }
    input.UngetString(tmp.lexeme);
    tmp.token_type = ERROR;
    return tmp;
}

Token LexicalAnalyzer::scanComment_or_div() {
    if (input.EndOfInput()) { // when div is the last token before end of file
        UngetToken(EOFToken());
        return tmp;  // return DIV token
    }
    
    char c;
    input.GetChar(c);

    if (c == '/') {
        tmp.token_type = COMMENT;
        while (true) {
            if (input.EndOfInput()) {
                UngetToken(EOFToken());
                break;
            }
            input.GetChar(c);
            if (c == '\n') {
                line_no++;
                break;
            }
        }
    }
    else {
        input.UngetChar(c);
        tmp.token_type = DIV;
    }
    return tmp;
}





// you should unget tokens in the reverse order in which they
// are obtained. If you execute
//
//    t1 = lexer.GetToken();
//    t2 = lexer.GetToken();
//    t3 = lexer.GetToken();
//
// in this order, you should execute
//
//    lexer.UngetToken(t3);
//    lexer.UngetToken(t2);
//    lexer.UngetToken(t1);
//
// if you want to unget all three tokens. Note that it does not
// make sense to unget t1 without first ungetting t2 and t3
//
TokenType LexicalAnalyzer::UngetToken(Token tok)
{
    tokens.push_back(tok);;
    return tok.token_type;
}

Token LexicalAnalyzer::GetToken()
{
    char c;

    // if there are tokens that were previously
    // stored due to UngetToken(), pop a token and
    // return it without reading from input
    if (!tokens.empty()) {
        tmp = tokens.back();
        tokens.pop_back();
        return tmp;
    }

    tmp.lexeme = "";
    tmp.line_no = line_no;

    if (input.EndOfInput()) {
        tmp.token_type = END_OF_FILE;
        return tmp;
    }

    SkipSpace();
    if(tmp.token_type == END_OF_FILE) {
        return tmp;
    }

    input.GetChar(c);
    switch (c) {
#ifdef CHECK_ALL_SYMBOL
        case '.':
            tmp.token_type = DOT;
            return tmp;
        case '+':
            tmp.token_type = PLUS;
            return tmp;
        case '-':
            tmp.token_type = MINUS;
            return tmp;
#endif
        case '/': //////////////////////////////////
            // tmp.token_type = DIV;
            // scanComment();
            // if (tmp.token_type != COMMENT && tmp.token_type != END_OF_FILE) {
            //     tmp.token_type = DIV;
            // }
            // return tmp;
            tmp.token_type = DIV;
            return scanComment_or_div();
        // case '*':
        //     tmp.token_type = MULT;
        //     return tmp;
        case '=':
            tmp.token_type = EQUAL;
            return tmp;
        case ':':
            tmp.token_type = COLON;
            return tmp;
        case ',':
            tmp.token_type = COMMA;
            return tmp;
        case ';':
            tmp.token_type = SEMICOLON;
            return tmp;
#ifdef CHECK_ALL_SYMBOL
        case '[':
            tmp.token_type = LBRAC;
            return tmp;
        case ']':
            tmp.token_type = RBRAC;
            return tmp;
        case '(':
            tmp.token_type = LPAREN;
            return tmp;
        case ')':
            tmp.token_type = RPAREN;
            return tmp;
#endif
        case '{':
            tmp.token_type = LBRACE;
            return tmp;
        case '}':
            tmp.token_type = RBRACE;
            return tmp;
#ifdef CHECK_ALL_SYMBOL
        case '<':
            input.GetChar(c);
            if (c == '=') {
                tmp.token_type = LTEQ;
            } else if (c == '>') {
                tmp.token_type = NOTEQUAL;
            } else {
                if (!input.EndOfInput()) {
                    input.UngetChar(c);
                }
                tmp.token_type = LESS;
            }
            return tmp;
        case '>':
            input.GetChar(c);
            if (c == '=') {
                tmp.token_type = GTEQ;
            } else {
                if (!input.EndOfInput()) {
                    input.UngetChar(c);
                }
                tmp.token_type = GREATER;
            }
            return tmp;
#endif
        default:
            // input.UngetChar(c);
            // ScanBASE16NUM();
            // if (tmp.token_type == BASE16NUM) {
            //     return tmp;
            // }
            // input.GetChar(c);
            if (isdigit(c)) {
                // input.UngetChar(c);
                // return ScanNumber();
                tmp.token_type = ERROR;
            }
            else if (isalpha(c)) {
                input.UngetChar(c);
                return ScanIdOrKeyword();
            }
            else
               tmp.token_type = ERROR;

            return tmp;
    }
}

/*
            if (input.EndOfInput()) {
                tmp.token_type = END_OF_FILE;
            }
            else if (isdigit(c) || isalpha(c)) {
                input.UngetChar(c);
                return ScanIdOrKeyword();
            }
            else {
                tmp.token_type = ERROR;
            }
*/

void exit_syntax_error() {
    cout << "Syntax Error" << endl;
    exit(1);
}

int main()
{
    //string test = "C:\\Users\\13681\\source\\repos\\CSE340ass1\\Debug\\test_parsing_ok02.txt";

   // string test = "/home/billy/Desktop/proj1/tests/test_parsing_ok02.txt";
    LexicalAnalyzer lexer;
    Token token;

   // lexer.input.readFile(test);

    token = lexer.GetToken();
    if (token.token_type != COMMENT && token.token_type !=  END_OF_FILE)
        lexer.tokenList.push_back(token);

    //token.Print();

    while (token.token_type != END_OF_FILE)
    {
        token = lexer.GetToken();
        //token.Print();
        if (token.token_type == ERROR) {
            exit_syntax_error();
        }
#ifndef CHECK_ALL_SYMBOL
        if (token.token_type == DIV) {
            exit_syntax_error();
        }
#endif

        if (token.token_type == COMMENT || token.token_type ==  END_OF_FILE) continue;
        lexer.tokenList.push_back(token);
    }

    // for (int i = 0; i < lexer.tokenList.size(); i++) {
    //     lexer.tokenList[i].Print();
    // }
    //cout << "\n\n\n";
    
    FormatAnalyzer fa = FormatAnalyzer(lexer.tokenList);
    Analysis_Status fStatus = fa.scan();
    if(fStatus == SYNTAX_ERROR) {
        exit_syntax_error();
    }

    //fa.globalScope.Print(0);
    //cout << "\n\n";

    LineAnalysis la = LineAnalysis(fa);
    auto lStatus = la.scan();
    if (lStatus == SYNTAX_ERROR) {
        exit_syntax_error();
    }

    //la.globalScope.Print(0);
    //cout << "\n\n";

    Evaluation evaluation = Evaluation(la);
    auto eStatus = evaluation.eval();

    if (eStatus == SYNTAX_ERROR) {
        exit_syntax_error();
    }

    evaluation.printExpressionOutput();

    return 0;
}

std::string getIndent(int indent) {
    string ret = "";
    for (size_t i = 0; i < indent; i++)
    {
        ret = ret + "    ";
    }
    return ret;
}
