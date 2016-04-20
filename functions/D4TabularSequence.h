// -*- mode: c++; c-basic-offset:4 -*-

// This file is part of libdap, A C++ implementation of the OPeNDAP Data
// Access Protocol.

// Copyright (c) 2002,2003 OPeNDAP, Inc.
// Author: James Gallagher <jgallagher@opendap.org>
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
//
// You can contact OPeNDAP, Inc. at PO Box 112, Saunderstown, RI. 02874-0112.

// (c) COPYRIGHT URI/MIT 1994-1999
// Please read the full copyright statement in the file COPYRIGHT_URI.
//
// Authors:
//      jhrg,jimg       James Gallagher <jgallagher@gso.uri.edu>

// Interface for the class Sequence. A sequence contains a single set
// of variables, all at the same lexical level just like a structure
// (and like a structure, it may contain other ctor types...). Unlike
// a structure, a sequence defines a pattern that is repeated N times
// for a sequence of N elements. Thus, Sequence { String name; Int32
// age; } person; means a sequence of N persons where each contain a
// name and age. The sequence can be arbitrarily long (i.e., you don't
// know N by looking at the sequence declaration.
//
// jhrg 9/14/94

#ifndef _d4_tabular_sequence_h
#define _d4_tabular_sequence_h 1

#include <D4Sequence.h>

namespace libdap {
class ConstraintEvaluator;
class DMR;
class Marshaller;
}

namespace functions {

/** @brief Specialization of Sequence for tables of data
 *
 * Assumes that the data are loaded into the Sequence using set_value()
 */
class D4TabularSequence: public libdap::D4Sequence
{
private:
protected:
    void load_prototypes_with_values(libdap::BaseTypeRow &btr, bool safe = true);

public:
    /** The Sequence constructor requires only the name of the variable
        to be created.  The name may be omitted, which will create a
        nameless variable.  This may be adequate for some applications.

        @param n A string containing the name of the variable to be
        created.

        @brief The Sequence constructor. */
    D4TabularSequence(const string &n) : D4Sequence(n) { }

    /** The Sequence server-side constructor requires the name of the variable
        to be created and the dataset name from which this variable is being
        created.

        @param n A string containing the name of the variable to be
        created.
        @param d A string containing the name of the dataset from which this
        variable is being created.

        @brief The Sequence server-side constructor. */
    D4TabularSequence(const string &n, const string &d) : D4Sequence(n, d) { }

    /** @brief The Sequence copy constructor. */
    D4TabularSequence(const D4TabularSequence &rhs) : D4Sequence(rhs) { }

    virtual ~D4TabularSequence() { }

    virtual BaseType *ptr_duplicate() { return new TabularSequence(*this); }

    D4TabularSequence &operator=(const D4TabularSequence &rhs) {
        if (this == &rhs)
            return *this;

        static_cast<D4Sequence &>(*this) = rhs; // run D4Sequence=

        return *this;
    }

    virtual bool serialize(libdap::ConstraintEvaluator &eval, libdap::DMR &dmr, libdap::Marshaller &m, bool ce_eval = true);
    virtual void intern_data(libdap::ConstraintEvaluator &eval, libdap::DMR &dmr);

    virtual void dump(ostream &strm) const;
};

} // namespace functions

#endif //_d4_tabular_sequence_h
