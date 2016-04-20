// -*- mode: c++; c-basic-offset:4 -*-

// This file is part of libdap, A C++ implementation of the OPeNDAP Data
// Access Protocol.

// Copyright (c) 2016 OPeNDAP, Inc.
// Author: Nathan David Potter <ndp@opendap.org>
//         James Gallagher <jgallagher@opendap.org>
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

#ifndef _d4_tabular_sequence_h
#define _d4_tabular_sequence_h 1

#include <D4Sequence.h>

namespace libdap {
class DMR;
class D4StreamMarshaller;
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

    virtual BaseType *ptr_duplicate() { return new D4TabularSequence(*this); }

    D4TabularSequence &operator=(const D4TabularSequence &rhs) {
        if (this == &rhs)
            return *this;

        static_cast<D4Sequence &>(*this) = rhs; // run D4Sequence=

        return *this;
    }

    // DAP4 serialization methods; This class is a server-side only class,
    // so no deserialize() method is defined.
    virtual void intern_data();
    virtual void serialize(D4StreamMarshaller &m, DMR &dmr, bool filter = false);

    virtual void dump(ostream &strm) const;
};

} // namespace functions

#endif //_d4_tabular_sequence_h
