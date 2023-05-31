#!/bin/bash

#
# args
#

FILEID=$1
INPUT=$2
CONFIG=$3
CHARGE=$4
DENSITY=$5
SAVEDIST=$6
COPYDIR=$7

echo "[wrapper] FILEID    = " ${FILEID}
echo "[wrapper] INPUT     = " ${INPUT}
echo "[wrapper] CONFIG    = " ${CONFIG}
echo "[wrapper] CHARGE    = " ${CHARGE}
echo "[wrapper] DENSITY   = " ${DENSITY}
echo "[wrapper] SAVEDIST  = " ${SAVEDIST}
echo "[wrapper] COPYDIR   = " ${COPYDIR}

# if [ -r "$OSGVO_CMSSW_Path"/cmsset_default.sh ]; then
#     echo "sourcing environment: source $OSGVO_CMSSW_Path/cmsset_default.sh"
#     source "$OSGVO_CMSSW_Path"/cmsset_default.sh
# elif [ -r "$OSG_APP"/cmssoft/cms/cmsset_default.sh ]; then
#     echo "sourcing environment: source $OSG_APP/cmssoft/cms/cmsset_default.sh"
#     source "$OSG_APP"/cmssoft/cms/cmsset_default.sh
# elif [ -r /cvmfs/cms.cern.ch/cmsset_default.sh ]; then
#     echo "sourcing environment: source /cvmfs/cms.cern.ch/cmsset_default.sh"
#     source /cvmfs/cms.cern.ch/cmsset_default.sh
# else
#     echo "ERROR! Couldn't find $OSGVO_CMSSW_Path/cmsset_default.sh or /cvmfs/cms.cern.ch/cmsset_default.sh or $OSG_APP/cmssoft/cms/cmsset_default.sh"
#     exit 1
# fi

#
# set up environment
#
# if [[ `uname -a` = *"el6"* ]]; then
    # export SCRAM_ARCH=slc6_amd64_gcc630
    # CMSSW_VERSION=CMSSW_9_4_14
# fi
# if [[ `uname -a` = *"el7"* ]]; then
#     export SCRAM_ARCH=slc6_amd64_gcc700
#     CMSSW_VERSION=CMSSW_10_2_15
# fi
echo

echo "[wrapper] uname     = " `uname -a`
echo "[wrapper] hostname  = " `hostname`
echo "[wrapper] date      = " `date`
echo "[wrapper] linux timestamp = " `date +%s`

#
# untar input sandbox
#

DESTDIR="/net/cms11/data/hmei/milliqan/tmp/"
TMPDIR=${DESTDIR}output_${DECAYMODE}${FILEID}
mkdir -p $TMPDIR
echo "current dir is: "$PWD
#cp ./input.tar.xz $DESTDIR
cp $PWD/input.tar.xz $TMPDIR
cd $TMPDIR

echo "[wrapper] extracting input sandbox"
#mv *.tar.xz input.tar.xz
tar -xJf input.tar.xz

echo "[wrapper] input contents are"
ls -a
ls MilliqanSim

#PYTHONPATH=./MilliqanSim:${PYTHONPATH}

source ~/.bashrc
micromamba activate milliqan_sim
#
# run it
#
echo "[wrapper] running: python -u run_sim.py --config ${CONFIG} --charge ${CHARGE} -d ${DENSITY} -s ${SAVEDIST} ${INPUT}"

python3 -u run_sim.py --config ${CONFIG} --charge ${CHARGE} -d ${DENSITY} -s ${SAVEDIST} ${INPUT}

#
# do something with output
#

echo "[wrapper] output is"
ls -lrth

#
# clean up
#

# Rigorous sweeproot which checks ALL branches for ALL events.
# If GetEntry() returns -1, then there was an I/O problem, so we will delete it
cat > rigorousSweepRoot.py << EOL
import ROOT as r
import os, sys

f1 = r.TFile("output.root")
if not f1 or not f1.IsOpen() or f1.IsZombie():
    print "[RSR] removing zombie output.root because it does not deserve to live"
    os.system("rm output.root")
    sys.exit()

t = f1.Get("Events")
if type(t)==type(r.TObject()):
    print "[RSR] no tree named 'Events' in file! Deleting."
    os.system("rm output.root")
    sys.exit()

print "[RSR] ntuple has %i events" % t.GetEntries()

foundBad = False
for i in range(0,t.GetEntries(),1):
    if t.GetEntry(i) < 0:
        foundBad = True
        print "[RSR] found bad event %i" % i
        break

if foundBad:
    print "[RSR] removing output.root because it does not deserve to live"
    os.system("rm output.root")
else:
    print "[RSR] passed the rigorous sweeproot"
EOL

date +%s
echo "[wrapper] running rigorousSweepRoot.py"
python rigorousSweepRoot.py
date +%s


echo "[wrapper] copying file"

if [ ! -d "${COPYDIR}" ]; then
    echo "creating output directory " ${COPYDIR}
    mkdir ${COPYDIR}
fi

cp output.root ${COPYDIR}/output_${FILEID}.root

echo "[wrapper] cleaning up"
for FILE in `find . -not -name "*stderr" -not -name "*stdout"`; do rm -rf $FILE; done
echo "[wrapper] cleaned up"
ls
