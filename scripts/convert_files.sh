#!/bin/sh

# this script creates all txt and ini files needed for an analysis. It uses a
# template of the ini file and just replaces the darkfiles to be loaded.
# If the darkfiles are not yet created they will be created before.


# config

# the diretory where the analysis is taken place
BASEDIR=/reg/d/ana12/cxi/cxii0512/scratch/cass

# the dir where all the .ini for the runs will be placed
INIDIR=$BASEDIR/ini

# the dir where the txt file containing the files to analyse will be placed
TXTDIR=$BASEDIR/txt

# the dir where the sh files that will be submitted to the batch farm will be placed
SHDIR=$BASEDIR/sh

# the dir where the darkcals will be stored in
DARKCALDIR=$BASEDIR/darkcal

# the dir where all the data file are residing
DATAFILESDIR=/reg/d/psdm/cxi/cxii0512/xtc

# the dir where the ouput files will be written to
OUTPUTDIR=$BASEDIR/hdf5

# the dir where the jobs output will be written to
JOBDIR=$BASEDIR/jobs

# sting the precedes the runnumber
FILEBASENAME=e220-r0

# the directory where all the output files should be written to
ANALYSISOUTPUTDIR=$BASEDIR/hdf5

# the ini file that works as template for the runs
INIRUNTEMPLATE=$INIDIR/Run.ini

# the ini file that works as template for creating darkcals
INIDARKCALTEMPALTE=$INIDIR/Darkcal.ini

# sh file template to be used as run
SHRUNTEMPLATE=$SHDIR/template.sh

# sh file template to be used for darkcal creation
SHDARKCALTEMPLATE=$SHDIR/template.sh

# command to submit the sh file to the cluster
#SUBMITCOMMAND="qsub -N "
SUBMITCOMMAND="bsub -q psfehq "

# command to list all submitted jobs
#QUERYCOMMAND="qstat -u foucar"
QUERYCOMMAND="bjobs"

# the extension that the output file should have
OUTPUTEXT=h5










# check whether the run has been anlyzed

check_analyzed()
{
  outputFilename=$OUTPUTDIR/run_$1.$OUTPUTEXT

  if [ ! -f "$outputFilename" ]
  then
    return 0
  fi
  return 1
}



# check whether the run is in the queue right now

check_in_queue()
{
  jobname=$1
  echo "check if $jobname is in the queue"
  retval=0
  for line in `$QUERYCOMMAND`
  do
    if [[ "$line" == *"$jobname"* ]]
    then
      retval=1
    fi
  done
  return $retval
}



# check if darkcals are not existent

check_darkcal_not_exists()
{
  darkcal0Filename=$1
  darkcal1Filename=$2

  #front detector
  if [ ! -f "$darkcal0Filename" ]
  then
    echo "$darkcal0Filename does not exist.";
    return 1;
  fi

  #back detector
#  if [ ! -f "$darkcal1Filename" ]
#  then
#    echo "$darkcal1Filename does not exist.";
#    return 1;
#  fi

  return 0;
}



# wait until the jobs appears in the list of jobs submitted to the queue

wait_until_submitted()
{
  in_queue=0
  while [ "$in_queue" -eq 0 ]
  do
    check_in_queue $1
    in_queue=$?
    sleep 5
  done
  echo "Job $1 is now submitted"
}



# check how many slices exist for a run

find_amount_of_slices_for_run()
{
  greatestslice=0
  for line in `ls $1*`;
  do
    slice=$(echo "$line" | cut -d"-" -f3 | cut -c 2-3)
    if [ "$slice" -gt "$greatestslice" ]
    then
      greatestslice=$slice
    fi
  done
  return $greatestslice
}



# wait unit the darkcal is created

wait_until_darkcal_created()
{
  darcalnotexist=1
  while [ "$darcalnotexist" -eq 1 ]
  do
    check_darkcal_not_exists $1 $2
    darcalnotexist=$?
    sleep 5
  done
  echo "Darkcalfiles $1 and $2 do exist now"
}


test()
{
  txtFilename=$TXTDIR/run_$1.txt

  echo "$DATAFILESDIR/$FILEBASENAME$1*"

  greatestslice=0

  for line in `ls $DATAFILESDIR/$FILEBASENAME$1*`;
  do
    runnbr=$(echo "$line" | cut -d"-" -f2)
    slice=$(echo "$line" | cut -d"-" -f3 | cut -c 2-3)
    chunk=$(echo "$line" | cut -d"-" -f4 | cut -d "." -f1)

    if [ "$slice" -gt "$greatestslice" ]
    then
      greatestslice=$slice
    fi
    echo " runnbr: $runnbr - slice:$slice - chunk:$chunk - greatestslice:$greatestslice"
  done
  echo "greatestslice:$greatestslice"
  return $greatestslice

}


# ------------------------- main ---------------------------------


# parameter 1 should contain the file that links run numbers to the corresponding
# darkcal run



# check if at least a file that contains the runs an the corresponding darkcal
# runs has been given
if [ -z "$1" ]
then
  echo "No file containing the run numbers $1 to process has been given";
else
  # if the file containing the run numbers has been provided read it line by line
  # assuming that the first is the runnumber and the second is the corresponding
  # darkcalrun number
  cat $1 | while read runnbr darkcalrunnbr;
  do
    # ignore lines that contain a '#'
    if [[ "$runnbr" == *#* ]]
    then
      echo "line containing $runnbr will be ignored";
    else
      #define the names that the darkcals should have
      darkcal0Filename=$DARKCALDIR/darkcal_run"$darkcalrunnbr"_7.cal
      darkcal1Filename=$DARKCALDIR/darkcal_run"$darkcalrunnbr"_8.cal

      # now check whether a darkcal for the darkcal run already exists
      check_darkcal_not_exists $darkcal0Filename $darkcal1Filename
      return_val=$?
      if [ "$return_val" -eq 1 ]
      then
        # when the darkrun needs to be created first, create the txt file
        # containing the datafiles for the darkcal run, the ini and the sh
        # file to submit it to the queue.
        txtFilename=$TXTDIR/darkrun_$darkcalrunnbr.txt
        iniFilename=$INIDIR/darkrun_$darkcalrunnbr.ini
        shFilename=$SHDIR/darkrun_$darkcalrunnbr.sh
        logFilename=darkrun_$darkcalrunnbr.log
        joboutput=$JOBDIR/run_$darkcalrunnbr.out
        jobname=run_$darkcalrunnbr

        ls $DATAFILESDIR/$FILEBASENAME$darkcalrunnbr* > $txtFilename

        if [ ! -f "$INIDARKCALTEMPALTE" ]
        then
          echo "The ini template to create darkcals $INIDARKCALTEMPALTE does not exist. Please provide it"
          exit 0
        fi
        sed -e 's:darkcal_7.cal:'"$darkcal0Filename"':' -e 's:darkcal_8.cal:'"$darkcal1Filename"':' -e 's:logfilename.log:'"$logFilename"':' <$INIDARKCALTEMPALTE > $iniFilename

        if [ ! -f "$SHDARKCALTEMPLATE" ]
        then
          echo "The sh template for darkcals $SHDARKCALTEMPLATE does not exist. Please provide it"
          exit 0
        fi
        sed -e 's:FilesToProcess.txt:'"$txtFilename"':' -e 's:IniFile.ini:'"$iniFilename"':' -e 's:output.txt:'"$joboutput"':'  <$SHDARKCALTEMPLATE > $shFilename
        chmod u+x $shFilename


        # now submit the job creating the darkcal to the queue and wait until
        # the darkcals are created
#        echo $SUBMITCOMMAND $jobname $shFilename #!!!
        $SUBMITCOMMAND -o $joboutput -J $jobname $shFilename #!!!
        wait_until_darkcal_created $darkcal0Filename $darkcal1Filename
      fi

      # at this point the darkcal for the run we want to analyze has been created
      # now find out into how many slices the run has been sliced.
      find_amount_of_slices_for_run $DATAFILESDIR/$FILEBASENAME$runnbr
      nbrSlices=$?
      for ((slice=1; slice<=$nbrSlices; ++slice ))
      do

        #check if slice exists, if not continue with the next slice
        if [ ! -f "$DATAFILESDIR/$FILEBASENAME$runnbr-s0$slice-c00.xtc" ]
        then
          echo "slice $slice does not exist, skipping it"
          continue
        fi

        # for each slice create the txt, ini and sh file and submit the sh file
        # to the queue
        txtFilename=$TXTDIR/run_"$runnbr"_slice_0$slice.txt
        iniFilename=$INIDIR/run_"$runnbr"_slice_0$slice.ini
        shFilename=$SHDIR/run_"$runnbr"_slice_0$slice.sh
#        outputFilename=$OUTPUTDIR/run_"$runnbr"_slice_0$slice.$OUTPUTEXT
        outputFilename=$OUTPUTDIR/run_"$runnbr"_slice_0"$slice"_
        logFilename=run_"$runnbr"_slice_0$slice.log
        joboutput=$JOBDIR/run_"$runnbr"_slice_0$slice.out
        jobname=run_"$runnbr"_$slice


        ls $DATAFILESDIR/$FILEBASENAME$runnbr-s0$slice* > $txtFilename

        if [ ! -f "$INIRUNTEMPLATE" ]
        then
          echo "The ini template for runs $INIRUNTEMPLATE does not exist. Please provide it";
          exit 0
        fi
#        sed -e 's:darkcal_0.lnk:'"$darkcal0Filename"':' -e 's:darkcal_1.lnk:'"$darkcal1Filename"':' -e 's:logfilename.log:'"$logFilename"':'  <$INIRUNTEMPLATE > $iniFilename
        sed -e 's:darkcal_7.lnk:'"$darkcal0Filename"':' -e 's:darkcal_8.lnk:'"$darkcal1Filename"':' -e 's:logfilename.log:'"$logFilename"':'  <$INIRUNTEMPLATE > $iniFilename

        if [ ! -f "$SHRUNTEMPLATE" ]
        then
          echo "The sh template for runs $SHRUNTEMPLATE does not exist. Please provide it"
          exit 0
        fi
        sed -e 's:FilesToProcess.txt:'"$txtFilename"':' -e 's:IniFile.ini:'"$iniFilename"':' -e 's:OutputFile.out:'"$outputFilename"':' -e 's:output.txt:'"$joboutput"':' <$SHRUNTEMPLATE > $shFilename
        chmod u+x $shFilename

        # now submit job to the queue and wait until its submitted
#        echo $SUBMITCOMMAND $jobname $shFilename
        $SUBMITCOMMAND -o $joboutput -J $jobname $shFilename
        wait_until_submitted $jobname
      done
    fi
  done
fi

