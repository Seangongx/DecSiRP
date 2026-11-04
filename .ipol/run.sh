#!/bin/bash
PATH=$PATH:/usr/bin/:/usr/local/bin:/opt/local/bin

INPUT_MESH=$1
INPUT_IMAGE=$2
LENGTH=$3
THRESHOLDACC=$4
THRESHOLDCONF=$5
DISTANCEMAX=$6
LAMBDA=$7
RHO=$8

cp /workdir/bin/random.png /workdir/exec/ 

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

COMMANDComputeMESH="decSiRP -m  ${INPUT_MESH}  -l ${LENGTH} -a ${THRESHOLDACC} -c ${THRESHOLDCONF} -d ${DISTANCEMAX} --lambda ${LAMBDA} --rho ${RHO} --export-segmentation resultSeg.obj --mergeMat  "

COMMANDComputeVOL="decSiRP -i  ${INPUT_IMAGE}  -l ${LENGTH} -a ${THRESHOLDACC} -c ${THRESHOLDCONF} -d ${DISTANCEMAX} --lambda ${LAMBDA} --rho ${RHO} --export-segmentation resultSeg.obj --mergeMat  "

export DISPLAY=:1; Xvfb "$DISPLAY" -screen 0 1280x1024x24 &

if test -f "${INPUT_MESH}"
then
    COMMANDUpdate="meshBasicEdit  ${INPUT_MESH}  --rescaleInterToCube 50 256 -o ${INPUT_MESH}"
    applyCommand COMMANDUpdate

    applyCommand COMMANDComputeMESH
    echo "Generate display from input:"
    meshViewer ${INPUT_MESH}   2>&1 |  while read -r line; do if [[ $line == "[display ready]" ]] ; then sleep 1s; import -window root -display :1 -screen visuInput.png; pkill meshViewer;  fi; done;
    meshViewer resultSeg.obj   2>&1 |  while read -r line; do if [[ $line == "[display ready]" ]] ; then sleep 1s; import -window root -display :1 -screen visuResult.png; pkill meshViewer;  fi; done;
    decSiRP -m "${INPUT_MESH}" -l "${LENGTH}" -a "${THRESHOLDACC}" -c "${THRESHOLDCONF}" -d "${DISTANCEMAX}" --lambda "${LAMBDA}" --rho "${RHO}" 2>&1 | while read -r line; do if [[ "$line" == "[display ready]" ]]; then sleep 1s; echo "import image.."; import -window root -display :1 -screen visuInterface.png; pkill decSiRP; fi; done;
    echo "[done]"
    convert visuInput.png -fuzz 3% -trim +repage -resize 800x600 visuInput.png 2>&1
    convert visuResult.png -fuzz 3% -trim +repage -resize 800x600 visuResult.png 2>&1
    convert visuInterface.png -fuzz 3% -trim +repage -resize 800x600 visuInterface.png 2>&1
    
fi



if test -f "${INPUT_IMAGE}"
then
    applyCommand COMMANDComputeVOL
    3dImageViewer ${INPUT_IMAGE} - 2>&1 |  while read -r line; do if [[ "$line" == "Image loaded:"* ]]; then sleep 2s; import -window root -display :1 -screen visuInput.png; pkill 3dImageViewer;  fi; done;
    meshViewer resultSeg.obj   2>&1 |  while read -r line; do if [[ $line == "[display ready]" ]] ; then sleep 1s; import -window root -display :1 -screen visuResult.png; pkill meshViewer;  fi; done;
    decSiRP -i "${INPUT_IMAGE}" -l "${LENGTH}" -a "${THRESHOLDACC}" -c "${THRESHOLDCONF}" -d "${DISTANCEMAX}" --lambda "${LAMBDA}" --rho "${RHO}" 2>&1 | while read -r line; do if [[ "$line" == "[display ready]" ]]; then sleep 1s; echo "import image.."; import -window root -display :1 -screen visuInterface.png; pkill decSiRP; fi; done;
    echo "[done]"
    convert visuInput.png -fuzz 3% -trim +repage -resize 800x600 visuInput.png 2>&1
    convert visuResult.png -fuzz 3% -trim +repage -resize 800x600 visuResult.png 2>&1
    convert visuInterface.png -fuzz 3% -trim +repage -resize 800x600 visuInterface.png 2>&1

fi







