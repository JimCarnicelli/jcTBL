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

#ifndef jctbl_rule_hpp
#define jctbl_rule_hpp

#include "jctbl.hpp"

namespace jctbl {


    /// A single transformation rule or rule template
    ///
    /// A rule is a set of conditions (i.e., a "predicate") to match (if A and
    /// B and C, etc., are true) and the output class to change the current
    /// element to if the conditions are met. The conditions (atoms) represent
    /// features at locations relative to the current element being considered
    /// (e.g., the part of speech of the element immediately to the left).
    class rule {
    public:

        /// The index of this rule in its list
        int index = -1;

        /// The list of input arguments that must be matched for a rule to apply
        /// to a given element (i.e., the rule's predicate)
        std::vector<rule_atom*> predicate;

        /// The characteristics of the output class to predict
        std::string output = "";

        /// Everything found after '#' while parsing via .from_string()
        ///
        /// If you want comments found while loading your rule set to be output
        /// again, try something like rule->to_string() + " # " + rule->comment.
        std::string comment = "";

        /// During training, we keep track of how many changes this rule would
        /// match that would result in corrections, when compared to those
        /// matching elements' training values
        int good_changes = 0;

        /// During training, we keep track of how many changes this rule would
        /// match that would result in errors, when compared to those matching
        /// elements' training values
        int bad_changes = 0;

        /// Tracks which element (index) this rule was first proposed for
        int first_seen_at = -1;

        /// If this is a rule, what template was used to create it?
        rule* from_template = nullptr;

        /// Destructor
        ~rule();

        /// Initialize myself by parsing the given textual version
        bool from_string(classifier& cls, const std::string& text);

        /// Output a plain-text representation that .from_string() can use
        std::string to_string(classifier& cls);

        /// Clears the cached version of .to_string(), refreshing its output
        void clear_to_string();

    private:

        /// Cached version of the output from .to_string()
        std::string to_string_;

        /// Parse out the next token of a rule definition
        std::string parse_next(std::string& text, const std::string& expecting);

        /// Called by .from_string() to validate that the given token just
        /// parsed is one of the options the caller is expecting
        std::string parse_validate(std::string& text,
            const std::string& expecting);

        /// Called by .to_string() to return a value as either a string of
        /// letters or surrounded by single quotes if it has other characters
        std::string to_literal(const std::string& text);

    };


}

#endif /* jctbl_rule_hpp */
