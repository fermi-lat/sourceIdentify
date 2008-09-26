#--------------------------------------------------------------------------------------------
# Source class: Blazars
#--------------------------------------------------------------------------------------------
#
# $Author: jurgen $
# $Revision: 1.1 $
# $Date: 2008/09/25 14:31:21 $
#--------------------------------------------------------------------------------------------
"""
Blazar Source Class (CGRaBS).
"""

# Catalogue identifier
catid = "CGRABS"

# Catalogue FITS filename
catname = "obj-cgrabs.fits"

# Probability method and threshold
prob_method      = "PROB_POST"        # Use posterior probability
prob_prior       = "nsrc() / ncpt()"  # Bayesian frequency prior
prob_thres       = 0.40               # Probability threshold
figure_of_merit  = ""                 # No FoM used

# Maximum number of counterparts per LAT source
max_counterparts = 1

# List of new catalogue quantities (setup up to 9 formulae in the list)
new_quantity = [ "" , "" ]

# List of source selection criteria (setup up to 9 criteria in the list)
selection = [ "" , "" ]

