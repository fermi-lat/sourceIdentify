###### Executables ######
add_executable(
  gtsrcid
  src/gtsrcid/Catalogue.cxx
  src/gtsrcid/Catalogue_fits.cxx
  src/gtsrcid/Catalogue_id.cxx
  src/gtsrcid/Catalogue_nr.cxx
  src/gtsrcid/GHealpix.cxx
  src/gtsrcid/GSkyDir.cxx
  src/gtsrcid/Log.cxx
  src/gtsrcid/Parameters.cxx
  src/gtsrcid/sourceIdentify.cxx
)
target_link_libraries(gtsrcid PRIVATE catalogAccess hoops st_app st_facilities)

###############################################################
# Installation
###############################################################
install(TARGETS gtsrcid RUNTIME DESTINATION ${FERMI_INSTALL_BINDIR})
install(DIRECTORY data/ DESTINATION ${FERMI_INSTALL_DATADIR})
install(DIRECTORY pfiles/ DESTINATION ${FERMI_INSTALL_PFILESDIR})
