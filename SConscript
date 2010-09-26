# -*- python -*-
# $Id: SConscript,v 1.15 2010/01/12 09:17:20 jurgen Exp $
# Authors: Jurgen Knodlseder <knodlseder@cesr.fr>
# Version: sourceIdentify-02-03-00
Import('baseEnv')
Import('listFiles')
progEnv = baseEnv.Clone()
progEnv.Tool('hoopsLib')
progEnv.Tool('st_facilitiesLib')
progEnv.Tool('st_appLib')
progEnv.Tool('catalogAccessLib')

gtsrcidBin = progEnv.Program('gtsrcid', listFiles(['src/gtsrcid/*.cxx']))

progEnv.Tool('registerTargets', package = 'sourceIdentify',
             binaryCxts = [[gtsrcidBin, progEnv]],
             pfiles = listFiles(['pfiles/*.par']),
             data = listFiles(['data/*'], recursive = True))
