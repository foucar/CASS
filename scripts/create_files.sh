#!/bin/sh

# this script creates all txt and ini files needed for an analysis. It uses a
# template of the ini file and just replaces the darkfiles to be loaded.
# If the darkfiles are not yet created they will be created before.


# config

# the diretory where the analysis is taken place
BASEDIR=$HOME/analysis/exp60512

# the dir where all the .ini for the runs will be placed
INIDIR=$BASEDIR/ini

# the dir where the txt file containing the files to analyse will be placed
TXTDIR=$BASEDIR/txt

# the dir where the sh files that will be submitted to the batch farm will be placed
SHDIR=$BASEDIR/sh

# the dir where the darkcals will be stored in
DARKCALDIR=$BASEDIR/darkcal

# the dir where all the data file are residing
DATAFILESDIR=/reg/d/psdm/amo/amo60512/xtc

# the dir where the ouput files will be written to
OUTPUTDIR=$BASEDIR/hdf5

# the dir where the jobs output will be written to
JOBDIR=$BASEDIR/jobs

# sting the precedes the runnumber
FILEBASENAME=e199-r0

# the directory where all the output files should be written to
ANALYSISOUTPUTDIR=$BASEDIR/hdf5

# the ini file that works as template for the runs
INIRUNTEMPLATE=$INIDIR/CroVTemplate.ini

# the ini file that works as template for creating darkcals
INIDARKCALTEMPALTE=$INIDIR/darkcalTemplate.ini

# sh file template to be used as run
SHRUNTEMPLATE=$SHDIR/template.sh

# sh file template to be used for darkcal creation
SHDARKCALTEMPLATE=$SHDIR/template.sh

# command to submit the sh file to the cluster
SUBMITCOMMAND="bsub -q psnehq -o"

# the extension that the output file should have
OUTPUTEXT=h5







# create the ini file for a run from a template

create_ini_file()
{
  iniFilename=$INIDIR/run_$1.ini
  darkcal0Filename=$DARKCALDIR/darkcal_run$2_0.cal
  darkcal1Filename=$DARKCALDIR/darkcal_run$2_1.cal

  if [ ! -f "$darkcal0Filename" ]
  then
    echo "$darkcal0Filename does not exist. Create it first";
    return 1;
  fi

  if [ ! -f "$darkcal1Filename" ]
  then
    echo "$darkcal1Filename does not exist. Create it first";
    return 1;
  fi

  if [ -f "$iniFilename" ]
  then
    echo "WARNING: $iniFilename does already exist, overwriting it"
  else
    echo "Creating ini file $iniFilename for run $1 using darkcal files $darkcal0Filename and $darkcal1Filename"
  fi

  if [ ! -f "$INIRUNTEMPLATE" ]
  then
    echo "The ini template for runs $INIRUNTEMPLATE does not exist. Please provide it";
    exit 0
  fi

  sed -e 's:darkcal_0.lnk:'"$darkcal0Filename"':' -e 's:darkcal_1.lnk:'"$darkcal1Filename"':' <$INIRUNTEMPLATE > $iniFilename
}


# create ini file for a darkcalrun from a template

create_darkcal_ini_file()
{
  iniFilename=$INIDIR/run_$1.ini
  darkcal0Filename=$DARKCALDIR/darkcal_run$1_0.cal
  darkcal1Filename=$DARKCALDIR/darkcal_run$1_1.cal

  if [ -f "$iniFilename" ]
  then
    echo "WARNING: $iniFilename does already exist, overwriting it"
  else
    echo "Creating ini file $iniFilename to create darkcal files $darkcal0Filename and $darkcal1Filename"
  fi

  if [ ! -f "$INIDARKCALTEMPALTE" ]
  then
    echo "The ini template to create darkcals $INIDARKCALTEMPALTE does not exist. Please provide it"
    exit 0
  fi

  sed -e 's:darkcal_0.cal:'"$darkcal0Filename"':' -e 's:darkcal_1.cal:'"$darkcal1Filename"':' <$INIDARKCALTEMPALTE > $iniFilename
}


# create the txt file that contains the files to be processed

create_txt_file()
{
  txtFilename=$TXTDIR/run_$1.txt

  if [ -f "$txtFilename" ]
  then
    echo "WARNING: $txtFilename does already exist, overwriting it"
  else
    echo "Creating txt file $txtFilename containing all data files of run $1"
  fi

  ls $DATAFILESDIR/$FILEBASENAME$1* > $txtFilename
}


# create the sh file that will run cass when submitted to the batch farm

create_sh_file()
{
  shFilename=$SHDIR/run_$1.sh
  txtFilename=$TXTDIR/run_$1.txt
  iniFilename=$INIDIR/run_$1.ini
  outputFilename=$OUTPUTDIR/run_$1.$OUTPUTEXT

  if [ -f "$shFilename" ]
  then
    echo "WARNING: $shFilename does already exist, overwriting it"
  else
    echo "Creating sh file $shFilename to submit the job for run $1"
  fi

  if [ ! -f "$SHRUNTEMPLATE" ]
  then
    echo "The sh template for runs $SHRUNTEMPLATE does not exist. Please provide it"
    exit 0
  fi

  sed -e 's:FilesToProcess.txt:'"$txtFilename"':' -e 's:IniFile.ini:'"$iniFilename"':' -e 's:OutputFile.out:'"$outputFilename"':' <$SHRUNTEMPLATE > $shFilename
  chmod u+x $shFilename
}


# create the sh file that will use cass to create the darkcal file

create_darkcal_sh_file()
{
  shFilename=$SHDIR/run_$1.sh
  txtFilename=$TXTDIR/run_$1.txt
  iniFilename=$INIDIR/run_$1.ini

  if [ -f "$shFileName" ]
  then
    echo "WARNING: $shFilename does already exist, overwriting it"
  else
    echo "Creating sh file $shFilename to submit the job for darkcal run $1"
  fi

  if [ ! -f "$SHDARKCALTEMPLATE" ]
  then
    echo "The sh template for darkcals $SHDARKCALTEMPLATE does not exist. Please provide it"
    exit 0
  fi

  sed -e 's:FilesToProcess.txt:'"$txtFilename"':' -e 's:IniFile.ini:'"$iniFilename"':' <$SHDARKCALTEMPLATE > $shFilename
  chmod u+x $shFilename
}



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
  jobname=$1.sh
  echo "check if $jobname is in the queue"
  bjobs | while read line
  do
    if [[ "$line" == *"$jobname"* ]]
    then
      return 1
    fi
  done
  retval=$?
  return $retval
}



# check whether the files of the run are present to the user yet

check_offline_files_not_present()
{

  datafilenames=$DATAFILESDIR/$FILEBASENAME$1*

  echo "check if $datafilenames are not present to be analyzed"
  ls $datafilenames | while read line
  do
    if [[ "$line" == *progress* ]]
    then
      return 1
    fi
  done
  retval=$?
  filecount=`ls $datafilenames | wc -l`
  if [ $filecount -eq 0 ]
  then
    retval=1
  fi
  return $retval
}







# ------------------------- main ---------------------------------


# rm -f $BASEDIR/run_all_$1.sh

# parameter 1 should contain the file that links run numbers to the corresponding
# darkcal run

if [ -z "$1" ]
then
  echo "No file containing the run numbers $1 to process has been given";
else
  cat $1 | while read runnbr darkcalrunnbr;
  do
    if [[ "$runnbr" == *#* ]]
    then
      echo "line containing $runnbr will be ignored";
    else
      create_ini_file $runnbr $darkcalrunnbr
      return_val=$?
      if [ "$return_val" -eq 1 ]
      # when the darkrun needs to be created first
      then
        check_offline_files_not_present $darkcalrunnbr
        return_val=$?
        if [ "$return_val" -eq 1 ]
        then
          echo "data files for run $darkcalrunnbr are not yet present"
        else
          echo "data files for run $darkcalrunnbr are present"
          create_txt_file $darkcalrunnbr
          create_darkcal_ini_file $darkcalrunnbr
          create_darkcal_sh_file $darkcalrunnbr
          SHFILETORUN=$SHDIR/run_$darkcalrunnbr.sh
          joboutput=$JOBDIR/run_$darkcalrunnbr.out
          # check whether job is already on the queue
          check_in_queue $darkcalrunnbr
          return_val=$?
          if [ "$return_val" -eq 0 ]
          then
            echo "$SHFILETORUN is not in the queue: Submitting job that will create the darkcals from run $darkcalrunnbr for run $runnbr"
            $SUBMITCOMMAND $joboutput $SHFILETORUN #!!!
            echo "check if job is submitted"
            in_queue=0
            while [ "$in_queue" -eq 0 ]
            do
              check_in_queue $darkcalrunnbr
              in_queue=$?
              sleep 5
            done
            echo "Job $SHFILETORUN is now submitted"
          else
            echo "The job $SHFILETORUN is already been submitted to the queue. Don't do anything"
          fi
        fi
      # when the run should be analyzed
      else
        check_offline_files_not_present $runnbr
        return_val=$?
        if [ "$return_val" -eq 1 ]
        then
          echo "data files for run $runnbr are not yet present"
        else
          create_txt_file $runnbr
          create_sh_file $runnbr $darkcalrunnbr
          SHFILETORUN=$SHDIR/run_$runnbr.sh
          joboutput=$JOBDIR/run_$runnbr.out
          # check whether this run has already been analyzed
#          check_analyzed $runnbr
#          retval=$?
#          if [ "$retval" -eq 0 ]
#          then
#            # check whether it is in the queue
#            check_in_queue $runnbr
#            return_val=$?
#            if [ "$return_val" -eq 0 ]
#            then
#              echo "submitting job that will analyze run $runnbr"
#              $SUBMITCOMMAND $joboutput $SHFILETORUN                              #!!!
#              echo "check if job $SHFILETORUN is submitted"
#              in_queue=0
#             while [ "$in_queue" -eq 0 ]
#              do
#                check_in_queue $runnbr
#               in_queue=$?
#                sleep 5
#              done
#             echo "Job $SHFILETORUN is now submitted"
#           else
#             echo "The job $SHFILETORUN is already been submitted to the queue. Don't do anything"
#            fi
#          else
#            echo "$runnbr has already been analyzed, Don't do anything"
#          fi
        fi
      fi
    fi
  done
fi

