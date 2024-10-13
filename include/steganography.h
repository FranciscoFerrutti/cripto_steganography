#ifndef EMBED_H
#define EMBED_H

#include "bitmap.h"
#include "common_libs.h"
#include "embedding.h"
#include "parse_args.h"

void embed(const char *carrierFile,
           const char *messageFile,
           const char *outputFile,
           steg        method,
           encryption  a,
           mode        m,
           const char *pass);
void extract(const char *carrierFile,
             const char *outputFile,
             steg        method,
             encryption  a,
             mode        m,
             const char *pass);

#endif