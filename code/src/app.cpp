#include "tokenizer.h"
#include <algorithm>
#include <iostream>

int main() {
    
    const ejson::FileName input_file {"./data/test-config.ejson"};

    ejson::ListOfTokenizedPairs list_of_pairs;
    ejson::Tokenizer tokenizer{};

    ejson::TokenizerFeedback feedback = tokenizer.tokenize(input_file, list_of_pairs);

    std::cout << std::endl;
    std::for_each(std::begin(list_of_pairs),
                  std::end(list_of_pairs),
                  [](ejson::TokenizedPairs const &pairs)
                  {
                    std::cout << "---" << std::endl;
                    std::for_each(std::begin(pairs), std::end(pairs),
                    [] (ejson::TokenizedPair const& pair)
                    {
                        std::cout << pair.token << ": " << pair.value << std::endl;
                    });
                      std::cout << "---" << std::endl << std::endl;
                  });
    
    std::cout   << std::endl
                << "feedback" << std::endl
                << "type: " << feedback.type << std::endl
                << "name: " << feedback.file << std::endl
                << "snap: " << feedback.snap << std::endl
                << "---" << std::endl;

    return (int) feedback.type;

}