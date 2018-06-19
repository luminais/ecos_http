/*
 * L2TP include file.
 *
 * Copyright (C) 2010, Broadcom Corporation
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 *
 * $Id: l2tp_proto.h,v 1.1 2010-07-02 07:55:45 Exp $
 */

#ifndef __L2TP_PROTO_H__
#define __L2TP_PROTO_H__

/* Bit definitions */
#define TYPE_BIT                        0x80
#define LENGTH_BIT                      0x40
#define SEQUENCE_BIT                    0x08
#define OFFSET_BIT                      0x02
#define PRIORITY_BIT                    0x01
#define RESERVED_BITS                   0x34
#define VERSION_MASK                    0x0F
#define VERSION_RESERVED                0xF0

#define AVP_MANDATORY_BIT               0x80
#define AVP_HIDDEN_BIT                  0x40
#define AVP_RESERVED_BITS               0x3C

#define	L2TP_PORT			1701

#endif	/* __L2TP_H__ */
