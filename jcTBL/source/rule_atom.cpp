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

#include "jctbl_for_source.hpp"


/******************************************************************************/
void jctbl::rule_atom::add_value(const string& value) {
    values.push_back(value);

    // Sorting the values helps make rules comparable (and versionable)
    std::sort(values.begin(), values.end());

    // String version of list for use in merging rules during training
    comparison_string = to_string(feat->index);
    comparison_string += "|";
    comparison_string += to_string(this->offset);
    for (auto it = values.begin(); it != values.end(); it++) {
        comparison_string += "|";
        comparison_string += *it;
    }

}