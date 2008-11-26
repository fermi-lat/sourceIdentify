# -*- python -*-
# $Id: SConscript,v 1.7 2008/09/26 08:30:37 glastrm Exp $
# Authors: Jurgen Knodlseder <knodlseder@cesr.fr>
# Version: sourceIdentify-02-01-00
Import('baseEnv')
Import('listFiles')
progEnv = baseEnv.Clone()
progEnv.Tool('hoopsLib')
progEnv.Tool('st_facilitiesLib')
progEnv.Tool('st_appLib')
progEnv.Tool('catalogAccessLib')

gtsrcidBin = progEnv.Program('gtsrcid', listFiles(['src/gtsrcid/*.cxx']))

progEnv.Tool('registerObjects', package = 'sourceIdentify', binaries = [gtsrcidBin], pfiles = listFiles(['pfiles/*.par']), data = listFiles(['data/*'], recursive = True))
