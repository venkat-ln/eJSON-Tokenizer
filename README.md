# eJSON Tokenizer
This package implements a C++14-based tokenizer for an extended-JSON (eJSON) object notation format.

## Repository structure
- `.code/src/` contains the source code for the tokenizer and a simple user application `app.cpp` for testing purposes.
- `.code/data/` contains sample eJSON files. For the included user application, this folder also acts as the root location for providing the eJSON files for tokenization.

## Developed for and using
- `Ubuntu 22.04.4 LTS`
- `gcc v11.4.0`
- `C++14 standard library`
- `C++14 standard template library`

## Compiling and testing the user application
- Download the repository to your local machine.
- Open a shell environment and change into the folder `.code/
- Compile and build the executable: `gcc -std=c++14 -Wall src/tokenizer.cpp src/importer.cpp src/scopes.cpp src/rules.cpp src/app.cpp -lstdc++ -o test`
- Run the code: `./test`

## Using the tokenizer function in your application
- The integration of the code for static linking is specific to the build system under use, hence not addressed here.
- Once the package has been integrated into your application project, you may use the following code snippets as reference for usage.
- You may also checkout the sample user application included in this repository

```
import "ejson/tokenizer.h"

// ... further application code

// set path information for the root eJSON file
// The paths to the nested eJSON files are referenced from the location of this file
const ejson::FileName input_file {"./data/test-config.ejson"};

// Instantiate a tokenizer object
ejson::Tokenizer tokenizer{};

// Also create a container to receive the results to be passed by reference
ejson::ListOfTokenizedPairs list_of_pairs;

// Call the tokenizer function
// In addition to the tokens, the tokenizers also provides a feedback 
ejson::TokenizerFeedback feedback = tokenizer.tokenize(input_file, list_of_pairs);

// The tokenizer feedback ejson::TokenizerFeedback provides a success or error code
// and additional information in the event of errors related to file-handling or file-grammar

// The tokens are available in the container ejson::ListOfTokenizedPairs
// The top level container ejson::ListOfTokenizedPairs has one unique element per eJSON file - imported files included
// Multiple imports of the same file in different source files do not result in duplicated elements
// The second level container ejson::TokenizedPairs is a list of single Token-Content pairs ejson::TokenizedPair for one unique file
// A single pair ejson::TokenizedPair conists of the token and its corresponding content

// Iterating through the tokens
std::for_each(std::begin(list_of_pairs), std::end(list_of_pairs), [](ejson::TokenizedPairs const &pairs)
{
    std::for_each(  std::begin(pairs), std::end(pairs), [] (ejson::TokenizedPair const& pair)
    {
        // application steps
    });
});
```
