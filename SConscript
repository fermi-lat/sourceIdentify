# -*- python -*-
# $Id: SConscript,v 1.12 2009/07/14 23:21:28 jrb Exp $
# Authors: Jurgen Knodlseder <knodlseder@cesr.fr>
# Version: sourceIdentify-02-02-02
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
