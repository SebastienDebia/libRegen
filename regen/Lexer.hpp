/**
 * MIT License
 * 
 * Copyright (c) 2018 Sébastien Débia
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
*/

#pragma once
#include <string>
#include <vector>
#include <set>
#include <sstream>
#include <iostream>

namespace regen
{
    struct Token
    {
        enum EType
        {
            CHAR,
            DOT,
            STAR,
            PLUS,
            MINUS,
            QUESTION,
            PIPE,
            OPAREN,
            CPAREN,
            OBRACKET, // [
            CBRACKET, // ]
            OSB,      // {
            CSB,      // }
            HAT,      // ^
            CHARCLASS
        };

        EType type;
        char data;
    };

    inline std::string token2str( Token::EType type )
    {
        if( type == Token::DOT )
            return ".";
        else if( type == Token::STAR )
            return "*";
        else if( type == Token::PLUS )
            return "+";
        else if( type == Token::MINUS )
            return "-";
        else if( type == Token::QUESTION )
            return "?";
        else if( type == Token::PIPE )
            return "|";
        else if( type == Token::OPAREN )
            return "opening parenthesis";
        else if( type == Token::CPAREN )
            return "closing parenthesis";
        else if( type == Token::OBRACKET )
            return "opening bracket";
        else if( type == Token::CBRACKET )
            return "closing bracket";
        else if( type == Token::OSB )
            return "opening brace";
        else if( type == Token::CSB )
            return "closing brace";
        else if( type == Token::HAT )
            return "^";
        else if( type == Token::CHARCLASS )
            return "character class";
        else
            return "char";

        throw std::logic_error("unknown token type");
    }

    inline Token::EType charToToken( char c )
    {
        if( c == '.' )
            return Token::DOT;
        else if( c == '*' )
            return Token::STAR;
        else if( c == '+' )
            return Token::PLUS;
        else if( c == '-' )
            return Token::MINUS;
        else if( c == '?' )
            return Token::QUESTION;
        else if( c == '|' )
            return Token::PIPE;
        else if( c == '(' )
            return Token::OPAREN;
        else if( c == ')' )
            return Token::CPAREN;
        else if( c == '[' )
            return Token::OBRACKET;
        else if( c == ']' )
            return Token::CBRACKET;
        else if( c == '{' )
            return Token::OSB;
        else if( c == '}' )
            return Token::CSB;
        else if( c == '^' )
            return Token::HAT;
        else
            return Token::CHAR;

        throw std::runtime_error("unknown token '" + std::string(1, c) + "'");
    }

    class TokenList
    {
    public:
        typedef std::vector<Token> token_list_t;
        typedef token_list_t::const_iterator const_iterator;
        typedef token_list_t::const_reverse_iterator const_reverse_iterator;

        void push_back( const Token& token )
        {
            m_tokens.push_back( token );
        }

        void push_back( Token&& token )
        {
            m_tokens.push_back( token );
        }

        const Token& peak( std::size_t n = 0 ) const
        {
            if( eof(n) )
                throw std::runtime_error( "Expected token got <eof>" );
            return m_tokens[m_i+n];
        }

        Token eat( const std::string& expected = "token" )
        {
            if( eof() )
            {
                throw std::runtime_error( "Expected '" + expected + "' got <eof>" );
            }

            return m_tokens[m_i++];
        }

        bool eof( std::size_t offset = 0) const
        {
            return m_i+offset >= m_tokens.size();
        }

        const_iterator begin() const {return m_tokens.begin();}
        const_iterator cbegin() const {return m_tokens.cbegin();}
        const_iterator end() const {return m_tokens.end();}
        const_iterator cend() const {return m_tokens.cend();}
        const_reverse_iterator rbegin() const {return m_tokens.rbegin();}
        const_reverse_iterator crbegin() const {return m_tokens.crbegin();}
        const_reverse_iterator rend() const {return m_tokens.rend();}
        const_reverse_iterator crend() const {return m_tokens.crend();}

    private:
        token_list_t m_tokens;
        std::size_t m_i = 0;
    };

    inline char readHexChar( std::string str )
    {
        str = "0" + str; // 0x..

        char res = '0';

        int num;
        std::stringstream ss;
        ss << std::hex << str;
        ss >> num;

        if( ss.fail() )
        {
            str[0] = '\\';
            throw std::runtime_error( "Error parsing hex character: '" + str + "'" );
        }

        res = static_cast<char>( num );

        return res;
    }

    inline TokenList lexer( const std::string& str )
    {
        static const std::set<char> charClassesSet = { 'w', 'd', 's', 't', 'r', 'n', 'v', 'f' };
        TokenList res;

        std::size_t i = 0;
        std::size_t startOfChar = std::string::npos;

        while( i < str.size() )
        {
            if( str[i] == '\\' && i+1 < str.size() )
            {
                ++i;

                if( charClassesSet.count( str[i] ) )
                    res.push_back( Token{Token::CHARCLASS, str[i]} );
                else if( str[i] == 'x' )
                    res.push_back( Token{Token::CHAR, readHexChar(str.substr(i, 3))} );
                else
                    res.push_back( Token{Token::CHAR, str[i]} );

                ++i;
                continue;
            }

            auto tokenType = charToToken( str[i] );

            res.push_back( Token{tokenType, str[i]} );
            
            ++i;
        }

        return res;
    }
}