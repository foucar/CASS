#!/bin/sh

# where to find the cass bare repo
GIT_REPO=/lfs/l3/asg/repos/diode.git

# where the tmp repo should be created
TMP_REPO=$HOME/tmp/diode



# check whether a commit was pushed to the master branch if so start the deploy-skript
while read oldrev newrev refname
do
    if [ "$refname" = "refs/heads/master" ] ; then
      echo "Branch master has changed; deploying website and binaries:"
      git clone $GIT_REPO $TMP_REPO
      $TMP_REPO/scripts/createDoc.sh $TMP_REPO
      $TMP_REPO/scripts/deploy.sh $TMP_REPO
      rm -rf $TMP_REPO
    else
      echo "Branch master has not been changed"
    fi
done
