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

#ifndef jctbl_element_hpp
#define jctbl_element_hpp

#include "jctbl.hpp"

namespace jctbl {


    /// A single character, word, or other element to be classified
    class element {
    public:

        /// Zero-based position of this element in the document
        int index;

        /// The a priori known values of each input feature
        ///
        /// Your application defines the features. See
        /// classifier.input_features for the list of definitions for each
        /// feature. The first should be the literal token (character, word,
        /// etc.). In the part-of-speech tagger example, you would typically
        /// only need the token, but you might alternatively have a major PoS
        /// (e.g., "Noun") and also the finer-pointed PoS (e.g., "NNS" or
        /// "NNP").
        std::vector<std::string> input_values;

        /// The output value generated during training or classification
        std::string output_value;

        /// The desired output value to target during training
        ///
        /// This uses the same feature as .output_value but indicates the true
        /// value that we want to train this classifier on. See
        /// classifier.output_feature for its definition.
        std::string training_value;

        /// Destructor
        ~element();

    };


}

#endif /* jctbl_element_hpp */
