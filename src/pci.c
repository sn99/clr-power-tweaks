/*
 *    Clear Linux Project for Intel Architecture Power tweaks
 *
 *      Copyright (C) 2012 Intel Corporation
 *
 *   This program is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, version 3 of the License.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 *   Authors:
 *         Arjan van de Ven <arjan@linux.intel.com>
 *
 */

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>

#include <clr_power.h>

#include <sys/types.h>
#include <dirent.h>

#include "pci-good.h"

void do_pci_pm(void)
{
	DIR *dir;
	struct dirent *entry;

	dir = opendir("/sys/bus/pci/devices/");
	if (!dir)
		return;

	do {
		FILE *file;
		char *filename;
		char *line;
		size_t size;
		int vendor, device;
		int i;

		entry = readdir(dir);
		if (!entry)
			break;
		if (strcmp(entry->d_name, ".") == 0)
			continue;
		if (strcmp(entry->d_name, "..") == 0)
			continue;

		if (asprintf(&filename, "/sys/bus/pci/devices/%s/vendor", entry->d_name) < 0)
			assert(0);

		file = fopen(filename, "r");
		if (!file) {
			free(filename);
			continue;
		}
		line = NULL;
		size = 0;
		if (getline(&line, &size, file) < 0)
			assert(0);
		vendor = strtoull(line, NULL, 16);
		free(line);
		free(filename);
		fclose(file);

		if (asprintf(&filename, "/sys/bus/pci/devices/%s/device", entry->d_name) < 0)
			assert(0);

		file = fopen(filename, "r");
		if (!file) {
			free(filename);
			continue;
		}
		line = NULL;
		size = 0;
		if (getline(&line, &size, file) < 0)
			assert(0);
		device = strtoull(line, NULL, 16);
		free(line);
		free(filename);
		fclose(file);

//		printf("Found PCI device %04x:%04x\n", vendor, device);
		i = 0;
		while (1) {
			if (known_good[i].vendor == 0xffff)
				break;
			if ( (known_good[i].vendor == vendor && known_good[i].device == device)) {
				if (asprintf(&filename, "/sys/bus/pci/devices/%s/power/control", entry->d_name) < 0)
					assert(0);
				write_string_to_file(filename, "auto");
				free(filename);
				break;
			}
			i++;
		}



	} while (1);

	closedir(dir);
}
