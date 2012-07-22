#!/bin/sh

# this script creates all txt and ini files needed for an analysis. It uses a
# template of the ini file and just replaces the darkfiles to be loaded.
# If the darkfiles are not yet created they will be created before.


# config

# the diretory where the analysis is taken place
BASEDIR=/home/ullrch/foucar/sacla/commisioning_feb2012/analysis

# the dir where all the .ini for the runs will be placed
INIDIR=$BASEDIR/ini

# the dir where the txt file containing the files to analyse will be placed
TXTDIR=$BASEDIR/txt

# the dir where the sh files that will be submitted to the batch farm will be placed
SHDIR=$BASEDIR/sh

# the dir where the darkcals will be stored in
DARKCALDIR=$BASEDIR/darkcal

# the dir where all the data file are residing
DATAFILESDIR=/reg/d/pdsm/amo/amoxxx/xtc

# the dir where the ouput files will be written to
OUTPUTDIR=$BASEDIR/hd5

# sting the precedes the runnumber
FILEBASENAME=e-xxx-xxx

# the cass binary
CASSBIN=/reg/neh/home/lutz/src/cass/bin/cass_offline

# the directory where all the output files should be written to
ANALYSISOUTPUTDIR=/some/where

# the ini file that works as template for the runs
INIRUNTEMPLATE=$INIDIR/run_template.ini

# the ini file that works as template for creating darkcals
INIDARKCALTEMPALTE=$INIDIR/darkcal_template.ini

# sh file template to be used as run
SHRUNTEMPLATE=$SHDIR/run_template.sh

# sh file template to be used for darkcal creation
SHDARKCALTEMPLATE=$SHDIR/darkcal_template.sh

# command to submit the sh file to the cluster
SUBMITCOMMAND=qsub

# the extension that the output file should have
OUTPUTEXT=h5







# create the ini file for a run from a template

create_ini_file()
{
  iniFilename=$INIDIR/run_$1.ini
  darkcal0Filename=$DARKCALDIR/darkcal_$2_0.cal
  darkcal1Filename=$DARKCALDIR/darkcal_$2_1.cal

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
    echo "The ini template for runs $INIRUNTEMPLATE does not exist. Please provide it"
    exit(0)
  fi

  sed 's:darkcal_0.cal:'"$darkcal0Filename"':' <$INIRUNTEMPLATE > $iniFilename
  sed 's:darkcal_1.cal:'"$darkcal1Filename"':' <iniFilename > $iniFilename
}


# create ini file for a darkcalrun from a template

create_darkcal_ini_file()
{
  iniFilename=$INIDIR/run_$1.ini
  darkcal0Filename=$DARKCALDIR/darkcal_$1_0.cal
  darkcal1Filename=$DARKCALDIR/darkcal_$1_1.cal

  if [ -f "$iniFilename" ]
  then
    echo "WARNING: $iniFilename does already exist, overwriting it"
  else
    echo "Creating ini file $iniFilename for to create darkcal files $darkcal0Filename and $darkcal1Filename"
  fi

  if [ ! -f "$INIDARKCALTEMPALTE" ]
  then
    echo "The ini template to create darkcals $INIDARKCALTEMPALTE does not exist. Please provide it"
    exit(0)
  fi

  sed 's:darkcal_0.cal:'"$darkcal0Filename"':' <$INIDARKCALTEMPALTE > $iniFilename
  sed 's:darkcal_1.cal:'"$darkcal1Filename"':' <iniFilename > $iniFilename
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
  shFileName=$SHDIR/run_$1.sh
  txtFilename=$TXTDIR/run_$1.txt
  iniFilename=$INIDIR/run_$1.ini
  outputFilename=$OUTPUTDIR/run_$1.$OUTPUTEXT

  if [ -f "$shFileName" ]
  then
    echo "WARNING: $shFileName does already exist, overwriting it"
  else
    echo "Creating sh file $shFileName to submit the job for run $1"
  fi

  if [ ! -f "$SHRUNTEMPLATE" ]
  then
    echo "The sh template for runs $SHRUNTEMPLATE does not exist. Please provide it"
    exit(0)
  fi

  sed 's:FilesToProcess.txt:'"$txtFilename"':' <$SHRUNTEMPLATE > $shFilename
  sed 's:IniFile.ini:'"$iniFilename"':' <$shFilename > $shFilename
  sed 's:OutputFile.out:'"$outputFilename"':' <$shFilename > $shFilename
}


# create the sh file that will use cass to create the darkcal file

create_darkcal_sh_file()
{
  shFileName=$SHDIR/run_$1.sh
  txtFilename=$TXTDIR/run_$1.txt
  iniFilename=$INIDIR/run_$1.ini

  if [ -f "$shFileName" ]
  then
    echo "WARNING: $shFileName does already exist, overwriting it"
  else
    echo "Creating sh file $shFileName to submit the job for darkcal run $1"
  fi

  if [ ! -f "$SHDARKCALTEMPLATE" ]
  then
    echo "The sh template for darkcals $SHDARKCALTEMPLATE does not exist. Please provide it"
    exit(0)
  fi

  sed 's:FilesToProcess.txt:'"$txtFilename"':' <$SHDARKCALTEMPLATE > $shFilename
  sed 's:IniFile.ini:'"$iniFilename"':' <$shFilename > $shFilename
}





# main


# rm -f $BASEDIR/run_all_$1.sh

# parameter 1 should contain the file that links run numbers to the corresponding
# darkcal run

if [ -n "$1" ]
then
  echo "No file containing the runumbers to process has been given";
else
  cat $1 | while read runnbr darkcalrunnbr;
  do
    if [[ "$runnbr" == *#* ]]
    then
      echo "line containing $runnbr will be ignored";
    else
      SHFILETORUN=""
      create_ini_file $runnbr $darkcalrunnbr
      return_val=$?
      if [ "$return_val" -eq 1 ]
      then
        create_txt_file $darkcalrunnbr
        create_darkcal_ini_file $darkcalrunnbr
        create_darkcal_sh_file $darkcalrunnbr
        $SHFILETORUN=$SHDIR/run_$darkcalrunnbr
      else
        create_txt_file $runnbr
        create_sh_file $runnbr $darkcalrunnbr
        $SHFILETORUN=$SHDIR/run_$runnbr
      fi
      echo "submitting job containing run $runnbr to cluster";
      $SUBMITCOMMAND $SHFILETORUN
    fi
  done
fi

