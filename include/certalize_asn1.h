/* certalize_asn1.h
 *
 * Copyright (C) 2019 Alexander Koeppe
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef CERTALIZE_ASN1_H
#define CERTALIZE_ASN1_H

#include <certalize.h>
#include <certalize_buf.h>

#define ASN1_TAG_EOC                0x00
#define ASN1_TAG_BOOLEAN            0x01
#define ASN1_TAG_INTEGER            0x02
#define ASN1_TAG_BITSTRING          0x03
#define ASN1_TAG_OCTETSTRING        0x04
#define ASN1_TAG_NULL               0x05
#define ASN1_TAG_OID                0x06
#define ASN1_TAG_OBJECT_DESCRIPTOR  0x07
#define ASN1_TAG_EXTERNAL           0x08
#define ASN1_TAG_REAL               0x09
#define ASN1_TAG_ENUMERATED         0x0A
#define ASN1_TAG_UTF8STRING         0x0C
#define ASN1_TAG_RELATIVE_OID       0x0D
#define ASN1_TAG_SEQUENCE           0x10
#define ASN1_TAG_SET                0x11
#define ASN1_TAG_NUMERIC_STRING     0x12
#define ASN1_TAG_PRINTABLE_STRING   0x13
#define ASN1_TAG_TG1_STRING         0x14
#define ASN1_TAG_VIDEO_STRING       0x15
#define ASN1_TAG_IA5_STRING         0x16
#define ASN1_TAG_UTC_TIME           0x17
#define ASN1_TAG_GERNERALIZED_TIME  0x18
#define ASN1_TAG_GRAPHIC_STRING     0x19
#define ASN1_TAG_VISIBLE_STRING     0x1A
#define ASN1_TAG_GENERAL_STRING     0x1B
#define ASN1_TAG_UNIVERSAL_STRING   0x1C
#define ASN1_TAG_BMP_STRING         0x1D

#define ASN1_CLASS_UNIVERSAL        0
#define ASN1_CLASS_APPLICATION      1
#define ASN1_CLASS_CONTEXT_SPECIFIC 2
#define ASN1_CLASS_PRIVATE          3

typedef struct asn1_hdr {
   guint8 class;
   guint8 constructed;
   guint32 tag;
   guint8 length_byte;
   guint32 length;
} asn1_hdr_t;

#define ASN1_MAX_OID_LEN            20
typedef struct asn1_oid {
   guint64 oid[ASN1_MAX_OID_LEN];
   size_t len;
} asn1_oid_t;

extern int asn1_parse_hdr(cbuf_t *cbuf, asn1_hdr_t *hdr);


#endif   /* CERTALIZE_ASN1_H */

/* EOF */

// vim:ts=3:expandtab
