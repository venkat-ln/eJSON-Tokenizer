#include "tokenizer.h"
#include <iostream>
#include <algorithm>
#include <limits>
#include <cctype>

namespace ejson
{
    bool Tokenizer::transitionRulesApplied (Scope &scope, const Token &token)
    {
        bool transition_allowed{true};
        ScopeType last_scope = scope.previous;
        switch (scope.current)
        {

        case ScopeType::SCOPE_EMPTY:
            switch (token)
            {
            case Token::ARRAY_BEGIN:
                scope.previous = scope.current;
                scope.current = ScopeType::SCOPE_ARRAY;
                pushStack(ScopeType::SCOPE_ARRAY);
                break;
            case Token::OBJECT_BEGIN:
                scope.previous = scope.current;
                scope.current = ScopeType::SCOPE_OBJECT;
                pushStack(ScopeType::SCOPE_OBJECT);
                break;
            default:
                transition_allowed = false;
                break;
            }
            break;

        case ScopeType::SCOPE_ARRAY:
            switch (token)
            {
            case Token::ARRAY_BEGIN:
                scope.previous = scope.current;
                scope.current = ScopeType::SCOPE_ARRAY;
                pushStack(ScopeType::SCOPE_ARRAY);
                break;
            case Token::OBJECT_BEGIN:
                scope.previous = scope.current;
                scope.current = ScopeType::SCOPE_OBJECT;
                pushStack(ScopeType::SCOPE_OBJECT);
                break;
            case Token::STRING_DELIMITER:
                scope.previous = scope.current;
                scope.current = ScopeType::SCOPE_STRING;
                pushStack(ScopeType::SCOPE_STRING);
                break;
            case Token::NUMBER:
                scope.previous = scope.current;
                scope.current = ScopeType::SCOPE_NUMBER;
                pushStack(ScopeType::SCOPE_NUMBER);
                break;
            case Token::LITERAL:
                scope.previous = scope.current;
                scope.current = ScopeType::SCOPE_LITERAL;
                pushStack(ScopeType::SCOPE_LITERAL);
                break;
            case Token::ARRAY_END:
                scope.previous = scope.current;
                popStack();
                scope.current = stackTop();
                break;
            default:
                transition_allowed = false;
                break;
            }
            break;

        case ScopeType::SCOPE_OBJECT:
            switch (token)
            {
            case Token::STRING_DELIMITER:
                scope.previous = scope.current;
                scope.current = ScopeType::SCOPE_KEY;
                break;
            case Token::OBJECT_END:
                scope.previous = scope.current;
                popStack();
                scope.current = stackTop();
                break;
            default:
                transition_allowed = false;
                break;
            }
            break;

        case ScopeType::SCOPE_KEY:
            switch (token)
            {
            case Token::ARRAY_BEGIN:
                scope.previous = scope.current;
                scope.current = ScopeType::SCOPE_ARRAY;
                pushStack(ScopeType::SCOPE_ARRAY);
                break;
            case Token::OBJECT_BEGIN:
                scope.previous = scope.current;
                scope.current = ScopeType::SCOPE_OBJECT;
                pushStack(ScopeType::SCOPE_OBJECT);
                break;
            case Token::STRING_DELIMITER:
                scope.previous = scope.current;
                scope.current = ScopeType::SCOPE_STRING;
                pushStack(ScopeType::SCOPE_STRING);
                break;
            case Token::NUMBER:
                scope.previous = scope.current;
                scope.current = ScopeType::SCOPE_NUMBER;
                pushStack(ScopeType::SCOPE_NUMBER);
                break;
            case Token::LITERAL:
                scope.previous = scope.current;
                scope.current = ScopeType::SCOPE_LITERAL;
                pushStack(ScopeType::SCOPE_LITERAL);
                break;
            default:
                transition_allowed = false;
                break;
            }
            break;
        
        case ScopeType::SCOPE_STRING:
        case ScopeType::SCOPE_NUMBER:
        case ScopeType::SCOPE_LITERAL:
            switch (token)
            {
            case Token::VALUE_END:
                if(scope.previous == ScopeType::SCOPE_ARRAY)
                {
                    scope.previous = scope.current;
                    scope.current = ScopeType::SCOPE_ARRAY;
                }
                else
                {
                    scope.previous = scope.current;
                    scope.current = ScopeType::SCOPE_OBJECT;
                }
                popStack();
                break;
            case Token::ARRAY_END:
                scope.previous = scope.current;
                if (last_scope == ScopeType::SCOPE_ARRAY)
                    scope.current = last_scope;
                else
                    scope.current = ScopeType::SCOPE_OBJECT;
                popStack();
                break;
            default:
                transition_allowed = false;
                break;
            }
            break;
        
        default:
            transition_allowed = false;
            break;
        }

        return transition_allowed;
    }
}