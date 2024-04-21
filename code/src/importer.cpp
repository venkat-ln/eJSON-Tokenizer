#include "tokenizer.h"
#include <iostream>
#include <algorithm>
#include <limits>
#include <cctype>

namespace ejson
{
    static const std::string IMPORT_STATEMENT = std::string{"import"};

    void Tokenizer::initialize(const std::string &input_file,
                               ListOfFiles &list_of_files,
                               TokenizerFeedback &feedback)
    {

        // Proceed with initialization only if the file isn't already on the list
        auto search_result = std::find_if( std::begin(list_of_files),
                                        std::end(list_of_files),
                                        [&input_file] (File const& file)
                                        {
                                            return (input_file == file.path);
                                        });

        if(search_result == std::end(list_of_files))
        {
            // Get path to folder hosting the input file
            feedback.file = input_file;
            std::size_t position = input_file.find_last_of( (char) Token::PATH_SEPARATOR,
                                                            input_file.length());
            if (position < input_file.length())
            {
                const std::string input_file_home = input_file.substr(0, (position + 1));
                
                // Open input stream for current input file and add it to list
                list_of_files.emplace_back( File
                                            {
                                                input_file,
                                                std::ifstream{input_file}
                                            });
                
                // Proceed further only if no file error is reported
                if(!list_of_files.back().stream.fail())
                {
                    // Resolve names of files to be imported
                    std::vector<std::string> import_files;
                    resolveImportStatements(    input_file_home,
                                                list_of_files.back().stream,
                                                import_files,
                                                feedback);
                    if(feedback.type == FeedbackType::OK)
                    {
                        // Initialize tokenization for the imported files
                        std::for_each(  std::begin(import_files),
                                        std::end(import_files),
                                        [this, &feedback, &list_of_files](std::string const &import_file)
                                        {
                                            // Initialization of next file is only continued
                                            // if previous initialization was successful
                                            if(feedback.type == FeedbackType::OK)
                                                initialize(import_file, list_of_files, feedback);
                                        });
                    }
                }
                else
                    feedback.type = FeedbackType::NOK_FILE_ERROR;
            }
            else
                feedback.type = FeedbackType::NOK_FILE_ERROR;
        
            // Clean up feedback before returning
            if(feedback.type == FeedbackType::OK)
                feedback.file = feedback.snap = std::string{""};
        }
    }

    void Tokenizer::resolveImportStatements(const std::string &input_file_home,
                                            std::ifstream &file,
                                            std::vector<std::string> &import_files,
                                            TokenizerFeedback &feedback)
    {
        
        std::string line {""};
        while(file.is_open() && !file.eof() && (feedback.type == FeedbackType::OK))
        {
            std::getline(file >> std::ws, line);
            if(!line.empty())
            {
                // Ignore the line if it is a comment
                // Stop scanning if an object definition begins
                // Assumption: import statements are always placed before object definitions

                // Check for presence of comments in the line and remove them if found.
                std::size_t first_comment_position = line.find_first_of((char) Token::COMMENT);
                if(first_comment_position < line.npos)
                    line.erase(first_comment_position);
                
                if(!line.empty())
                {
                    if(line.front() != (char) ejson::Token::OBJECT_BEGIN)
                        checkForAndParseImportStatement(    input_file_home,
                                                            line,
                                                            import_files,
                                                            feedback);
                    else
                    {
                        line.push_back(Token::NEW_LINE);
                        std::for_each(std::rbegin(line),
                                      std::rend(line),
                                      [&file](char const& c)
                                      {
                                          file.putback(c);
                                      });
                        file.setstate(std::ios_base::eofbit);
                    }
                }
            }
        }

        // Clear file stream state 
        file.clear();
    }

    void Tokenizer::checkForAndParseImportStatement(const std::string &input_file_home,
                                                    const std::string &line,
                                                    std::vector<std::string> &import_files,
                                                    TokenizerFeedback &feedback)
    {
        
        feedback.snap = line;
        std::stringstream stream {line};
        std::string import_keyword {""};
        std::string& import_file = import_keyword;
        
        // Check and extract import statement
        std::getline(stream, import_keyword, (char) Token::BLANK_SPACE);
        if(!stream.eof() && (import_keyword == IMPORT_STATEMENT))
        {
            stream.ignore(std::numeric_limits<std::streamsize>::max(), (char) Token::STRING_DELIMITER);
            if(!stream.eof())
            {
                std::getline(stream, import_file, (char) Token::STRING_DELIMITER);
                if (!stream.eof() && !import_file.empty())
                    import_files.emplace_back(std::string{input_file_home + import_file});
                else if(stream.eof())
                    feedback.type = FeedbackType::NOK_PARSER_ERROR;
            }
            else
                feedback.type = FeedbackType::NOK_PARSER_ERROR;
        }
        else if(!import_keyword.empty())
            feedback.type = FeedbackType::NOK_PARSER_ERROR;
    }
}