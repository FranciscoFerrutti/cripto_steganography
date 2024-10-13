#ifndef EMBED_H
#define EMBED_H

#include "bitmap.h"
#include "common_libs.h"
#include "parse_args.h"
#include "steg_utils.h"

void embed(const char *carrierFile, const char *messageFile, const char *outputFile, steg method);
void extract(const char *carrierFile, const char *outputFile, steg method);

#endif