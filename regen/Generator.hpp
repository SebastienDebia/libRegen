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

#include <boost/random/mersenne_twister.hpp>
#include <boost/random/uniform_int_distribution.hpp>
#include <ctime>

namespace regen
{
    /**
     * Generates a random string matching the given regular expression
     * 
     * Some parameters of the generator can be configured:
     * - maximum number of repetitions for + and * (default to 5)
     * - range of characters that can be generated
     *   given in regex notation e.g. "[a-zA-Z]"
     * 
     * The generation takes in a Re object which is created using a parser.
     * @see regen::Parser
     */
    class Generator
    {
    public:

        /**
         * generates a random string matching the given regular expression
         * 
         * @param repetition_max max number of repetitions for + and *
         *                       defaults to 5
         * @param restricted_range range of characters that can be generated
         *                         given in regex notation e.g. "[a-zA-Z]"
         */
        Generator( std::size_t repetition_max = 5,
            const std::string& restricted_range = "" )
        : m_rng(std::time(0)),
        m_repetition_max( repetition_max ),
        m_fullSetRegex( "[\\w:!\\?\\-\\+=]" )
        {
            {
                auto tokens = lexer( m_fullSetRegex );
                auto fullSet = Parser().parseStandAloneSet( tokens );
                m_fullSet = generateChoices( fullSet );
                std::sort( m_fullSet.begin(), m_fullSet.end() );
            }

            if( !restricted_range.empty() )
            {
                auto tokens = lexer(restricted_range);
                auto restrictedSet = Parser().parseStandAloneSet( tokens );
                m_restrictedSet = generateChoices( restrictedSet );
                std::sort( m_restrictedSet.begin(), m_restrictedSet.end() );
            }
        }

        /**
         * generates a random string matching the given regular expression
         * 
         * @param re regular expression ast (@see regen::Parser to create it)
         * 
         * @return the generated string
         */
        std::string generate( const Re& re ) const
        {
            boost::random::uniform_int_distribution<> union_dice(0,re.unionRes.size()-1);

            return generate( re.unionRes[union_dice(m_rng)] );
        }

    private:
        std::string generate( const SimpleRe& sre ) const
        {
            std::string res;

            for( const BasicRe& br : sre.concatRes )
            {
                res += generate( br );
            }

            return res;
        }

        std::string generate( const BasicRe& bre ) const
        {
            if( auto ptr = dynamic_cast<const Star*>( bre.sub.get() ) )
                return generate( *ptr );
            if( auto ptr = dynamic_cast<const Plus*>( bre.sub.get() ) )
                return generate( *ptr );
            if( auto ptr = dynamic_cast<const Question*>( bre.sub.get() ) )
                return generate( *ptr );
            if( auto ptr = dynamic_cast<const NumericRange*>( bre.sub.get() ) )
                return generate( *ptr );
            if( auto ptr = dynamic_cast<const ElementaryRe*>( bre.sub.get() ) )
                return generate( *ptr );

            throw std::logic_error( "unknown basic-re type" );
        }

        std::string generate( const ElementaryRe& ere ) const
        {
            if( auto ptr = dynamic_cast<const Group*>( &ere ) )
                return generate( *ptr );
            if( auto ptr = dynamic_cast<const Any*>( &ere ) )
                return generate( *ptr );
            if( auto ptr = dynamic_cast<const Char*>( &ere ) )
                return generate( *ptr );
            if( auto ptr = dynamic_cast<const Set*>( &ere ) )
                return generate( *ptr );

            throw std::logic_error( "unknown elementary-re type" );
        }

        std::string generate( const Star& star ) const
        {
            return generateRepetition( *star.re, 0, m_repetition_max );
        }

        std::string generate( const Plus& plus ) const
        {
            return generateRepetition( *plus.re, 1, m_repetition_max );
        }

        std::string generate( const Question& question ) const
        {
            return generateRepetition( *question.re, 0, 1 );
        }

        std::string generate( const NumericRange& nrange ) const
        {
            return generateRepetition( *nrange.re, nrange.min, nrange.max );
        }

        std::string generateRepetition( const ElementaryRe& ere, int min, int max ) const
        {
            std::string res = "";
            boost::random::uniform_int_distribution<> iter_dice(min,max);

            std::size_t iterations = iter_dice(m_rng);
            for( std::size_t i = 0; i < iterations; ++i )
                res += generate( ere );

            return res;
        }

        std::string generate( const Group& gr ) const
        {
            return generate( gr.re );
        }

        std::string generate( const Any& any ) const
        {
            auto tokens = lexer( m_fullSetRegex );
            auto fullSet = Parser().parseStandAloneSet( tokens );
            return generate( fullSet );
        }

        std::string generate( const Char& c ) const
        {
            return std::string( 1, c.c );
        }

        std::string generate( const Set& se ) const
        {
            // generates a list of choices and pick one
            std::string choices = generateChoices( se );

            boost::random::uniform_int_distribution<> choice_dice(0,choices.size()-1);

            return std::string( 1, choices[choice_dice(m_rng)] );
        }

        std::string generateChoices( const Set& se ) const
        {
            std::string choices = "";

            for( const std::unique_ptr<SetItem>& item : se.items )
            {
                choices += generate( *item );
            }

            if( se.negative )
            {
                std::string restrictedChoices;
                restrictedChoices.resize(m_fullSet.size());
                std::sort( choices.begin(), choices.end() );
                auto it = std::set_difference( m_fullSet.begin(), m_fullSet.end(),
                                                choices.begin(), choices.end(),
                                                restrictedChoices.begin() );
                restrictedChoices.resize(it-restrictedChoices.begin());
                choices = restrictedChoices;
            }

            if( !m_restrictedSet.empty() )
            {
                std::string restrictedChoices;
                restrictedChoices.resize(m_restrictedSet.size());
                std::sort( choices.begin(), choices.end() );
                auto it = std::set_intersection( m_restrictedSet.begin(), m_restrictedSet.end(),
                                                choices.begin(), choices.end(),
                                                restrictedChoices.begin() );
                restrictedChoices.resize(it-restrictedChoices.begin());
                choices = restrictedChoices;
            }

            return choices;
        }

        std::string generate( const SetItem& si ) const
        {
            if( auto ptr = dynamic_cast<const CharItem*>( &si ) )
                return generate( *ptr );
            if( auto ptr = dynamic_cast<const Range*>( &si ) )
                return generate( *ptr );

            throw std::logic_error( "unknown elementary-re type" );
        }

        std::string generate( const CharItem& c ) const
        {
            return std::string( 1, c.c );
        }

        std::string generate( const Range& r ) const
        {
            std::string res = "";

            for( char c = r.start; c <= r.end; ++c )
            {
                res += c;
            }

            return res;
        }


    private:
        /** random number generator */
        mutable boost::random::mt19937 m_rng;

        /** max number of repetitions for * and + */
        std::size_t m_repetition_max;

        /**
         * this set is used with [^...] the negative set items are substracted from this one
         * the purpose is to avoid always generating junk out of this construct
         */
        std::string m_fullSet;

        /** string used to generate m_fullSet */
        const std::string m_fullSetRegex;

        /**
         * this set is used to restric the pool of characters to pick from, as the user might not 
         * want to generate too "garbage" looking strings
         */
        std::string m_restrictedSet;
    };
}
