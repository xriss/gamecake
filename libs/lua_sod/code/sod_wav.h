/*
-- Copyright (C) 2012 Kriss Blank < Kriss@XIXs.com >
-- This file is distributed under the terms of the MIT license.
-- http://en.wikipedia.org/wiki/MIT_License
-- Please ping me if you use it for anything cool...
*/


sod * sod_wav_load_file(sod *sd, const char* file_name);
sod * sod_wav_load_data(sod *sd, const unsigned char* data,int len);

sod * sod_wav_save_file(sod *sd , const char* file_name);
