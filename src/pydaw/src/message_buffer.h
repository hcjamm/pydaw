/*
This file is part of the PyDAW project, Copyright PyDAW Team

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; version 3 of the License.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
*/

#ifndef MESSAGE_BUFFER_H
#define MESSAGE_BUFFER_H

#define MB_MESSAGE(fmt...) { \
	char _m[256]; \
	snprintf(_m, 255, fmt); \
	add_message(_m); \
}

void mb_init(const char *prefix);

void add_message(const char *msg);

#endif
