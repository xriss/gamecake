/** \file
 * \brief BASE64 encoding
 *
 */

#ifndef __BASE64_H
#define __BASE64_H

#ifdef __cplusplus
extern "C" {
#endif

int base64_encode(unsigned char *source, int sourcelen, char *target, int targetlen);
int base64_decode(char *source, unsigned char *target, int targetlen);

#ifdef __cplusplus
}
#endif

#endif /* ifndef __BASE64_ */
