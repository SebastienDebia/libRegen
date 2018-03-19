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

#include "regen/Lexer.hpp"
#include "regen/Parser.hpp"
#include "regen/Generator.hpp"

std::string generate( const std::string& regextr,
            std::size_t repetition_max = 5,
            const std::string& restricted_range = "" )
{
    std::cout << regextr << "\n";

    auto tokens = regen::lexer(regextr);
    
    std::string res;
    try
    {
        auto regex = regen::Parser().parse( tokens );
        res = regen::Generator(regex, repetition_max, restricted_range).generate();
    }
    catch( std::runtime_error& ex )
    {
        std::cout << "Error: " << ex.what() << "\n";
    }
    catch( std::logic_error& ex )
    {
        std::cout << "Logic error: " << ex.what() << "\n";
    }

    return res;
}

std::string test( const std::string& regex )
{
    std::stringstream sstr;
    sstr << regex << "\n" << generate( regex ) << "\n";
    return sstr.str();
}

int main( void )
{
    std::cout << generate( R"(1?[0-9][0-9]\.1?[0-9][0-9]\.1?[0-9][0-9]\.1?[0-9][0-9])" ) << std::endl;
    std::cout << generate( R"(.*[0-9a-fA-F]+)" ) << std::endl;
    std::cout << generate( R"(([A-Z][a-z]+ )([a-z]+ )+[A-Z][a-z]+\.)" ) << std::endl;
    std::cout << generate( R"(([A-Z]{1}[a-z]{3,5} )([a-z]{2,} )+[a-z]{3,6}\.)" ) << std::endl;
    std::cout << generate( R"((([A-Z]{1}[a-z]{3,5} )([a-z]{2,} )+[a-z]{3,6}\.|a|bb|ccc|dddd)|111|222|333|444|555)" ) << std::endl;
    std::cout << generate( R"(a{12})" ) << std::endl;
    std::cout << generate( R"(ex-(a?e|æ|é)quo)" ) << std::endl;
    std::cout << generate( R"(([A-Z]\w+\s){5,7})" ) << std::endl;
    std::cout << generate( R"(([A-Z]\w+\x20){5,7})" ) << std::endl;
    std::cout << generate( R"([^a-z]{20})" ) << std::endl;

    std::cout << generate( R"(.+)" ) << std::endl;
    std::cout << generate( R"(.+)", 20 ) << std::endl;
    std::cout << generate( R"(.+)", 20, R"([A-Z])" ) << std::endl;

    return 0;
}