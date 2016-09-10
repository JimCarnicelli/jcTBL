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
jctbl::rule::~rule() {
    for (auto it = predicate.begin(); it != predicate.end(); it++) {
        delete *it;
    }
}


/******************************************************************************/
bool jctbl::rule::from_string(classifier& cls, const string& text) {

    string text2 = text;
    string token;

    token = parse_next(text2, "T?");  // First atom name (e.g., "Token")
    if (token == "") return false;

    while (!text2.empty() && token != "") {

        rule_atom* atom = new rule_atom;
        predicate.push_back(atom);

        if (cls.output_feature == nullptr) throw runtime_error(
            "classifier.output_feature must be set before adding rules "
            "or rule templates");
        if (cls.input_features.empty()) throw runtime_error(
            "classifier.input_features must be added before adding rules "
            "or rule templates");


        if (token[0] == '\'') token.erase(0, 1);
        if (token == cls.output_feature->name) {
            atom->feat = cls.output_feature;

        } else {
            auto f_it = cls.input_feature_by_code.find(token);
            if (f_it == cls.input_feature_by_code.end()) throw
                runtime_error("'" + token +
                "' is not a known input feature, nor is it the output feature");
            atom->feat = f_it->second;

        }

        token = parse_next(text2, "N");  // Numeric offset (e.g., '-2' or '+0')
        atom->offset = stoi(token);

        token = parse_next(text2, "T:&>?");  // Feature, ':', '&', '=>', or end
        if (token == "" || token == "=>") break;

        // There's one or more explicit values for this atom
        if (token == ":") {

            token = parse_next(text2, "[T");  // '[' or value text

            if (token == "[") {

                token = parse_next(text2, "T");  // Value text
                if (token[0] == '\'') token.erase(0, 1);
                atom->add_value(token);

                token = parse_next(text2, ",]");  // ',' or ']'
                while (token == ",") {

                    token = parse_next(text2, "T");  // Value text
                    if (token[0] == '\'') token.erase(0, 1);
                    atom->add_value(token);

                    token = parse_next(text2, ",]");  // ',' or ']'
                }

            } else {  // token != '['

                if (token[0] == '\'') token.erase(0, 1);
                atom->add_value(token);

            }

            token = parse_next(text2, "&>?");  // '&' or '=>' or end of text

            if (token == "" || token == "=>") break;

        }

        token = parse_next(text2, "T");  // Feature name

    }

    if (token == "") {

        // The text may be whitespace or nothing but a comment
        if (predicate.empty()) return false;

        return true;  // No specific output defined

    } else if (token == "=>") {

        if (predicate.empty()) throw runtime_error(
            "Expecting a predicate before '=>'");

        token = parse_next(text2, "T");

    } else if (token[0] == '\'') {
        token.erase(0, 1);
        throw runtime_error("Expecting +/- and an integer '" + token +
            "' but ran out of text");
    }

    // The last token will contain a literal value
    if (token[0] == '\'') token.erase(0, 1);
    output = token;

    // Should be nothing more
    token = parse_next(text2, "?");

    if (token != "") throw runtime_error(
        "Expecting end of text but found '" + token + "'");

    return true;
}


/******************************************************************************/
string jctbl::rule::to_string(classifier& cls) {
    if (to_string_ != "") return to_string_;  // Cached from last call

    string text = "";

    // Loop through all the atoms in the predicate
    for (auto a_it = predicate.begin(); a_it != predicate.end(); a_it++) {
        rule_atom* atom = *a_it;

        if (text != "") text += " & ";

        if (atom->feat == cls.output_feature) {
            text += cls.output_feature->name;
        } else {
            text += atom->feat->name;
        }
        if (atom->offset >= 0) text += "+";
        text += std::to_string(atom->offset);

        if (!atom->values.empty()) {
            text += ":";

            if (atom->values.size() > 1) text += "[";

            int count = 0;
            for (auto v_it = atom->values.begin(); v_it != atom->values.end();
                 v_it++
            ) {
                if (count > 0) text += ", ";

                string token = *v_it;
                text += to_literal(token);

                count++;
            }

            if (atom->values.size() > 1) text += "]";
        }
    }

    // Consider the output atom
    if (output != "") {
        text += " => ";
        text += to_literal(output);
    }

    to_string_ = text;
    return to_string_;
}


/******************************************************************************/
void jctbl::rule::clear_to_string() {
    to_string_ = "";
}


/******************************************************************************/
string jctbl::rule::parse_next(string& text, const string& expecting) {
    string token = "";

    if (text.empty()) parse_validate(token, expecting);

    string expecting_string = "";
    expecting_string = expecting;

    // Skip white-space first

    while (!text.empty()) {
        char c = text[0];
        if (!(c == ' ' || c == '\t' || c == '\r' || c == '\n')) break;
        text.erase(0, 1);  // Eat the whitespace character
    }

    // Now parse the token

    while (!text.empty()) {
        char c = text[0];

        //--------------------------------------------------------------------//
        if (c == ' ' || c == '\t' || c == '\r' || c == '\n') {
            text.erase(0, 1);
            return parse_validate(token, expecting);

        //--------------------------------------------------------------------//
        } else  if (c == '#') {  // Comment to end of line
            text.erase(0, 1);

            // Trim leading whitespace
            while (!text.empty()) {
                char c = text[0];
                if (!(c == ' ' || c == '\t' || c == '\r' || c == '\n')) break;
                text.erase(0, 1);  // Eat the whitespace character
            }

            comment = text;  // Whatever's left

            text.erase();
            token = "";
            return parse_validate(token, expecting);

        //--------------------------------------------------------------------//
        } else  if (c == '[' || c == ',' || c == ']' || c == ':') {
            if (token != "") return parse_validate(token, expecting);
            text.erase(0, 1);
            token = c;
            return parse_validate(token, expecting);

        //--------------------------------------------------------------------//
        } else  if (c == '-' || c == '+') {
            if (token != "") return parse_validate(token, expecting);
            text.erase(0, 1);
            token = c;

        //--------------------------------------------------------------------//
        } else if (c == '=') {
            text.erase(0, 1);
            if (text.empty()) throw runtime_error(
                "Expecting '=>' but found only '=' before end of text");
            if (text[0] != '>') {
                token = "=";
                token += c;
                throw runtime_error(
                    "Expecting '=>' but found '" + token + "' instead");
            }
            text.erase(0, 1);
            token = "=>";
            return parse_validate(token, expecting);

        //--------------------------------------------------------------------//
        } else if (c == '\'') {  // Start of a string literal
            text.erase(0, 1);
            token += c;  // Prefix with a single quote for validation purposes
            while (!text.empty()) {
                char c2 = text[0];
                text.erase(0, 1);
                if (c2 == '\'') {  // Could be end of literal or escaped quote
                    if (text.empty()) return parse_validate(token, expecting);
                    if (text[0] != '\'') {
                        return parse_validate(token, expecting);
                    }
                    // It is instead an doubled-up escaped single quote ('')
                    text.erase(0, 1);  // Eat the extra quote
                }
                token += c2;
            }
            throw runtime_error("Expecting closing quote (') but no more text");

        //--------------------------------------------------------------------//
        } else {
            text.erase(0, 1);
            token += c;

        }

    }

    return parse_validate(token, expecting);

}


/******************************************************************************/
string jctbl::rule::parse_validate(string& text, const string& expecting) {

    string explanation = "";

    // Loop through each alternative expectation
    for (auto it = expecting.begin(); it != expecting.end(); it++) {
        char c = *it;

        if (explanation != "") explanation += " or ";

        //--------------------------------------------------------------------//
        if (c == '?') {  // End of text
            explanation += "end of text";

            if (text != "") continue;  // Not valid

            return text;  // Valid

        //--------------------------------------------------------------------//
        } else if (c == 'N') {  //
            explanation += "+/- and an integer";

            if (text == "") continue;  // Not valid

            bool negative = false;
            string token = text;
            if (token[0] == '+') {
                token.erase(0, 1);
            } else if (token[0] == '-') {
                token.erase(0, 1);
                negative = true;
            }

            if (token.find_first_not_of("0123456789") != string::npos) continue;
            if (token.length() > 3) continue;  // Not valid

            if (negative) token = "-" + token;
            return token;  // Valid

        //--------------------------------------------------------------------//
        } else if (c == 'T') {
            explanation += "a name";

            if (text == "") continue;  // Not valid

            if (text[0] == '\'') {  // Clearly a quoted string literal
                return text;  // Anything inside quotes is a valid value
            }

            // First character must be a letter or digit
            char c = text[0];
            if (!(
                (c >= 'A' && c <= 'Z') ||
                (c >= 'a' && c <= 'z') ||
                (c >= '0' && c <= '9')
            )) continue;  // Not valid

            return "'" + text;  // Valid

        //--------------------------------------------------------------------//
        } else if (c == '>') {  // =>
            explanation += "=>";

            if (text == "") continue;  // Not valid

            if (text != "=>") continue;  // Not valid

            return text;  // Valid

        //--------------------------------------------------------------------//
        } else {  // Any specific character
            explanation += "'";
            explanation += c;
            explanation += "'";

            if (text == "") continue;  // Not valid

            string character = "";
            character += c;
            if (text != character) continue;  // Not valid
            return text;  // Valid
        }

    }

    if (text == "") throw runtime_error("Expecting " + explanation +
        " but ran out of text");

    throw runtime_error("Expecting " + explanation + " but found '" + text +
        "' instead");

}


/******************************************************************************/
string jctbl::rule::to_literal(const string& text) {

    if (text.find_first_not_of(
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz"
    ) == string::npos) {
        // No need to quote the literal value
        return text;
    }

    // We'll quote it
    string quoted_text = "'";
    for (auto it = text.begin(); it != text.end(); it++) {
        quoted_text += *it;
        if (*it == '\'') quoted_text += '\'';
    }

    return quoted_text + "'";
}
