ulimit -s 8192

export LIBRARY_PATH=/lfs/l3/asg/lib:$LIBRARY_PATH

export ROOTSYS=/lfs/l3/asg/root_v5.32.00
export LD_LIBRARY_PATH=$ROOTSYS/lib:$LD_LIBRARY_PATH
export PATH=$ROOTSYS/bin:$PATH

/lfs/l3/ullrch/foucar/source/cass_test/bin/cass_offline -q -r -i FilesToProcess.txt -f IniFile.ini -o OutputFile.out
