#ifndef EJSON_TOKENIZER_H
#define EJSON_TOKENIZER_H

#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <stack>

/* ejson library namespace */
namespace ejson
{

    /* Types and datastructures for file-handling */
    typedef std::string FileName;
    struct File
    {
        const std::string path;
        std::ifstream stream;
    };
    typedef std::vector<File> ListOfFiles;

    /* Types and datastructures for token-handling */
    enum ScopeType
    {
        SCOPE_EMPTY,
        SCOPE_ARRAY,
        SCOPE_OBJECT,
        SCOPE_KEY,
        SCOPE_STRING,
        SCOPE_NUMBER,
        SCOPE_LITERAL
    };
    struct Scope
    {
        ScopeType previous {ScopeType::SCOPE_EMPTY};
        ScopeType current {ScopeType::SCOPE_EMPTY};
    };      
    enum Token : char
    {
        UNDEFINED                   = 'U',
        COMMENT                     = '#',
        OBJECT_BEGIN                = '{',
        OBJECT_END                  = '}',
        ARRAY_BEGIN                 = '[',
        ARRAY_END                   = ']',
        KEY_END                     = ':',
        VALUE_END                   = ',',
        KEY                         = 'K',
        NUMBER                      = 'N',
        LITERAL                     = 'L',
        STRING                      = 'S',
        LITERAL_NULL                = 'n',
        LITERAL_TRUE                = 't',
        LITERAL_FALSE               = 'f',
        PATH_SEPARATOR              = '/',
        STRING_DELIMITER            = '"',
        BLANK_SPACE                 = ' ',
        NEW_LINE                    = '\n'
    };
    typedef std::vector<Token> Tokens;
    struct TokenizedPair
    {
        Token token {Token::UNDEFINED};
        std::string value {""};
    };
    typedef std::vector<TokenizedPair> TokenizedPairs;
    typedef std::vector<TokenizedPairs> ListOfTokenizedPairs;

    /* Types and datastructures for feedback-handling */
    enum FeedbackType
    {
        OK,
        NOK_FILE_ERROR,
        NOK_PARSER_ERROR
    };
    struct TokenizerFeedback
    {
        FeedbackType type {FeedbackType::OK};
        std::string file {""};
        std::string snap {""};
    };

    /* Principal class for the ejson tokenizer */
    class Tokenizer
    {
    private:
        std::stack<ScopeType> _last_begun{};
        void initialize(const std::string &, ListOfFiles &, TokenizerFeedback &);
        void resolveImportStatements(const std::string &, std::ifstream &, std::vector<std::string> &, TokenizerFeedback &);
        void checkForAndParseImportStatement(const std::string &, const std::string &, std::vector<std::string> &, TokenizerFeedback &);
        void generateTokens(File &, TokenizedPairs &, TokenizerFeedback &);
        void scopeEmpty(Scope &, std::stringstream &, TokenizedPairs &, TokenizerFeedback &);
        void scopeArray(Scope &, std::stringstream &, TokenizedPairs &, TokenizerFeedback &);
        void scopeNumber(Scope &, std::stringstream &, TokenizedPairs &, TokenizerFeedback &);
        void scopeString(Scope &, std::stringstream &, TokenizedPairs &, TokenizerFeedback &);
        void scopeLiteral(Scope &, std::stringstream &, TokenizedPairs &, TokenizerFeedback &);
        void scopeObject(Scope &, std::stringstream &, TokenizedPairs &, TokenizerFeedback &);
        void scopeKey(Scope &, std::stringstream &, TokenizedPairs &, TokenizerFeedback &);
        bool transitionRulesApplied (Scope &, const Token &);
        void pushStack(ScopeType);
        void popStack();
        ScopeType stackTop();
        void cleanup(ListOfFiles &);

    public:
        TokenizerFeedback tokenize(const std::string &, ListOfTokenizedPairs &);
    };
}

#endif