/*------------------------------------------------------------------------------
Id ........: $Id: Catalogue_id.cxx,v 1.3 2007/09/20 16:28:21 jurgen Exp $
Author ....: $Author: jurgen $
Revision ..: $Revision: 1.3 $
Date ......: $Date: 2007/09/20 16:28:21 $
--------------------------------------------------------------------------------
$Log: Catalogue_id.cxx,v $
Revision 1.3  2007/09/20 16:28:21  jurgen
Enhance catalogue interface for column recognition

Revision 1.2  2006/02/07 16:05:04  jurgen
Use ObjectInfo structure to hold catalogue object information

Revision 1.1  2006/02/03 12:11:37  jurgen
New file that contains routines that have formerly been found in
Catalogue.cxx. The routines have also been renamed and preceeded
by "cid_". The routines handle source identification at the low
level.

------------------------------------------------------------------------------*/

/* Includes _________________________________________________________________ */
#include "sourceIdentify.h"
#include "Catalogue.h"
#include "Log.h"
#include "src/quantity.h"


/* Definitions ______________________________________________________________ */
#define CATALOGUE_TIMING   0                     // Enables timing measurements


/* Namespace definition _____________________________________________________ */
namespace sourceIdentify {
using namespace catalogAccess;


/* Type defintions __________________________________________________________ */


/* Private Prototypes _______________________________________________________ */


/*----------------------------------------------------------------------------*/
/*                             Catalogue::cid_get                             */
/* -------------------------------------------------------------------------- */
/* Private method: get counterpart candidates for source 'iSrc'               */
/*----------------------------------------------------------------------------*/
Status Catalogue::cid_get(Parameters *par, long iSrc, Status status) {

    // Declare local variables
    ObjectInfo *src;

    // Debug mode: Entry
    if (par->logDebug())
      Log(Log_0, " ==> ENTRY: Catalogue::cid_get");

    // Single loop for common exit point
    do {
    
      // Get pointer to source object
      src = &(m_src.object[iSrc]);

      // Dump source information (optionally)
      if (par->logNormal()) {
        if (src->pos_valid) {
          Log(Log_2, " Source %5d .....................: %18s"SRC_FORMAT,
              iSrc+1, src->name.c_str(), 
              src->pos_eq_ra, src->pos_eq_dec, 
              src->pos_err_maj, src->pos_err_min, src->pos_err_ang);
        }
        else {
          Log(Log_2, " Source %5d .....................: %18s"
              " No position information found.",
              iSrc+1, src->name.c_str());
        }
      }
      
      // Fall through if no position information has been found
      if (!src->pos_valid)
        continue;
        
      // Filter step: Get counterparts near the source position
      status = cid_filter(par, iSrc, status);
      if (status != STATUS_OK) {
        if (par->logTerse())
          Log(Error_2, "%d : Unable to perform filter step for source %d.", 
              (Status)status, iSrc+1);
        continue;
      }

      // Refine step: Assign probability for each counterpart
      status = cid_refine(par, iSrc, status);
      if (status != STATUS_OK) {
        if (par->logTerse())
          Log(Error_2, "%d : Unable to perform refine step for source %d.", 
              (Status)status, iSrc+1);
        continue;
      }

      // Add all counterpart candidates to output catalogue
      status = cfits_add(m_outFile, iSrc, par, status);
      if (status != STATUS_OK) {
        if (par->logTerse())
          Log(Error_2, "%d : Unable to add counterpart candidates for source"
              " %d to FITS output catalogue '%s'.", 
              (Status)status, iSrc+1, par->m_outCatName.c_str());
        continue;
      }

      // Optionally dump counterpart candidats
      if (par->logNormal()) {
        cid_dump(par, status);
        Log(Log_2, "");
      }

     } while (0); // End of main do-loop

    // Debug mode: Entry
    if (par->logDebug())
      Log(Log_0, " <== EXIT: Catalogue::cid_get (status=%d)", 
          status);
    
    // Return status
    return status;

}


/*----------------------------------------------------------------------------*/
/*                           Catalogue::cid_filter                            */
/* -------------------------------------------------------------------------- */
/* Private method: Filter step of counterpart identification                  */
/* -------------------------------------------------------------------------- */
/* If the counterpart catalogue is big we load here only the part of the      */
/* catalogue that spatially overlaps with the specified source position.      */
/* Otherwise we load the entire catalogue (only on the first call).           */
/* For all counterpart candidates that were loaded we calculate the angular   */
/* separation to the source of interest and keep only those candidates that   */
/* are spatially sufficient close to the source.                              */
/*----------------------------------------------------------------------------*/
Status Catalogue::cid_filter(Parameters *par, long iSrc, Status status) {

    // Declare local variables
    long        iCpt;
    long        numNoPos;
    long        numRA;
    long        numDec;
    double      src_dec_sin;
    double      src_dec_cos;
    double      cpt_dec_min;
    double      cpt_dec_max;
    double      cpt_ra_min;
    double      cpt_ra_max;
    double      filter_maxsep;
    ObjectInfo *src;
    ObjectInfo *cpt;
    CCElement  *cpt_ptr;

    // Timing measurements
    #if CATALOGUE_TIMING
    clock_t t_start_loop;
    clock_t t_start_tot   = clock();
    float   t_elapse      = 0.0;
    float   t_elapse_loop = 0.0;
    #endif

    // Debug mode: Entry
    if (par->logDebug())
      Log(Log_0, " ==> ENTRY: Catalogue::cid_filter");

    // Single loop for common exit point
    do {

      // Reset statistics
      m_numCC  = 0;
      numNoPos = 0;
      numRA    = 0;
      numDec   = 0;

      // Get pointer to source object
      src = &(m_src.object[iSrc]);

      // Calculate sin and cos of source latitude
      src_dec_sin = sin(src->pos_eq_dec * deg2rad);
      src_dec_cos = cos(src->pos_eq_dec * deg2rad);
      
      // Define bounding box around source position. The declination
      // range of the bounding box is constrained to [-90,90] deg, the
      // Right Ascension boundaries are put into the interval [0,360[ deg.
      cpt_dec_min = src->pos_eq_dec - m_filter_maxsep;
      cpt_dec_max = src->pos_eq_dec + m_filter_maxsep;
      if (cpt_dec_min < -90.0) cpt_dec_min = -90.0;
      if (cpt_dec_max >  90.0) cpt_dec_max =  90.0;
      if (src_dec_cos > 0.0) {
        filter_maxsep = m_filter_maxsep / src_dec_cos;
        if (filter_maxsep > 180.0)
          filter_maxsep = 180.0;
      }
      else
        filter_maxsep = 180.0;
      cpt_ra_min  = src->pos_eq_ra - filter_maxsep;
      cpt_ra_max  = src->pos_eq_ra + filter_maxsep;
      cpt_ra_min = cpt_ra_min - double(long(cpt_ra_min / 360.0) * 360.0);
      if (cpt_ra_min < 0.0) cpt_ra_min += 360.0;
      cpt_ra_max = cpt_ra_max - double(long(cpt_ra_max / 360.0) * 360.0);
      if (cpt_ra_max < 0.0) cpt_ra_max += 360.0;
    
      // If the counterpart catalogue is big then load only sources from a
      // region around the specified position ...
      if (m_cpt.numTotal > m_maxCptLoad) {
      }
      
      // ... otherwise load the entire counterpart catalogue
      else {

        // Load only if noy yet done ...
        if (!m_fCptLoaded) {

          // Load counterpart catalogue
          status = get_input_catalogue(par, &m_cpt, par->m_cptPosError, 
                                       m_cpt_name, status);
          if (status != STATUS_OK) {
            if (par->logTerse())
              Log(Error_2, "%d : Unable to load counterpart catalogue '%s'"
                  " data.", (Status)status, par->m_cptCatName.c_str());
            continue;
          }
          else {
            if (par->logVerbose())
              Log(Log_2, " Counterpart catalogue loaded.");
          }

          // Stop if the counterpart catalogue is empty
          if (m_cpt.numLoad < 1) {
            status = STATUS_CAT_EMPTY;
            if (par->logTerse())
              Log(Error_2, "%d : Counterpart catalogue is empty. Stop.", 
                  (Status)status);
            continue;
          }
          else {
            if (par->logVerbose())
              Log(Log_2, " Counterpart catalogue contains %d sources.", 
                  m_cpt.numLoad);
          }
          
          // Allocate memory for counterpart candidate search
          m_cc = new CCElement[m_cpt.numLoad];
          if (m_cc == NULL) {
            status = STATUS_MEM_ALLOC;
            if (par->logTerse())
              Log(Error_2, "%d : Memory allocation failure.", (Status)status);
            continue;
          }

          // Initialise memory for counterpart search
          for (int i = 0; i < m_cpt.numLoad; ++i)
            m_src_cpts[i] = 0;
           
          // Bookkeep catalogue loading
          m_fCptLoaded = 1;
          
        } // endif: catalogue was not yet loaded
      } // endelse: entire catalogue requested

      // Initialise counterpart pointers
      cpt     = m_cpt.object;
      cpt_ptr = m_cc;

      // Loop over counterparts
      #if CATALOGUE_TIMING
      t_start_loop = clock();
      #endif
      for (iCpt = 0; iCpt < m_cpt.numLoad; iCpt++, cpt++) {
      
        // Filter counterparts that have no positional information
        if (!cpt->pos_valid) {
          numNoPos++;
          continue;
        }

        // Filter counterpart if it falls outside the declination range.
        if (cpt->pos_eq_dec < cpt_dec_min || 
            cpt->pos_eq_dec > cpt_dec_max) {
          numDec++;
          continue;
        }

        // Filter source if it falls outside the Right Ascension range. The
        // first case handles no R.A. wrap around ...
        if (cpt_ra_min < cpt_ra_max) {
          if (cpt->pos_eq_ra < cpt_ra_min || cpt->pos_eq_ra > cpt_ra_max) {
            numRA++;
            continue;
          }
        }
        // ... and this one R.A wrap around
        else {
          if (cpt->pos_eq_ra < cpt_ra_min && cpt->pos_eq_ra > cpt_ra_max) {
            numRA++;
            continue;
          }
        }
        
        // If we are still alive then we keep the counterpart as candidat
        cpt_ptr->id          = "NULL";
        cpt_ptr->pos_eq_ra   = 0.0;
        cpt_ptr->pos_eq_dec  = 0.0;
        cpt_ptr->pos_err_maj = 0.0;
        cpt_ptr->pos_err_min = 0.0;
        cpt_ptr->pos_err_ang = 0.0;
        cpt_ptr->prob        = 0.0;
        cpt_ptr->index       = iCpt;
        cpt_ptr->angsep      = 0.0;
        cpt_ptr->prob_angsep = 0.0;
        cpt_ptr++;
        m_numCC++;
        
      } // endfor: looped over all counterpart candidates

      // Optionally dump counterpart filter statistics
      if (par->logExplicit()) {
        Log(Log_2, "  Filter step candidates ..........: %5d", m_numCC);
        if (par->logVerbose()) {
          Log(Log_2, "    Outside declination range .....: %5d [%7.3f - %7.3f]", 
              numDec, cpt_dec_min, cpt_dec_max);
          Log(Log_2, "    Outside Right Ascension range .: %5d [%7.3f - %7.3f[", 
              numRA, cpt_ra_min, cpt_ra_max);
        }
        if (numNoPos > 0)
          Log(Warning_2, "    No positions ..................: %5d", numNoPos);
      }
      #if CATALOGUE_TIMING
      t_elapse_loop += (float)(clock() - t_start_loop) / (float)CLOCKS_PER_SEC;
      #endif

    } while (0); // End of main do-loop

    // Timing measurements
    #if CATALOGUE_TIMING
    t_elapse += (float)(clock() - t_start_tot) / (float)CLOCKS_PER_SEC;
    Log(Log_0, "  Filter step timing ..............:");
    Log(Log_0, "    Total CPU sec used ............: %.5f", t_elapse);
    Log(Log_0, "    CPU sec spent in loop .........: %.5f", t_elapse_loop);
    #endif

    // Debug mode: Entry
    if (par->logDebug())
      Log(Log_0, " <== EXIT: Catalogue::cid_filter (status=%d)", 
          status);
    
    // Return status
    return status;

}


/*----------------------------------------------------------------------------*/
/*                            Catalogue::cid_refine                           */
/* -------------------------------------------------------------------------- */
/* Private method: Refine step of counterpart identification                  */
/* -------------------------------------------------------------------------- */
/* Calculates the counterpart probability, sorts all counterpart candidates   */
/* by decreasing probability and eliminates all candidtates with a too low    */
/* probability.                                                               */
/*----------------------------------------------------------------------------*/
Status Catalogue::cid_refine(Parameters *par, long iSrc, Status status) {

    // Declare local variables
    long                           iCC;
    long                           numUseCC;
    double                         prob;
    char                           cid[OUTCAT_MAX_STRING_LEN];
    std::vector<double>            prob_add;
    std::vector<double>            prob_val;
    std::vector<long>              prob_too_small;
    std::vector<long>              prob_too_large;
    std::vector<double>::size_type i_add;
    std::vector<double>::size_type num_add;

    // Debug mode: Entry
    if (par->logDebug())
      Log(Log_0, " ==> ENTRY: Catalogue::cid_refine (%d candidates)", 
          m_numCC);

    // Single loop for common exit point
    do {
    
      // Fall through in case of an error
      if (status != STATUS_OK)
        continue;
        
      // Fall through if there are no counterpart candidates
      if (m_numCC < 1)
        continue;

      // Initialise cumulative additional probabilities and clear counter-
      // part vectors
      prob_add.clear();
      for (iCC = 0; iCC < m_numCC; iCC++) {
        prob_add.push_back(1.0);
        m_cc[iCC].prob_add.clear();
      }

      // Determine counterpart probabilities based on angular separation
      status = cid_prob_angsep(par, iSrc, status);
      if (status != STATUS_OK) {
        if (par->logTerse())
          Log(Error_2, "%d : Unable to determine counterpart probabilities"
              " for source %d based on angular separation.", 
              (Status)status, iSrc+1);
        continue;
      }
      
      // Determine number of additional probabilites
      num_add = par->m_probColNames.size();      
      
      // If we need additional probability information then calculate it now
      if (num_add > 0) {

        // Initialise probability boundary violation counters
        prob_too_small.clear();
        prob_too_large.clear();
        for (i_add = 0; i_add < num_add; i_add++) {
          prob_too_small.push_back(0);
          prob_too_large.push_back(0);
        }
      
        // Clear in memory catalogue        
        status = cfits_clear(m_memFile, par, status);
        if (status != STATUS_OK) {
          if (par->logTerse())
            Log(Error_2, "%d : Unable to clear in-memory FITS catalogue.", 
            (Status)status);
          continue;
        }
      
        // Add quantities to in memory catalogue
        status = cfits_add(m_memFile, iSrc, par, status);
        if (status != STATUS_OK) {
          if (par->logTerse())
            Log(Error_2, "%d : Unable to add counterpart candidates to"
                " in-memory FITS catalogue.", (Status)status);
          continue;
        }
        
        // Evaluate in memory catalogue quantities
        status = cfits_eval(m_memFile, par, status);
        if (status != STATUS_OK) {
          if (par->logTerse())
            Log(Error_2, "%d : Unable to evaluate new quantities for in-memory"
                " FITS catalogue.", (Status)status);
          continue;
        }

        // Loop over the additional probability columns
        for (i_add = 0; i_add < num_add; i_add++) {
        
          // Extract probability information from column
          status = cfits_colval(m_memFile, 
                                (char*)par->m_probColNames[i_add].c_str(),
                                par, &prob_val, status);
          if (status != STATUS_OK) {
            if (par->logTerse())
              Log(Error_2, "%d : Unable to determine probability from column"
                  " '%s' of in-memory FITS catalogue.", 
                  (Status)status, par->m_probColNames[i_add].c_str());
            break;
          }
          
          // Push probabilities in vector and cumulate additional probabilities.
          // Make sure that the probabilities are composed between 0 and 1 ...
          for (iCC = 0; iCC < m_numCC; iCC++) {
          
            // Set probability in the range [0,1]
            prob = prob_val[iCC];
            if (prob < 0.0) {
              prob = 0.0;
              prob_too_small[i_add]++;
            }
            else if (prob > 1.0) {
              prob = 1.0;
              prob_too_large[i_add]++;
            }

            // Cumulate additional probabilities
            prob_add[iCC] *= prob;
            
            // Save probability for each counterpart candidate (for logging)
            m_cc[iCC].prob_add.push_back(prob);
            
          } // endfor: looped over all counterpart candidates
        
        } // endfor: looped over the additional probability columns
        if (status != STATUS_OK)
          continue;
        
      } // endif: calculated additional quantities for probability estimation
            
      // Assign the counterpart probabilities
      for (iCC = 0; iCC < m_numCC; iCC++)
        m_cc[iCC].prob = m_cc[iCC].prob_angsep * prob_add[iCC];              

      // Sort counterpart candidates by decreasing probability
      status = cid_sort(par, status);
      if (status != STATUS_OK) {
        if (par->logTerse())
          Log(Error_2, "%d : Unable to sort counterpart candidates.", 
              (Status)status);
        continue;
      }

      // Determine the number of counterpart candidates above the probability 
      // threshold
      numUseCC = 0;
      for (iCC = 0; iCC < m_numCC; iCC++) {
        if (m_cc[iCC].prob >= par->m_probThres)
          numUseCC++;
        else
          break;
      }

      // Apply the maximum number of counterpart threshold
      if (numUseCC > par->m_maxNumCtp)
        numUseCC = par->m_maxNumCtp;

      // Eliminate counterpart candidates below threshold. Fall through if no 
      // counterparts are left
      m_numCC = numUseCC;
      if (m_numCC < 1) {
        if (par->logExplicit()) {
          Log(Log_2, "  Refine step candidates ..........: no");
          for (i_add = 0; i_add < num_add; i_add++) {
            if (prob_too_small[i_add] > 1)
              Log(Warning_2, 
                  "  Probabilities < 0.0 .............: %5d (Quantity='%s')", 
                  prob_too_small[i_add],
                  par->m_probColNames[i_add].c_str());
            if (prob_too_large[i_add] > 1)
              Log(Warning_2, 
                  "  Probabilities > 1.0 .............: %5d (Quantity='%s')", 
                  prob_too_large[i_add],
                  par->m_probColNames[i_add].c_str());
          }
        }
        continue;
      }

      // Set unique counterpart candidate identifier
      // DUMMY: Build string from indices
      for (iCC = 0; iCC < m_numCC; iCC++) {
        sprintf(cid, "CC_%5.5ld_%5.5ld", iSrc+1, iCC+1);
        m_cc[iCC].id = cid;
      }

      // Optionally dump counterpart refine statistics
      if (par->logExplicit()) {
        Log(Log_2, "  Refine step candidates ..........: %5d", m_numCC);
        for (i_add = 0; i_add < num_add; i_add++) {
          if (prob_too_small[i_add] > 1)
            Log(Warning_2, 
                "  Probabilities < 0.0 .............: %5d (Quantity='%s')", 
                prob_too_small[i_add],
                par->m_probColNames[i_add].c_str());
          if (prob_too_large[i_add] > 1)
            Log(Warning_2, 
                "  Probabilities > 1.0 .............: %5d (Quantity='%s')", 
                prob_too_large[i_add],
                par->m_probColNames[i_add].c_str());
        }
      }
    
    } while (0); // End of main do-loop

    // Debug mode: Entry
    if (par->logDebug())
      Log(Log_0, " <== EXIT: Catalogue::cid_refine (status=%d)", 
          status);
    
    // Return status
    return status;

}


/*----------------------------------------------------------------------------*/
/*                          Catalogue::cid_prob_angsep                        */
/* -------------------------------------------------------------------------- */
/* Private method: calculate the counterpart probability based on angular     */
/*                 separation for all candidates                              */
/* -------------------------------------------------------------------------- */
/* TBW                                                                        */
/* -------------------------------------------------------------------------- */
/* The following members of the counterpart candidates are set:               */
/* pos_eq_ra    Right Ascension of counterpart candidate (deg)                */
/* pos_eq_dec   Declination of counterpart candidate (deg)                    */
/* pos_err_maj  Uncertainty ellipse major axis (deg)                          */
/* pos_err_min  Uncertainty ellipse minor axis (deg)                          */
/* pos_err_ang  Uncertainty ellipse positron angle (deg)                      */
/* angsep       Angular separation of counterpart candidate from source       */
/* prob_angsep  Probability of counterpart candidate based on position        */
/*----------------------------------------------------------------------------*/
Status Catalogue::cid_prob_angsep(Parameters *par, long iSrc, Status status) {

    // Declare local variables
    long        iCC;
    long        iCpt;
    double      src_dec_sin;
    double      src_dec_cos;
    double      cpt_dec_sin;
    double      cpt_dec_cos;
    double      arg;
    double      error;
    ObjectInfo *src;
    ObjectInfo *cpt;

    // Debug mode: Entry
    if (par->logDebug())
      Log(Log_0, " ==> ENTRY: Catalogue::cid_prob_angsep (%d candidates)", 
          m_numCC);

    // Single loop for common exit point
    do {
    
      // Fall through in case of an error
      if (status != STATUS_OK)
        continue;
        
      // Fall through if there are no counterpart candidates
      if (m_numCC < 1)
        continue;

      // Get pointer to source object
      src = &(m_src.object[iSrc]);
      
      // Fall through if no source position is available
      if (!src->pos_valid)
        continue;
        
      // Calculate sin and cos of source declination
      src_dec_sin = sin(src->pos_eq_dec * deg2rad);
      src_dec_cos = cos(src->pos_eq_dec * deg2rad);
    
      // Loop over counterpart candidates
      for (iCC = 0; iCC < m_numCC; iCC++) {

        // Get index of candidate in counterpart catalogue
        iCpt = m_cc[iCC].index;

        // Get pointer to counterpart object
        cpt = &(m_cpt.object[iCpt]);
        
        // Fall through if no counterpart position is available
        if (!cpt->pos_valid)
          continue;
        
        // Calculate angular separation between source and counterpart in
        // degrees. Make sure that the separation is always comprised between 
        // [0,180] (out of range arguments lead to a floating exception)
        cpt_dec_sin = sin(cpt->pos_eq_dec * deg2rad);
        cpt_dec_cos = cos(cpt->pos_eq_dec * deg2rad);
        arg         = src_dec_sin * cpt_dec_sin +
                      src_dec_cos * cpt_dec_cos * 
                      cos((src->pos_eq_ra - cpt->pos_eq_ra)*deg2rad);
        if (arg <= -1.0)
          m_cc[iCC].angsep = 180.0;
        else if (arg >= 1.0)
          m_cc[iCC].angsep = 0.0;
        else
          m_cc[iCC].angsep = acos(arg) * rad2deg;

        // Calculate counterpart probability from angular separation
        switch (par->m_posProbType) {
        case Exponential:
          error = sqrt(cpt->pos_err_maj * cpt->pos_err_maj +
                       src->pos_err_maj * src->pos_err_maj);
          if (error > 0.0)
            m_cc[iCC].prob_angsep = exp(-m_cc[iCC].angsep / error);
          else
            m_cc[iCC].prob_angsep = 0.0;
          break;
        case Gaussian:
          m_cc[iCC].prob_angsep = 1.0;
          break;
        default:
          m_cc[iCC].prob_angsep = 1.0;
          break;
        }

        // Assign position and error ellipse
        // DUMMY: we assign the position and error ellipse of the partner
        //        that has the smaller positional uncertainty
        if (cpt->pos_err_maj < src->pos_err_maj) {
          m_cc[iCC].pos_eq_ra   = cpt->pos_eq_ra;
          m_cc[iCC].pos_eq_dec  = cpt->pos_eq_dec;
          m_cc[iCC].pos_err_maj = cpt->pos_err_maj;
          m_cc[iCC].pos_err_min = cpt->pos_err_min;
          m_cc[iCC].pos_err_ang = cpt->pos_err_ang;
        }
        else {
          m_cc[iCC].pos_eq_ra   = src->pos_eq_ra;
          m_cc[iCC].pos_eq_dec  = src->pos_eq_dec;
          m_cc[iCC].pos_err_maj = src->pos_err_maj;
          m_cc[iCC].pos_err_min = src->pos_err_min;
          m_cc[iCC].pos_err_ang = src->pos_err_ang;
        }
              
      } // endfor: looped over counterpart candidates

    } while (0); // End of main do-loop

    // Debug mode: Entry
    if (par->logDebug())
      Log(Log_0, " <== EXIT: Catalogue::cid_prob_angsep (status=%d)", 
          status);
    
    // Return status
    return status;

}


/*----------------------------------------------------------------------------*/
/*                             Catalogue::cid_sort                            */
/* -------------------------------------------------------------------------- */
/* Private method: sort counterpart candidates by increasing probability and  */
/* in case of equal probability) by decreasing angular separation.            */
/*----------------------------------------------------------------------------*/
Status Catalogue::cid_sort(Parameters *par, Status status) {

    // Declare local variables
    long      iCC;
    long      jCC;
    long      imax;
    double    max_prob;
    double    min_sep;
    CCElement swap;

    // Debug mode: Entry
    if (par->logDebug())
      Log(Log_0, " ==> ENTRY: Catalogue::cid_sort");

    // Single loop for common exit point
    do {
    
      // Fall through in case of an error
      if (status != STATUS_OK)
        continue;
        
      // Fall through if there are no counterpart candidates
      if (m_numCC < 1)
        continue;
    
      // Sort counterpart candidats (dirty brute force method, to be replaced
      // by more efficient method if needed ...)
      for (iCC = 0; iCC < m_numCC; iCC++) {
        max_prob =   0.0;
        min_sep  = 180.0;
        imax     = iCC;
        for (jCC = iCC; jCC < m_numCC; jCC++) {

          // Check if we found a better candidate
          if (m_cc[jCC].prob > max_prob) {        // Increasing probability
            imax     = jCC;
            max_prob = m_cc[jCC].prob;
            min_sep  = m_cc[jCC].angsep;
          }
          else if (m_cc[jCC].prob == max_prob) {  // Equal probability &
            if (m_cc[jCC].angsep < min_sep) {     // decreasing separation
              imax     = jCC;
              max_prob = m_cc[jCC].prob;
              min_sep  = m_cc[jCC].angsep;
            }
          }

        }
        if (iCC != imax) {
          swap       = m_cc[iCC];
          m_cc[iCC]  = m_cc[imax];
          m_cc[imax] = swap;
        } 
      }
    
    } while (0); // End of main do-loop

    // Debug mode: Entry
    if (par->logDebug())
      Log(Log_0, " <== EXIT: Catalogue::cid_sort (status=%d)", status);
    
    // Return status
    return status;

}


/*----------------------------------------------------------------------------*/
/*                            Catalogue::cid_dump                             */
/* -------------------------------------------------------------------------- */
/* Private method: dump counterpart candidates that are actually in buffer    */
/*----------------------------------------------------------------------------*/
Status Catalogue::cid_dump(Parameters *par, Status status) {

    // Declare local variables
    long                            iCC;
    long                            iCpt;
    std::vector<double>::size_type  i_add;
    std::vector<double>::size_type  num_add;
    ObjectInfo                     *cpt;

    // Debug mode: Entry
    if (par->logDebug())
      Log(Log_0, " ==> ENTRY: Catalogue::cid_dump");

    // Single loop for common exit point
    do {
    
      // Fall through in case of an error
      if (status != STATUS_OK)
        continue;
        
      // Fall through if there are no counterpart candidates
      if (m_numCC < 1)
        continue;

      // Loop over counterpart candidates
      for (iCC = 0; iCC < m_numCC; iCC++) {
      
        // Get index of candidate in counterpart catalogue
        iCpt = m_cc[iCC].index;

        // Get pointer to counterpart object
        cpt = &(m_cpt.object[iCpt]);
        
        // Determine number of additional probabilites
        num_add = m_cc[iCC].prob_add.size();      

        // Normal log level
        if (par->logNormal()) {
          if (cpt->pos_valid) {
            Log(Log_2, "  Cpart %5d P=%7.3f%% S=%7.2f': %18s"SRC_FORMAT,
                iCC+1, 
                m_cc[iCC].prob*100.0, m_cc[iCC].angsep*60.0,
                cpt->name.c_str(), 
                cpt->pos_eq_ra, cpt->pos_eq_dec, 
                cpt->pos_err_maj, cpt->pos_err_min, cpt->pos_err_ang);
          }
          else {
            Log(Log_2, "  Cpart %5d P=%7.3f%% S=.......': %18s"
                " No position information found",
                iCC+1, 
                m_cc[iCC].prob*100.0,
                cpt->name.c_str());
          }
        }

        // Explicit log level
        if (par->logVerbose()) {
          if (cpt->pos_valid) {
            if (num_add > 0) {
              Log(Log_2, "   Angular separation probability .: %7.3f %%",
                  m_cc[iCC].prob_angsep*100.0);
              for (i_add = 0; i_add < num_add; i_add++) {
                Log(Log_2, 
                    "   Additional probability .........: %7.3f %%"
                    "  (Quantity='%s')",
                    m_cc[iCC].prob_add[i_add]*100.0,
                    par->m_probColNames[i_add].c_str());
              }
            }
          }
        }

      } // endfor: looped over counterpart candidats
    
    } while (0); // End of main do-loop

    // Debug mode: Entry
    if (par->logDebug())
      Log(Log_0, " <== EXIT: Catalogue::cid_dump (status=%d)", status);
    
    // Return status
    return status;

}


/*----------------------------------------------------------------------------*/
/*                      Catalogue::cid_assign_src_name                        */
/* -------------------------------------------------------------------------- */
/* Private method: assign source name if string                               */
/*----------------------------------------------------------------------------*/
std::string Catalogue::cid_assign_src_name(std::string name, int row) {

    // If string is empty then assign a default name
    std::string empty = name;
    std::remove(empty.begin(), empty.end(), ' ');
    if (empty == " ") {
      std::ostringstream number;
      number << (row+1);
      name = "CAT_ROW_" + number.str();
    }

    // Return name
    return name;
    
}


/* Namespace ends ___________________________________________________________ */
}
