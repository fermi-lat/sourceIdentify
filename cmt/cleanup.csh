if ( $?CMTROOT == 0 ) then
  setenv CMTROOT /opt/projects/glast/extlib/CMT/v1r16p20040701
endif
source ${CMTROOT}/mgr/setup.csh
set tempfile=`${CMTROOT}/mgr/cmt build temporary_name -quiet`
if $status != 0 then
  set tempfile=/tmp/cmt.$$
endif
${CMTROOT}/mgr/cmt -quiet cleanup -csh -pack=sourceIdentify -version=v0r1p1 -path=/home/knodlseder/glast/src $* >${tempfile}; source ${tempfile}
/bin/rm -f ${tempfile}
