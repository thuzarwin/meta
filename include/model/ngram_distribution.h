/**
 * @file ngram_distribution.h
 * @author Sean Massung
 * Declaration of a smoothed ngram language model class.
 *
 * All files in META are released under the MIT license. For more details,
 * consult the file LICENSE in the root of the project.
 */

#ifndef META_NGRAM_DISTRIBUTION_
#define META_NGRAM_DISTRIBUTION_

#include <iostream>
#include <random>
#include <string>
#include <deque>
#include <unordered_map>
#include <utility>
#include "corpus/document.h"
#include "tokenizers/ngram/ngram_tokenizer.h"

namespace meta
{
namespace language_model
{

/**
 * Represents a smoothed distribution of ngrams of either words, POS tags,
 * function words, or characters. Smoothing is done with absolute discounting
 * with the (n-1)-gram model recursively to the unigram level.
 */
template <size_t N>
class ngram_distribution
{
  public:
    typedef std::unordered_map<std::string,
                               std::unordered_map<std::string, size_t>> FreqMap;
    typedef std::unordered_map<std::string,
                               std::unordered_map<std::string, double>> ProbMap;

    /**
     * Constructor.
     * @param doc_path The training document.
     */
    ngram_distribution(const std::string& doc_path);

    /**
     * @param prev The first word
     * @param word The second word
     * @return the probability of seeing "prev+word".
     */
    double prob(const std::string& prev, const std::string& word) const;

    /**
     * @param str The string token
     * @return the probability of seeing "str".
     */
    double prob(const std::string& str) const;

    /**
     * Calculates the log-likelihood of the given document using the current
     * language model.
     * @param document The document to calculate log-likelihood for
     * @return the log-likelihood
     */
    double log_likelihood(const corpus::document& document) const;

    /**
     * Calculates the perplexity of the given document using the current
     * language model.
     * @param document The document to calculate perplexity for
     * @return the perplexity of the document
     */
    double perplexity(const corpus::document& document) const;

    /**
     * Generates a random sentence using the current language model.
     * @param seed A random seed
     * @param num_words The number of words to generate
     * @return a random sentence likely to be generated with this model
     */
    std::string random_sentence(unsigned int seed, size_t num_words) const;

    /**
     * @return the value of N for this model
     */
    inline size_t n_value() const
    {
        return N;
    }

    /**
     * @param k A smaller distribution
     * @return the language model for k-grams, where 1 <= k <= N.
     */
    const ProbMap& kth_distribution(size_t k) const;

  private:
    /**
     * Selects a word (token) from a probability distribution given a random
     * number.
     * @param rand A uniform random number in [0,1] used to select a token.
     * @param dist The distribution to select from
     * @return a random, though likely token from the distribution
     */
    std::string get_word(double rand, const std::unordered_map
                         <std::string, double>& dist) const;

    /**
     * @param rand A random number on [0,1]
     * @return N-1 previous tokens based on a uniform random number in
     * [0,1].
     */
    std::string get_prev(double rand) const;

    /**
     * Tokenizes a training document, counting frequencies of ngrams.
     * @param doc_path The training document
     */
    void calc_freqs(const std::string& doc_path);

    /**
     * Calculates a smoothed probability distribution over ngrams.
     * \f$P_{AD}(word|prev) = \frac{ max(c(prevword) - D, 0) }{ c(prev) } +
     *  \frac{D}{c(prev)} \cdot |S_w| \cdot P_{AD}(word)\f$
     */
    void calc_dist();

    /**
     * Calculate D = n1 / (n1 + 2 * n2).
     * n1 = number of ngrams that appear exactly once.
     * n2 = number of ngrams that appear exactly twice.
     */
    void calc_discount_factor();

    /**
     * @param words A string "w1 w2 w3 w4"
     * @return "w4" from "w1 w2 w3 w4"
     */
    std::string get_last(const std::string& words) const;

    /**
     * @param words A string "w1 w2 w3 w4"
     * @return "w1 w2 w3" from "w1 w2 w3 w4"
     */
    std::string get_rest(const std::string& words) const;

    /**
     * Converts a deque into a human-readable string representation of
     * tokens.
     * @param ngram The deque of tokens
     * @return the list of tokens as a string
     */
    std::string to_prev(const std::deque<std::string>& ngram) const;

    /// Frequency of each ngram, used for probability calculation
    FreqMap freqs_;

    /// Distribution for this ngram
    ProbMap dist_;

    /// N-1 prior distribution
    ngram_distribution<N - 1> lower_;

    /// Discounting factor for absolute discounting smoothing
    double discount_;
};

/**
 * Specialized "base case" class for a unigram model's prior distribution.
 */
template <>
class ngram_distribution<0>
{
  public:
    typedef std::unordered_map<std::string,
                               std::unordered_map<std::string, double>
                              > ProbMap;

    /**
     * @param str The string token
     * @return the probability of seeing this token: always zero
     */
    double prob(const std::string& str)
    {
        return 0.0;
    }

    /**
     * @param k The previous distribution number
     * @return the previous distribution
     */
    const ProbMap& kth_distribution(size_t k) const
    {
        return dist_;
    }

  private:
    /// The (fake) underlying distribution of this distribution
    ProbMap dist_;
};
}
}

#include "model/ngram_distribution.tcc"
#endif