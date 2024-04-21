#include "tokenizer.h"
#include <iostream>
#include <algorithm>
#include <limits>
#include <cctype>

namespace ejson {

    TokenizerFeedback Tokenizer::tokenize(const std::string &input_file,
                                          ListOfTokenizedPairs &list_of_tokenized_pairs)
    {
        // Initialize tokenization
        ListOfFiles list_of_files{};
        TokenizerFeedback feedback{};
        initialize(input_file, list_of_files, feedback);

        // Continue to generate tokens if there were no initialization errors.
        if(feedback.type == FeedbackType::OK)
        {
            std::for_each(std::begin(list_of_files),
                            std::end(list_of_files),
                            [this, &list_of_tokenized_pairs, &feedback] (File &file)
            {
                if(feedback.type == FeedbackType::OK)
                {
                list_of_tokenized_pairs.emplace_back(TokenizedPairs{});
                generateTokens(file, list_of_tokenized_pairs.back(), feedback);
                }
            });
        }

        // Cleanup
        cleanup(list_of_files);
        
        return feedback;
    }

    void Tokenizer::generateTokens(File &file, TokenizedPairs &pairs,
                                   TokenizerFeedback &feedback)
    {
        feedback.type = FeedbackType::OK;
        feedback.file = file.path;
        Scope scope{ScopeType::SCOPE_EMPTY, ScopeType::SCOPE_EMPTY};
        std::string line{""};
        pushStack(scope.current);

        while (file.stream.is_open() && !file.stream.eof() && (feedback.type == FeedbackType::OK))
        {
            std::getline(file.stream >> std::ws, line);

            if(!line.empty())
            {
                // Check for and remove comments in the line if any
                std::size_t first_comment_position = line.find_first_of((char) Token::COMMENT);
                if(first_comment_position < line.npos)
                    line.erase(first_comment_position);

                if(!line.empty())
                {
                    std::stringstream stream {line};

                    while (!stream.eof() && (feedback.type == FeedbackType::OK))
                    {
                        switch (scope.current)
                        {
                        case ScopeType::SCOPE_EMPTY:
                            scopeEmpty(scope, stream, pairs, feedback);
                            break;
                        case ScopeType::SCOPE_ARRAY:
                            scopeArray(scope, stream, pairs, feedback);
                            break;
                        case ScopeType::SCOPE_OBJECT:
                            scopeObject(scope, stream, pairs, feedback);
                            break;
                        case ScopeType::SCOPE_KEY:
                            scopeKey(scope, stream, pairs, feedback);
                            break;
                        case ScopeType::SCOPE_STRING:
                            scopeString(scope, stream, pairs, feedback);
                            break;
                        case ScopeType::SCOPE_NUMBER:
                            scopeNumber(scope, stream, pairs, feedback);
                            break;
                        case ScopeType::SCOPE_LITERAL:
                            scopeLiteral(scope, stream, pairs, feedback);
                            break;
                        default:
                            break;
                        }
                    }
                }
            }
        }
        popStack();

        // Clean up feedback before returning
        if (feedback.type == FeedbackType::OK)
            feedback.file = feedback.snap = std::string{""};
    }
    
    void Tokenizer::pushStack(ScopeType scope_type)
    {
        _last_begun.push(scope_type);
    }

    void Tokenizer::popStack()
    {
        _last_begun.pop();
    }
    
    ScopeType Tokenizer::stackTop()
    {
        return _last_begun.top();
    }
    
    void Tokenizer::cleanup(ListOfFiles& list_of_files)
    {
        std::for_each(std::begin(list_of_files), std::end(list_of_files), [] (File& file) {
            file.stream.close();
        });
    }
}