# -*- python -*-
# $Id: SConscript,v 1.19 2012/01/17 11:11:12 jurgen Exp $
# Authors: Jurgen Knodlseder <jknodlseder@irap.omp.eu>
# Version: sourceIdentify-02-04-00
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
