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

#include "Lexer.hpp"
#include "Parser.hpp"
#include "Generator.hpp"

namespace regen
{
    /**
     * generates a random string matching the given regular expression
     * 
     * @param regex regular expression
     * @param repetition_max max number of repetitions for + and *
     *                       defaults to 5
     * @param restricted_range range of characters that can be generated
     *                         given in regex notation e.g. "[a-zA-Z]"
     * 
     * @return the generated string
     */
    inline std::string generate( const std::string& regextr,
                std::size_t repetition_max = 5,
                const std::string& restricted_range = "" )
    {
        auto tokens = lexer( regextr );
        auto regex = Parser().parse( tokens );
        return Generator( repetition_max, restricted_range ).generate( regex );
    }
}