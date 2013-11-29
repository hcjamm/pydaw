#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
This file is part of the PyDAW project, Copyright PyDAW Team

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; version 3 of the License.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
"""

import os

rpm_dir = "%s/rpmbuild/RPMS/x86_64/" % (os.path.expanduser("~"))

os.system('cp %s/pydaw*rpm ..' % (rpm_dir,))
