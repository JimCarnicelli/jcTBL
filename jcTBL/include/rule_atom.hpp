/*

    jcTBL Transformation-based learning classifier system
    - 9/6/2015: Created by Jim Carnicelli

    ----------------------------------------------------------------------------

    MIT License

    Copyright (c) 2016, Jim Carnicelli

    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files (the "Software"), to 
    deal in the Software without restriction, including without limitation the 
    rights to use, copy, modify, merge, publish, distribute, sublicense, and/or 
    sell copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in 
    all copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING 
    FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS 
    IN THE SOFTWARE.

*/

#ifndef jctbl_rule_atom_hpp
#define jctbl_rule_atom_hpp

#include "jctbl.hpp"

namespace jctbl {


    /// One predicate in a rule's list of predicates that map that rule to a
    /// specific element
    class rule_atom {
    public:

        /// Which input feature (e.g., part of speech) does this consider?
        feature* feat = nullptr;

        /// Which element, relative to the current one, is considered?
        int offset = 0;

        /// The values that are allowed for the element's feature
        ///
        /// if this is part of the predicate (inputs) for a rule and this is
        /// empty, the implication is that it can take any value. If this is the
        /// output for a rule template, it must be empty. If this is the output
        /// for a rule, it must contain exactly one value, representing the
        /// predicted output class.
        std::vector<std::string> values;

        /// ?
        std::string comparison_string;

        /// ?
        void add_value(const std::string& value);

    };


}

#endif /* jctbl_rule_atom_hpp */
