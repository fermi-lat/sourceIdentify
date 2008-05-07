#--------------------------------------------------------------------------------------------
# Source class: Blazars
#--------------------------------------------------------------------------------------------
#
# $Author: jurgen $
# $Revision: 1.1 $
# $Date: 2008/05/06 16:00:04 $
#--------------------------------------------------------------------------------------------
"""
Blazar Source Class (CGRaBS).
"""

# Catalogue identifier
catid = "BLZ"

# Catalogue FITS filename
catname = "obj-blazar.fits"

# Probability method and threshold
prob_method      = "PROB_POST"    # Formula used to calculate probability
prob_prior       = "0.01"         # Formula used to calculate prior
prob_thres       = 0.10           # Probability threshold
figure_of_merit  = ""             # Formula used to calculate FoM

# Maximum number of counterparts per LAT source
max_counterparts = 10

# List of new catalogue quantities (setup up to 9 formulae in the list)
new_quantity = [ "" , "" ]

# List of source selection criteria (setup up to 9 criteria in the list)
selection = [ "" , "" ]

