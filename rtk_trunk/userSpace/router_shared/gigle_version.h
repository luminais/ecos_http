/**
 * \addtogroup common
 */
/*@{*/

/***************************************************
 * Header name: gigle_version.h
 *
 * Copyright 2010 Gigle Semiconductor as an unpublished work.
 * All Rights Reserved.
 *
 *  The information contained herein is confidential
 * property of Company. The user, copying, transfer or
 * disclosure of such information is prohibited except
 * by express written agreement with Company.
 *
 * First written on 18/10/2010 by Toni Homedes
 *
 ***************************************************/
/** \file gigle_version.h
 *
 * \brief This contains the version number and description
 *
 * Version 0.0.1 - 20/10/2010
 *       First compilable version
 *
 * $Id: gigle_version.h 246063 2011-03-12 06:39:32Z rnuti $
 ****************************************************/

/* FILE-CSTYLED */

#ifndef VERSION_H_
#define VERSION_H_

/***************************************************
 *                 Public Defines Section
 ***************************************************/

/** \brief Version branch */
#define VERSION_BRANCH  (6) // WIFI

/** \brief Version major */
#define VERSION_MAJOR   (2)
 
/** \brief Version revison */
#define VERSION_REVISION (0)

/** \brief Version minor */
#define VERSION_MINOR   (9)

/*************** HW version defines ******************/
// only the lower 16 bits can be used to identify hw version

//Gigle-WiFi
#define HWVER_RELEASE_MII_GGL541AC 0x80000701
#define HWVER_RELEASE_WBROADCOM_A2XMIIHPAV 0x80000703

/************** Release defines ***********************/

#ifdef RELEASE
  /* */
#endif

#define RAW_SOCKET_INPUT_IF            "vlan7"
#define RAW_SOCKET_OUTPUT_IF           "vlan7"


//#define PRECONFIGURE_NVRAM_ON_GIGLED

#define FW_VERSION (VERSION_BRANCH << 24 | VERSION_MAJOR << 16 | VERSION_REVISION << 8 | VERSION_MINOR)

#else /*VERSION_H_*/
#error "Header file __FILE__ has already been included!"
#endif /*VERSION_H_*/


/*@}*/
