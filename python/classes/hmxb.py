#--------------------------------------------------------------------------------------------
# Source class: High-Mass X-ray Binarys
#--------------------------------------------------------------------------------------------
#
# $Author: jurgen $
# $Revision: 1.2 $
# $Date: 2008/05/07 16:30:53 $
#--------------------------------------------------------------------------------------------
"""
High-Mass X-ray Binary Source Class.
"""

# Catalogue identifier
catid = "HMXB"

# Catalogue FITS filename
catname = "obj-hmxb.fits"

# Probability method and threshold
prob_method      = "PROB_POST"    # Formula used to calculate probability
prob_prior       = "0.01"         # Formula used to calculate prior
prob_thres       = 0.40           # Probability threshold
figure_of_merit  = ""             # Formula used to calculate FoM

# Maximum number of counterparts per LAT source
max_counterparts = 1

# List of new catalogue quantities (setup up to 9 formulae in the list)
new_quantity = [ "" , "" ]

# List of source selection criteria (setup up to 9 criteria in the list)
selection = [ "" , "" ]

