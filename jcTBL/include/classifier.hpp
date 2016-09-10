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

#ifndef jctbl_classifier_hpp
#define jctbl_classifier_hpp

#include "jctbl.hpp"

namespace jctbl {


    /// The main classification engine, including its own trainer
    class classifier {
    public:

        /// Defines number and names of the features we'll be using as input to
        /// any given training or classifying session
        ///
        /// The first should always be the token, which would be a character,
        /// word (in a sentence), or other raw literal. But you can also provide
        /// additional context that can be used in constructing transformation
        /// rules. For example, if a literal character in a tokenizing task is
        /// "a", you might also tag it as a letter (versus digit, space, symbol,
        /// etc.).
        std::vector<feature*> input_features;

        /// Quick lookup to find the zero-based index of each input feature by
        /// its mnemonic name
        std::map<std::string, feature*> input_feature_by_code;

        /// Defines the single features we'll be developing as output during a
        /// classification session
        feature* output_feature = nullptr;

        /// Complete sequence of the characters, words, etc. to classify
        std::vector<element*> elements;

        /// Mapping of token to first guess of output value
        ///
        /// You can fill this from your own lexicon or use the
        /// .discover_naive_guesses() method to construct it from your training
        /// document. Then use the .apply_naive_guesses() method after loading
        /// elements.
        std::map<std::string, std::string> naive_guesses;

        /// The set of templates for constructing new rules
        std::vector<rule*> rule_templates;

        /// List of rule  templates by their unique, .to_string() derived
        /// signatures
        std::map<std::string, rule*> rule_templates_by_signature;

        /// The ordered sequence of rules to execute to transform a document
        std::vector<rule*> rules;

        /// A setting governing what rules are worth exploring and keeping
        ///
        /// Practically, this is also a throttle governing how quickly the
        /// training proceeds and how many rules there will be at the end.
        /// 1 is the minimum value implied. Having more rules also means that
        /// processing a document at classify time will take longer. But it may
        /// also result in a higher degree of correctness. That said, the rules
        /// in the long tail that only correct a tiny number of mistakes in the
        /// training document are more likely to over-fit that training document
        /// and thus may actually degrate quality in classifying other
        /// documents.
        int min_corrections_required = 5;

        /// The maximum number of rules that can be created during the training
        /// process
        ///
        /// Use -1 for no maximum. This is a crude instrument. Consider using
        /// .min_corrections_required as a more dynamic governor of quality and
        /// classification speed.
        int max_rules = -1;

        /// How many threads should be running in parallel during construction?
        ///
        /// If you don't want to spawn any separate threads, keep this at 1.
        /// Because training is a pure CPU-intensive process, expect this to
        /// consume 100% of the CPUs you throw at it. If you have 4 processor
        /// Cores available, consider making this 3 to save at least some CPU
        /// for other tasks.
        int training_threads = 1;

        /// Once candidate rules have been devised, how many should be used?
        ///
        /// In Brill's original algorithm, the single best rule, measured by how
        /// much good it does overall, gets added to the list in each iteration.
        /// This should generate the highest quality rule set, but it comes at
        /// the cost of time-intensive iterations. The most expensive parts of
        /// each iteration are proposing new rules and scoring each one one.
        ///
        /// Choosing a value greater than 1 tells the trainer to reuse the most
        /// expensive parts of each iteration by not chosing the single best,
        /// but the N best. If you choose 3, expect up to a 3 times increase in
        /// training speed (and down to 1/3 the total training time). But also
        /// expect that you might get marginally poorer quality rules.
        ///
        /// The reason for potentially poorer quality is that each rule, when
        /// applied, changes the input context for rules. If the second best
        /// rule is likely independent (per Hepple) of the first best rule,
        /// then the application of the first best shouldn't affect the utility
        /// of the second best. And so on.
        ///
        /// Probably best to keep this number under 10, but a 10x increase in
        /// training speed is welcome.
        int use_best_rules = 1;

        /// If true, during training, merge related rules
        ///
        /// There are two cases where two proposed rules can be merged together.
        /// If one rule is a subset of another -- if the subset one is the same
        /// as the superset except that it has one extra predicate atom -- then
        /// there's no reason for the subset rule. The other case is when two
        /// rules are essentially the same, but for one atom. In that case, we
        /// can merge the two into one rule by adding the values for that one
        /// differing atom together. In this case, we also add together their
        /// good-change and bad-change stats, which may make a merged rule score
        /// more highly (or lowly) than either would separately.
        ///
        /// This feature may marginally improve the quality of matching, but it
        /// does add training time, so that's a cost. The major benefit is that
        /// it generally reduces the number of rules generated. In my tests, I
        /// found it could more than half the rule count. This reduction can
        /// significantly improve the classification process in production.
        bool use_merging = true;

        /// If true, the .train() method is still executing
        ///
        /// This is especially handy if you use the .train_async() method to
        /// kick off a thread to run .train(). You won't need to monitor the
        /// thread. It will suffice just to check this value periodically in
        /// your polling, assuming you are trying to monitor incremental
        /// progress in some wait loop.
        bool busy_training = false;

        /// During training, how many rules are currently being considered?
        ///
        /// This gets set during training to allow external monitoring.
        int training_rules_being_considered;

        /// During training, what score did the current best rule have?
        ///
        /// This gets set during training to allow external monitoring.
        int training_best_score;

        /// During training, what is the current accuracy percent?
        ///
        /// This gets set during training to allow external monitoring. Values
        /// are from 0 to 1 (100%).
        double training_current_fidelity;

        /// Destructor
        ~classifier();

        /// Clear out the current document in preparation for loading another
        void clear_document();

        /// Defines a new input feature
        feature* add_input_feature(std::string name);

        /// Defines the output feature
        feature* set_output_feature(std::string name);

        /// Add the given element to the list of all in the document
        void add_element(element* el);

        /// Adds 
        void add_naive_guess(const std::string& token,
            const std::string& guess);

        /// Fill the .naive_guesses collection
        ///
        /// If you're not going to fill .naive_guesses from your own custom
        /// lexicon, then this is a good way to learn an initial guess for each
        /// token in your training set by surveying the most popular training
        /// values. In a part-of-speech tagger, for instance, the word "run"
        /// may be used 90% of the time as a verb and 10% of the time as a noun,
        /// so our naive guess for "run" will be that it's a verb.
        ///
        /// Note that if you call this method, you still need to call the
        /// .apply_naive_guesses() method afterward for your training set.
        void discover_naive_guesses();

        /// Populate every element's .output_value using .naive_guesses values
        ///
        /// Your code would call this if you didn't supply initial .output_value
        /// values as part of your input. This is assuming you've filled
        /// the .naive_guesses collection from your own lexicon or called the
        /// .discover_naive_guesses() method during training.
        void apply_naive_guesses();

        /// Add the given rule template to my list
        rule* add_rule_template(rule* r);

        /// Parse and add the given rule template to my list
        ///
        /// Note that this method will throw an error if the signature of a new
        /// template is found, but won't check for same-meaning templates that
        /// have different signatures. For example, "PoS-1 & PoS+0" and
        /// "PoS+0 & PoS-1" have different signatures, but their behavior would
        /// be identical. A good habit to avoid accidentally doing this is to
        /// always define your same-feature predicate atoms in groups and in
        /// offset order (e.g., "PoS-1 & PoS+0 & PoS+1 & Token-1 & Token+0").
        ///
        /// This ordering of atoms is also important for the merging process
        /// that, during training, combines related rules. If you don't order
        /// your features (primary sort) and offsets (secondary sort) in order,
        /// the merging won't work right and the resulting rules will be more
        /// verbose by having redundancies and thus take longer to process at
        /// classification time.
        rule* add_rule_template(std::string rule_text);

        /// Add the given rule to my list
        rule* add_rule(rule* r);

        /// Parse and add the given rule to my list
        rule* add_rule(std::string rule_text);

        /// Construct the tranformation rules that will be used to classify
        void train();

        /// Launches .train() in a separate thread and returns the thread
        ///
        /// Naturally, you can use the returned thread handle to monitor for an
        /// end condition or cut it short if your process is shutting down, but
        /// you can also call mythread->detach(); and delete mythread; and just
        /// let it go. If you set up a polling loop to, say, report perodically
        /// on progress, just watch the .busy_training flag, which gets cleared
        /// as the very last line of code in .train().
        ///
        /// Some of the properties you can monitor include .rules.size(),
        /// .training_rules_being_considered, .training_best_score, and
        /// .training_current_fidelity.
        ///
        /// As the rules pile up, .training_best_score score goes gradually down
        /// toward .min_corrections_required. Once the best score goes below
        /// that value, training stops, so that's a good way to see how close to
        /// done training is.
        std::thread* train_async();

        /// Once a document is loaded, this classifies each element loaded
        ///
        /// This is the main workhorse. If you didn't provide your own initial
        /// guesses for the elements, try calling the .apply_naive_guesses()
        /// method before calling this. That's assuming you loaded the rules
        /// (via .add_rule() ) and document (via .add_element() ). The process
        /// typically involves running .clear_document() before loading each
        /// document, calling .apply_naive_guesses() (assuming you want to use
        /// that feature and used .add_naive_guess() to load your lexicon), and
        /// then calling .classify_elements(). To get the results, you would
        /// iterate through the .elements collection looking at each one's
        /// .output_value property.
        void classify_elements();

        /// Count what percent of the elements' outputs correctly match their
        /// training values
        ///
        /// This is only useful when we are given tagged training (or test)
        /// data. Return value is from 0 to 1 (100%).
        double check_fidelity();

    private:

        /// Determine if the given rule fits the local conditions at the
        /// given element's location
        bool fit_rule(element* el, rule* r);

        /// Fills proposed_rule_sets with novel rules discovered by finding
        /// which rule templates fit at that element's location and constructing
        /// rules using the local features of that element and its neighbors
        ///
        /// This is designed to run in a separate thread along-side other
        /// instances. Each thread chews on a subset of the total set of
        /// elements.
        void propose_rules(std::mutex* mutex, std::vector<element*>* elements,
            std::vector< std::map<std::string, rule*>* >* proposed_rule_sets);

        /// Propose all possible rules by applying one specific template at the
        /// given element's location
        void propose_rule(element* el, rule* rule_template,
            std::map<std::string, rule*>* proposed_rules);

        /// Check each of the candidate rules to see where in the document it
        /// would fail in preparation for finding the best rule(s) to add to the
        /// master list
        ///
        /// This is designed to run in a separate thread along-side other
        /// instances. Each thread chews on a subset of the total set of
        /// candidate rules.
        void evaluate_rules(std::mutex* mutex, int* threads_active,
            std::vector<rule*>* candidate_rules,
            std::map<std::string, rule*>* proposed_rules);

        /// ?
        bool merge_rules(rule* r1, rule* r2);

        /// Apply the single rule to the entire document
        void apply_rule(rule* r);

    };


}

#endif /* jctbl_classifier_hpp */
