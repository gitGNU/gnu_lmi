# PETE with std::vector: makefile.
#
# Copyright (C) 2008 Gregory W. Chicares.
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License version 2 as
# published by the Free Software Foundation.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software Foundation,
# Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA
#
# http://savannah.nongnu.org/projects/lmi
# email: <gchicares@sbcglobal.net>
# snail: Chicares, 186 Belle Woods Drive, Glastonbury CT 06033, USA

################################################################################

uname := $(shell uname -s 2>/dev/null)

EXEEXT :=
ifeq (CYGWIN,$(findstring CYGWIN,$(uname)))
  EXEEXT := .exe
endif

all: et_vector_test$(EXEEXT)

submake_options := \
  --directory=PETE/Tools/ \
  --file=makefile \
  EXEEXT='$(EXEEXT)' \

et_vector_test$(EXEEXT): et_vector_test.cpp
	$(MAKE) $(submake_options) clean
	$(MAKE) $(submake_options) newpeteops
	$(MAKE) $(submake_options) installheaders
	$(CXX) -I. $^ -o $@

et_vector_test.cpp: et_vector_operators.hpp

et_vector_operators.hpp: et_vector.in
	PETE/Tools/MakeOperators \
	  --guard et_vector_operators_hpp \
	  --classes et_vector.in \
	  --assign-ops \
	  --o $@

clean:
	rm -f et_vector_test$(EXEEXT)
	rm -f $(wildcard *~ *.bak *eraseme*)

maintainer-clean: clean
	rm -f et_vector_operators.hpp

