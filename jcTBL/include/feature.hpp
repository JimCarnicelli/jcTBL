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

#ifndef jctbl_feature_hpp
#define jctbl_feature_hpp

#include "jctbl.hpp"

namespace jctbl {


    /// Definition for a single input or output feature
    ///
    /// The classifier maintains one set each of input and output feature
    /// definitions. Each document element (character, word, etc.) has a set
    /// of input, output, and (optionally) training values for each such
    /// feature defined.
    class feature {
    public:

        /// The zero-based position of this feature in a list of them
        int index = 0;

        /// A unique mnemonic name for data representation (e.g., "Pos" or
        /// "ChunkType")
        std::string name;

        /// The set of all unique values found in the source data (e.g., "NNP",
        /// "NNS", "VBP", and so on) and their occurrence counts
        //std::map<std::string, int> values;

        /// Constructor
        feature(std::string name);

    };


}

#endif /* jctbl_feature_hpp */
