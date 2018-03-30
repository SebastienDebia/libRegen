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

#include "regen/regen.hpp"

static const std::size_t s_default_rep_max = 5;

void test( const std::string& regex,
            std::size_t repetition_max = s_default_rep_max,
            const std::string& restricted_range = "" )
{
    std::cout << regex;
    if( repetition_max != s_default_rep_max )
        std::cout << " {max rep: " << repetition_max << "}";
    if( !restricted_range.empty() )
        std::cout << " {restricted range: " << restricted_range << "}";
    std::cout << "\n";

    try
    {
        std::cout << regen::generate( regex, regen::Generator( repetition_max, restricted_range ) ) << "\n";
    }
    catch( std::runtime_error& ex )
    {
        std::cerr << "Error: " << ex.what() << "\n";
    }
    catch( std::logic_error& ex )
    {
        std::cerr << "Logic error: " << ex.what() << "\n";
    }

    std::cout << std::endl;
}

int main( void )
{
    test( R"(1?[0-9][0-9]\.1?[0-9][0-9]\.1?[0-9][0-9]\.1?[0-9][0-9])" );
    test( R"(.*[0-9a-fA-F]+)" );
    test( R"(([A-Z][a-z]+ )([a-z]+ )+[A-Z][a-z]+\.)" );
    test( R"(([A-Z]{1}[a-z]{3,5} )([a-z]{2,} )+[a-z]{3,6}\.)" );
    test( R"((([A-Z]{1}[a-z]{3,5} )([a-z]{2,} )+[a-z]{3,6}\.|a|bb|ccc|dddd)|111|222|333|444|555)" );
    test( R"(a{12})" );
    test( R"(ex-(a?e|æ|é)quo)" );
    test( R"(([A-Z]\w+\s){5,7})" );
    test( R"(([A-Z]\w+\x20){5,7})" );
    test( R"([^a-z]{20})" );

    test( R"(.+)" );
    test( R"(.+)", 20 );
    test( R"(.+)", 20, R"([A-Z])" );

    return 0;
}