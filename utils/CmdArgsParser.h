/*
	Copyright (c) 2000-2016 Michael Pozhidaev<michael.pozhidaev@gmail.com>
   This file is part of the VoiceMan speech service.

   VoiceMan speech service is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 3 of the License, or (at your option) any later version.

   VoiceMan speech service is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.
*/

#ifndef __VOICEMAN_CMD_ARGS_PARSER_H__
#define __VOICEMAN_CMD_ARGS_PARSER_H__

/**\brief Stores information about available command line argument
 *
 * This structure is used to define a static array and dstore all
 * available command line arguments for application. Information from
 * this array used to parse command line arguments provided by user and
 * print help message.
 *
 * \sa CmdArgsParser
 */
struct CmdArg
{
  /**\brief Single-letter argument name (space means no more arguments in table)*/
  char shortName;

  /**\brief Single-word argument name*/
  const char* longName;

  /**\brief Additional parameter name if this argument has any (empty string means no parameter)*/
  const char* param;

  /**\brief The string to comment this argument in help screen*/
  const char*descr;
}; //struct CmdArg;

/**\brief Parses command line arguments and prints help screen
 *
 * This class receives command line arguments and compares them with the
 * table specified in application source code to determine invalid usage
 * and easy command line check. Also this class can print
 * prepared help screen with all available arguments and their
 * descriptions.
 */
class CmdArgsParser
{
public:
  /**\brief The constructor
   *
   * Number of all available arguments is determined implicitly by the
   * items of allParam argument. The last item in this array must have
   * space in its shortName field.
   *
   * \param [in] allParams The array with available command line arguments
   */
  CmdArgsParser(CmdArg* allParams);

  /**\brief Prints description of all available arguments
   *
   * Use this method to generate complete help screen.
   */
  void printHelp() const;

  /**\brief Checks if specified parameter was used at program call
   *
   * Use this method to check command line argument using.
   *
   * \param [in] name The parameter name to check specified by longName field value
   *
   * \return Non-zero if parameter was used
   */
  bool used(const std::string& name) const;

  /**\brief Returns additional argument value of command line parameter
   *
   * Use this operator to get value of the specified parameter.
   *
   * \param [in] The name of parameter to get argument of, specified by longName field
   *
   * \return Requested value of the specified parameter
   */
  const std::string operator [](const std::string& name) const;

  /**\brief Performs parsing of arguments provided by user
   *
   * This method performs complete command line parsing and fills all
   * internal structures with the result. If there are invalid arguments
   * specified by user this method prints error description and returns
   * zero.
   *
   * \param [in] argc The argc parameter received from main() function call
   * \param [in] argv The argv parameter received from main() function call
   *
   * \return Non-zero if there are no errors or zero if there are invalid command line arguments
   */
  bool parse(int argc, char* argv[]);

public:
  /**\brief The set of arguments specified after known command line parameters*/
  const std::vector<std::string>& files;

private:
  struct USEDPARAM
  {
    USEDPARAM() 
      : hasValue(0) {}

    std::string name, value;
    bool hasValue;
  }; //struct USEDPARAM;

  typedef std::vector<USEDPARAM> UsedParamVector;

private:
  size_t identifyParam(char* p) const;

private:
  typedef std::vector<std::string> StringVector;

  StringVector m_files;
    UsedParamVector m_usedParams;
    CmdArg* m_availableParams;
  size_t m_availableParamCount;
}; // class Cmdargs;

#endif //__VOICEMAN_CMD_ARGS_PARSER_H__
