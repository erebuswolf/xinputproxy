/* Copyright 2021 Mircea-Dacian Munteanu
 *
 * The Source Code is this file is released under the terms of the New BSD License,
 * see LICENSE file, or the project homepage: https://github.com/mircead52/xinputproxy
 */

#ifndef CONFIG_H
#define CONFIG_H

#include <map>
#include <set>
#include <string>

struct proxyconfig;

extern struct proxyconfig configx;

struct proxyconfig
{
	const char * dllpath;
	float crop_left_stick_value;
	uint32_t crop_left_stick_override;
};


void read_conf(TCHAR* module_name);

#endif