#!/bin/sh

# this script creates all txt and ini files needed for an analysis. It uses a
# template of the ini file and just replaces the darkfiles to be loaded.
# If the darkfiles are not yet created they will be created before.


# config

# the diretory where the analysis is taken place
BASEDIR=/reg/d/psdm/cxi/cxii0512/scratch/cass

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

# string the precedes the runnumber
FILEBASENAME=e220-r0

# string that indicates that the data file cannot be read yet
PROGRESSSUFFIX=inprogress

# name of the file that contains the runnumbers that have been processed
PROCESSEDRUNSFILE=$BASEDIR/processedRuns.txt

#name of the file where the submit command will ba added to
RUNSUBMITCOMMANDS=$BASEDIR/allruns.sh

#name of the file where the submit command will ba added to
DARKCALSUBMITCOMMANDS=$BASEDIR/alldarkcals.sh

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

#set this to one if you only want to create a file that contains all the submit command
$CREATEONLYSUBMITCOMMANDS=1










# check if run number is in file

check_runnumber_in_file()
{
  runnbr=$1
  filename=$2
  for number in `cat "$filename"`
  do
    if [ "$runnbr" -eq "$number" ]
    then
      return 1
    fi
  done
  return 0
}



# check if a directoy had contents added to it (just an example)

wait_until_contents_added_to_dir()
{
  dir="$1"
  sum1=`find "$dir" -maxdepth 1 -type f -print | wc -l`
  sum2=$sum1

#    chsum1=`digest -a md5 $dir | awk '{print $1}'`

  while [ $sum1 -eq $sum2 ]
  do
    sleep 10
    sum2=`find "$dir" -maxdepth 1 -type f -print | wc -l`
  done

 #eval $2
}



# check whether the run has been anlyzed (obsolete)

check_analyzed()
{
  outputFilename=$OUTPUTDIR/run_$1.$OUTPUTEXT

  if [ ! -f "$outputFilename" ]
  then
    return 0
  fi
  return 1
}



# check if a file has changed contents, if so call this script with the monitored
# file as argument

monitor_file()
{
  echo "Monitoring file $1 for changes. If changes occur calls \"sh $0 $1\""
  file="$1"
  checksumcommand="md5sum "$file" | awk '{print $1}'"
#  chsum1=`md5sum "$file" | awk '{print $1}'`
  chsum1=`$checksumcommand`
  chsum2=$chsum1
  while [[ TRUE ]]
  do
#    chsum1=`md5sum "$file" | awk '{print $1}'`
    chsum1=`$checksumcommand`
    while [ "$chsum1" == "$chsum2" ]
    do
      sleep 10
#      chsum2=`md5sum "$file" | awk '{print $1}'`
      chsum2=`$checksumcommand`
      echo ""$chsum1" "$chsum2""
    done
#    echo "call \"sh $0 $1\""
    sh ./$0 $1
  done
}



# check if run number is in file

check_runnumber_in_file()
{
  runnbr=$1
  filename=$2
  for number in `cat "$filename"`
  do
    if [ "$runnbr" -eq "$number" ]
    then
      return 1
    fi
  done
  return 0
}



# check how many slices exist for a run

find_amount_of_slices_for_run()
{
  greatestslice=-1
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



# check if files with a base name do not exist or are still marked the the
# progress suffix

check_files_not_exists()
{
  filebasename=$1
  sumall=`ls $filebasenme* 2> /dev/null | wc -l`
  sumprogress=`ls $filebasenme* 2> /dev/null | grep "$PROGRESSSUFFIX" | wc -l`
  if [ "$sumall" -eq  0 ]
  then
    echo "files with basename $filebasename do not exist.";
    return 1;
  fi
  if [ $sumprogress -ne 0 ]
  then
    echo "files with basename $filebasename exist, but some contain the $PROGRESSUFFIX extension"
    return 1;
  fi
  return 0;
}



# wait until the files with a basename exist

wait_until_files_exist()
{
  check_files_not_exists $1
  fileDoesNotExist=$?
  fileDoesNotExistInBeginning=$fileDoesNotExist
  while [ "$fileDoesNotExist" -eq 1 ]
  do
    echo "Files with basename $1 do not exist. Wait a little and check again"
    check_file_not_exists $1
    fileDoesNotExist=$?
    sleep 5
  done
  if [ "$fileDoesNotExistInBeginning" -eq 1 ]
  then
    echo "Files with basename $1 exist now"
  fi
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



# check whether the run is in the queue right now

check_in_queue()
{
  jobname=$1
#  echo "check if $jobname is in the queue"
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



# wait until the jobs appears in the list of jobs submitted to the queue

wait_until_submitted()
{
  check_in_queue $1
  in_queue=$?
  in_queue_at_beginning=$in_queue
  while [ "$in_queue" -eq 0 ]
  do
    echo "Job $1 is not submitted. Wait and then check again"
    check_in_queue $1
    in_queue=$?
    sleep 5
  done
  if [ "$in_queue_at_beginning" -eq 0 ]
  then
    echo "Job $1 is now submitted"
  fi
}



#check if file does not exist

check_file_not_exists()
{
  filename=$1
  if [ ! -f "$filename" ]
  then
    echo "$filename does not exist.";
    return 1;
  fi
  return 0;
}



# wait until a file exists

wait_until_file_exists()
{
  check_file_not_exists $1
  fileDoesNotExist=$?
  fileDidNotExistInBeginning=$fileDidNotExistInBeginning
  while [ "$fileDoesNotExist" -eq 1 ]
  do
    echo "File $1 does not exist yet. Wait and check again"
    check_file_not_exists $1
    fileDoesNotExist=$?
    sleep 5
  done
  if [ "$fileDidNotExistInBeginning" -eq 1 ]
  then
    echo "File $1 exists now"
  fi
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

if [ "$2" == "-m" ]
then
  monitor_file "$1"
else
rm -f $RUNSUBMITCOMMANDS
rm -f $DARKCALSUBMITCOMMANDS
fi


# parameter 1 should contain the file that links run numbers to the corresponding
# darkcal run

# check if at least a file that contains the runs an the corresponding darkcal
# runs has been given
if [ -z "$1" ]
then
  echo "No file containing the run numbers $1 to process has been given";
  echo "Usage: "$0" datafile-darkfile_assiciation txt file"
  echo "optional parameter -m lets this script monitor the datafile-darkfile_assiciation txt file for changes and then calls this script recursively"
else
  # if the file containing the run numbers has been provided read it line by line
  # assuming that the first is the runnumber and the second is the corresponding
  # darkcalrun number
  echo "processing runs-darkcals in file $1"
  cat $1 | while read runnbr darkcalrunnbr;
  do
    # ignore lines that contain a '#'
    if [[ "$runnbr" == *#* ]]
    then
      echo "line containing $runnbr will be ignored";
    else
      # find out whether this run has already been processed, by checking if the
      # runnumber is in the file that contains the processed runs. If so skip
      # this run
      check_runnumber_in_file $runnbr $PROCESSEDRUNSFILE
      return_val=$?
      if [ "$return_val" -eq 1 ]
      then
        echo "Run $runnbr has already been processed. Skipping it"
        continue
      fi

      #define the names that the darkcals should have
      darkcal0Filename=$DARKCALDIR/darkcal_run"$darkcalrunnbr"_7.cal
      darkcal1Filename=$DARKCALDIR/darkcal_run"$darkcalrunnbr"_8.cal

      # now check whether a darkcal for the darkcal run already exists
#      check_darkcal_not_exists $darkcal0Filename $darkcal1Filename
      check_file_not_exists $darkcal0Filename
      return_val=$?
#      check_file_not_exists $darkcal1Filename
#      return_val= $? + "$return_val"
      if [ "$return_val" -ne 0 ]
      then
        # wait until all the data files exist if necessary
        darkcalDataFilesBaseName=$DATAFILESDIR/$FILEBASENAME$darkcalrunnbr
        wait_until_files_exist $darkcalDataFilesBaseName

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
        finalsubmitcommand="$SUBMITCOMMAND -o $joboutput -J $jobname $shFilename"
#        echo $SUBMITCOMMAND $jobname $shFilename #!!!
#        echo $SUBMITCOMMAND -o $joboutput -J $jobname $shFilename >> $DARKCALSUBMITCOMMANDS
        echo $finalsubmitcomand >> $DARKCALSUBMITCOMMANDS
        if [ "$CREATEONLYSUBMITCOMMANDS" -ne 1 ]
        then
           $finalsubmitcommand
#          $SUBMITCOMMAND -o $joboutput -J $jobname $shFilename #!!!
#          wait_until_darkcal_created $darkcal0Filename $darkcal1Filename
          wait_until_file_exists $darkcal0Filename
#          wait_until_file_exists $darkcal1Filename
        fi
      fi

      # wait until all the data files exist if necessary
      DataFilesBaseName=$DATAFILESDIR/$FILEBASENAME$runnbr
      wait_until_files_exist $DataFilesBaseName

      # at this point the darkcal for the run we want to analyze has been created
      # now find out into how many slices the run has been sliced. If there are
      # any slices, write the runnumber into the processed runs file. Then go
      # through all slices
      find_amount_of_slices_for_run $DataFilesBaseName
      nbrSlices=$?
      if [ "$return_val" -ne -1 ]
      then
        echo ""$runnbr"" >> $PROCESSEDRUNSFILE
      fi
      for ((slice=0; slice<=$nbrSlices; ++slice ))
      do

        #check if slice exists, if not continue with the next slice
        if [ ! -f "$DataFilesBaseName-s0$slice-c00.xtc" ]
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
        echo "Submitting $jobname"
        finalsubmitcommand= $SUBMITCOMMAND -o $joboutput -J $jobname $shFilename
        echo $finalsubmitcommand >> $RUNSUBMITCOMMANDS
        if [ "$CREATEONLYSUBMITCOMMANDS" -ne 1 ]
        then
          $finalsubmitcommand
#          $SUBMITCOMMAND -o $joboutput -J $jobname $shFilename
          # what is the reason to wait until the job is submitted? Disabling this
          # for now.
#          wait_until_submitted $jobname
        fi
      done
    fi
  done
fi

