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
jctbl::classifier::~classifier() {

    clear_document();

    for (auto it = input_features.begin(); it != input_features.end(); it++) {
        delete *it;
    }
    if (output_feature != nullptr) delete output_feature;
    for (auto it = rules.begin(); it != rules.end(); it++) {
        delete *it;
    }
}


/******************************************************************************/
void jctbl::classifier::clear_document() {

    for (auto it = elements.begin(); it != elements.end(); it++) {
        delete *it;
    }
    elements.clear();

}


/******************************************************************************/
jctbl::feature* jctbl::classifier::add_input_feature(string name) {

    // Validate
    if (name.empty()) throw runtime_error("A code is required");

    if (name.find_first_not_of(
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz"
        "0123456789_"
    ) != string::npos) throw runtime_error(
        "Code may only be letters, digits, and underscores");

    char c = name[0];
    if (!(
          (c >= 'A' && c <= 'Z') ||
          (c >= 'a' && c <= 'z')
    )) throw runtime_error(
        "Code must begin with a letter");

    feature* f = new jctbl::feature(name);
    f->index = (int) input_features.size();
    input_features.push_back(f);

    auto it = input_feature_by_code.find(f->name);
    if (it != input_feature_by_code.end()) throw runtime_error(
        "There's already a feature with '" + name + "' as its code");

    input_feature_by_code[f->name] = f;

    return f;
}


/******************************************************************************/
jctbl::feature* jctbl::classifier::set_output_feature(string name) {
    if (output_feature != nullptr) delete output_feature;
    output_feature = new jctbl::feature(name);
    return output_feature;
}


/******************************************************************************/
void jctbl::classifier::add_element(element* el) {
    el->index = (int) elements.size();
    elements.push_back(el);
}


/******************************************************************************/
void jctbl::classifier::add_naive_guess(const string& token, const string& guess
) {
    naive_guesses[token] = guess;
}


/******************************************************************************/
void jctbl::classifier::discover_naive_guesses() {

    // Start by finding the prevailing output values for each unique token

    map<string, map<string, int>*> output_value_counts;

    for (auto e_it = elements.begin(); e_it != elements.end(); e_it++) {
        jctbl::element* el = *e_it;
        string token = el->input_values.at(0);

        map<string, int>* value_counts;

        auto o_it = output_value_counts.find(token);
        if (o_it == output_value_counts.end()) {  // Not found
            value_counts = new map<string, int>;
            output_value_counts[token] = value_counts;
        } else {  // Found
            value_counts = o_it->second;
        }

        auto v_it = value_counts->find(el->training_value);
        if (v_it == value_counts->end()) { // Not found
            value_counts->emplace(el->training_value, 1);
        } else {  // Found
            v_it->second++;
        }

    }

    // Now determine the best naive guesses based on the prevailing values

    for (auto ovc_it = output_value_counts.begin();
        ovc_it != output_value_counts.end(); ovc_it++
    ) {
        string token = ovc_it->first;
        map<string, int>* value_counts = ovc_it->second;

        // Find the value with the highest count

        int best_count = -1;
        string best_value = "";

        for (auto vc_it = value_counts->begin();
            vc_it != value_counts->end(); vc_it++
        ) {
            if (vc_it->second > best_count) {
                best_count = vc_it->second;
                best_value = vc_it->first;
            }
        }

        naive_guesses[token] = best_value;

    }


    // Clean up

    for (auto it = output_value_counts.begin();
        it != output_value_counts.end(); it++
    ) {
        delete it->second;
    }

}


/******************************************************************************/
void jctbl::classifier::apply_naive_guesses() {

    if (naive_guesses.empty()) throw runtime_error(
        ".naive_guesses doesn't contain anything to work from. Did you forget "
        "to load it or to call .discover_naive_guesses() during training?");

    for (auto it = elements.begin(); it != elements.end(); it++) {
        element* el = *it;
        string token = el->input_values.at(0);  // First input should be token

        // See if we have a match for this element's token
        auto g_it = naive_guesses.find(token);
        if (g_it == naive_guesses.end()) { // Not found
            el->output_value = "?";  // Don't know

        } else {  // Found
            el->output_value = g_it->second;  // Our initial guess

        }

    }

}


/******************************************************************************/
jctbl::rule* jctbl::classifier::add_rule_template(rule* r) {

    // Validate
    if (r->predicate.empty()) throw runtime_error("At least one predicate "
        "atom (e.g., 'PoS+1') is required for a rule template");
    if (!r->output->values.empty()) throw runtime_error(
        "No output value may be specified for a rule template");

    r->index = (int) rule_templates.size();

    auto it = rule_templates_by_signature.find(r->to_string(*this));
    if (it != rule_templates_by_signature.end()) throw runtime_error(
        "There is already a rule template with this signature: " +
        r->to_string(*this));

    rule_templates_by_signature[r->to_string(*this)] = r;

    rule_templates.push_back(r);
    return r;
}


/******************************************************************************/
jctbl::rule* jctbl::classifier::add_rule_template(std::string rule_text) {
    rule* r = new rule;
    if (!r->from_string(*this, rule_text)) {
        delete r;
        return nullptr;
    }
    add_rule_template(r);
    return r;
}


/******************************************************************************/
jctbl::rule* jctbl::classifier::add_rule(rule* r) {

    // Validate
    if (r->predicate.empty()) throw runtime_error("At least one predicate "
        "atom (e.g., 'PoS-1:NN') is required for a rule");
    if (r->output->values.size() != 1) throw runtime_error(
        "A single output value is required for a rule");

    // We don't guarantee uniqueness in this case because it may be appopriate
    // for the same rule to appear later by another rule that messes up some
    // data that was previously fixed by the same rule. This should be a rare
    // occurrence.

    r->index = (int) rule_templates.size();
    rules.push_back(r);
    return r;
}


/******************************************************************************/
jctbl::rule* jctbl::classifier::add_rule(std::string rule_text) {
    rule* r = new rule;
    if (!r->from_string(*this, rule_text)) {
        delete r;
        return nullptr;
    }
    add_rule(r);
    return r;
}


/******************************************************************************/
void jctbl::classifier::train() {

    busy_training = true;

    // Validate
    if (elements.empty()) throw runtime_error("No elements to train on");
    if (rule_templates.empty()) throw runtime_error(
        "No rule templates defined");

    mutex mutex;

    // Keep looking for new rules until we run out or reach our quota
    while (max_rules < 1 || rules.size() < max_rules) {

        //--------------------------------------------------------------------//
        // Propose rules

        // We'll split this task among multiple threads and wait until they're
        // all done. Since each element can be considered in isolation, we
        // simply apportion the elements evenly among the threads

        // Make a copy of the thread limit now to allow it to be changed in
        // another thread without breaking this process
        int thread_count = training_threads;
        if (thread_count < 1) thread_count = 1;

        // Construct work-lists for each of the threads
        auto element_sets = new vector<element*>[thread_count];
        vector< map<string, rule*>* > proposed_rule_sets;

        // Parcel elements out to each thread
        for (auto e_it = elements.begin(); e_it != elements.end(); e_it++) {
            element* el = *e_it;
            int for_thread = el->index % thread_count;
            element_sets[for_thread].push_back(el);
        }

        // Kick off threads
        if (thread_count == 1) {
            // Look ma, no threads!
            propose_rules(&mutex, &element_sets[0], &proposed_rule_sets);
        } else {
            for (int i = 0; i < thread_count; i++) {
                thread t(&classifier::propose_rules, this, &mutex,
                    &element_sets[i], &proposed_rule_sets);
                t.detach();
            }
        }

        // Sit back and wait for all the threads to return
        while (proposed_rule_sets.size() < thread_count) {
            this_thread::sleep_for(chrono::milliseconds(10));
        }
        // We're now back down to just the main thread

        delete[] element_sets;  // Don't need these anymore

        map<string, rule*>* proposed_rules = nullptr;
        if (thread_count == 1) {

            // There's only one in the set, so no need to merge
            proposed_rules = proposed_rule_sets.at(0);

        } else {

            // Merge the resulting rule sets together
            for (auto t_it = proposed_rule_sets.begin();
                t_it != proposed_rule_sets.end(); t_it++
            ) {
                map<string, rule*>* prs = *t_it;

                // The first in the list will be the seed rule set
                if (proposed_rules == nullptr) {
                    proposed_rules = prs;
                    continue;
                }

                // Find and update existing rules or add new ones
                for (auto r_it = prs->begin(); r_it != prs->end(); r_it++) {
                    string signature = r_it->first;
                    rule* r = r_it->second;

                    auto existing = proposed_rules->find(signature);
                    if (existing == proposed_rules->end()) {  // Not yet defined

                        proposed_rules->emplace(signature, r);

                    } else {  // Rule already defined

                        rule* existing_rule = existing->second;
                        existing_rule->good_changes += r->good_changes;
                        existing_rule->bad_changes += r->bad_changes;
                        if (r->first_seen_at < existing_rule->first_seen_at) {
                            existing_rule->first_seen_at = r->first_seen_at;
                        }

                        delete r;  // Ditch the duplicate rule
                    }

                }

                delete *t_it;
            }

        }

        // Track training stats
        training_rules_being_considered = (int) proposed_rules->size();


        //--------------------------------------------------------------------//
        // Evaluate proposed rules

        // We'll split this task among multiple threads and wait until they're
        // all done. Since each proposed rule can be considered in isolation, we
        // simply apportion the rules evenly among the threads

        // Make a copy of the thread limit now to allow it to be changed in
        // another thread without breaking this process
        thread_count = training_threads;
        if (thread_count < 1) thread_count = 1;

        // Construct work-lists for each of the threads
        auto candidate_rules = new vector<rule*>[thread_count];
        int threads_active = 0;

        // Parcel rules out to each thread
        int i = 0;
        for (auto r_it = proposed_rules->begin(); r_it != proposed_rules->end();
            r_it++
        ) {
            rule* r = r_it->second;
            int for_thread = i % thread_count;
            candidate_rules[for_thread].push_back(r);
            i++;
        }

        // Kick off threads
        if (thread_count == 1) {
            // Look ma, no threads!
            threads_active = 1;
            evaluate_rules(&mutex, &threads_active, &candidate_rules[0],
                proposed_rules);
        } else {
            for (int i = 0; i < thread_count; i++) {
                threads_active++;
                thread t(&classifier::evaluate_rules, this, &mutex,
                    &threads_active, &candidate_rules[i], proposed_rules);
                t.detach();
            }
        }

        // Sit back and wait for all the threads to return
        while (threads_active > 0) {
            this_thread::sleep_for(chrono::milliseconds(10));
        }
        // We're now back down to just the main thread

        delete[] candidate_rules;  // Don't need these anymore


        //--------------------------------------------------------------------//
        // From the rules we've considered so far, find the N best

        if (use_best_rules < 1) use_best_rules = 1;

        bool found_one = false;

        for (int times = 0; times < use_best_rules; times++) {

            rule* best_rule = nullptr;

            // If we run out of rules, bail
            if (proposed_rules->empty()) break;

            int highest_score = 0;
            int lowest_bads = 0;

            // Loop through proposed rules
            for (auto r_it = proposed_rules->begin();
                r_it != proposed_rules->end();
            ) {
                rule* r = r_it->second;
                int score = r->good_changes - r->bad_changes;

                if (best_rule == nullptr) {
                    // No best yet, so first one we come across is the best

                    highest_score = r->good_changes - r->bad_changes;
                    lowest_bads = r->bad_changes;
                    best_rule = r;

                } else if (score > highest_score) {
                    // Highest score wins

                    highest_score = score;
                    lowest_bads = r->bad_changes;
                    best_rule = r;

                } else if (
                    score == highest_score &&
                    r->bad_changes < lowest_bads
                ) {
                    // Same overall score? Favor the lower bad-changes count

                    highest_score = score;
                    lowest_bads = r->bad_changes;
                    best_rule = r;

                }

                r_it++;
            }  // Every proposed rule

            if (best_rule == nullptr) {
                break;  // No more rules to discover
            }

            found_one = true;
            training_best_score = highest_score;


            //----------------------------------------------------------------//
            // Apply and save the best rule so we can build on its success

            apply_rule(best_rule);

            // Track training stats
            training_current_fidelity = check_fidelity();

            best_rule->index = (int) rules.size();
            rules.push_back(best_rule);

            // Remove this from the candidates because we'll consider others
            proposed_rules->erase(best_rule->to_string(*this));

        }  // Repeat for use_best_rules times

        // Delete the remaining candidates
        for (auto it = proposed_rules->begin(); it != proposed_rules->end();
            it++
        ) {
            delete it->second;
        }
        delete proposed_rules;

        if (!found_one) break;  // No more rules to discover

    }  // Keep looking for new rules

    busy_training = false;
}


/******************************************************************************/
thread* jctbl::classifier::train_async() {
    busy_training = true;
    return new thread(&jctbl::classifier::train, this);
}


/******************************************************************************/
void jctbl::classifier::classify_elements() {

    // Validate
    if (elements.empty()) throw runtime_error("No elements to classify");
    if (rules.empty()) throw runtime_error("No rules defined");

    for (auto it = rules.begin(); it != rules.end(); it++) {
        apply_rule(*it);
    }
}


/******************************************************************************/
double jctbl::classifier::check_fidelity() {
    int count = 0;
    int correct = 0;

    if (elements.empty()) return 0;

    for (auto it = elements.begin(); it != elements.end(); it++) {
        count++;
        element* el = *it;
        if (el->output_value == el->training_value) correct++;
    }

    return (double) correct / count;
}


/******************************************************************************/
bool jctbl::classifier::fit_rule(element* el, rule* r) {

    // Check the predicate's atoms to see that they match, too
    for (auto p_it = r->predicate.begin(); p_it != r->predicate.end(); p_it++) {
        rule_atom* atom = *p_it;

        // If there's no input specified, there's no need to test this atom
        if (atom->values.empty()) continue;

        // Is the target element outside this rule's search range?
        int pos = el->index + atom->offset;
        if (pos < 0 || pos >= (int) elements.size()) return false;

        element* el2 = elements.at(pos);

        bool found_one = false;
        string current_value;

        if (atom->feat == output_feature) {

            current_value = el2->output_value;

        } else {

            // If there's no value to compare to, any value is fine; move on
            if (atom->values.empty()) continue;

            current_value = el2->input_values.at(atom->feat->index);

        }

        // See if that element's current value is on our list
        for (auto v_it = atom->values.begin(); v_it != atom->values.end();
            v_it++
        ) {
            string proposed_value = *v_it;
            if (current_value == proposed_value) {
                found_one = true;
                break;
            }
        }

        if (!found_one) return false;

    }

    return true;
}


/******************************************************************************/
void jctbl::classifier::propose_rules(mutex* mutex,
    vector<element*>* elements,
    vector< map<string, rule*>* >* proposed_rule_sets
) {
    auto proposed_rules = new map<string, rule*>;

    // Loop through every document element
    for (auto e_it = elements->begin(); e_it != elements->end(); e_it++) {
        element* el = *e_it;

        // Only consider creating transformation rules for incorrect values
        if (el->output_value == el->training_value) continue;

        // Loop through every template
        for (auto t_it = rule_templates.begin();
            t_it != rule_templates.end(); t_it++
        ) {
            rule* rule_template = *t_it;

            // See if this rule template matches enough to construct a new
            // rule from
            if (fit_rule(el, rule_template)) {

                // There's no need to propose a change rule if the rule
                // would not cause any actual change
                if (
                    rule_template->output->values.empty() ||
                    el->output_value != rule_template->output->values.at(0)
                ) {
                    propose_rule(el, rule_template, proposed_rules);
                }
            }

        }  // Every template

    }  // Every document element

    // Only allow one thread may append to the rule set list
    lock_guard<std::mutex> guard(*mutex);

    proposed_rule_sets->push_back(proposed_rules);
}


/******************************************************************************/
void jctbl::classifier::propose_rule(element* el, rule* rule_template,
    map<string, rule*>* proposed_rules
) {
    bool something_changed = false;

    rule* r = new rule;
    r->good_changes = 1;
    r->first_seen_at = el->index;
    r->from_template = rule_template;

    // Loop through all the predicate's input atoms
    for (auto a_it = rule_template->predicate.begin();
        a_it != rule_template->predicate.end(); a_it++
    ) {
        rule_atom* atom = *a_it;

        // Is the target element outside this rule's search range?
        int pos = el->index + atom->offset;
        if (pos < 0 || pos >= (int) elements.size()) {
            delete r;
            return;
        }

        rule_atom* new_atom = new rule_atom;
        r->predicate.push_back(new_atom);

        new_atom->feat = atom->feat;
        new_atom->offset = atom->offset;

        element* el2 = elements.at(pos);
        string current_value;
        if (atom->feat == output_feature) {
            current_value = el2->output_value;
        } else {
            current_value = el2->input_values.at(atom->feat->index);
        }
        new_atom->values.push_back(current_value);

        if (new_atom->values.size() == atom->values.size()) {
            if (current_value != atom->values.at(0)) {
                something_changed = true;
            }
        }

    }  // Each predicate atom

    // Copy the training value to the output class label
    r->output = new rule_atom;
    if (rule_template->output->values.empty()) {

        r->output->values.push_back(el->training_value);
        something_changed = true;

    } else {

        throw runtime_error("Hey, this rule template doesn't actually fit!");

    }

    // If this new rule is identical to or a subset of the template rule, don't
    // bother proposing it as though it were new
    if (!something_changed) {
        delete r;
        return;
    }

    // Now let's see if this rule has already been proposed in this round
    string signature = r->to_string(*this);
    auto it = proposed_rules->find(signature);
    if (it == proposed_rules->end()) {  // Not found

        proposed_rules->emplace(r->to_string(*this), r);

    } else {  // Found

        delete r;
        r = it->second;
        r->good_changes++;

    }

}


/******************************************************************************/
void jctbl::classifier::evaluate_rules(mutex* mutex, int* threads_active,
    vector<rule*>* candidate_rules, map<string, rule*>* proposed_rules
) {

    // Loop through proposed rules
    for (auto r_it = candidate_rules->begin(); r_it != candidate_rules->end();
         r_it++
    ) {
        rule* r = *r_it;

        // So far we've only calculated how much good this rule would do. If
        // it's not good enough, let's just skip this one and move on.
        if (r->good_changes < min_corrections_required) {
            {
                lock_guard<std::mutex> guard(*mutex);
                proposed_rules->erase(r->to_string(*this));
            }
            delete r;
            continue;
        }

        // So now we'll calculate how much bad this rule would do
        for (auto e_it = elements.begin(); e_it != elements.end(); e_it++) {
            element* el = *e_it;

            // Only if this rule fits this location should be consider it
            if (fit_rule(el, r)) {

                // Does this rule have an incorrect result?
                if (r->output->values.at(0) != el->training_value) {
                    r->bad_changes++;

                    // If the bad outweighs the good, let's move on
                    if (r->good_changes - r->bad_changes <
                        min_corrections_required
                    ) {
                        {
                            lock_guard<std::mutex> guard(*mutex);
                            proposed_rules->erase(r->to_string(*this));
                        }
                        delete r;
                        r = nullptr;
                        break;
                    }

                }
            }

        }  // Every document element
        if (r == nullptr) continue;  // This got set in the loop as a flag

    }  // Every proposed rule

    // Only allow one thread may append to the rule set list
    lock_guard<std::mutex> guard(*mutex);

    int& count = *threads_active;
    count--;  // Take myself out of the count

}


/******************************************************************************/
void jctbl::classifier::apply_rule(rule* r) {
    for (auto it = elements.begin(); it != elements.end(); it++) {
        element* el = *it;
        if (fit_rule(el, r)) {
            string new_value = r->output->values.at(0);
            if (el->output_value != new_value) {
                el->output_value = new_value;
            }
        }
    }
}
