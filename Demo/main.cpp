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

#include <iostream>
#include <fstream>
#include <thread>
#include "jctbl.hpp"

using namespace std;



/******************************************************************************/
string to_literal(const string& text) {
    if (text == "") return "''";

    if (text.find_first_not_of(
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz"
        "0123456789."
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


/******************************************************************************/
string parse_literal(string& text) {
    if (text.empty()) return "";

    string literal = "";

    if (text[0] == '\'') {
        text.erase(0, 1);

        while (!text.empty()) {
            char c = text[0];
            text.erase(0, 1);
            if (c == '\'') {
                if (text.empty()) break;
                if (text[0] != '\'') break;
                text.erase(0, 1);
            }
            literal += c;
        }

    } else {

        while (!text.empty()) {
            char c = text[0];
            if (c == ' ' || c == '\t' || c == '\r' || c == '\n' || c == ':')
                break;
            text.erase(0, 1);
            literal += c;
        }

    }
    return literal;
}


/******************************************************************************/
void load_training_document(jctbl::classifier& cls, string path) {

    ifstream infile(path);
    if (!infile.is_open()) throw runtime_error("Error opening file: " + path);

    jctbl::element* el;
    int line_number = 0;
    std::string line;
    while (std::getline(infile, line)) {
        line_number++;

        el = new jctbl::element;

        if (line == "") {
            el->input_values.push_back("");  // Token
            el->input_values.push_back("");  // Raw token
            el->input_values.push_back("");  // Chunk tag
            el->input_values.push_back("");  // Phrase type
            el->training_value = "SEP";  // Sentence separator
            cls.add_element(el);
            continue;
        }

        // First column will be the raw token
        size_t loc = line.find(' ');

        string raw_token = line;
        raw_token.resize(loc);

        string token = raw_token;
        std::transform(token.begin(), token.end(), token.begin(), ::tolower);

        el->input_values.push_back(token);  // Token
        el->input_values.push_back(raw_token);  // Raw token

        line.erase(0, loc + 1);

        // Second column will be the target part of speech

        loc = line.find(' ');
        string pos = line;
        pos.resize(loc);
        el->training_value = pos;

        line.erase(0, loc + 1);

        if (line == "O") {
            el->input_values.push_back("O");  // Chunk tag
            el->input_values.push_back("");  // Phrase type
        } else {
            loc = line.find('-');
            string chunk_tag = line;
            chunk_tag.resize(loc);
            el->input_values.push_back(chunk_tag);  // Chunk tag
            line.erase(0, loc + 1);

            el->input_values.push_back(line);  // Phrase type
        }

        cls.add_element(el);

    }
}


/******************************************************************************/
void load_naive_guesses(jctbl::classifier& cls, const string& path) {

    ifstream infile(path);
    if (!infile.is_open()) throw runtime_error("Error opening file: " + path);

    int line_number = 0;
    std::string line;
    while (std::getline(infile, line)) {
        line_number++;
        if (line == "") continue;
        if (line[0] == '#') continue;  // Comment line

        try {

            string token = parse_literal(line);

            string junk = line;  junk.resize(2);
            if (junk != ": ") throw runtime_error(
                "Expecting ': ' after token");
            line.erase(0, 2);

            string guess = parse_literal(line);

            if (line != "") throw runtime_error(
                "Expecting end of line after guess value");

            cls.add_naive_guess(token, guess);
            cls.naive_guesses[token] = guess;

        } catch (exception& ex) {
            throw runtime_error("Error on line " + to_string(line_number) +
                ": " + ex.what());
        }

    }

}


/******************************************************************************/
void load_rules(jctbl::classifier& cls, bool templates, const string& path) {

    ifstream infile(path);
    if (!infile.is_open()) throw runtime_error("Error opening file: " + path);

    int line_number = 0;
    std::string line;
    while (std::getline(infile, line)) {
        line_number++;

        try {
            if (templates) {
                cls.add_rule_template(line);
            } else {
                cls.add_rule(line);
            }
        } catch (exception& ex) {
            throw runtime_error("Error on line " + to_string(line_number) +
                ": " + ex.what());
        }

    }

}


/******************************************************************************/
int main(int argc, const char * argv[]) {
    cout << "- Starting\n";

    string data_path = "/Users/Jim/Google Drive/Experiments/jcTBL/Data/";

    jctbl::classifier cls;
    cls.min_corrections_required = 20;
    cls.training_threads = 4;
    cls.use_best_rules = 10;
    cls.use_merging = true;

    // Define input and output features
    cls.add_input_feature("Token");   // Lower-case token
    cls.add_input_feature("Raw");   // Raw token
    cls.add_input_feature("ChunkTag");    // Chunk tag
    cls.add_input_feature("PhraseType");   // Phrase type
    cls.set_output_feature("PoS");  // Part of speech

    load_rules(cls, true,  data_path + "pos_rule_templates.txt");

    if (false) {
        // Already trained

        load_rules(cls, false, data_path + "pos_rules.txt");
        load_naive_guesses(cls, data_path + "pos_naive_guesses.txt");

    } else {
        // Need to train

        // Load and preprocess the training document
        cout << "- Loading training data\n";
        load_training_document(cls, data_path + "pos_train.txt");
        cout << "- Discover naive guesses\n";
        cls.discover_naive_guesses();

        // Output the naive-guesses lexicon
        if (true) {
            ofstream out(data_path + "_naive_guesses.txt");
            for (auto it = cls.naive_guesses.begin();
                it != cls.naive_guesses.end(); it++
            ) {
                string token = it->first;
                string guess = it->second;
                out << to_literal(token) << ": " << to_literal(guess)
                    << "\n";
            }
            out.close();
        }

        cout << "- Apply naive guesses\n";

        cls.apply_naive_guesses();

        double train_baseline = cls.check_fidelity();
        cout << "- Baseline: " << 100.0 * train_baseline
            << "% success rate\n";

        // Here's where the magic happens
        thread* t = cls.train_async();
        t->detach();
        delete t;

        int latest_rule_count = -1;
        while (cls.busy_training) {
            int rule_count = (int) cls.rules.size();
            if (rule_count > latest_rule_count) {
                latest_rule_count = rule_count;
                cout << "----------\n";
                cout << "- There are " << rule_count << " rules\n";
                cout << "- Best score: " << cls.training_best_score << "\n";
                cout << "- " << 100.0 * cls.training_current_fidelity
                    << "% fidelity so far\n";
                cout << "- " << cls.training_rules_being_considered
                    << " rules were considered\n";
            }
            this_thread::sleep_for(chrono::milliseconds(100));
        }

        cout << "------------------------\n";
        cout << "- There are " << cls.rules.size() << " rules total\n";

        double train_success = cls.check_fidelity();
        cout << "- Training set: " << 100.0 * train_success
            << "% success rate ("
            << 100.0 * (train_success - train_baseline) / train_baseline
            << "% improvement)\n";

        // Output the discovered rules
        if (true) {
            ofstream out(data_path + "_rules.txt");
            for (auto it = cls.rules.begin(); it != cls.rules.end(); it++) {
                jctbl::rule* r = *it;

                string line = r->to_string(cls);
                while (line.size() < 60) line += ' ';

                out << line
                    << "  # From template " << r->from_template->index + 1
                    << ", From training line " << r->first_seen_at + 1
                    << ", Score: +" << r->good_changes
                    << " -" << r->bad_changes
                    << " = " << r->good_changes - r->bad_changes
                    << "\n";
            }
            out.close();
        }

        cout << "------------------------\n";

        for (auto it = cls.rule_templates.begin();
            it != cls.rule_templates.end(); it++) {
            jctbl::rule* rt = *it;
            cout << rt->rules_created_from_me
                << " rules created from template:  "
                << rt->to_string(cls) << "\n";
        }

        cout << "------------------------\n";

        cls.clear_document();

    }

    cout << "- Loading test data\n";
    load_training_document(cls, data_path + "pos_test.txt");
    cout << "- Done loading test file\n";

    cls.apply_naive_guesses();

    double test_baseline = cls.check_fidelity();
    cout << "- Baseline: " << 100.0 * test_baseline << "%\n";

    cls.classify_elements();

    double test_success = cls.check_fidelity();
    cout << "- Test set: " << 100.0 * test_success << "% success rate ("
        << 100.0 * (test_success - test_baseline) / test_baseline
        << "% improvement)\n";

    cout << "- Done\n";
    return 0;
}
