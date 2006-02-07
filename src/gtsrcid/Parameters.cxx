/*------------------------------------------------------------------------------
Id ........: $Id: Parameters.cxx,v 1.5 2006/02/06 13:26:14 jurgen Exp $
Author ....: $Author: jurgen $
Revision ..: $Revision: 1.5 $
Date ......: $Date: 2006/02/06 13:26:14 $
--------------------------------------------------------------------------------
$Log: Parameters.cxx,v $
Revision 1.5  2006/02/06 13:26:14  jurgen
Remove whitespace in input parameter strings

Revision 1.4  2006/02/03 12:14:52  jurgen
New version that allows additional probabilities to be taken
into account. The code has been considerably reorganised. Also
catalogue column prefixes are now handled differently.

Revision 1.3  2006/02/01 13:33:36  jurgen
Tried to fix Win32 compilation bugs.
Change revision number to 1.3.2.
Replace header information with CVS typeset information.

------------------------------------------------------------------------------*/

/* Includes _________________________________________________________________ */
#include <stdio.h>                       // for "sprintf" function
#include "sourceIdentify.h"
#include "Parameters.h"
#include "Log.h"	                     // for parameter dumping/errors

/* Namespace definition _____________________________________________________ */
namespace sourceIdentify {


/* Globals __________________________________________________________________ */
int g_u9_verbosity;


/* Type defintions __________________________________________________________ */


/* Prototypes _______________________________________________________________ */
std::string trim(std::string str);


/*============================================================================*/
/*                              Private functions                             */
/*============================================================================*/

/*----------------------------------------------------------------------------*/
/*                                     trim                                   */
/* -------------------------------------------------------------------------- */
/* Private function: remove leading and trailing whitespace from string       */
/*----------------------------------------------------------------------------*/
std::string trim(std::string str) {

    // Declare variables
    char const* delims = " \t\r\n";

    // Declare variables
    int                    pos;
    std::string::size_type notwhite;
    
    // Trim leading whitespace
    notwhite = str.find_first_not_of(delims);
    str.erase(0,notwhite);
    
    // Trim trailing whitespace
    notwhite = str.find_last_not_of(delims);
    str.erase(notwhite+1);

    // Return string
    return str;
    
}
    

/*============================================================================*/
/*                   Low-level FITS catalogue handling methods                */
/*============================================================================*/

/*----------------------------------------------------------------------------*/
/*                           Parameters::init_memory                          */
/* -------------------------------------------------------------------------- */
/* Private method: init memory                                                */
/*----------------------------------------------------------------------------*/
void Parameters::init_memory(void) {

    // Declare local variables

    // Single loop for common exit point
    do {

      // Intialise private members
      m_srcCatName.clear();
      m_cptCatName.clear();
      m_outCatName.clear();
      m_srcCatQty.clear();
      m_cptCatQty.clear();
      m_srcCatPrefix.clear();
      m_cptCatPrefix.clear();
      m_outCatQtyName.clear();
      m_outCatQtyFormula.clear();
      m_select.clear();
      m_probColNames.clear();
      m_posProbType = NotUsed;
      m_probThres   = 0.0;
      m_srcPosError = 0.0;
      m_cptPosError = 0.0;
      m_maxNumCtp   = 0;
      m_chatter     = 0;
      m_clobber     = 0;
      m_debug       = 0;
      m_mode.clear();

    } while (0); // End of main do-loop
    
    // Return
    return;

}


/*----------------------------------------------------------------------------*/
/*                            Parameters::free_memory                         */
/* -------------------------------------------------------------------------- */
/* Private member: free memory                                                */
/*----------------------------------------------------------------------------*/
void Parameters::free_memory(void) {

    // Declare local variables

    // Single loop for common exit point
    do {
      
      // Initialise memory
      init_memory();

    } while (0); // End of main do-loop
    
    // Return
    return;

}


/*----------------------------------------------------------------------------*/
/*                              Parameters::load                              */
/* -------------------------------------------------------------------------- */
/* Load parameters from task parameter file.                                  */
/*----------------------------------------------------------------------------*/
Status Parameters::load(st_app::AppParGroup &pars, Status status) {

    // Declare local variables
    int                    blank;
    int                    found;
    long                   i;
    char                   parname[MAX_CHAR];
    std::string            prob_name;
    std::string::size_type len;
    std::string::size_type pos;
    std::string::size_type len_name;
    std::string::size_type start_formula;
    std::string::size_type len_formula;
    std::string::size_type start_name;
        
    // Single loop for common exit point
    do {

      // Fall through in case of an error
      if (status != STATUS_OK)
        continue;

      // Prompt for parameters
      pars.Prompt();

      // Recover task parameters
      std::string s_srcCatName   = pars["srcCatName"];
      std::string s_cptCatName   = pars["cptCatName"];
      std::string s_outCatName   = pars["outCatName"];
      std::string s_srcCatPrefix = pars["srcCatPrefix"];
      std::string s_cptCatPrefix = pars["cptCatPrefix"];
      std::string s_srcCatQty    = pars["srcCatQty"];
      std::string s_cptCatQty    = pars["cptCatQty"];
      std::string probMethod     = pars["probMethod"];
      std::string s_mode         = pars["mode"];
      m_srcCatName               = s_srcCatName;
      m_cptCatName               = s_cptCatName;
      m_outCatName               = s_outCatName;
      m_srcCatPrefix             = "@" + s_srcCatPrefix + "_";
      m_cptCatPrefix             = "@" + s_cptCatPrefix + "_";
      m_srcCatQty                = s_srcCatQty;
      m_cptCatQty                = s_cptCatQty;
      m_probThres                = pars["probThres"];
      m_srcPosError              = pars["srcPosError"];
      m_cptPosError              = pars["cptPosError"];
      m_maxNumCtp                = pars["maxNumCtp"];
      m_chatter                  = pars["chatter"];
      m_clobber                  = pars["clobber"];
      m_debug                    = pars["debug"];
      m_mode                     = s_mode;
      
      // Set U9 verbosity
      if (m_debug)
        g_u9_verbosity = 3;
      else
        g_u9_verbosity = 0;
      
      // Retrieve new output quantities and decompose them into quantity name 
      // and evaluation string
      for (i = MIN_OUTCAT_QTY; i <= MAX_OUTCAT_QTY; i++) {

        // Extract parameter name
        sprintf(parname, "outCatQty%2.2ld", i);
        std::string outCatQty = pars[parname];
        
        // Fall through if parameter is empty
        outCatQty = trim(outCatQty);
        len       = outCatQty.length();
        if (len < 1)
          continue;
        
        // Decompose string in part before and after "=" symbol
        pos           = outCatQty.find("=",0);
        len_name      = pos;
        start_formula = pos + 1;
        len_formula   = len - start_formula;
        
        // Catch invalid parameters
        if (pos == std::string::npos) {
          status = STATUS_PAR_BAD_PARAMETER;
          Log(Error_2, "%d : No equality symbol found in new output catalogue"
              " quantity string <%s='%s'>.", 
              (Status)status, parname, outCatQty.c_str());
          break;
        }
        if (len_name < 1) {
          status = STATUS_PAR_BAD_PARAMETER;
          Log(Error_2, "%d : No quantity name found for new output catalogue"
              " quantity <%s='%s'>.", 
              (Status)status, parname, outCatQty.c_str());
          break;
        }
        if (len_formula < 1) {
          status = STATUS_PAR_BAD_PARAMETER;
          Log(Error_2, "%d : No quantity evaluation string found for new"
              " output catalogue quantity <%s='%s'>.", 
              (Status)status, parname, outCatQty.c_str());
          break;
        }
          
        // Set name and formula (remove whitespace)
        m_outCatQtyName.push_back(trim(outCatQty.substr(0, len_name)));
        m_outCatQtyFormula.push_back(trim((outCatQty.substr(start_formula,
                                                            len_formula))));
          
      } // endfor: looped over quantities
      if (status != STATUS_OK)
        continue;

      // Retrieve selection strings
      for (i = MIN_OUTCAT_SEL; i <= MAX_OUTCAT_SEL; i++) {
        sprintf(parname, "select%2.2ld", i);
        std::string select = pars[parname];
        select             = trim(select);
        len                = select.length();
        if (len > 0) {
          m_select.push_back(select);
        }
      }
      if (status != STATUS_OK)
        continue;
        
      // Retrieve additional probability column names from probability method
      len   = probMethod.length();
      blank = 0;
      found = 0;
      for (pos = 0; pos < len; pos++) {
      
        // Reset found flag
        found = 0;
      
        // Search first non blank and non '*'
        if (!blank && probMethod[pos] != ' ' && probMethod[pos] != '*') {
          start_name = pos;
          blank      = 1;
        }
          
        // Search first blank or "*"
        else if (blank && (probMethod[pos] == ' ' || 
                           probMethod[pos] == '*')) {
          len_name = pos - start_name;
          blank    = 0;
          found    = 1;
        }
        
        // Signal if the end of the string is reached
        if (pos == len-1) {
          len_name = pos - start_name + 1;
          blank    = 0;
          found    = 1;
        }
        
        // Extract name if found
        if (found) {
          prob_name = probMethod.substr(start_name, len_name);
          if (prob_name == "POSITION")
            m_posProbType = Exponential;
          else if (prob_name == "POS-EXP")
            m_posProbType = Exponential;
          else if (prob_name == "POS-GAUSS")
            m_posProbType = Gaussian;
          else
            m_probColNames.push_back(prob_name);
        }
      
      } // endfor: looped over method string     

    } while (0); // End of main do-loop
    
    // Return status
    return status;

}


/*----------------------------------------------------------------------------*/
/*                              Parameters::dump                              */
/* -------------------------------------------------------------------------- */
/* Dump task parameters into log file.                                        */
/*----------------------------------------------------------------------------*/
Status Parameters::dump(Status status) {

    // Declare local variables
    std::string::size_type         i;
    std::string::size_type         n;
    std::vector<double>::size_type i_add;
    std::vector<double>::size_type num_add;
        
    // Single loop for common exit point
    do {

      // Determine number of additional probabilites
      num_add = m_probColNames.size();      

      // Dump task parameters
      Log(Log_1, "Task Parameters:");
      Log(Log_1, "================");
      Log(Log_1, " Source catalogue filename ........: %s", 
          m_srcCatName.c_str());
      Log(Log_1, " Counterpart catalogue name .......: %s", 
          m_cptCatName.c_str());
      Log(Log_1, " Output catalogue name ............: %s", 
          m_outCatName.c_str());
      Log(Log_1, " Source catalogue prefix ..........: %s", 
          m_srcCatPrefix.c_str());
      Log(Log_1, " Counterpart catalogue prefix .....: %s", 
          m_cptCatPrefix.c_str());
      Log(Log_1, " Source catalogue quantities ......: %s", 
          m_srcCatQty.c_str());
      Log(Log_1, " Counterpart catalogue quantities .: %s", 
          m_cptCatQty.c_str());
      Log(Log_1, " Source catalogue uncertainty .....: %f arcmin",
          m_srcPosError*60.0);
      Log(Log_1, " Counterpart catalogue uncertainty : %f arcmin",
          m_cptPosError*60.0);
      if ((n = m_outCatQtyName.size()) > 0) {
        for (i = 0; i < n; i++) {
          Log(Log_1, " New output catalogue quantity %2d .: %s = %s", 
              i+1, m_outCatQtyName[i].c_str(), m_outCatQtyFormula[i].c_str());
        }
      }
      switch (m_posProbType) {
      case NotUsed:
        Log(Log_1, " Position probability method ......: not used");
        break;
      case Exponential:
        Log(Log_1, " Position probability method ......: Exponential");
        break;
      case Gaussian:
        Log(Log_1, " Position probability method ......: Gaussian");
        break;
      default:
        Log(Log_1, " Position probability method ......: undefined");
        break;
      }
      for (i_add = 0; i_add < num_add; i_add++) {
        Log(Log_1, " Additional probability column ....: %s", 
            m_probColNames[i_add].c_str());
      }
      Log(Log_1, " Probability threshold ............: %e", m_probThres);
      Log(Log_1, " Maximum number of counterparts  ..: %d", m_maxNumCtp);
      if ((n = m_select.size()) > 0) {
        for (i = 0; i < n; i++) {
          Log(Log_1, " Output catalogue selection %2d ....: %s", 
              i+1, m_select[i].c_str());
        }
      }
      Log(Log_1, " Chatter level of output ..........: %d", m_chatter);
      Log(Log_1, " U9 verbosity .....................: %d", g_u9_verbosity);
      Log(Log_1, " Clobber ..........................: %d", m_clobber);
      Log(Log_1, " Debugging mode activated .........: %d", m_debug);
      Log(Log_1, " Mode of automatic parameters .....: %s", m_mode.c_str());
    
    } while (0); // End of main do-loop
    
    // Return status
    return status;

}


/* Namespace ends ___________________________________________________________ */
}
