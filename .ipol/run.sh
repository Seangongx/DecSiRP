#!/bin/bash
PATH=$PATH:/usr/bin/:/usr/local/bin:/opt/local/bin

INPUT=$1
LENGTH=$2
THRESHOLDACC=$3
THRESHOLDCONF=$4
DISTANCEMAX=$5
LAMBDA=$6
PROB=$7

function applyCommand
{
  for c in $*
  do
      echo "Starting command: ${!c}  "
      utime="$( TIMEFORMAT='%lU';time ( ${!c} 2> /dev/null  ) 2>&1 1>/dev/null )"
      if [ $? -ne 0 ] 
      then
          exit 1
      else
          echo "[done] (in $utime)"
      fi
  done
}


echo "----------------------------------------"
echo "-----Computing segmentation-------------"
echo "----------------------------------------"

COMMANDCompute="viz -m  ${INPUT}  -l ${LENGTH} -a ${THRESHOLDACC} -c ${THRESHOLDCONF} -d ${DISTANCEMAX} --proba ${PROB} --lambda ${LAMBDA}
 --export-segmentation resultSeg.obj  "
applyCommand COMMANDCompute


