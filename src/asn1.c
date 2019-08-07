/* asn1.c - decoding ASN.1/DER
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


#include <certalize.h>
#include <certalize_asn1.h>
#include <certalize_debug.h>

/* globals    */

/* prototypes */



/*************/


int asn1_parse_hdr(cbuf_t *cbuf, asn1_hdr_t *asn1)
{
   int offset = cbuf->offset;
   guchar buf, *ptr, tmp;

   memset(asn1, 0, sizeof(asn1_hdr_t));

   ptr = &buf;

   /* get 1st byte of ASN.1 header */
   cbuf_get_bytes(cbuf, &ptr, offset++, 1);

   asn1->class = buf >> 6; // first MSB's of ID are the CLASS
   asn1->constructed = (buf & (1<<5)); // constructed if 6th bit is set

   if ((buf & 0x1f) == 0x1f) {
      // High Tag if all 5 LSB's are set
      asn1->tag = 0;

      cbuf_get_bytes(cbuf, &ptr, offset++, 1);

      do {
         // append the 7 LSB's to the tag value
         // TODO prevent against integer overflow
         asn1->tag = (asn1->tag << 7) | (buf & 0x7f);

         tmp = buf;
         if (buf & 0x80) { // still one more byte for the tag left
            cbuf_get_bytes(cbuf, &ptr, offset++, 1);
         }

      } while (tmp & 0x80);
   }
   else {
      // tag is just the 5 LSB's of the ID
      asn1->tag = buf & 0x1f;
   }
   

   /* 
    * parsing tag byte(s) complete, 
    * now proceed with the length byte(s)
    */
   cbuf_get_bytes(cbuf, &ptr, offset++, 1);
   asn1->length_byte = buf;
   
   if (asn1->length_byte & 0x80) {
      tmp = asn1->length_byte & 0x7f;
      while (tmp--) {
         cbuf_get_bytes(cbuf, &ptr, offset++, 1);
         // TODO avoid integer overflow
         asn1->length = (asn1->length << 8) | buf;
      }
   }
   else {
      asn1->length = asn1->length_byte & 0x7f;
   }


   /*
    * offset is now the amount of bytes processed for the ASN.1 header
    */
   return offset;

}

/* EOF */

// vim:ts=3:expandtab
