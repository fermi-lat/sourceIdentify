# -*- python -*-
# $Id: SConscript,v 1.4 2008/06/19 23:50:53 glastrm Exp $
# Authors: Jurgen Knodlseder <knodlseder@cesr.fr>
# Version: sourceIdentify-02-00-00
Import('baseEnv')
Import('listFiles')
progEnv = baseEnv.Clone()
progEnv.Tool('hoopsLib')
progEnv.Tool('st_facilitiesLib')
progEnv.Tool('st_appLib')
progEnv.Tool('catalogAccessLib')

gtsrcidBin = progEnv.Program('gtsrcid', listFiles(['src/gtsrcid/*.cxx']))

progEnv.Tool('registerObjects', package = 'sourceIdentify', binaries = [gtsrcidBin], pfiles = listFiles(['pfiles/*.par']), data = listFiles(['data/*'], recursive = True))
