CURRDIR=${PWD}
cd ../../..
SRCBASE=${PWD}
cd ${CURRDIR}

export ECOS_REPOSITORY=${SRCBASE}/ecos/ecos-3.0/packages
export PATH=${PATH}:${SRCBASE}/ecos/ecos-3.0/tools/bin
