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

    ----------------------------------------------------------------------------


    This includes all other includes that declare the essential classes in this
    engine. This should be the only include you need in your own project.

    This work is based on research by Eric Brill of Johns Hopkins University
    into what he called "transformation-based error-driven learning", though 
    that usually gets shortened to "transformation-based learning", or "TBL".
    See Brill's "Transformation-Based Error-Driven Learning and Natural 
    Language Processing: A Case Study in Part-of-Speech Tagging" paper from 
    1995:

        http://www.aclweb.org/anthology/J95-4004

    Radu Florian and Grace Ngai, also of Johns Hopkins, created their own 
    implementation of Brill's algorithm:
    
        https://www.cs.jhu.edu/~rflorian/fntbl/tbl-toolkit/
        
    I did not get it to compile on my iMac or spend more than a few minutes
    trying to read their code, but I did find their documentation of their
    toolkit instructive. Their 2001 paper "Transformation-Based Learning in the 
    Fast Lane" also helped me understand the original algorithm and give clues
    as to how I might improve my own algorithm's performance:

        https://www.aclweb.org/anthology/N/N01/N01-1006.pdf

    I did take inspiration from the Mark Hepple's (University of Sheffield)
    2000 contribution in "Independence and commitment: assumptions for rapid 
    training and execution of rule-based POS taggers" paper, as referenced by
    Ngai and Florian. His "independence assumption" inspired the 
    classifier.use_best_rules property that dramatically speeds up training.

        https://aclweb.org/anthology/P/P00/P00-1036.pdf

    Here's the script you'll want to follow in creating your own application
    for training:

        // Construct and configure
        jctbl::classifier cls;
        cls.min_corrections_required = 5;

        // Repeated calls to this
        cls.add_input_feature("Token");  // Lower-cased version of the token
        cls.add_input_feature("RawToken");  // Original token text
        cls.add_input_feature("Caps");  // Whether the word is capitalized

        cls.set_output_feature("PoS");

        // Repeated calls to this
        cls.add_rule_template("PoS-1 & PoS+0 & PoS+1");
        cls.add_rule_template("PoS-1 & PoS+0");
        cls.add_rule_template("PoS-2 & PoS-1 & PoS+0");

        jctbl::element* el;

        // Repeated iterations like this to load your document
        el = new jctbl::element;
        el->input_values.push_back("banana");  // Token
        el->input_values.push_back("Banana");  // Raw token
        el->input_values.push_back("C");  // (C)apitalized | (L)ower case
        el->training_value = "NN";
        cls.add_element(el);

        // Here's the learning part

        cls.discover_naive_guesses();  // Or you can use cls.add_naive_guess()
        cls.apply_naive_guesses();

        cls.apply_naive_guesses();
        double training_baseline = cls.check_fidelity();

        cls.train();
        double training_success = cls.check_fidelity();

        cout << "- Result: " << 100.0 * success << "% success rate\n";

    When it comes time to test, you'll follow a similar script:

        // Construct and configure
        jctbl::classifier cls;

        // Repeated calls to this
        cls.add_input_feature("Token");  // Lower-cased version of the token
        cls.add_input_feature("RawToken");  // Original token text
        cls.add_input_feature("Caps");  // Whether the word is capitalized

        cls.set_output_feature("PoS");

        // Repeated calls to this
        cls.add_rule("PoS-1:TO & PoS+0:NN => VB");
        cls.add_rule("PoS-1:NNP & PoS+0:NN & PoS+1:NNP => NNP");

        jctbl::element* el;

        // Repeated iterations like this to load your document
        el = new jctbl::element;
        el->input_values.push_back("banana");  // Token
        el->input_values.push_back("Banana");  // Raw token
        el->input_values.push_back("C");  // (C)apitalized | (L)ower case
        el->training_value = "NN";
        cls.add_element(el);

        // Here's the testing part

        cls.apply_naive_guesses();
        cls.classify_elements();
        double success = cls.check_fidelity();

        cout << "- Result: " << 100.0 * success << "% success rate\n";

    If you just wanted to add testing immediately after training, you'd add the
    following to your training script instead:
    
        cls.clear_document();
        
        // More calls here to cls.add_element(el) to supply your test document

        cls.apply_naive_guesses();
        double testing_baseline = cls.check_fidelity();
        
        cls.classify_elements();
        double testing_success = cls.check_fidelity();

    Finally, at runtime, you're following a trimmed down version of the testing
    script:

        // Construct and configure
        jctbl::classifier cls;

        // Repeated calls to this
        cls.add_input_feature("Token");  // Lower-cased version of the token
        cls.add_input_feature("RawToken");  // Original token text
        cls.add_input_feature("Caps");  // Whether the word is capitalized

        cls.set_output_feature("PoS");

        // Repeated calls to this
        cls.add_rule("PoS-1:TO & PoS+0:NN => VB");
        cls.add_rule("PoS-1:NNP & PoS+0:NN & PoS+1:NNP => NNP");

        jctbl::element* el;

        // Once your classifier is configured, you can process many docs
        for (<document loop>) {

            cls.clear_document();

            // Repeated iterations like this to load your document
            el = new jctbl::element;
            el->input_values.push_back("banana");  // Token
            el->input_values.push_back("Banana");  // Raw token
            el->input_values.push_back("C");  // (C)apitalized | (L)ower case
            cls.add_element(el);

            // Here's the classifying part

            cls.apply_naive_guesses();
            cls.classify_elements();

        }

    The key distinctions between testing and run-time classifying are that you 
    can't set el->training_value and thus you have no reason to call
    cls.check_fidelity(), for lack of training values.
    
    If it isn't obvious yet, file loading and saving is your responsibility. I
    didn't want to presuppose that you had to use any particular file format.
    The one convenience provided is in .add_rule() (and .add_rule_template() ),
    which will accept the raw lines of a source file, provided you format them
    with nothing but blank lines, comment lines (prefixed by '#'), and rule
    lines (which can end with '#'-prefixed comments, too). That said, you can
    also pack your rules (output using rule.to_string() ) in a string in your
    custom file format. Or, of course, construct rule objects from scratch and
    use the versions of .add_rule() and .add_rule_template() that accept them.

*/

#ifndef jctbl_jctbl_hpp
#define jctbl_jctbl_hpp

#include <string>
#include <array>
#include <vector>
#include <deque>
#include <map>
#include <thread>
#include <mutex>


namespace jctbl {

    class feature;
    class element;
    class rule_atom;
    class rule;
    class classifier;

}


#include "feature.hpp"
#include "element.hpp"
#include "rule_atom.hpp"
#include "rule.hpp"
#include "classifier.hpp"


#endif /* jctbl_jctbl_hpp */
