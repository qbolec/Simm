#ifndef TOKENIZE_H
#define TOKENIZE_H

#include "TextInfo.h"

#include <cctype>
/**
    A GlueRule object describes which and how substrings should be glued.
    glueString is a resulting (glued string)
    newType is a new type of the token. If equal to NOTYPE, it means
    the type does not change.
**/
struct GlueRule {
    string glueString;
    CharacterType::Type newType;
    GlueRule(const string& s, CharacterType::Type t=CharacterType::NOTYPE) :
        glueString(s),
        newType(t) {}
    GlueRule(const char* s = "", CharacterType::Type t=CharacterType::NOTYPE) :
        glueString(s),
        newType(t) {}
    bool operator<(const GlueRule& other) const {
        // First go longer rules
        return make_pair(-glueString.size(),glueString) <
               make_pair(-other.glueString.size(),other.glueString);
    }
};

#include <algorithm>
/**
returns 0: not prefix
returns 1: proper prefix
returns 2: the same
**/
inline int isPrefix(const string &sshort, const string &slong) {
    if (sshort.size() > slong.size()) return false;
    bool nonproperPrefix = std::mismatch(sshort.begin(), sshort.end(), slong.begin()).first == sshort.end();
    if (nonproperPrefix && sshort.size() == slong.size()) return 2;
    return (int)nonproperPrefix;
}
static const int INDENT_SPACES = 2;
vector<Token> tokenize(const string &raw_text)
{
    cerr << "Tokenize text of length " << raw_text.size() << endl;
    using namespace CharacterType;
    // 1. find tokens findable online (numbers, identifiers, escaped)
    vector<Token> nongluedTokens;
    bool isEscaped = false;
    bool isNumber = false;
    bool isId = false;
    for (unsigned int i=0; i<raw_text.size(); i++) {
        char c = raw_text[i];
        Type type;
        if (isalpha(c)) type = ALPHA;
        else if (isdigit(c)) type = NUMERIC;
        else switch (c) {
        case '>':
        case '<':
        case '=':
        case '+':
        case '-':
        case '*':
        case '/':
        case '^':
        case '|':
        case '&':
            type = OPERATOR;
            break;
        case '\'':
        case '"':
            type = QUOTE;
            break;
        case '(':
        case '[':
        case '{':
            type = OPEN_BRACKET;
            break;
        case ')':
        case ']':
        case '}':
            type = CLOSE_BRACKET;
            break;
        case '\t':
            type = INDENT;
            break;
        case ' ':
        case '\r':
        case '\n':
            type = WHITESPACE;
            break;
        default:
            type = OTHER;
        }

        bool startNewToken = true;
        if (isEscaped) {
            isEscaped = false;
            startNewToken = false;
        }
        else if (isNumber) {
            if (type == ALPHA || type == NUMERIC || c == '.') {
                startNewToken = false;
            }
            else {
                isNumber = false;
                startNewToken = true;
            }
        }
        else if (isId) {
            if (type == ALPHA || type == NUMERIC || c == '_') {
                startNewToken = false;
            }
            else {
                isId = false;
                startNewToken = true;
            }
        }
        else {
            startNewToken = true;
        }

        // cerr << "Process char \'" << c << "\', type = " << static_cast<int>(type) << " start new? " << startNewToken << endl;

        if (!startNewToken) {
            nongluedTokens.back().length ++;
            continue; // nothing more to do here
        }

        Token newToken(i, 1, type);
        if (c == '\\')
            isEscaped = true;
        if (type == ALPHA || c == '_')
            isId = true;
        if (type == NUMERIC)
            isNumber = true;
        nongluedTokens.push_back(newToken);
    }

    // 2. glue together some strings
    vector<GlueRule> glueStrings = {
        "<=",
        ">=",
        "==",
        ">>",
        "<<",
        ">>=",
        "<<=",
        "+=",
        "-=",
        "*=",
        "/=",
        "&=",
        "^=",
        "|=",
        "++",
        "--", // still operators...
        "::",
        {"//",OTHER},
        {"/*",OPEN_BRACKET},
        {"*/",CLOSE_BRACKET}
    };
    sort(glueStrings.begin(), glueStrings.end());

    vector<Token> gluedTokens;
    for (unsigned int i=0; i<nongluedTokens.size(); i++) {
        int start = nongluedTokens[i].start;
        cerr << "Process token " << start << ": \'" << raw_text.substr(start, nongluedTokens[i].length) << "\'" << endl;
        GlueRule goodRule;
        bool success = false;
        for (int j=0; j<(int)glueStrings.size(); j++) {
            string str = glueStrings[j].glueString;
            if (str.size()+start > raw_text.size()) continue;
            if (std::mismatch(str.begin(), str.end(), raw_text.begin()+start).first == str.end()) {
                goodRule = glueStrings[j];
                success = true;
                break;
            }
        }

        if (success) {
            Type newType = goodRule.newType == NOTYPE ? nongluedTokens[i].type : goodRule.newType;
            unsigned int sumLength = nongluedTokens[i].length;
            while (sumLength < goodRule.glueString.size())
            {
                i ++;
                sumLength += nongluedTokens[i].length;
            }
            if (sumLength != goodRule.glueString.size()) {
                cerr << "FAIL. Sum length = " << sumLength << ", glue rule = \'" << goodRule.glueString << "\'" << endl;
            }
            gluedTokens.push_back(Token(start, sumLength, newType));
            // i has already been changed
        }
        else {
            gluedTokens.push_back(nongluedTokens[i]);
        }
    }
    
#define TOKEN_VALUE(token) raw_text.substr((token).start, (token).length)
    
    vector<Token> withIndent;
    for (unsigned int i=0; i<gluedTokens.size(); i++) {
      withIndent.push_back(gluedTokens[i]);
      if (TOKEN_VALUE(gluedTokens[i]) == "\n") {
        while (true)
        {
          bool nextIndent = true;
          for (int j=0; j<INDENT_SPACES; j++) {
            if (TOKEN_VALUE(gluedTokens[i+1+j]) != " ") {
              nextIndent = false;
              break;
            }
          }
          
          if (!nextIndent) break;
          
          withIndent.push_back(Token(gluedTokens[i+1].start, INDENT_SPACES, INDENT));
          i += INDENT_SPACES;
        }
      }
    } 
    
    return withIndent;

}







#endif // TOKENIZE_H

