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

#include <memory>

#include <boost/lexical_cast.hpp>

namespace regen
{
    /*
        <RE>	::=	<union> | <simple-RE>
        <union>	::=	<RE> "|" <simple-RE>
        <simple-RE>	::=	<concatenation> | <basic-RE>
        <concatenation>	::=	<simple-RE> <basic-RE>
        <basic-RE>	::=	<star> | <plus> | <elementary-RE>
        <star>	::=	<elementary-RE> "*"
        <plus>	::=	<elementary-RE> "+"
        <elementary-RE>	::=	<group> | <any> | <eos> | <char> | <set>
        <group>	::=	"(" <RE> ")"
        <any>	::=	"."
        <eos>	::=	"$"
        <char>	::=	any non metacharacter | "\" metacharacter
        <set>	::=	<positive-set> | <negative-set>
        <positive-set>	::=	"[" <set-items> "]"
        <negative-set>	::=	"[^" <set-items> "]"
        <set-items>	::=	<set-item> | <set-item> <set-items>
        <set-items>	::=	<range> | <char>
        <range>	::=	<char> "-" <char>


        <<< without left recursion >>>

        <RE>	::=	<simple-RE> <RE'>
        <RE'>	::=	"|" <simple-RE> <RE'> | Epsilon
        
        <simple-RE>	::=	<basic-RE> <simple-RE'>
        <simple-RE'>::=	<basic-RE> <simple-RE'> | Epsilon

        <basic-RE>	::=	<star> | <plus> | <question> | <elementary-RE>
        <star>	::=	<elementary-RE> "*"
        <plus>	::=	<elementary-RE> "+"
        <question>	::=	<elementary-RE> "?"
        <numericRange>	::=	<elementary-RE> "{" <integer> "} | <elementary-RE> "{" <integer> "," "}" | <elementary-RE> "{" <integer> "," <integer> "}"
        <elementary-RE>	::=	<group> | <any> | <eos> | <char> | <set>
        <group>	::=	"(" <RE> ")"
        <any>	::=	"."
        <eos>	::=	"$"
        <char>	::=	any non metacharacter | "\" metacharacter
        <set>	::=	<positive-set> | <negative-set>
        <positive-set>	::=	"[" <set-items> "]"
        <negative-set>	::=	"[^" <set-items> "]"
        <set-items>	::=	<set-item> | <set-item> <set-items>
        <set-item>	::=	<range> | <char>
        <range>	::=	<char> "-" <char>
    */
    struct SimpleRe;

    struct Re
    {
        std::vector<SimpleRe> unionRes;
    };

    struct SetItem
    {
        virtual ~SetItem() {};
    };

    struct Range : public SetItem
    {
        Range( char s, char e ) : start( s ), end( e ) {}
        char start;
        char end;
    };

    struct CharItem : public SetItem
    {
        CharItem( char c ) : c( c ) {}
        char c;
    };

    struct BasicReSub
    {
        virtual ~BasicReSub() {};
    };

    struct ElementaryRe : public BasicReSub
    {
        virtual ~ElementaryRe() {};
    };

    struct Star : public BasicReSub
    {
        Star(std::unique_ptr<ElementaryRe>&& er) : re(std::move(er)) {}
        std::unique_ptr<ElementaryRe> re;
    };

    struct Plus : public BasicReSub
    {
        Plus(std::unique_ptr<ElementaryRe>&& er) : re(std::move(er)) {}
        std::unique_ptr<ElementaryRe> re;
    };

    struct Question : public BasicReSub
    {
        Question(std::unique_ptr<ElementaryRe>&& er) : re(std::move(er)) {}
        std::unique_ptr<ElementaryRe> re;
    };

    struct NumericRange : public BasicReSub
    {
        NumericRange(std::unique_ptr<ElementaryRe>&& er, std::size_t mi, std::size_t ma)
        : re(std::move(er)), min(mi), max(ma)
        {}

        std::unique_ptr<ElementaryRe> re;
        std::size_t min;
        std::size_t max;
    };

    struct Group : public ElementaryRe
    {
        Re re;
    };

    struct Any : public ElementaryRe
    {
    };

    struct Char : public ElementaryRe
    {
        Char( char c ) : c( c ) {}
        char c;
    };

    struct Set : public ElementaryRe
    {
        bool negative = false;
        std::vector<std::unique_ptr<SetItem>> items;
    };

    struct BasicRe
    {
        std::unique_ptr<BasicReSub> sub;
    };

    struct SimpleRe
    {
        std::vector<BasicRe> concatRes;
    };

    class Parser
    {
    public:
        Re parse( TokenList& tokens )
        {
            return parseRe( tokens );
        }

        Set parseStandAloneSet( TokenList& tokens )
        {
            return Set( std::move( *parseSet( tokens ) ) );
        }

    private:
        std::unique_ptr<Group> parseGroup( TokenList& tokens )
        {
            std::unique_ptr<Group> res = std::make_unique<Group>();

            tokens.eat( "(" );
            res->re = parseRe( tokens );
            tokens.eat( ")" );

            return res;
        }

        std::unique_ptr<SetItem> parsetSetItem( TokenList& tokens )
        {
            // <set-items>	::=	<set-item> | <set-item> <set-items>
            // <set-items>	::=	<range> | <char>
            // <range>	::=	<char> "-" <char>
            std::unique_ptr<SetItem> res;

            if( tokens.peak().type == Token::CHAR && tokens.peak(1).type == Token::MINUS )
            {
                char start = tokens.eat().data;
                tokens.eat();
                char end = tokens.eat().data;

                if( end < start )
                    throw std::runtime_error( "Invalid range: " + std::string(1, start) + "-" + std::string(1, end) );

                res = std::make_unique<Range>( start, end );
            }
            else
            {
                res = std::make_unique<CharItem>( tokens.eat().data );
            }

            return res;
        }

        std::vector<std::unique_ptr<SetItem>> expandCharClass( TokenList& tokens )
        {
            std::vector<std::unique_ptr<SetItem>> res;

            // \w	A-Za-z0-9_
            // \d   0-9
            // \s   \t\r\n\v\f
            // 
            // \t   \x09
            // \r   \x0d
            // \n   \x0a
            // \v   \x0b
            // \f   \x0c

            auto tok = tokens.eat();

            if( tok.data == 'w' )
            {
                res.push_back( std::make_unique<Range>( 'A', 'Z' ) );
                res.push_back( std::make_unique<Range>( 'a', 'z' ) );
                res.push_back( std::make_unique<Range>( '0', '9' ) );
                res.push_back( std::make_unique<CharItem>( '_' ) );
            }
            else if( tok.data == 'd' )
            {
                res.push_back( std::make_unique<Range>( '0', '9' ) );
            }
            else if( tok.data == 's' )
            {
                res.push_back( std::make_unique<CharItem>( '\t' ) );
                res.push_back( std::make_unique<CharItem>( '\r' ) );
                res.push_back( std::make_unique<CharItem>( '\n' ) );
                res.push_back( std::make_unique<CharItem>( '\v' ) );
                res.push_back( std::make_unique<CharItem>( '\f' ) );
            }
            else if( tok.data == 't' )
                res.push_back( std::make_unique<CharItem>( '\t' ) );
            else if( tok.data == 'r' )
                res.push_back( std::make_unique<CharItem>( '\r' ) );
            else if( tok.data == 'n' )
                res.push_back( std::make_unique<CharItem>( '\n' ) );
            else if( tok.data == 'v' )
                res.push_back( std::make_unique<CharItem>( '\v' ) );
            else if( tok.data == 'f' )
                res.push_back( std::make_unique<CharItem>( '\f' ) );

            return res;
        }

        std::unique_ptr<Set> parseSet( TokenList& tokens )
        {
            // <set>	::=	<positive-set> | <negative-set>
            // <positive-set>	::=	"[" <set-items> "]"
            // <negative-set>	::=	"[^" <set-items> "]"
            
            std::unique_ptr<Set> res = std::make_unique<Set>();

            tokens.eat( "[" );

            if( tokens.peak().type == Token::HAT )
            {
                res->negative = true;
                tokens.eat( "^" );
            }

            if( tokens.peak().type != Token::CHAR && tokens.peak().type != Token::CHARCLASS )
                throw std::runtime_error( "Expected <" + token2str(Token::CHAR) + "> or <" + token2str(Token::CHARCLASS) + "> got <" + token2str(tokens.peak().type) + ">" );

            while( tokens.peak().type != Token::CBRACKET )
            {
                if( tokens.peak().type == Token::CHAR )
                    res->items.push_back( parsetSetItem( tokens ) );
                else // CHARCLASS
                {
                    auto vRanges = expandCharClass( tokens );
                    std::move(vRanges.begin(), vRanges.end(), std::back_inserter(res->items));
                }
            }

            tokens.eat( "]" );

            return res;
        }

        std::unique_ptr<ElementaryRe> parseElementaryRe( TokenList& tokens )
        {
            // <elementary-RE>	::=	<group> | <any> ( | <eos> ) | <char> | <set>

            std::unique_ptr<ElementaryRe> res;

            if( tokens.peak().type == Token::OPAREN )
            {
                res = parseGroup( tokens );
            }
            else if( tokens.peak().type == Token::DOT )
            {
                tokens.eat();
                res = std::make_unique<Any>();
            }
            else if( tokens.peak().type == Token::OBRACKET )
            {
                res = parseSet( tokens );
            }
            else if( tokens.peak().type == Token::CHARCLASS )
            {
                auto set = std::make_unique<Set>();
                set->items = expandCharClass( tokens );
                res = std::move( set );
            }
            else if( tokens.peak().type == Token::CHAR || tokens.peak().type == Token::MINUS )
            {
                res = std::make_unique<Char>( tokens.eat().data );
            }
            else
            {
                throw std::logic_error( "Expected <" + token2str(Token::OPAREN) + "> or <"
                                                    + token2str(Token::DOT) + "> or <"
                                                    + token2str(Token::OBRACKET) + "> or <"
                                                    + token2str(Token::CHARCLASS) + "> or <"
                                                    + token2str(Token::CHAR) + ">" );
            }

            return res;
        }

        int readInteger( TokenList& tokens )
        {
            std::string str;
            while( tokens.peak().type == Token::CHAR && tokens.peak().data >= '0' && tokens.peak().data <= '9' )
                str += std::string(1,tokens.eat().data);
            
            try
            {
                return boost::lexical_cast<int>( str );
            }
            catch( ... )
            {
                throw std::runtime_error( "Expected <integer>" );
            }
        }

        BasicRe parseBasicRe( TokenList& tokens )
        {
            // <basic-RE>	::=	<star> | <plus> | <elementary-RE>
            // <star>	::=	<elementary-RE> "*"
            // <plus>	::=	<elementary-RE> "+"
            
            BasicRe res;

            auto elementaryRe = parseElementaryRe( tokens );
            if( !tokens.eof() )
            {
                if( tokens.peak().data == '*' )
                {
                    tokens.eat();
                    res.sub = std::make_unique<Star>( std::move(elementaryRe) );
                }
                else if( tokens.peak().data == '+' )
                {
                    tokens.eat();
                    res.sub = std::make_unique<Plus>( std::move(elementaryRe) );
                }
                else if( tokens.peak().data == '?' )
                {
                    tokens.eat();
                    res.sub = std::make_unique<Question>( std::move(elementaryRe) );
                }
                else if( tokens.peak().data == '{' )
                {
                    tokens.eat();
                    int min, max;
                    min = readInteger( tokens );
                    max = min;
                    if( tokens.peak().type == Token::CHAR && tokens.peak().data == ',' )
                    {
                        tokens.eat();
                        if( tokens.peak().type == Token::CSB && tokens.peak().data == '}' )
                            max = min + 5;
                        else
                        {
                            max = readInteger( tokens );
                        }
                    }

                    auto tok = tokens.eat( "}" );
                    if( tok.data != '}' )
                        throw std::runtime_error( "expected <" + token2str(Token::CSB) + "> got <" + token2str(tok.type) + ">" );

                    res.sub = std::make_unique<NumericRange>( std::move(elementaryRe), min, max );
                }
                else
                    res.sub = std::move(elementaryRe);
            }
            else
                res.sub = std::move(elementaryRe);

            return res;
        }

        SimpleRe parseSimpleRe( TokenList& tokens )
        {
            SimpleRe res;

            res.concatRes.push_back( parseBasicRe(tokens) );

            while( !tokens.eof() )
            {
                try
                {
                    res.concatRes.push_back( parseBasicRe(tokens) );
                }
                catch( ... )
                {
                    break;
                }
            }

            return res;
        }

        Re parseRe( TokenList& tokens )
        {
            Re res;

            res.unionRes.push_back( parseSimpleRe(tokens) );

            while( !tokens.eof() && tokens.peak().type == Token::PIPE )
            {
                tokens.eat();
                res.unionRes.push_back( parseSimpleRe(tokens) );
            }

            if( !tokens.eof() && tokens.peak().type != Token::CPAREN ) // hack
                throw std::runtime_error( "invalid regex caused parsing to stop prematurely" );

            return res;
        }
    };
}