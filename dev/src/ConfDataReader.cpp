#pragma ident "$Id$"

/**
 * @file ConfDataReader.cpp
 * Class to parse and manage configuration data files.
 */

//============================================================================
//
//  This file is part of GPSTk, the GPS Toolkit.
//
//  The GPSTk is free software; you can redistribute it and/or modify
//  it under the terms of the GNU Lesser General Public License as published
//  by the Free Software Foundation; either version 2.1 of the License, or
//  any later version.
//
//  The GPSTk is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU Lesser General Public License for more details.
//
//  You should have received a copy of the GNU Lesser General Public
//  License along with GPSTk; if not, write to the Free Software Foundation,
//  Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
//  Dagoberto Salazar - gAGE ( http://www.gage.es ). 2008
//
//============================================================================




#include "ConfDataReader.hpp"



namespace gpstk
{

      // Method to store conf data in this class' data map
   void ConfDataReader::loadData(void)
      throw(ConfFileException)
   {

         // By default, section name is "DEFAULT"
      std::string sectionName("DEFAULT");

         // Do this until end-of-file reached or something else happens
      while(1)
      {
         try
         {

            std::string line;
            std::string variable;
            std::string value;

            formattedGetLine(line, true);

               // If line is too long, we throw an exception
            if (line.size()>255)
            {
               ConfFileException e( "Line too long in configuration file '" +
                                     filename + "'." );
               GPSTK_THROW(e);
            }

               // Let's find and strip comment lines
            if( (StringUtils::firstWord(line)[0] == '#') ||
                (StringUtils::firstWord(line)[0] == ';')  )
            {
               formattedGetLine(line, true);
            }

               // Let's strip comments at the end of lines
            std::string::size_type idx(line.find('#'));
            if( !(idx == std::string::npos) )
            {
               line = line.substr(0, idx);
            }

            idx = line.find(';');
            if( !(idx == std::string::npos) )
            {
               line = line.substr(0, idx);
            }

               // Remove trailing and leading blanks
            line = StringUtils::strip(line);

               // Skip blank lines
            if (line.size()==0)
            {
               continue;
            }


               // Let's start to get data out of file

               // First, handle section names

               // Test if this line declares a new section. Check for '['
            idx = line.find('[');
            if( !(idx == std::string::npos) )
            {

                  // Now, check if there is a closing ']'
               std::string::size_type idx2(line.find(']'));
               if( !(idx2 == std::string::npos) )
               {
                     // Extract name and remove trailing and leading blanks
                  line = StringUtils::strip( line.substr(idx+1, idx2-idx-1) );

                     // Check if section name is appropriate
                  if( checkName(line) )
                  {

                        // Update 'sectionName': make it uppercase
                     sectionName = StringUtils::upperCase(line);

                  }
                  else
                  {
                        // Throw an exception if section name isn't appropriate
                     ConfFileException e( "Section name '" +
                                          line + "' in configuration file '" +
                                          filename +
                                          "' does not comply with rules.");

                     GPSTK_THROW(e);
                  }

                     // If this was a section line, continue with next line
                  continue;

               }
               else
               {
                     // Throw an exception if section line is not closed
                  ConfFileException e( "Section line '" +
                                       line +
                                       "' in configuration file '" +
                                       filename +
                                       "' was improperly closed" );

                  GPSTK_THROW(e);
               }

            }

               // Second, handle variables

               // Separate variable name from value. Look for separators
            idx = line.find('=');
            if( idx == std::string::npos )
            {
               idx = line.find(':');
            }


               // If we found a separator, keep processing
            if( !(idx == std::string::npos) )
            {

                  // Read variable and value
               variable = StringUtils::strip( line.substr(0, idx) );
               value = StringUtils::strip( line.substr(idx+1) );

                  // Now separate comments

                  // Work on 'variable'
               std::string varComment;

               idx = variable.find(',');
               if( !(idx == std::string::npos) )
               {
                  varComment = StringUtils::strip(variable.substr(idx+1));
                  variable   = StringUtils::strip(variable.substr(0, idx));
               }

                  // Check if variable name is appropriate
               if( checkName(variable) )
               {
                     // Make 'variable' uppercase
                  variable = StringUtils::upperCase(variable);

               }
               else
               {
                     // Throw an exception if variable name isn't appropriate
                  ConfFileException e( "Variable name '" +
                                       variable + "' in configuration file '" +
                                       filename +
                                       "' does not comply with rules.");

                  GPSTK_THROW(e);
               }

                  // Now work on 'value'
               std::string valueComment;

               idx = value.find(',');
               if( !(idx == std::string::npos) )
               {
                  valueComment = StringUtils::strip(value.substr(idx+1));
                  value        = StringUtils::strip(value.substr(0, idx));
               }

                  // Store configuration data
               variableData varData;
               varData.varComment   = varComment;
               varData.value        = value;
               varData.valueComment = valueComment;

               confData[sectionName][variable] = varData;

            }

         }  // End of try block
         catch (ConfFileException& e)
         {
            GPSTK_THROW(e);
         }
         catch (EndOfFile& e)
         {
            return;
         }
         catch (...)
         {
            return;
         }

      } // End of 'while(1)'

   }  // End of ConfDataReader::loadData()



      /* Method to get the value of a given variable as a string
       *
       * @param variable   Variable name.
       * @param section    Section the variable belongs to.
       *
       */
   string ConfDataReader::getValue( string variable,
                                    string section )
      throw(ConfFileException)
   {

         // Let's make sure that section and variable names are uppercase
      section  = StringUtils::upperCase(section);
      variable = StringUtils::upperCase(variable);

      try
      {
            // Check if section and variable exist
         if( ifExist(variable, section) )
         {
            return confData[section][variable].value;
         }
         else
         {
            return "";
         }
      }
      catch (ConfFileException& e)
      {
         GPSTK_RETHROW(e);
      }

   }  // End of 'ConfDataReader::getValue()'



      /* Method to get the value of a given variable as a boolean
       *
       * @param variable   Variable name.
       * @param section    Section the variable belongs to.
       *
       */
   bool ConfDataReader::getValueAsBoolean( string variable,
                                           string section )
      throw(ConfFileException)
   {

         // Let's make sure that section and variable names are uppercase
      section  = StringUtils::upperCase(section);
      variable = StringUtils::upperCase(variable);

      try
      {
            // Check if section and variable exist
         if( ifExist(variable, section) )
         {
               // Get value and convert to uppercase
            string result(confData[section][variable].value);

            result = StringUtils::upperCase(result);

               // Test if it is "TRUE" or "FALSE"
            if( result == "TRUE" )
            {
               return true;
            }
            else
            {
               if( result == "FALSE" )
               {
                  return false;
               }
               else
               {
                     // Throw an exception if value is neither TRUE nor FALSE
                  ConfFileException e( "Variable name '" +
                                       variable + "' in configuration file '" +
                                       filename +
                                       "' is neither TRUE nor FALSE.");

                  GPSTK_THROW(e);
               }
            }

         }
         else
         {
               // Return false by default if variable does not exist
            return false;
         }

      }
      catch (ConfFileException& e)
      {
         GPSTK_RETHROW(e);
      }

   }  // End of 'ConfDataReader::getValueAsBoolean()'



      /* Method to get the description of a given variable
       *
       * @param variable   Variable name.
       * @param section    Section the variable belongs to.
       *
       */
   string ConfDataReader::getVariableDescription( string variable,
                                                  string section )
      throw(ConfFileException)
   {

         // Let's make sure that section and variable names are uppercase
      section  = StringUtils::upperCase(section);
      variable = StringUtils::upperCase(variable);

      try
      {
            // Check if section and variable exist
         if( ifExist(variable, section) )
         {
            return confData[section][variable].varComment;
         }
         else
         {
            return "";
         }
      }
      catch (ConfFileException& e)
      {
         GPSTK_RETHROW(e);
      }

   }  // End of 'ConfDataReader::getVariableDescription()'



      /* Method to get the description of a given value
       *
       * @param variable   Variable name.
       * @param section    Section the variable belongs to.
       *
       */
   string ConfDataReader::getValueDescription( string variable,
                                               string section )
      throw(ConfFileException)
   {

         // Let's make sure that section and variable names are uppercase
      section  = StringUtils::upperCase(section);
      variable = StringUtils::upperCase(variable);

      try
      {
            // Check if section and variable exist
         if( ifExist(variable, section) )
         {
            return confData[section][variable].valueComment;
         }
         else
         {
            return "";
         }
      }
      catch (ConfFileException& e)
      {
         GPSTK_RETHROW(e);
      }

   }  // End of 'ConfDataReader::getVariableDescription()'


      /* Method to check if a given section/variable pair exists.
       *
       * @param variable   Variable name.
       * @param section    Section the variable belongs to.
       *
       */
   bool ConfDataReader::ifExist( string variable,
                                 string section )
      throw(ConfFileException)
   {

         // Let's make sure that section and variable names are uppercase
      section  = StringUtils::upperCase(section);
      variable = StringUtils::upperCase(variable);

         // Check if section and variable exist
      confMap::const_iterator it;
      it = confData.find(section);
      if( it != confData.end() )
      {
         variableMap::const_iterator it2;
         it2 = (*it).second.find(variable);
         if( it2 != (*it).second.end() )
         {
            // Return the corresponding value, if it exists
            return true;
         }
         else
         {

            if(issueException)
            {
                  // Throw an exception if variable name doesn't exist
               ConfFileException e( "Variable '" + variable
                                    + "' in configuration file '" + filename
                                    + "' does not exist.");

               GPSTK_THROW(e);
            }
            else
            {
               return false;
            }

         }
      }
      else
      {

         if(issueException)
         {
               // Throw an exception if section name doesn't exist
            ConfFileException e( "Section '" + section
                                 + "' in configuration file '" + filename
                                 + "' does not exist.");

            GPSTK_THROW(e);
         }
         else
         {
            return false;
         }

      }

   }  // End of 'ConfDataReader::ifExist()'



      // Method to open AND load configuration data file.
   void ConfDataReader::open(const char* fn)
   {

      FFTextStream::open(fn, std::ios::in);

      loadData();

      return;

   }  // End of 'ConfDataReader::open()'


      // Method to open AND load configuration data file.
   void ConfDataReader::open(const string& fn)
   {

      FFTextStream::open(fn.c_str(), std::ios::in);

      loadData();

      return;

   }  // End of 'ConfDataReader::open()'


      /* Method to check if the given name is properly formed.
       *
       * @param name    Name to the checked.
       */
   bool ConfDataReader::checkName(string name)
   {

      try
      {

            // Test if first character is alphabetic
         if( StringUtils::isAlphaString(name.substr(0, 1)) )
         {

               // Translate all allowed characters to 'a'
            name = StringUtils::translate( name,"0123456789-_", "a", 'a');

               // If the result is an alphabetic string, the name is valid
            return ( StringUtils::isAlphaString(name) );

         }
         else
         {
               // If first character is not alphabetic, we return false
            return false;
         }

      }
      catch (...)
      {
            // In case of any problem we return false
         return false;
      }

   }  // End of 'ConfDataReader::checkName()'


} // namespace


