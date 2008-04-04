/*------------------------------------------------------------------------------
Id ........: $Id: Catalogue_id.cxx,v 1.20 2008/03/26 16:57:30 jurgen Exp $
Author ....: $Author: jurgen $
Revision ..: $Revision: 1.20 $
Date ......: $Date: 2008/03/26 16:57:30 $
--------------------------------------------------------------------------------
$Log: Catalogue_id.cxx,v $
Revision 1.20  2008/03/26 16:57:30  jurgen
implement global counterpart density evaluation

Revision 1.19  2008/03/26 13:37:10  jurgen
Generalize probability calculation and implement Bayesian method

Revision 1.18  2008/03/21 16:42:56  jurgen
Update documentation

Revision 1.17  2008/03/21 15:27:03  jurgen
Estimate number of false associations

Revision 1.16  2008/03/21 09:10:12  jurgen
Enhance code documentation.

Revision 1.15  2008/03/20 21:56:26  jurgen
implement local counterpart density

Revision 1.14  2008/02/23 10:36:57  jurgen
remove redundant catalogAccess header inclusion

Revision 1.13  2007/11/08 11:18:31  jurgen
Correctly handle missing name column

Revision 1.12  2007/10/11 13:20:54  jurgen
Correctly remove FITS special function columns

Revision 1.11  2007/10/09 16:46:23  jurgen
Write counterpart catalogue reference (row) to output catalogue

Revision 1.10  2007/10/09 08:17:40  jurgen
Correctly interpret positional errors and correctly evaluate PROB_POS
as likelihood

Revision 1.9  2007/10/08 11:02:25  jurgen
Implement search for catalogue table information and handle different
position error types

Revision 1.8  2007/10/03 09:06:08  jurgen
Add chance coincidence probability PROB_CHANCE

Revision 1.7  2007/10/02 22:01:16  jurgen
Change parameter name maxNumCtp to maxNumCpt

Revision 1.6  2007/09/21 20:27:14  jurgen
Correct cfits_collect bug (unstable row selection)

Revision 1.5  2007/09/21 14:29:03  jurgen
Correct memory bug and updated test script

Revision 1.4  2007/09/21 12:49:10  jurgen
Enhance log-file output and chatter level

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
/**
 * @file Catalogue_id.cxx
 * @brief Implements source identification methods of Catalogue class.
 * @author J. Knodlseder
 */

/* Includes _________________________________________________________________ */
#include "sourceIdentify.h"
#include "Catalogue.h"
#include "Log.h"


/* Definitions ______________________________________________________________ */
#define CATALOGUE_TIMING   0                     // Enables timing measurements


/* Namespace definition _____________________________________________________ */
namespace sourceIdentify {
using namespace catalogAccess;


/* Type defintions __________________________________________________________ */


/* Private Prototypes _______________________________________________________ */


/**************************************************************************//**
 * @brief Get counterpart candidates for a source
 *
 * @param[in] par Pointer to gtsrcid parameters.
 * @param[in] src Pointer to source information.
 * @param[in] status Error status.
 *
 * This is the main counterpart association driving routine. Counterpart
 * association is done in a two step process:\n
 * (1) a filter step, and \n
 * (2) a refine step.
 *
 * The filter step gathers all counterparts within a rectangular bounding box
 * (in Right Ascension and Declination) that encloses a circular region around
 * the source position. The radius of the circular region is given by
 * Catalogue::m_filter_maxsep.
 *
 * The refine step calculates the association probabilities for all filtered
 * sources.
 *
 * Finally, all candidates are added to the output catalogue.
 ******************************************************************************/
Status Catalogue::cid_get(Parameters *par, SourceInfo *src, Status status) {

    // Debug mode: Entry
    if (par->logDebug())
      Log(Log_0, " ==> ENTRY: Catalogue::cid_get");

    // Single loop for common exit point
    do {

      // Dump source information (optionally)
      if (par->logNormal()) {
        if (src->info->pos_valid) {
          Log(Log_2, " Source %5d .....................: %20s"SRC_FORMAT,
              src->iSrc+1, src->info->name.c_str(),
              src->info->pos_eq_ra, src->info->pos_eq_dec,
              src->info->pos_err_maj, src->info->pos_err_min,
              src->info->pos_err_ang);
        }
        else {
          Log(Log_2, " Source %5d .....................: %20s"
              " No position information found.",
              src->iSrc+1, src->info->name.c_str());
        }
      }

      // Fall through if no position information has been found
      if (!src->info->pos_valid)
        continue;

      // Filter step: Get counterparts near the source position
      status = cid_filter(par, src, status);
      if (status != STATUS_OK) {
        if (par->logTerse())
          Log(Error_2, "%d : Unable to perform filter step for source %d.",
              (Status)status, src->iSrc+1);
        continue;
      }

      // Refine step: Assign probability for each counterpart
      status = cid_refine(par, src, status);
      if (status != STATUS_OK) {
        if (par->logTerse())
          Log(Error_2, "%d : Unable to perform refine step for source %d.",
              (Status)status, src->iSrc+1);
        continue;
      }

      // Optionally dump counterpart candidats
      if (par->logNormal()) {
        cid_dump(par, src, status);
        Log(Log_2, "");
      }

      // Add all counterpart candidates to output catalogue
      status = cfits_add(m_outFile, par, src, status);
      if (status != STATUS_OK) {
        if (par->logTerse())
          Log(Error_2, "%d : Unable to add counterpart candidates for source"
              " %d to FITS output catalogue '%s'.", 
              (Status)status, src->iSrc+1, par->m_outCatName.c_str());
        continue;
      }

     } while (0); // End of main do-loop

    // Debug mode: Entry
    if (par->logDebug())
      Log(Log_0, " <== EXIT: Catalogue::cid_get (status=%d)",
          status);

    // Return status
    return status;

}


/**************************************************************************//**
 * @brief Filter step of counterpart identification
 *
 * @param[in] par Pointer to gtsrcid parameters.
 * @param[in] src Pointer to source information.
 * @param[in] status Error status.
 *
 * @todo Load counterpart catalogue only partially if number of objects is large.
 * @todo Proper prior assignment
 *
 * The filter step gets all counterpart candidates from the catalogue for a
 * given source that are sufficiently close to the source.
 * This is done by defining a rectangular bounding box around the source
 * that extends from -m_filter_rad to +m_filter_rad in Declination
 * and from -m_filter_rad/cos(dec) to +m_filter_rad/cos(dec) in Right
 * Ascension.
 ******************************************************************************/
Status Catalogue::cid_filter(Parameters *par, SourceInfo *src, Status status) {

    // Declare local variables
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
      numNoPos = 0;
      numRA    = 0;
      numDec   = 0;

      // Calculate sin and cos of source latitude
      src_dec_sin = sin(src->info->pos_eq_dec * deg2rad);
      src_dec_cos = cos(src->info->pos_eq_dec * deg2rad);

      // Set bounding box enclosing radius
      double radius   = src->info->pos_err_maj * 5.0 / 2.0; // 5 sigma radius
      src->filter_rad = (radius > c_filter_maxsep) ? radius : c_filter_maxsep;

      // Set local counterpart density ring radii
      src->ring_rad_min = 0.0;
      src->ring_rad_max = src->filter_rad;

      // Compute solid angle of error ellipse
      src->omega = pi * src->info->pos_err_maj * src->info->pos_err_min;

      // Define bounding box around source position. The declination
      // range of the bounding box is constrained to [-90,90] deg, the
      // Right Ascension boundaries are put into the interval [0,360[ deg.
      cpt_dec_min = src->info->pos_eq_dec - src->filter_rad;
      cpt_dec_max = src->info->pos_eq_dec + src->filter_rad;
      if (cpt_dec_min < -90.0) cpt_dec_min = -90.0;
      if (cpt_dec_max >  90.0) cpt_dec_max =  90.0;
      if (src_dec_cos > 0.0) {
        filter_maxsep = src->filter_rad / src_dec_cos;
        if (filter_maxsep > 180.0)
          filter_maxsep = 180.0;
      }
      else
        filter_maxsep = 180.0;
      cpt_ra_min  = src->info->pos_eq_ra - filter_maxsep;
      cpt_ra_max  = src->info->pos_eq_ra + filter_maxsep;
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
          status = get_input_catalogue(par, &m_cpt, par->m_cptPosError, status);
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

          // Bookkeep catalogue loading
          m_fCptLoaded = 1;

        } // endif: catalogue was not yet loaded
      } // endelse: entire catalogue requested

      // Start timing
      #if CATALOGUE_TIMING
      t_start_loop = clock();
      #endif

      // Determine number of counterpart candidates that fall in the
      // bounding box and that have a valid position
      cpt = m_cpt.object;
      for (int iCpt = 0; iCpt < m_cpt.numLoad; iCpt++, cpt++) {

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

        // If we are still alive then we count this counterpart
        src->numCC++;

      } // endfor: looped over all counterpart candidates

      // Collect all counterpart candidates
      if (src->numCC > 0) {

        // Allocate memory for counterpart candidates
        src->cc = new CCElement[src->numCC];
        if (src->cc == NULL) {
          status = STATUS_MEM_ALLOC;
          if (par->logTerse())
            Log(Error_2, "%d : Memory allocation failure.", (Status)status);
          continue;
        }


        // Collect all counterpart candidates
        cpt     = m_cpt.object;
        cpt_ptr = src->cc;
        for (int iCpt = 0; iCpt < m_cpt.numLoad; iCpt++, cpt++) {

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

          // If we are still alive then we keep this counterpart
          cpt_ptr->id          = "NULL";
          cpt_ptr->pos_eq_ra   = 0.0;
          cpt_ptr->pos_eq_dec  = 0.0;
          cpt_ptr->pos_err_maj = 0.0;
          cpt_ptr->pos_err_min = 0.0;
          cpt_ptr->pos_err_ang = 0.0;
          cpt_ptr->prob        = 0.0;
          cpt_ptr->index       = iCpt;
          cpt_ptr->angsep      = 0.0;
          cpt_ptr->psi         = 0.0;
          cpt_ptr->posang      = 0.0;
          cpt_ptr->lambda      = 0.0;
          cpt_ptr->prob_pos    = 0.0;
          cpt_ptr->prob_chance = 0.0;
          cpt_ptr->prob_prior  = 0.0;
          cpt_ptr->prob_post   = 0.0;
          cpt_ptr->pdf_pos     = 0.0;
          cpt_ptr->pdf_chance  = 0.0;
          cpt_ptr->likrat      = 0.0;
          cpt_ptr++;

        } // endfor: looped over all counterpart candidates
      } // endif: there were counterpart candidates

      // Optionally dump counterpart filter statistics
      if (par->logExplicit()) {
        Log(Log_2, "  Filter step candidates ..........: %5d", src->numCC);
        if (par->logVerbose()) {
          Log(Log_2, "    Filter bounding box radius ....: %7.3f deg",
              src->filter_rad);
          Log(Log_2, "    Error ellipse solid angle .....: %7.3f deg^2",
              src->omega);
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


/**************************************************************************//**
 * @brief Refine step of counterpart identification
 *
 * @param[in] par Pointer to gtsrcid parameters.
 * @param[in] src Pointer to source information.
 * @param[in] status Error status.
 *
 * @todo Develop other than local counterpart density methods.
 *
 * Calculates the counterpart probability, sorts all counterpart candidates
 * by decreasing probability and eliminates all candidtates with a too low
 * probability.
 ******************************************************************************/
Status Catalogue::cid_refine(Parameters *par, SourceInfo *src, Status status) {

    // Declare local variables

    // Debug mode: Entry
    if (par->logDebug())
      Log(Log_0, " ==> ENTRY: Catalogue::cid_refine (%d candidates)",
          src->numCC);

    // Single loop for common exit point
    do {

      // Fall through in case of an error
      if (status != STATUS_OK)
        continue;

      // Store number of counterpart candidates before selection
      m_cpt_stat[src->iSrc*(m_num_Sel+1)] = src->numCC;

      // Fall through if there are no counterpart candidates
      if (src->numCC < 1)
        continue;

      // Set unique counterpart candidate identifier for in-memory catalogue
      char cid[OUTCAT_MAX_STRING_LEN];
      for (int iCC = 0; iCC < src->numCC; ++iCC) {
        sprintf(cid, "CC_%5.5d_%5.5d", src->iSrc+1, iCC+1);
        src->cc[iCC].id = cid;
      }

      // Compute PROB_POS and PDF_POS. We have to do this before setting up
      // the in-memory catalogue in order to make sure that the in-memory
      // catalogue contains the required information
      status = cid_prob_pos(par, src, status);
      if (status != STATUS_OK) {
        if (par->logTerse())
          Log(Error_2, "%d : Unable to compute PROB_POS.", (Status)status);
        continue;
      }

      // Clear in-memory catalogue
      status = cfits_clear(m_memFile, par, status);
      if (status != STATUS_OK) {
        if (par->logTerse())
          Log(Error_2, "%d : Unable to clear in-memory FITS catalogue.",
                       (Status)status);
        continue;
      }

      // Setup in-memory catalogue
      status = cfits_add(m_memFile, par, src, status);
      if (status != STATUS_OK) {
        if (par->logTerse())
          Log(Error_2, "%d : Unable to add counterpart candidates to"
                       " in-memory FITS catalogue.", (Status)status);
        continue;
      }

      // Evaluate in-memory catalogue quantities
      status = cfits_eval(m_memFile, par, status);
      if (status != STATUS_OK) {
        if (par->logTerse())
          Log(Error_2, "%d : Unable to evaluate new quantities in in-memory"
                       " FITS catalogue.", (Status)status);
        continue;
      }

      // Compute PROB_PRIOR
      status = cid_prob_prior(par, src, status);
      if (status != STATUS_OK) {
        if (par->logTerse())
          Log(Error_2, "%d : Unable to compute PROB_PRIOR.", (Status)status);
        continue;
      }

      // Select counterparts on basis of the selection string. All catalogue
      // quantities may be used here except of PROB_CHANCE, PDF_CHANCE,
      // PROB_POST and PROB (these are calculated later!)
      status = cid_select(par, src, status);
      if (status != STATUS_OK) {
        if (par->logTerse())
          Log(Error_2, "%d : Unable to select counterparts.", (Status)status);
        continue;
      }

      // Determine counterpart density
      status = cid_local_density(par, src, status);
//      status = cid_global_density(par, src, status);
      if (status != STATUS_OK) {
        if (par->logTerse())
          Log(Error_2, "%d : Unable to determine counterpart density.",
              (Status)status);
        continue;
      }

      // Compute PROB_CHANCE and PDF_CHANCE
      status = cid_prob_chance(par, src, status);
      if (status != STATUS_OK) {
        if (par->logTerse())
          Log(Error_2, "%d : Unable to determine chance coincidence probability.",
              (Status)status);
        continue;
      }

      // Compute PROB_POST
      status = cid_prob_post(par, src, status);
      if (status != STATUS_OK) {
        if (par->logTerse())
          Log(Error_2, "%d : Unable to determine posterior probability.",
              (Status)status);
        continue;
      }

      // Compute PROB
      status = cid_prob(par, src, status);
      if (status != STATUS_OK) {
        if (par->logTerse())
          Log(Error_2, "%d : Unable to determine association probability.",
              (Status)status);
        continue;
      }

      // Sort counterpart candidates by decreasing probability
      status = cid_sort(par, src, status);
      if (status != STATUS_OK) {
        if (par->logTerse())
          Log(Error_2, "%d : Unable to sort counterpart candidates.",
              (Status)status);
        continue;
      }

      // Sum up probabilities (before thresholding)
      src->cc_pid = 0.0;
      src->cc_pc  = 0.0;
      int n_sum = (src->numCC > 0) ? 1 : 0;
//      int n_sum = (src->numCC > 0) ? src->numCC : 0;
      for (int iCC = 0; iCC < n_sum; ++iCC) {
        src->cc_pid += src->cc[iCC].prob;
        src->cc_pc  += 1.0 - src->cc[iCC].prob;
      }

      // Determine the number of counterpart candidates above the probability
      // threshold
      int numUseCC = 0;
      for (int iCC = 0; iCC < src->numCC; ++iCC) {
        if (src->cc[iCC].prob >= par->m_probThres)
          numUseCC++;
        else
          break;
      }

      // Apply the maximum number of counterpart threshold
      if (numUseCC > par->m_maxNumCpt)
        numUseCC = par->m_maxNumCpt;

      // Eliminate counterpart candidates below threshold.
      src->numCC = numUseCC;

      // Fall through if no counterparts are left
      if (src->numCC < 1) {
        if (par->logExplicit())
          Log(Log_2, "  Refine step candidates ..........: no");
        continue;
      }

      // Sum up probabilities (after thresholding)
      src->cc_pid_thr = 0.0;
      src->cc_pc_thr  = 0.0;
      n_sum = (src->numCC > 0) ? 1 : 0;
//      n_sum = (src->numCC > 0) ? src->numCC : 0;
      for (int iCC = 0; iCC < n_sum; ++iCC) {
        src->cc_pid_thr += src->cc[iCC].prob;
        src->cc_pc_thr  += 1.0 - src->cc[iCC].prob;
      }

      // Set unique counterpart candidate identifier (overwrite former ID)
      for (int iCC = 0; iCC < src->numCC; ++iCC) {
        sprintf(cid, "CC_%5.5d_%5.5d", src->iSrc+1, iCC+1);
        src->cc[iCC].id = cid;
      }

      // Optionally dump counterpart refine statistics
      if (par->logExplicit()) {
        Log(Log_2, "  Refine step candidates ..........: %5d", src->numCC);
        Log(Log_2, "    Expected real associations ....: %.1f", src->cc_pid);
        Log(Log_2, "    Local counterpart density .....: %.5f src/deg^2", src->rho);
        Log(Log_2, "    Local density ring ............: %.3f - %.3f deg",
            src->ring_rad_min, src->ring_rad_max);
    }

    } while (0); // End of main do-loop

    // Debug mode: Entry
    if (par->logDebug())
      Log(Log_0, " <== EXIT: Catalogue::cid_refine (status=%d)",
          status);

    // Return status
    return status;

}


/**************************************************************************//**
 * @brief Select counterparts
 *
 * @param[in] par Pointer to gtsrcid parameters.
 * @param[in] src Pointer of source information.
 * @param[in] status Error status.
 *
 * Only the counterparts are retained that satisfy the specified selection
 * criteria. Selection is performed using the CFITSIO row selection
 * function.
 *
 * Requires catalogue information in FITS memory file.
 ******************************************************************************/
Status Catalogue::cid_select(Parameters *par, SourceInfo *src, Status status) {

    // Declare local variables
    std::vector<std::string> col_id;

    // Debug mode: Entry
    if (par->logDebug())
      Log(Log_0, " ==> ENTRY: Catalogue::cid_select (%d candidates)", src->numCC);

    // Single loop for common exit point
    do {

      // Fall through in case of an error
      if (status != STATUS_OK)
        continue;

      // Fall through if no selection strings are specified.
      int m_num_Sel = par->m_select.size();
      if (m_num_Sel < 1)
        continue;

      // Select counterparts in memory
      status = cfits_select(m_memFile, par, src, status);
      if (status != STATUS_OK) {
        if (par->logTerse())
          Log(Error_2, "%d : Unable to select catalogue counterparts.",
              (Status)status);
        continue;
      }

      // Get list of counterpart IDs that survived
      status = cfits_get_col_str(m_memFile, par, OUTCAT_COL_ID_NAME, col_id,
                                 status);
      if (status != STATUS_OK) {
        if (par->logTerse())
          Log(Error_2, "%d : Unable to read counterpart IDs from memory.",
              (Status)status);
        continue;
      }

      // If list is empty then stop now
      int nSelected = (int)col_id.size();
      if (nSelected < 1) {
        src->numCC = 0;
        continue;
      }

      // Collect all counterparts that survived
      int inx = 0;
      for (int iCC = 0; iCC < src->numCC; ++iCC) {
        for (int i = 0; i < nSelected; ++i) {
          if (src->cc[iCC].id == col_id[i]) {
            src->cc[inx] = src->cc[iCC];
            inx++;
          }
        }
      }

      // Check that we found everybody
      if (inx != nSelected) {
        status = STATUS_CAT_SEL_FAILED;
        if (par->logTerse())
          Log(Error_2, "%d : In-memory counterpart selection error (%d/%d).",
              (Status)status, inx, nSelected);
        continue;
      }

      // Set number of remaining counterparts
      src->numCC = nSelected;

    } while (0); // End of main do-loop

    // Debug mode: Entry
    if (par->logDebug())
      Log(Log_0, " <== EXIT: Catalogue::cid_select (status=%d)",
          status);

    // Return status
    return status;

}


/**************************************************************************//**
 * @brief Calculate the counterpart probability based on position
 *
 * @param[in] par Pointer to gtsrcid parameters.
 * @param[in] src Pointer of source information.
 * @param[in] status Error status.
 *
 * Computes the probability \b PROB_POS that the true source position is
 * located at an angular distance greater than \f$ r \f$ from the position
 * measured by LAT.
 * The computation is done using
 * \f[ {\tt PROB\_POS} = \exp(-5.991 r^2 / \Psi^2) \f]
 * where \f$ \Psi \f$ is the effective 95% error radius of the LAT source.
 * \f$ \Psi \f$ is computed using
 * \f[ \Psi = \left[ \frac{\cos^2(\theta-\phi)}{a^2} + 
 *                   \frac{\sin^2(\theta-\phi)}{b^2}\right]^{-1/2} \f]
 * where
 * \f$ a \f$ and \f$ b \f$ are the semimajor and semiminor axes of the
 * LAT error ellipse,
 * \f$ \phi \f$ is the position angle of the LAT error ellipse (measured
 * eastwards from north in celestial coordinates) and
 * \f$ \theta \f$ is the position angle of the counterpart source with respect
 * to the LAT source.
 *
 * Further computes \b PDF_POS, which is the probability density of
 * 1-PROB_POS.
 * The probability density is computed using
 * \f[ {\tt PDF\_POS} = 5.991 \frac{r}{\Psi^2} exp(-2.996 r^2 / \Psi^2) \f]
 *
 * @todo Define more intelligent scheme to attribute counterpart position errors.
 *
 * This method updates the following fields \n
 * CCElement::angsep (angular separation of counterpart candidate from source) \n
 * CCElement::posang (position angle of counterpart candidate w/r to source) \n
 * CCElement::psi (effective 95% error ellipse radius) \n
 * CCElement::prob_pos (position association probability) \n
 * CCElement::pdf_pos (position association probability density) \n
 * CCElement::pos_eq_ra (Right Ascension of counterpart candidate) \n
 * CCElement::pos_eq_dec (Declination of counterpart candidate) \n
 * CCElement::pos_err_maj (uncertainty ellipse major axis) \n
 * CCElement::pos_err_min (uncertainty ellipse minor axis) \n
 * CCElement::pos_err_ang (uncertainty ellipse positron angle) \n
 ******************************************************************************/
Status Catalogue::cid_prob_pos(Parameters *par, SourceInfo *src, Status status) {

    // Debug mode: Entry
    if (par->logDebug())
      Log(Log_0, " ==> ENTRY: Catalogue::cid_prob_pos (%d candidates)", src->numCC);

    // Single loop for common exit point
    do {

      // Fall through in case of an error
      if (status != STATUS_OK)
        continue;

      // Fall through if there are no counterpart candidates
      if (src->numCC < 1)
        continue;

      // Fall through if no source position is available
      if (!src->info->pos_valid)
        continue;

      // Calculate sin and cos of source declination
      double dec         = src->info->pos_eq_dec * deg2rad;
      double src_dec_sin = sin(dec);
      double src_dec_cos = cos(dec);

      // Loop over counterpart candidates
      for (int iCC = 0; iCC < src->numCC; ++iCC) {

        // Get index of candidate in counterpart catalogue
        int iCpt = src->cc[iCC].index;

        // Get pointer to counterpart object
        ObjectInfo *cpt = &(m_cpt.object[iCpt]);

        // Fall through if no counterpart position is available
        if (!cpt->pos_valid)
          continue;

        // Perform trigonometric operations to prepare angular separation
        // and position angle calculations
        double ra_diff     = (cpt->pos_eq_ra - src->info->pos_eq_ra) * deg2rad;
        double dec         = cpt->pos_eq_dec * deg2rad;
        double cpt_dec_sin = sin(dec);
        double cpt_dec_cos = cos(dec);
        double cpt_dec_tan = tan(dec);
        double arg         = src_dec_sin * cpt_dec_sin +
                             src_dec_cos * cpt_dec_cos * cos(ra_diff);

        // Calculate angular separation between source and counterpart in
        // degrees. Make sure that the separation is always comprised between
        // [0,180] (out of range arguments lead to a floating exception).
        if (arg <= -1.0)
          src->cc[iCC].angsep = 180.0;
        else if (arg >= 1.0)
          src->cc[iCC].angsep = 0.0;
        else
          src->cc[iCC].angsep = acos(arg) * rad2deg;

        // Calculate position angle, counterclockwise from celestial north
        src->cc[iCC].posang = atan2(sin(ra_diff), src_dec_cos*cpt_dec_tan -
                                    src_dec_sin*cos(ra_diff)) * rad2deg;

        // Calculate 95% source error ellipse
        double angle     = (src->cc[iCC].posang - src->info->pos_err_ang) * deg2rad;
        double cos_angle = cos(angle);
        double sin_angle = sin(angle);
        double a         = (src->info->pos_err_maj > 0.0) ? (cos_angle*cos_angle) /
                           (src->info->pos_err_maj*src->info->pos_err_maj) : 0.0;
        double b         = (src->info->pos_err_min > 0.0) ? (sin_angle*sin_angle) /
                           (src->info->pos_err_min*src->info->pos_err_min) : 0.0;
        arg              = a + b;
        double error2    = (arg > 0.0) ? 1.0/arg : 0.0;

        // Calculate counterpart probability from angular separation
        if (error2 > 0.0) {
          double arg1           = src->cc[iCC].angsep / error2;
          double arg2           = src->cc[iCC].angsep * arg1;
          double expval         = exp(-dnorm * arg2);
          src->cc[iCC].psi      = sqrt(error2);
          src->cc[iCC].pdf_pos  = twodnorm * arg1 * expval;
          src->cc[iCC].prob_pos = expval;
        }
        else {
          src->cc[iCC].psi      = 0.0;
          src->cc[iCC].pdf_pos  = 0.0;
          src->cc[iCC].prob_pos = 0.0;
        }

        // Assign position and error ellipse
        // DUMMY: we assign the position and error ellipse of the partner
        //        that has the smaller positional uncertainty
        if (cpt->pos_err_maj < src->info->pos_err_maj) {
          src->cc[iCC].pos_eq_ra   = cpt->pos_eq_ra;
          src->cc[iCC].pos_eq_dec  = cpt->pos_eq_dec;
          src->cc[iCC].pos_err_maj = cpt->pos_err_maj;
          src->cc[iCC].pos_err_min = cpt->pos_err_min;
          src->cc[iCC].pos_err_ang = cpt->pos_err_ang;
        }
        else {
          src->cc[iCC].pos_eq_ra   = src->info->pos_eq_ra;
          src->cc[iCC].pos_eq_dec  = src->info->pos_eq_dec;
          src->cc[iCC].pos_err_maj = src->info->pos_err_maj;
          src->cc[iCC].pos_err_min = src->info->pos_err_min;
          src->cc[iCC].pos_err_ang = src->info->pos_err_ang;
        }

      } // endfor: looped over counterpart candidates

    } while (0); // End of main do-loop

    // Debug mode: Entry
    if (par->logDebug())
      Log(Log_0, " <== EXIT: Catalogue::cid_prob_pos (status=%d)",
          status);

    // Return status
    return status;

}


/**************************************************************************//**
 * @brief Computes chance coincidence probability and probability density
 *
 * @param[in] par Pointer to gtsrcid parameters.
 * @param[in] src Pointer to source information.
 * @param[in] status Error status.
 *
 * Computes the chance coincidence probability \b PROB_CHANCE and the
 * chance coincidence probability density \b PDF_CHANCE for a given LAT
 * source.
 * The chance coincidence probability is computed using
 * \f[ {\tt PROB\_CHANCE} = 1 - \exp(-r^2 / r_0^2) \f]
 * where \f$ r_0 \f$ is the characteristic angle between confusing sources
 * and \f$ r \f$ is the angular separation between the LAT source and the
 * counterpart candidate.
 * The chance coincidence probability density is computed using
 * \f[ {\tt PDF\_CHANCE} = 2 \frac{r}{r_0^2} \exp(-r^2 / r_0^2) \f]
 *
 * This method updates the following fields \n
 * CCElement::lambda (expected number of confusing sources)\n
 * CCElement::prob_chance (chance coincidence probability)\n
 * CCElement::pdf_chance (chance coincidence probability density)\n
 * The method also updates the corresponding columns in the in-memory FITS file.
 ******************************************************************************/
Status Catalogue::cid_prob_chance(Parameters *par, SourceInfo *src, Status status) {

    // Debug mode: Entry
    if (par->logDebug())
      Log(Log_0, " ==> ENTRY: Catalogue::cid_prob_chance (%d candidates)",
          src->numCC);

    // Single loop for common exit point
    do {

      // Fall through in case of an error
      if (status != STATUS_OK)
        continue;

      // Fall through if there are no counterpart candidates
      if (src->numCC < 1)
        continue;

      // Fall through if no source position is available
      if (!src->info->pos_valid)
        continue;

      // Allocate vector columns for in-memory FITS file update
      std::vector<double> col_lambda;
      std::vector<double> col_prob_chance;
      std::vector<double> col_pdf_chance;

      // Compute chance coincidence probabilities for all sources
      for (int iCC = 0; iCC < src->numCC; iCC++) {

        // Get index of candidate in counterpart catalogue
        int iCpt = src->cc[iCC].index;

        // Get pointer to counterpart object
        ObjectInfo *cpt = &(m_cpt.object[iCpt]);

        // Fall through if no counterpart position is available
        if (!cpt->pos_valid)
          continue;

        // Compute the expected number of sources within the area given
        // by the angular separation between source and counterpart
        double r_div_r02    = src->cc[iCC].angsep * pi * src->rho;
        src->cc[iCC].lambda = r_div_r02 * src->cc[iCC].angsep;

        // Compute chance coincidence probability
        double exp_lambda        = exp(-src->cc[iCC].lambda);
        src->cc[iCC].prob_chance = 1.0 - exp_lambda;
        src->cc[iCC].pdf_chance  = 2.0 * r_div_r02 * exp_lambda;

        // Add result to column vectors
        col_lambda.push_back(src->cc[iCC].lambda);
        col_prob_chance.push_back(src->cc[iCC].prob_chance);
        col_pdf_chance.push_back(src->cc[iCC].pdf_chance);

      } // endfor: looped over all counterpart candidates

      // Update in-memory columns
      status = cfits_set_col(m_memFile, par, OUTCAT_COL_LAMBDA_NAME,
                             col_lambda, status);
      status = cfits_set_col(m_memFile, par, OUTCAT_COL_PROB_CHANCE_NAME,
                             col_prob_chance, status);
      status = cfits_set_col(m_memFile, par, OUTCAT_COL_PDF_CHANCE_NAME,
                             col_pdf_chance, status);
      if (status != STATUS_OK) {
        if (par->logTerse())
          Log(Error_2, "%d : Unable to update columns of in-memory FITS file.",
                       (Status)status);
        continue;
      }

    } while (0); // End of main do-loop

    // Debug mode: Entry
    if (par->logDebug())
      Log(Log_0, " <== EXIT: Catalogue::cid_prob_chance (status=%d)",
          status);

    // Return status
    return status;

}


/**************************************************************************//**
 * @brief Compute prior probabilities for all counterpart candidates
 *
 * @param[in] par Pointer to gtsrcid parameters.
 * @param[in] src Pointer to source information.
 * @param[in] status Error status.
 *
 * Computes \n
 * CCElement::prob_prior (prior association probability)
 *
 * The method updates the corresponding column in the in-memory FITS file.
 *
 * The prior probability is constrained to the interval [0,1].
 *
 * Requires catalogue information in FITS memory file.
 ******************************************************************************/
Status Catalogue::cid_prob_prior(Parameters *par, SourceInfo *src, Status status) {

    // Debug mode: Entry
    if (par->logDebug())
      Log(Log_0, " ==> ENTRY: Catalogue::cid_prob_prior (%d candidates)", src->numCC);

    // Single loop for common exit point
    do {

      // Fall through in case of an error
      if (status != STATUS_OK)
        continue;

      // Fall through if there are no counterpart candidates
      if (src->numCC < 1)
        continue;

      // Get column and formula
      std::string column  = "PROB_PRIOR";

      // Evaluate PROB_PRIOR column
      status = cfits_eval_column(m_memFile, par, column, par->m_probPrior, status);
      if (status != STATUS_OK) {
        if (par->logTerse())
          Log(Error_2, "%d : Unable to evaluate expression <%s='%s'> in"
                       " formula.",
                       (Status)status, column.c_str(), par->m_probPrior.c_str());
        continue;
      }

      // Extract PROB_PRIOR vector
      std::vector<double> prob_prior;
      status = cfits_get_col(m_memFile, par, column, prob_prior, status);
      if (status != STATUS_OK) {
        if (par->logTerse())
          Log(Error_2, "%d : Unable to extract column <%s> from"
                       "in-memory FITS catalogue.",
                       (Status)status, column.c_str());
        continue;
      }

      // Allocate vector column for in-memory FITS file update
      std::vector<double> col_prob_prior;

      // Set PROB_PRIOR information
      for (int iCC = 0; iCC < src->numCC; ++iCC) {

        // Get probability in the range [0,1]
        double p = prob_prior[iCC];
        if (p < 0.0)      p = 0.0;
        else if (p > 1.0) p = 1.0;

        // Save probability for each counterpart candidate
        src->cc[iCC].prob_prior = p;

        // Add result to column vector
        col_prob_prior.push_back(src->cc[iCC].prob_prior);

      } // endfor: looped over all counterpart candidates

      // Update in-memory column
      status = cfits_set_col(m_memFile, par, OUTCAT_COL_PROB_PRIOR_NAME,
                             col_prob_prior, status);
      if (status != STATUS_OK) {
        if (par->logTerse())
          Log(Error_2, "%d : Unable to update column of in-memory FITS file.",
                       (Status)status);
        continue;
      }

    } while (0); // End of main do-loop

    // Debug mode: Entry
    if (par->logDebug())
      Log(Log_0, " <== EXIT: Catalogue::cid_prob_prior (status=%d)",
          status);

    // Return status
    return status;

}


/**************************************************************************//**
 * @brief Compute posterior probabilities
 *
 * @param[in] par Pointer to gtsrcid parameters.
 * @param[in] src Pointer to source information.
 * @param[in] status Error status.
 *
 * @todo Improve zero counterpart density handling.
 *
 * Requires \n
 * CCElement::prob_prior (prior probabilities) \n
 * CCElement::psi (effective radius of 95% error ellipse) \n
 * Catalogue::m_r0 (average confusing source distance)
 *
 * Computes \n
 * CCElement::likrat (likelihood ratio) \n
 * CCElement::prob_post (posterior probabilities)
 *
 * The method updates the corresponding column in the in-memory FITS file.
 ******************************************************************************/
Status Catalogue::cid_prob_post(Parameters *par, SourceInfo *src, Status status) {

    // Debug mode: Entry
    if (par->logDebug())
      Log(Log_0, " ==> ENTRY: Catalogue::cid_prob_post (%d candidates)",
          src->numCC);

    // Single loop for common exit point
    do {

      // Fall through in case of an error
      if (status != STATUS_OK)
        continue;

      // Fall through if there are no counterpart candidates
      if (src->numCC < 1)
        continue;

      // Allocate vector columns for in-memory FITS file update
      std::vector<double> col_likrat;
      std::vector<double> col_prob_post;

      // Compute r0^2
      double r02 = (src->rho > 0.0) ? 1.0 / (pi * src->rho) : src->ring_rad_max;

      // Loop over all counterpart candidates
      for (int iCC = 0; iCC < src->numCC; ++iCC) {

        // Initialise results
        src->cc[iCC].likrat    = 0.0;
        src->cc[iCC].prob_post = 0.0;

        // Compute likelihood ratio. If Psi^2 > 2.996 r0^2 then the likelihood
        // ratio diverges. In this case we set LR = 0.
        double psi2 = src->cc[iCC].psi * src->cc[iCC].psi;
        if (psi2 > 0.0 && r02 > 0.0) {
          double scale = 2.996 / psi2 - 1.0 / r02;
          if (scale >= 0.0) {
            double arg          = scale * src->cc[iCC].angsep * src->cc[iCC].angsep;
            src->cc[iCC].likrat = 2.996 * r02 / psi2 * exp(-arg);
          }
        }

        // Compute posterior probability. There are some special cases:
        // PROB_PRIOR >= 1 => PROB_POST = 1
        // PROB_PRIOR <= 0 => PROB_POST = 0
        // LR = 0          => PROB_POST = 0
        if (src->cc[iCC].prob_prior >= 1.0)
          src->cc[iCC].prob_post = 1.0;
        else if (src->cc[iCC].prob_prior <= 0.0)
          src->cc[iCC].prob_post = 0.0;
        else {
          if (src->cc[iCC].likrat > 0.0) {
            double eta = src->cc[iCC].prob_prior / (1.0 - src->cc[iCC].prob_prior);
            double arg = eta * src->cc[iCC].likrat;
            src->cc[iCC].prob_post = (arg > 0.0) ? 1.0 / (1.0 + 1.0 / arg) : 0.0;
          }
          else
            src->cc[iCC].prob_post = 0.0;
        }

        // Add results to column vectors
        col_likrat.push_back(src->cc[iCC].likrat);
        col_prob_post.push_back(src->cc[iCC].prob_post);

      } // endfor: looped over all counterpart candidates

      // Update in-memory columns
      status = cfits_set_col(m_memFile, par, OUTCAT_COL_LR_NAME,
                             col_likrat, status);
      status = cfits_set_col(m_memFile, par, OUTCAT_COL_PROB_POST_NAME,
                             col_prob_post, status);
      if (status != STATUS_OK) {
        if (par->logTerse())
          Log(Error_2, "%d : Unable to update columns of in-memory FITS file.",
                       (Status)status);
        continue;
      }

    } while (0); // End of main do-loop

    // Debug mode: Entry
    if (par->logDebug())
      Log(Log_0, " <== EXIT: Catalogue::cid_prob_post (status=%d)",
          status);

    // Return status
    return status;

}


/**************************************************************************//**
 * @brief Compute association probability
 *
 * @param[in] par Pointer to gtsrcid parameters.
 * @param[in] src Pointer to source information.
 * @param[in] status Error status.
 *
 * Requires catalogue information in FITS memory file.
 ******************************************************************************/
Status Catalogue::cid_prob(Parameters *par, SourceInfo *src, Status status) {

    // Debug mode: Entry
    if (par->logDebug())
      Log(Log_0, " ==> ENTRY: Catalogue::cid_prob (%d candidates)", src->numCC);

    // Single loop for common exit point
    do {

      // Fall through in case of an error
      if (status != STATUS_OK)
        continue;

      // Fall through if there are no counterpart candidates
      if (src->numCC < 1)
        continue;

      // Get column and formula
      std::string column  = "PROB";

      // Evaluate PROB column
      status = cfits_eval_column(m_memFile, par, column, par->m_probMethod, status);
      if (status != STATUS_OK) {
        if (par->logTerse())
          Log(Error_2, "%d : Unable to evaluate expression <%s='%s'> in"
                       " formula.",
                       (Status)status, column.c_str(), par->m_probMethod.c_str());
        continue;
      }

      // Extract PROB vector
      std::vector<double> prob;
      status = cfits_get_col(m_memFile, par, column, prob, status);
      if (status != STATUS_OK) {
        if (par->logTerse())
          Log(Error_2, "%d : Unable to extract column <%s> from"
                       "in-memory FITS catalogue.",
                       (Status)status, column.c_str());
        continue;
      }

      // Allocate vector column for in-memory FITS file update
      std::vector<double> col_prob;

      // Set PROB information
      for (int iCC = 0; iCC < src->numCC; ++iCC) {

        // Get probability in the range [0,1]
        double p = prob[iCC];
        if (p < 0.0)      p = 0.0;
        else if (p > 1.0) p = 1.0;

        // Save probability for each counterpart candidate
        src->cc[iCC].prob = p;

        // Add results to column vectors
        col_prob.push_back(src->cc[iCC].prob);

      } // endfor: looped over all counterpart candidates

      // Update in-memory column
      status = cfits_set_col(m_memFile, par, OUTCAT_COL_PROB_NAME,
                             col_prob, status);
      if (status != STATUS_OK) {
        if (par->logTerse())
          Log(Error_2, "%d : Unable to update column of in-memory FITS file.",
                       (Status)status);
        continue;
      }

    } while (0); // End of main do-loop

    // Debug mode: Entry
    if (par->logDebug())
      Log(Log_0, " <== EXIT: Catalogue::cid_prob (status=%d)",
          status);

    // Return status
    return status;

}


/**************************************************************************//**
 * @brief Compute local counterpart density at the position of a given source
 *
 * @param[in] par Pointer to gtsrcid parameters.
 * @param[in] src Pointer to source information.
 * @param[in] status Error status.
 *
 * Computes the local counterpart density at the position of a given source
 * by collecting the number of counterparts within a ring around the source
 * position. The ring is defined by a minimum and maximum radius, stored in
 * the class members Catalogue::m_ring_rad_min and Catalogue::m_ring_rad_max.
 *
 * The density is calculated by dividing the number of counterpart sources in
 * the ring by the solid angle of the ring. The density is stored in the class
 * member Catalogue::m_rho.
 *
 * If no counterpart was found in the ring the number of counterparts is set to
 * one. In this case the local counterpart density is to be considered as an
 * upper limit to the true counterpart density.
 ******************************************************************************/
Status Catalogue::cid_local_density(Parameters *par, SourceInfo *src,
                                    Status status) {

    // Debug mode: Entry
    if (par->logDebug())
      Log(Log_0, " ==> ENTRY: Catalogue::cid_density_local (%d candidates)",
          src->numCC);

    // Single loop for common exit point
    do {

      // Fall through in case of an error
      if (status != STATUS_OK)
        continue;

      // Initialise local counterpart density
      src->rho = 0.0;

      // Fall through if there are no counterpart candidates
      if (src->numCC < 1)
        continue;

      // Fall through if no source position is available
      if (!src->info->pos_valid)
        continue;

      // Compute solid angle of ring
      double omega = twopi * (cos(src->ring_rad_min * deg2rad) -
                              cos(src->ring_rad_max * deg2rad)) * rad2deg * rad2deg;

      // Compute number of counterparts within acceptance ring
      int num = 0;
      for (int iCC = 0; iCC < src->numCC; iCC++) {

        // Get index of candidate in counterpart catalogue
        int iCpt = src->cc[iCC].index;

        // Get pointer to counterpart object
        ObjectInfo *cpt = &(m_cpt.object[iCpt]);

        // Fall through if no counterpart position is available
        if (!cpt->pos_valid)
          continue;

        // Collect all sources in ring
        if (src->cc[iCC].angsep >= src->ring_rad_min &&
            src->cc[iCC].angsep <= src->ring_rad_max)
          num++;

      } // endfor: looped over all counterpart candidates

      // Make sure that we have at least one source. This provides an upper limit
      // for the counterpart density in case that we have found no source in the
      // acceptance ring
      if (num < 1) num = 1;

      // Compute local counterpart density
      src->rho = (omega > 0.0) ? double(num) / omega : 0.0;

    } while (0); // End of main do-loop

    // Debug mode: Entry
    if (par->logDebug())
      Log(Log_0, " <== EXIT: Catalogue::cid_density_local (status=%d)",
          status);

    // Return status
    return status;

}


/**************************************************************************//**
 * @brief Compute global counterpart density
 *
 * @param[in] par Pointer to gtsrcid parameters.
 * @param[in] src Pointer to source information.
 * @param[in] status Error status.
 ******************************************************************************/
Status Catalogue::cid_global_density(Parameters *par, SourceInfo *src,
                                     Status status) {

    // Debug mode: Entry
    if (par->logDebug())
      Log(Log_0, " ==> ENTRY: Catalogue::cid_global_density (%d candidates)",
          src->numCC);

    // Single loop for common exit point
    do {

      // Fall through in case of an error
      if (status != STATUS_OK)
        continue;

      // Compute global density
      src->rho = double(m_cpt.numLoad) / 41252.961;

    } while (0); // End of main do-loop

    // Debug mode: Entry
    if (par->logDebug())
      Log(Log_0, " <== EXIT: Catalogue::cid_global_density (status=%d)",
          status);

    // Return status
    return status;

}


/**************************************************************************//**
 * @brief Sort counterpart candidates
 *
 * @param[in] par Pointer to gtsrcid parameters.
 * @param[in] src Pointer to source information.
 * @param[in] status Error status.
 *
 * Sort by increasing probability and in case of equal probability) by 
 * decreasing angular separation
 ******************************************************************************/
Status Catalogue::cid_sort(Parameters *par, SourceInfo *src, Status status) {

    // Debug mode: Entry
    if (par->logDebug())
      Log(Log_0, " ==> ENTRY: Catalogue::cid_sort (%d candidates)", src->numCC);

    // Single loop for common exit point
    do {

      // Fall through in case of an error
      if (status != STATUS_OK)
        continue;

      // Fall through if there are no counterpart candidates
      if (src->numCC < 1)
        continue;

      // Sort counterpart candidats (dirty brute force method, to be replaced
      // by more efficient method if needed ...)
      for (int iCC = 0; iCC < src->numCC; ++iCC) {
        double max_prob =   0.0;
        double min_sep  = 180.0;
        int    imax     = iCC;
        for (int jCC = iCC; jCC < src->numCC; ++jCC) {

          // Check if we found a better candidate
          if (src->cc[jCC].prob > max_prob) {        // Increasing probability
            imax     = jCC;
            max_prob = src->cc[jCC].prob;
            min_sep  = src->cc[jCC].angsep;
          }
          else if (src->cc[jCC].prob == max_prob) {  // Equal probability &
            if (src->cc[jCC].angsep < min_sep) {     // decreasing separation
              imax     = jCC;
              max_prob = src->cc[jCC].prob;
              min_sep  = src->cc[jCC].angsep;
            }
          }

        }
        if (iCC != imax) {
          CCElement swap = src->cc[iCC];
          src->cc[iCC]   = src->cc[imax];
          src->cc[imax]  = swap;
        }
      }

    } while (0); // End of main do-loop

    // Debug mode: Entry
    if (par->logDebug())
      Log(Log_0, " <== EXIT: Catalogue::cid_sort (status=%d)", status);

    // Return status
    return status;

}


/**************************************************************************//**
 * @brief Dump counterpart candidates for source
 *
 * @param[in] par Pointer to gtsrcid parameters.
 * @param[in] src Pointer to source information.
 * @param[in] status Error status.
 ******************************************************************************/
Status Catalogue::cid_dump(Parameters *par, SourceInfo *src, Status status) {

    // Declare local variables

    // Debug mode: Entry
    if (par->logDebug())
      Log(Log_0, " ==> ENTRY: Catalogue::cid_dump (%d candidates)", src->numCC);

    // Single loop for common exit point
    do {

      // Fall through in case of an error
      if (status != STATUS_OK)
        continue;

      // Fall through if there are no counterpart candidates
      if (src->numCC < 1)
        continue;

      // Loop over counterpart candidates
      for (int iCC = 0; iCC < src->numCC; ++iCC) {

        // Get index of candidate in counterpart catalogue
        int iCpt = src->cc[iCC].index;

        // Get pointer to counterpart object
        ObjectInfo *cpt = &(m_cpt.object[iCpt]);

        // Normal log level
        if (par->logNormal()) {
          if (cpt->pos_valid) {
            Log(Log_2, "  Cpt%5d P=%3.0f%% S=%6.2f' PA=%4.0f: %20s"SRC_FORMAT,
                iCC+1,
                src->cc[iCC].prob*100.0,
                src->cc[iCC].angsep*60.0,
                src->cc[iCC].posang,
                cpt->name.c_str(),
                cpt->pos_eq_ra, cpt->pos_eq_dec,
                cpt->pos_err_maj, cpt->pos_err_min, cpt->pos_err_ang);
          }
          else {
            Log(Log_2, "  Cpt%5d P=%3.0f%% .................: %20s"
                " No position information found",
                iCC+1,
                src->cc[iCC].prob*100.0,
                cpt->name.c_str());
          }
        }

        // Explicit log level
        if (par->logVerbose()) {
          if (cpt->pos_valid) {
            Log(Log_2, "    Angular separation ............: %7.3f arcmin",
                src->cc[iCC].angsep*60.0);
            Log(Log_2, "    Effective 95%% error radius ....: %7.3f arcmin",
                src->cc[iCC].psi*60.0);
            Log(Log_2, "    Angular separation probability : %7.3f %%"
                " (dP/dr=%11.4e)",
                src->cc[iCC].prob_pos*100.0, src->cc[iCC].pdf_pos);
            Log(Log_2, "    Chance coincidence probability : %7.3f %%"
                " (dP/dr=%11.4e, lambda=%.3f)",
                src->cc[iCC].prob_chance*100.0, src->cc[iCC].pdf_chance,
                src->cc[iCC].lambda);
            Log(Log_2, "    Prior association probability .: %7.3f %%",
                src->cc[iCC].prob_prior*100.0);
            Log(Log_2, "    Posterior association prob. ...: %7.3f %%",
                src->cc[iCC].prob_post*100.0);
            Log(Log_2, "    Likelihood ratio ..............: %.3f",
                src->cc[iCC].likrat);
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


/**************************************************************************//**
 * @brief Assign source name
 *
 * @param[in] name Source name.
 * @param[in] row Catalogue row (starting from 0).
 *
 * Assigns source name for output catalogue. Leading and trailing whitespace
 * are stripped from the source name provided on input. If the source name
 * string is empty, a dummy name will be constructed from the catalogue row
 * number (CAT_ROW_xxx, where xxx is the row number starting from 1).
 ******************************************************************************/

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
