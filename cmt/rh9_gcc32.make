CMT_tag=$(tag)
CMTROOT=/opt/projects/glast/applications/CMT/v1r16p20040701
CMT_root=/opt/projects/glast/applications/CMT/v1r16p20040701
CMTVERSION=v1r16p20040701
CMTrelease=15
cmt_hardware_query_command=uname -m
cmt_hardware=`$(cmt_hardware_query_command)`
cmt_system_version_query_command=${CMTROOT}/mgr/cmt_linux_version.sh | ${CMTROOT}/mgr/cmt_filter_version.sh
cmt_system_version=`$(cmt_system_version_query_command)`
cmt_compiler_version_query_command=${CMTROOT}/mgr/cmt_gcc_version.sh | ${CMTROOT}/mgr/cmt_filter_version.sh
cmt_compiler_version=`$(cmt_compiler_version_query_command)`
PATH=/opt/projects/glast/applications/CMT/v1r16p20040701/${CMTBIN}:.:.:/home/knodlseder/glast/bin:/home/knodlseder/dev/integral/bin:/usr/local/rsi/idl_5.4/bin:/opt/projects/gri/v1.0/Linux_i686/bin:/opt/projects/glast/packages/ScienceTools-v6r2/bin:/opt/common/root/v4.03.02/bin:/opt/projects/integral/cesr/bin:/opt/projects/integral/osa/v5.1/bin:/opt/common/root/v4.03.02/bin:/opt/common/lheasoft/v5.3.1/i686-pc-linux-gnu-libc2.1/scripts:/opt/common/lheasoft/v5.3.1/i686-pc-linux-gnu-libc2.1/bin:/usr/kerberos/bin:/usr/local/bin:/bin:/usr/bin:/usr/X11R6/bin:/sbin:/home/knodlseder/bin:${ROOTSYS}/bin
CLASSPATH=/opt/projects/glast/applications/CMT/v1r16p20040701/java
debug_option=-g
cc=gcc
cdebugflags=$(debug_option)
pp_cflags=-Di586
ccomp=$(cc) -c $(includes) $(cdebugflags) $(cflags) $(pp_cflags)
clink=$(cc) $(clinkflags) $(cdebugflags)
ppcmd=-I
preproc=c++ -MD -c 
cpp=g++
cppdebugflags=$(debug_option)
cppflags=-pipe -ansi -W -Wall  -fPIC -shared -D_GNU_SOURCE -Dlinux -Dunix  -I../src -DTRAP_FPE 
pp_cppflags=-D_GNU_SOURCE
cppcomp=$(cpp) -c $(includes) $(cppoptions) $(cppflags) $(pp_cppflags)
cpplinkflags=-Wl,-Bdynamic  $(linkdebug)
cpplink=$(cpp)   $(cpplinkflags)
for=g77
fflags=$(debug_option)
fcomp=$(for) -c $(fincludes) $(fflags) $(pp_fflags)
flink=$(for) $(flinkflags)
javacomp=javac -classpath $(src):$(CLASSPATH) 
javacopy=cp
jar=jar
X11_cflags=-I/usr/include
Xm_cflags=-I/usr/include
X_linkopts=-L/usr/X11R6/lib -lXm -lXt -lXext -lX11 -lm
lex=flex $(lexflags)
yaccflags= -l -d 
yacc=yacc $(yaccflags)
ar=ar r
ranlib=ranlib
make_shlib=${CMTROOT}/mgr/cmt_make_shlib_common.sh extract
shlibsuffix=so
shlibbuilder=g++ $(cmt_installarea_linkopts) 
shlibflags=-shared
symlink=/bin/ln -fs 
symunlink=/bin/rm -f 
build_library_links=$(cmtexe) build library_links -quiet -tag=$(tags)
remove_library_links=$(cmtexe) remove library_links -quiet -tag=$(tags)
cmtexe=${CMTROOT}/${CMTBIN}/cmt.exe
build_prototype=$(cmtexe) build prototype
build_dependencies=$(cmtexe) -quiet -tag=$(tags) build dependencies
build_triggers=$(cmtexe) build triggers
implied_library_prefix=-l
SHELL=/bin/sh
src=../src/
doc=../doc/
inc=../src/
mgr=../cmt/
application_suffix=.exe
library_prefix=lib
lock_command=chmod -R a-w ../*
unlock_command=chmod -R g+w ../*
MAKEFLAGS= --no-print-directory 
gmake_hosts=lx1 rsplus lxtest as7 dxplus ax7 hp2 aleph hp1 hpplus papou1-fe atlas
make_hosts=virgo-control1 rio0a vmpc38a
everywhere=hosts
install_command=cp 
uninstall_command=/bin/rm -f 
cmt_installarea_command=ln -s 
cmt_uninstallarea_command=/bin/rm -f 
cmt_install_area_command=$(cmt_installarea_command)
cmt_uninstall_area_command=$(cmt_uninstallarea_command)
cmt_install_action=$(CMTROOT)/mgr/cmt_install_action.sh
cmt_installdir_action=$(CMTROOT)/mgr/cmt_installdir_action.sh
cmt_uninstall_action=$(CMTROOT)/mgr/cmt_uninstall_action.sh
cmt_uninstalldir_action=$(CMTROOT)/mgr/cmt_uninstalldir_action.sh
mkdir=mkdir
cmt_installarea_prefix=InstallArea
CMT_PATH_remove_regexp=/[^/]*/
CMT_PATH_remove_share_regexp=/share/
NEWCMTCONFIG=i686-rh9-gcc32
sourceIdentify_tag=$(tag)
SOURCEIDENTIFYROOT=/home/knodlseder/dev/glast/sourceIdentify/v1r0p1
sourceIdentify_root=/home/knodlseder/dev/glast/sourceIdentify/v1r0p1
SOURCEIDENTIFYVERSION=v1r0p1
sourceIdentify_cmtpath=/home/knodlseder/dev/glast
sourceIdentify_project=Project1
STpolicy_tag=$(tag)
STPOLICYROOT=/opt/projects/glast/packages/ScienceTools-v6r2/STpolicy/v1r0p6
STpolicy_root=/opt/projects/glast/packages/ScienceTools-v6r2/STpolicy/v1r0p6
STPOLICYVERSION=v1r0p6
STpolicy_cmtpath=/opt/projects/glast/packages/ScienceTools-v6r2
STpolicy_project=Project2
GlastPolicy_tag=$(tag)
GLASTPOLICYROOT=/opt/projects/glast/packages/ScienceTools-v6r2/GlastPolicy/v6r4
GlastPolicy_root=/opt/projects/glast/packages/ScienceTools-v6r2/GlastPolicy/v6r4
GLASTPOLICYVERSION=v6r4
GlastPolicy_cmtpath=/opt/projects/glast/packages/ScienceTools-v6r2
GlastPolicy_project=Project2
GlastPatternPolicy_tag=$(tag)
GLASTPATTERNPOLICYROOT=/opt/projects/glast/packages/ScienceTools-v6r2/GlastPolicy/GlastPatternPolicy/v0r1
GlastPatternPolicy_root=/opt/projects/glast/packages/ScienceTools-v6r2/GlastPolicy/GlastPatternPolicy/v0r1
GLASTPATTERNPOLICYVERSION=v0r1
GlastPatternPolicy_cmtpath=/opt/projects/glast/packages/ScienceTools-v6r2
GlastPatternPolicy_offset=GlastPolicy
GlastPatternPolicy_project=Project2
GlastMain=${GLASTPOLICYROOT}/src/GlastMain.cxx
TestGlastMain=${GLASTPOLICYROOT}/src/TestGlastMain.cxx
GlastCppPolicy_tag=$(tag)
GLASTCPPPOLICYROOT=/opt/projects/glast/packages/ScienceTools-v6r2/GlastPolicy/GlastCppPolicy/v0r2p3
GlastCppPolicy_root=/opt/projects/glast/packages/ScienceTools-v6r2/GlastPolicy/GlastCppPolicy/v0r2p3
GLASTCPPPOLICYVERSION=v0r2p3
GlastCppPolicy_cmtpath=/opt/projects/glast/packages/ScienceTools-v6r2
GlastCppPolicy_offset=GlastPolicy
GlastCppPolicy_project=Project2
BINDIR=rh9_gcc32
cppoptions=$(cppdebugflags_s)
cppdebugflags_s=-g
cppoptimized_s=-O2
cppprofiled_s=-pg
linkdebug=-g 
makeLinkMap=-Wl,-Map,Linux.map
componentshr_linkopts=-fPIC  -ldl 
libraryshr_linkopts=-fPIC -ldl 
TMP=/tmp
hoops_tag=$(tag)
HOOPSROOT=/opt/projects/glast/packages/ScienceTools-v6r2/hoops/v0r4p5
hoops_root=/opt/projects/glast/packages/ScienceTools-v6r2/hoops/v0r4p5
HOOPSVERSION=v0r4p5
hoops_cmtpath=/opt/projects/glast/packages/ScienceTools-v6r2
hoops_project=Project2
pil_tag=$(tag)
PILROOT=/opt/projects/glast/packages/ScienceTools-v6r2/IExternal/pil/v1r198
pil_root=/opt/projects/glast/packages/ScienceTools-v6r2/IExternal/pil/v1r198
PILVERSION=v1r198
pil_cmtpath=/opt/projects/glast/packages/ScienceTools-v6r2
pil_offset=IExternal
pil_project=Project2
pil_native_version=1.9.8
pil_DIR=${GLAST_EXT}/pil/$(pil_native_version)
pil_linkopts=-L $(pil_DIR)/lib -lpil -ltermcap -lreadline
hoopsDir=${HOOPSROOT}/${BINDIR}
hoops_linkopts=-L${hoops_root}/${BINDIR} -lhoopsLib 
hoops_stamps=${HOOPSROOT}/${BINDIR}/hoopsLib.stamp 
st_facilities_tag=$(tag)
ST_FACILITIESROOT=/opt/projects/glast/packages/ScienceTools-v6r2/st_facilities/v0r6p1
st_facilities_root=/opt/projects/glast/packages/ScienceTools-v6r2/st_facilities/v0r6p1
ST_FACILITIESVERSION=v0r6p1
st_facilities_cmtpath=/opt/projects/glast/packages/ScienceTools-v6r2
st_facilities_project=Project2
astro_tag=$(tag)
ASTROROOT=/opt/projects/glast/packages/ScienceTools-v6r2/astro/v1r13p2
astro_root=/opt/projects/glast/packages/ScienceTools-v6r2/astro/v1r13p2
ASTROVERSION=v1r13p2
astro_cmtpath=/opt/projects/glast/packages/ScienceTools-v6r2
astro_project=Project2
facilities_tag=$(tag)
FACILITIESROOT=/opt/projects/glast/packages/ScienceTools-v6r2/facilities/v2r12p3
facilities_root=/opt/projects/glast/packages/ScienceTools-v6r2/facilities/v2r12p3
FACILITIESVERSION=v2r12p3
facilities_cmtpath=/opt/projects/glast/packages/ScienceTools-v6r2
facilities_project=Project2
facilities_linkopts=-L${facilities_root}/${BINDIR} -lfacilities 
cfitsio_tag=$(tag)
CFITSIOROOT=/opt/projects/glast/packages/ScienceTools-v6r2/IExternal/cfitsio/v1r2470p5
cfitsio_root=/opt/projects/glast/packages/ScienceTools-v6r2/IExternal/cfitsio/v1r2470p5
CFITSIOVERSION=v1r2470p5
cfitsio_cmtpath=/opt/projects/glast/packages/ScienceTools-v6r2
cfitsio_offset=IExternal
cfitsio_project=Project2
cfitsio_native_version=v2470
cfitsio_DIR=${GLAST_EXT}/cfitsio/$(cfitsio_native_version)
cfitsio_libs=-L${cfitsio_DIR}/lib -lcfitsio 
cfitsio_linkopts=$(cfitsio_libs) 
LD_LIBRARY_PATH=/opt/projects/gri/v1.0/Linux_i686/lib:/opt/projects/glast/extlib/xerces/2.6.0/lib:/opt/projects/glast/extlib/fftw/3.0.1/lib:/opt/projects/glast/extlib/cfitsio/v2470/lib:/opt/projects/glast/packages/ScienceTools-v6r2/lib:/opt/common/root/v4.03.02/lib:/opt/common/root/v4.03.02/lib:/opt/common/lheasoft/v5.3.1/i686-pc-linux-gnu-libc2.1/lib:/opt/projects/glast/extlib/CLHEP/1.8.0.0/lib:/opt/projects/glast/extlib/ROOT/v4.02.00/root/bin:/opt/projects/glast/extlib/ROOT/v4.02.00/root/lib:/opt/projects/glast/extlib/cppunit/1.9.14/lib:/opt/projects/glast/packages/ScienceTools-v6r2/st_graph/v1r4p3/${BINDIR}
CLHEP_tag=$(tag)
CLHEPROOT=/opt/projects/glast/packages/ScienceTools-v6r2/IExternal/CLHEP/v2r1800p5
CLHEP_root=/opt/projects/glast/packages/ScienceTools-v6r2/IExternal/CLHEP/v2r1800p5
CLHEPVERSION=v2r1800p5
CLHEP_cmtpath=/opt/projects/glast/packages/ScienceTools-v6r2
CLHEP_offset=IExternal
CLHEP_project=Project2
CLHEP_native_version=1.8.0.0
CLHEP_DIR=$(GLAST_EXT)/CLHEP
CLHEPBASE=${CLHEP_DIR}/$(CLHEP_native_version)
CLHEP_linkopts=-L$(CLHEPBASE)/lib -lCLHEP
extFiles_tag=$(tag)
EXTFILESROOT=/opt/projects/glast/packages/ScienceTools-v6r2/IExternal/extFiles/v0r4
extFiles_root=/opt/projects/glast/packages/ScienceTools-v6r2/IExternal/extFiles/v0r4
EXTFILESVERSION=v0r4
extFiles_cmtpath=/opt/projects/glast/packages/ScienceTools-v6r2
extFiles_offset=IExternal
extFiles_project=Project2
extFiles_DIR=${GLAST_EXT}/extFiles
extFiles_native_version=v0r4
extFiles_PATH=${extFiles_DIR}/$(extFiles_native_version)
EXTFILESSYS=/opt/projects/glast/extlib/extFiles/v0r4
tip_tag=$(tag)
TIPROOT=/opt/projects/glast/packages/ScienceTools-v6r2/tip/v2r8p1
tip_root=/opt/projects/glast/packages/ScienceTools-v6r2/tip/v2r8p1
TIPVERSION=v2r8p1
tip_cmtpath=/opt/projects/glast/packages/ScienceTools-v6r2
tip_project=Project2
ROOT_tag=$(tag)
ROOTROOT=/opt/projects/glast/packages/ScienceTools-v6r2/IExternal/ROOT/v3r40200p4
ROOT_root=/opt/projects/glast/packages/ScienceTools-v6r2/IExternal/ROOT/v3r40200p4
ROOTVERSION=v3r40200p4
ROOT_cmtpath=/opt/projects/glast/packages/ScienceTools-v6r2
ROOT_offset=IExternal
ROOT_project=Project2
ROOT_DIR=${GLAST_EXT}/ROOT
ROOT_native_version=v4.02.00
ROOT_PATH=${ROOT_DIR}/$(ROOT_native_version)/root
ROOTSYS=/opt/projects/glast/extlib/ROOT/v4.02.00/root
dict=../dict/
rootcint=rootcint
ROOT_libs=-L$(ROOT_PATH)/lib -lCore -lCint -lTree -lMatrix -lPhysics -lpthread -lm -ldl -rdynamic
ROOT_GUI_libs=-L$(ROOT_PATH)/lib -lHist -lGraf -lGraf3d -lGpad -lRint -lPostscript -lTreePlayer 
ROOT_linkopts=$(ROOT_libs)
ROOT_cppflagsEx=$(ppcmd) "$(ROOT_PATH)/include" -DUSE_ROOT
ROOT_cppflags=-fpermissive
tip_linkopts=-lHist -L${tip_root}/${BINDIR} -ltipLib 
tipDir=${TIPROOT}/${BINDIR}
tip_stamps=${TIPROOT}/${BINDIR}/tipLib.stamp 
astro_linkopts=-L${astro_root}/${BINDIR} -lastro 
astro_stamps=${ASTROROOT}/${BINDIR}/astro.stamp 
f2c_tag=$(tag)
F2CROOT=/opt/projects/glast/packages/ScienceTools-v6r2/f2c/v2r2
f2c_root=/opt/projects/glast/packages/ScienceTools-v6r2/f2c/v2r2
F2CVERSION=v2r2
f2c_cmtpath=/opt/projects/glast/packages/ScienceTools-v6r2
f2c_project=Project2
f2c_linkopts= -lg2c 
cppunit_tag=$(tag)
CPPUNITROOT=/opt/projects/glast/packages/ScienceTools-v6r2/IExternal/cppunit/v1r1914p3
cppunit_root=/opt/projects/glast/packages/ScienceTools-v6r2/IExternal/cppunit/v1r1914p3
CPPUNITVERSION=v1r1914p3
cppunit_cmtpath=/opt/projects/glast/packages/ScienceTools-v6r2
cppunit_offset=IExternal
cppunit_project=Project2
cppunit_native_version=1.9.14
cppunit_DIR=${GLAST_EXT}/cppunit/$(cppunit_native_version)
cppunit_linkopts=-L ${cppunit_DIR}/lib/ -lcppunit -ldl 
st_facilities_linkopts=-L${st_facilities_root}/${BINDIR} -lst_facilities 
st_facilities_stamps=${ST_FACILITIESROOT}/${BINDIR}/st_facilities.stamp 
source=*.cxx
STTEST=sttest
st_app_tag=$(tag)
ST_APPROOT=/opt/projects/glast/packages/ScienceTools-v6r2/st_app/v1r7p1
st_app_root=/opt/projects/glast/packages/ScienceTools-v6r2/st_app/v1r7p1
ST_APPVERSION=v1r7p1
st_app_cmtpath=/opt/projects/glast/packages/ScienceTools-v6r2
st_app_project=Project2
st_graph_tag=$(tag)
ST_GRAPHROOT=/opt/projects/glast/packages/ScienceTools-v6r2/st_graph/v1r4p3
st_graph_root=/opt/projects/glast/packages/ScienceTools-v6r2/st_graph/v1r4p3
ST_GRAPHVERSION=v1r4p3
st_graph_cmtpath=/opt/projects/glast/packages/ScienceTools-v6r2
st_graph_project=Project2
RootcintPolicy_tag=$(tag)
ROOTCINTPOLICYROOT=/opt/projects/glast/packages/ScienceTools-v6r2/GlastPolicy/RootcintPolicy/v5r0p2
RootcintPolicy_root=/opt/projects/glast/packages/ScienceTools-v6r2/GlastPolicy/RootcintPolicy/v5r0p2
ROOTCINTPOLICYVERSION=v5r0p2
RootcintPolicy_cmtpath=/opt/projects/glast/packages/ScienceTools-v6r2
RootcintPolicy_offset=GlastPolicy
RootcintPolicy_project=Project2
st_stream_tag=$(tag)
ST_STREAMROOT=/opt/projects/glast/packages/ScienceTools-v6r2/st_stream/v0r4
st_stream_root=/opt/projects/glast/packages/ScienceTools-v6r2/st_stream/v0r4
ST_STREAMVERSION=v0r4
st_stream_cmtpath=/opt/projects/glast/packages/ScienceTools-v6r2
st_stream_project=Project2
st_streamDir=${ST_STREAMROOT}/${BINDIR}
st_stream_linkopts=-L${st_stream_root}/${BINDIR} -lst_streamLib 
st_stream_stamps=${ST_STREAMROOT}/${BINDIR}/st_streamLib.stamp 
st_graph_linkopts=-L${st_graph_root}/${BINDIR} -lst_graph ${st_graph_libs}
root_packages_import=-import=st_graph 
root_packages_include=$(ppcmd)"$(st_graph_root)" 
st_graph_libs=-L$(ROOT_PATH)/lib -lCore -lCint -lHist -lGraf -lGraf3d -lGpad -lTree -lRint -lPostscript -lMatrix -lPhysics -lGui -ldl 
st_appDir=${ST_APPROOT}/${BINDIR}
st_app_linkopts=-L${st_app_root}/${BINDIR} -lst_appLib 
st_app_stamps=${ST_APPROOT}/${BINDIR}/st_appLib.stamp 
PFILES=.:/home/knodlseder/dev/integral/pfiles:/home/knodlseder/pfiles/glast:.:/home/knodlseder/pfiles/integral:/home/knodlseder/pfiles:/opt/common/lheasoft/v5.3.1/i686-pc-linux-gnu-libc2.1/syspfiles:/home/knodlseder/glast/pfiles:/opt/projects/glast/packages/ScienceTools-v6r2/st_app/v1r7p1/pfiles:/home/knodlseder/dev/glast/sourceIdentify/v1r0p1/pfiles
catalogAccess_tag=$(tag)
CATALOGACCESSROOT=/home/knodlseder/dev/glast/catalogAccess/v0r2p6
catalogAccess_root=/home/knodlseder/dev/glast/catalogAccess/v0r2p6
CATALOGACCESSVERSION=v0r2p6
catalogAccess_cmtpath=/home/knodlseder/dev/glast
catalogAccess_project=Project1
catalogAccessDir=${CATALOGACCESSROOT}/${BINDIR}
catalogAccess_linkopts=-L${catalogAccess_root}/${BINDIR} -lcatalogAccessLib 
catalogAccess_stamps=${CATALOGACCESSROOT}/${BINDIR}/catalogAccessLib.stamp 
gtsrcidstamps=gtsrcid.stamp
ST_apps=gtsrcid=/home/knodlseder/dev/glast/sourceIdentify/v1r0p1/${BINDIR}/gtsrcid.exe
GlastPatternPolicyDir=${GLASTPATTERNPOLICYROOT}/${BINDIR}
GlastPolicyDir=${GLASTPOLICYROOT}/${BINDIR}
STpolicyDir=${STPOLICYROOT}/${BINDIR}
facilitiesDir=${FACILITIESROOT}/${BINDIR}
astroDir=${ASTROROOT}/${BINDIR}
f2cDir=${F2CROOT}/${BINDIR}
st_facilitiesDir=${ST_FACILITIESROOT}/${BINDIR}
RootcintPolicyDir=${ROOTCINTPOLICYROOT}/${BINDIR}
st_graphDir=${ST_GRAPHROOT}/${BINDIR}
sourceIdentifyDir=${SOURCEIDENTIFYROOT}/${BINDIR}
tag=rh9_gcc32
package=sourceIdentify
version=v1r0p1
PACKAGE_ROOT=$(SOURCEIDENTIFYROOT)
srcdir=../src
bin=../$(sourceIdentify_tag)/
javabin=../classes/
mgrdir=cmt
project=Project1
use_requirements=requirements $(CMTROOT)/mgr/requirements $(ST_APPROOT)/cmt/requirements $(HOOPSROOT)/cmt/requirements $(CATALOGACCESSROOT)/cmt/requirements $(ST_FACILITIESROOT)/cmt/requirements $(ASTROROOT)/cmt/requirements $(TIPROOT)/cmt/requirements $(ST_GRAPHROOT)/cmt/requirements $(ST_STREAMROOT)/cmt/requirements $(STPOLICYROOT)/cmt/requirements $(FACILITIESROOT)/cmt/requirements $(F2CROOT)/cmt/requirements $(ROOTCINTPOLICYROOT)/cmt/requirements $(GLASTPOLICYROOT)/cmt/requirements $(GLASTPATTERNPOLICYROOT)/cmt/requirements $(GLASTCPPPOLICYROOT)/cmt/requirements $(PILROOT)/cmt/requirements $(CFITSIOROOT)/cmt/requirements $(CLHEPROOT)/cmt/requirements $(EXTFILESROOT)/cmt/requirements $(ROOTROOT)/cmt/requirements $(CPPUNITROOT)/cmt/requirements 
use_includes= $(ppcmd)"$(ST_APPROOT)" $(ppcmd)"$(HOOPSROOT)" $(ppcmd)"$(CATALOGACCESSROOT)" $(ppcmd)"$(ST_FACILITIESROOT)" $(ppcmd)"$(ASTROROOT)" $(ppcmd)"$(TIPROOT)" $(ppcmd)"$(ST_GRAPHROOT)" $(ppcmd)"$(ST_STREAMROOT)" $(ppcmd)"$(FACILITIESROOT)" $(ppcmd)"$(F2CROOT)" $(ppcmd)"$(pil_DIR)/include" $(ppcmd)"${cfitsio_DIR}/include" $(ppcmd)"$(CLHEPBASE)/include" $(ppcmd)"$(ROOT_PATH)/include" $(ppcmd)"${cppunit_DIR}/include" 
use_fincludes= $(use_includes)
use_stamps= $(sourceIdentify_stamps)  $(st_app_stamps)  $(hoops_stamps)  $(catalogAccess_stamps)  $(st_facilities_stamps)  $(astro_stamps)  $(tip_stamps)  $(st_graph_stamps)  $(st_stream_stamps)  $(STpolicy_stamps)  $(facilities_stamps)  $(f2c_stamps)  $(RootcintPolicy_stamps)  $(GlastPolicy_stamps)  $(GlastPatternPolicy_stamps)  $(GlastCppPolicy_stamps)  $(pil_stamps)  $(cfitsio_stamps)  $(CLHEP_stamps)  $(extFiles_stamps)  $(ROOT_stamps)  $(cppunit_stamps) 
use_cflags=  $(sourceIdentify_cflags)  $(st_app_cflags)  $(hoops_cflags)  $(catalogAccess_cflags)  $(st_facilities_cflags)  $(astro_cflags)  $(tip_cflags)  $(st_graph_cflags)  $(st_stream_cflags)  $(STpolicy_cflags)  $(facilities_cflags)  $(f2c_cflags)  $(RootcintPolicy_cflags)  $(GlastPolicy_cflags)  $(pil_cflags)  $(cfitsio_cflags)  $(CLHEP_cflags)  $(extFiles_cflags)  $(ROOT_cflags)  $(cppunit_cflags) 
use_pp_cflags=  $(sourceIdentify_pp_cflags)  $(st_app_pp_cflags)  $(hoops_pp_cflags)  $(catalogAccess_pp_cflags)  $(st_facilities_pp_cflags)  $(astro_pp_cflags)  $(tip_pp_cflags)  $(st_graph_pp_cflags)  $(st_stream_pp_cflags)  $(STpolicy_pp_cflags)  $(facilities_pp_cflags)  $(f2c_pp_cflags)  $(RootcintPolicy_pp_cflags)  $(GlastPolicy_pp_cflags)  $(pil_pp_cflags)  $(cfitsio_pp_cflags)  $(CLHEP_pp_cflags)  $(extFiles_pp_cflags)  $(ROOT_pp_cflags)  $(cppunit_pp_cflags) 
use_cppflags=  $(sourceIdentify_cppflags)  $(st_app_cppflags)  $(hoops_cppflags)  $(catalogAccess_cppflags)  $(st_facilities_cppflags)  $(astro_cppflags)  $(tip_cppflags)  $(st_graph_cppflags)  $(st_stream_cppflags)  $(STpolicy_cppflags)  $(facilities_cppflags)  $(f2c_cppflags)  $(RootcintPolicy_cppflags)  $(GlastPolicy_cppflags)  $(pil_cppflags)  $(cfitsio_cppflags)  $(CLHEP_cppflags)  $(extFiles_cppflags)  $(ROOT_cppflags)  $(cppunit_cppflags) 
use_pp_cppflags=  $(sourceIdentify_pp_cppflags)  $(st_app_pp_cppflags)  $(hoops_pp_cppflags)  $(catalogAccess_pp_cppflags)  $(st_facilities_pp_cppflags)  $(astro_pp_cppflags)  $(tip_pp_cppflags)  $(st_graph_pp_cppflags)  $(st_stream_pp_cppflags)  $(STpolicy_pp_cppflags)  $(facilities_pp_cppflags)  $(f2c_pp_cppflags)  $(RootcintPolicy_pp_cppflags)  $(GlastPolicy_pp_cppflags)  $(pil_pp_cppflags)  $(cfitsio_pp_cppflags)  $(CLHEP_pp_cppflags)  $(extFiles_pp_cppflags)  $(ROOT_pp_cppflags)  $(cppunit_pp_cppflags) 
use_fflags=  $(sourceIdentify_fflags)  $(st_app_fflags)  $(hoops_fflags)  $(catalogAccess_fflags)  $(st_facilities_fflags)  $(astro_fflags)  $(tip_fflags)  $(st_graph_fflags)  $(st_stream_fflags)  $(STpolicy_fflags)  $(facilities_fflags)  $(f2c_fflags)  $(RootcintPolicy_fflags)  $(GlastPolicy_fflags)  $(pil_fflags)  $(cfitsio_fflags)  $(CLHEP_fflags)  $(extFiles_fflags)  $(ROOT_fflags)  $(cppunit_fflags) 
use_pp_fflags=  $(sourceIdentify_pp_fflags)  $(st_app_pp_fflags)  $(hoops_pp_fflags)  $(catalogAccess_pp_fflags)  $(st_facilities_pp_fflags)  $(astro_pp_fflags)  $(tip_pp_fflags)  $(st_graph_pp_fflags)  $(st_stream_pp_fflags)  $(STpolicy_pp_fflags)  $(facilities_pp_fflags)  $(f2c_pp_fflags)  $(RootcintPolicy_pp_fflags)  $(GlastPolicy_pp_fflags)  $(pil_pp_fflags)  $(cfitsio_pp_fflags)  $(CLHEP_pp_fflags)  $(extFiles_pp_fflags)  $(ROOT_pp_fflags)  $(cppunit_pp_fflags) 
use_linkopts= $(cmt_installarea_linkopts)   $(sourceIdentify_linkopts)  $(st_app_linkopts)  $(hoops_linkopts)  $(catalogAccess_linkopts)  $(st_facilities_linkopts)  $(astro_linkopts)  $(tip_linkopts)  $(st_graph_linkopts)  $(st_stream_linkopts)  $(STpolicy_linkopts)  $(facilities_linkopts)  $(f2c_linkopts)  $(RootcintPolicy_linkopts)  $(GlastPolicy_linkopts)  $(pil_linkopts)  $(cfitsio_linkopts)  $(CLHEP_linkopts)  $(extFiles_linkopts)  $(ROOT_linkopts)  $(cppunit_linkopts) 
use_libraries= $(st_app_libraries)  $(hoops_libraries)  $(catalogAccess_libraries)  $(st_facilities_libraries)  $(astro_libraries)  $(tip_libraries)  $(st_graph_libraries)  $(st_stream_libraries)  $(STpolicy_libraries)  $(facilities_libraries)  $(f2c_libraries)  $(RootcintPolicy_libraries)  $(GlastPolicy_libraries)  $(GlastPatternPolicy_libraries)  $(GlastCppPolicy_libraries)  $(pil_libraries)  $(cfitsio_libraries)  $(CLHEP_libraries)  $(extFiles_libraries)  $(ROOT_libraries)  $(cppunit_libraries) 
includes= $(use_includes)
fincludes= $(includes)
gtsrcid_use_linkopts=  $(sourceIdentify_linkopts)  $(st_app_linkopts)  $(hoops_linkopts)  $(catalogAccess_linkopts)  $(st_facilities_linkopts)  $(astro_linkopts)  $(tip_linkopts)  $(st_graph_linkopts)  $(st_stream_linkopts)  $(STpolicy_linkopts)  $(facilities_linkopts)  $(f2c_linkopts)  $(RootcintPolicy_linkopts)  $(GlastPolicy_linkopts)  $(pil_linkopts)  $(cfitsio_linkopts)  $(CLHEP_linkopts)  $(extFiles_linkopts)  $(ROOT_linkopts)  $(cppunit_linkopts) 
constituents= gtsrcid 
all_constituents= $(constituents)
constituentsclean= gtsrcidclean 
all_constituentsclean= $(constituentsclean)
cmt_installarea_paths=$(cmt_installarea_prefix)/$(tag)/bin $(cmt_installarea_prefix)/$(tag)/lib $(cmt_installarea_prefix)/share/lib $(cmt_installarea_prefix)/share/bin
