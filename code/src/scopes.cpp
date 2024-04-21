#include "tokenizer.h"
#include <iostream>
#include <algorithm>
#include <limits>
#include <cctype>

namespace ejson
{
    static const std::string LITERAL_STATEMENT_NULL = std::string{"null"};
    static const std::string LITERAL_STATEMENT_TRUE = std::string{"true"};
    static const std::string LITERAL_STATEMENT_FALSE = std::string{"false"};

    void Tokenizer::scopeEmpty(Scope &scope,
                               std::stringstream &stream,
                               TokenizedPairs &pairs,
                               TokenizerFeedback &feedback)
    {
        char token{(char) Token::UNDEFINED};
        (stream >> std::ws).get(token);
        if(token != (char) Token::UNDEFINED)
        {
            if(transitionRulesApplied(scope, (Token) token))
            {
                pairs.emplace_back(TokenizedPair{.token = (Token)token,
                                                 .value = std::string{token}});
            }
            else
            {
                feedback.type = FeedbackType::NOK_PARSER_ERROR;
                feedback.snap = stream.str();
            }
        }
    }

    void Tokenizer::scopeArray(Scope &scope,
                               std::stringstream &stream,
                               TokenizedPairs &pairs,
                               TokenizerFeedback &feedback)
    {
        char token{(char) Token::UNDEFINED};
        (stream >> std::ws).get(token);
        if(token != (char) Token::UNDEFINED)
        {
            bool success{true};
            // Check for NUMBER and LITERAL tokens
            if(std::isdigit(token) != 0)
            {
                // Write token back into stream since it is part of value
                stream.putback(token);
                token = (char) Token::NUMBER;
            }
            else
            {
                char separator{(char)Token::UNDEFINED};
                switch(token)
                {
                    case (char) Token::LITERAL_NULL:
                    case (char) Token::LITERAL_TRUE:
                    case (char) Token::LITERAL_FALSE:
                        stream.putback(token);
                        token = (char)Token::LITERAL;
                        break;
                    case (char) Token::ARRAY_END:
                        pairs.emplace_back(TokenizedPair{.token = (Token) token,
                                                         .value = std::string{token}});
                        // Remove value separator at the end of array
                        (stream >> std::ws).get(separator);
                        if (separator != (char)Token::VALUE_END)
                            success = false;
                        break;
                    default:
                        break;
                }
            }
            if(!transitionRulesApplied(scope, (Token) token) || !success)
            {
                feedback.type = FeedbackType::NOK_PARSER_ERROR;
                feedback.snap = stream.str();
            }
        }
    }

    void Tokenizer::scopeNumber(Scope &scope,
                               std::stringstream &stream,
                               TokenizedPairs &pairs,
                               TokenizerFeedback &feedback)
    {
        std::string digits {""};
        char token{(char) Token::UNDEFINED};
        bool stop{false}, success{true};
        while(!stop && !stream.eof())
        {
            (stream >> std::ws).get(token);
            if(token != (char) Token::UNDEFINED)
            {
                if(std::isdigit(token) != 0)
                    digits.push_back(token);
                else
                    stop = true;
            }
        }
        if(!stream.eof())
        {
            pairs.emplace_back(TokenizedPair{.token = Token::NUMBER,
                                             .value = digits});
            if(token == Token::ARRAY_END)
                stream.putback(token);
            success = transitionRulesApplied(scope, (Token) token);
        }
        if(stream.eof() || !success) 
        {
            feedback.type = FeedbackType::NOK_PARSER_ERROR;
            feedback.snap = stream.str();
        }
    }

    void Tokenizer::scopeString(Scope &scope,
                               std::stringstream &stream,
                               TokenizedPairs &pairs,
                               TokenizerFeedback &feedback)
    {
        std::string value {""};
        bool success{true};
        std::getline(stream, value, (char) Token::STRING_DELIMITER);
        if(!stream.eof())
        {
            char token{(char) Token::UNDEFINED};
            (stream >> std::ws).get(token);
            if(token != (char) Token::UNDEFINED)
            {
                pairs.emplace_back(TokenizedPair{.token = Token::STRING,
                                                 .value = value});
                
                if(token == Token::ARRAY_END)
                    stream.putback(token);
                success = transitionRulesApplied(scope, (Token) token);
            }
        }
        if(stream.eof() || !success) 
        {
            feedback.type = FeedbackType::NOK_PARSER_ERROR;
            feedback.snap = stream.str();
        }
    }

    void Tokenizer::scopeLiteral(Scope &scope,
                               std::stringstream &stream,
                               TokenizedPairs &pairs,
                               TokenizerFeedback &feedback)
    {
        std::string literal {""};
        char token{(char) Token::UNDEFINED};
        bool stop{false}, success{true};
        while(!stop && !stream.eof())
        {
            stream.get(token);
            if(token != (char) Token::UNDEFINED)
            {
                if(token == Token::VALUE_END || token == Token::ARRAY_END)
                {
                    stop = true;
                    if(token == Token::ARRAY_END)
                        stream.putback(token);
                }
                else if(token == Token::BLANK_SPACE)
                {
                    while (!stop && !stream.eof())
                    {
                        (stream >> std::ws).get(token);
                        if (token != (char)Token::UNDEFINED)
                        {
                            if (token == Token::VALUE_END || token == Token::ARRAY_END)
                            {
                                stop = true;
                                if (token == Token::ARRAY_END)
                                    stream.putback(token);
                            }
                            else
                                success = false;
                        }
                    }
                }
                else
                    literal.push_back(token);
            }
        }
        if(!stream.eof())
        {
            Token result{Token::UNDEFINED};
            if (literal == LITERAL_STATEMENT_NULL)
                result = Token::LITERAL_NULL;
            else if (literal == LITERAL_STATEMENT_TRUE)
                result = Token::LITERAL_TRUE;
            else if (literal == LITERAL_STATEMENT_FALSE)
                result = Token::LITERAL_FALSE;
            else
                success = false;
            if(success)
            {
                pairs.emplace_back(TokenizedPair{.token = result,
                                                 .value = literal});
                success = transitionRulesApplied(scope, (Token)token);
            }
        }
        if(stream.eof() || !success) 
        {
            feedback.type = FeedbackType::NOK_PARSER_ERROR;
            feedback.snap = stream.str();
        }
    }
    
    void Tokenizer::scopeObject(Scope &scope,
                                std::stringstream &stream,
                                TokenizedPairs &pairs,
                                TokenizerFeedback &feedback)
    {
        char token{(char) Token::UNDEFINED};
        (stream >> std::ws).get(token);
        if(token != (char) Token::UNDEFINED)
        {
            bool success{true};
            if(token == Token::OBJECT_END)
            {
                pairs.emplace_back(TokenizedPair{.token = (Token)token,
                                                 .value = std::string{token}});

                // Remove value separator at the end of array
                char separator{(char) Token::UNDEFINED};
                (stream >> std::ws).get(separator);
                if (separator != (char)Token::UNDEFINED)
                {
                    if (separator != (char)Token::VALUE_END)
                        success = false;
                }
            }
            if(!transitionRulesApplied(scope, (Token) token) || !success)
            {
                feedback.type = FeedbackType::NOK_PARSER_ERROR;
                feedback.snap = stream.str();
            }
        }
    }

    void Tokenizer::scopeKey(Scope &scope,
                             std::stringstream &stream,
                             TokenizedPairs &pairs,
                             TokenizerFeedback &feedback)
    {
        std::string value {""};
        bool success{true};
        std::getline(stream, value, (char) Token::STRING_DELIMITER);
        if(!stream.eof())
        {
            std::string residue {""};
            std::getline(stream >> std::ws, residue, (char) Token::KEY_END);
            if (!stream.eof())
            {
                char token{(char)Token::UNDEFINED};
                (stream >> std::ws).get(token);

                if (token != (char)Token::UNDEFINED)
                {
                    // Check for NUMBER and LITERAL tokens
                    if (std::isdigit(token) != 0)
                    {
                        // Write token back into stream since it is part of value
                        stream.putback(token);
                        token = (char)Token::NUMBER;
                    }
                    else
                    {
                        switch (token)
                        {
                        case (char)Token::LITERAL_NULL:
                        case (char)Token::LITERAL_TRUE:
                        case (char)Token::LITERAL_FALSE:
                            stream.putback(token);
                            token = (char)Token::LITERAL;
                            break;
                        default:
                            break;
                        }
                    }

                    pairs.emplace_back(TokenizedPair{.token = Token::KEY,
                                                     .value = value});
                    success = transitionRulesApplied(scope, (Token)token);
                    if(success)
                    {
                        switch (token)
                        {
                        case (char)Token::OBJECT_BEGIN:
                        case (char)Token::ARRAY_BEGIN:
                            pairs.emplace_back(TokenizedPair{.token = (Token) token,
                                                             .value = std::string{token}});
                            break;
                        default:
                            break;
                        }
                    }
                }
            }
        }
        if(stream.eof() || !success) 
        {
            feedback.type = FeedbackType::NOK_PARSER_ERROR;
            feedback.snap = stream.str();
        }
    }
    
}