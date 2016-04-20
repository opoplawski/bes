// -*- mode: c++; c-basic-offset:4 -*-

// This file is part of libdap, A C++ implementation of the OPeNDAP Data
// Access Protocol.

// Copyright (c) 2015 OPeNDAP, Inc.
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

#include "config.h"

#include <algorithm>
#include <string>
#include <sstream>

//#define DODS_DEBUG

#include <BaseType.h>
#include <Byte.h>
#include <Int16.h>
#include <Int32.h>
#include <UInt16.h>
#include <UInt32.h>
#include <Float32.h>
#include <Float64.h>
#include <Str.h>
#include <Url.h>

#include <DDS.h>
#include <ConstraintEvaluator.h>
#include <Marshaller.h>
#include <UnMarshaller.h>
#include <D4StreamMarshaller.h>
#include <debug.h>

#include "D4TabularSequence.h"

using namespace std;
using namespace libdap;

namespace functions {

void D4TabularSequence::load_prototypes_with_values(D4SeqRow &btr, bool safe)
{
    // For each of the prototype variables in the Sequence, load it
    // with a values from the BaseType* vector. The order should match.
    // Test the type, but assume if that matches, the value is correct
    // for the variable.
    Vars_iter i = d_vars.begin(), e = d_vars.end();
    for (D4SeqRow::iterator vi = btr.begin(), ve = btr.end(); vi != ve; ++vi) {

        if (safe && (i == e || ((*i)->type() != (*vi)->var()->type())))
            throw InternalErr(__FILE__, __LINE__, "Expected number and types to match when loading values for selection expression evaluation.");

        // Ugly... but faster than the generic code that allocates storage for each scalar?
        switch ((*i)->type()) {
        case dods_byte_c:
            static_cast<Byte*>(*i++)->set_value(static_cast<Byte*>(*vi)->value());
            break;
        case dods_int16_c:
            static_cast<Int16*>(*i++)->set_value(static_cast<Int16*>((*vi))->value());
            break;
        case dods_int32_c:
            static_cast<Int32*>(*i++)->set_value(static_cast<Int32*>((*vi))->value());
            break;
        case dods_uint16_c:
            static_cast<UInt16*>(*i++)->set_value(static_cast<UInt16*>((*vi))->value());
            break;
        case dods_uint32_c:
            static_cast<UInt32*>(*i++)->set_value(static_cast<UInt32*>((*vi))->value());
            break;
        case dods_float32_c:
            static_cast<Float32*>(*i++)->set_value(static_cast<Float32*>((*vi))->value());
            break;
        case dods_float64_c:
            static_cast<Float64*>(*i++)->set_value(static_cast<Float64*>((*vi))->value());
            break;
        case dods_str_c:
            static_cast<Str*>(*i++)->set_value(static_cast<Str*>((*vi))->value());
            break;
        case dods_url_c:
            static_cast<Url*>(*i++)->set_value(static_cast<Url*>((*vi))->value());
            break;
        default:
            throw InternalErr(__FILE__, __LINE__, "Expected a scalar type when loading values for selection expression evaluation.");
        }
    }
}

// Public member functions

/**
 * Specialized version of D4TabularSequenceSequence::serialize() for tables that already
 * hold their data. This will not work for nested Sequences.
 *
 * @note This version ignores CE filters. It also assumes that the requester
 * (the caller of the tabular() function) did not include arrays that were
 * subsequently not 'projected.' Improve this code by improving the way
 * D4TabularSequence stores values from the Arrays. One way would be to use
 * local storage for the values and use a list, not a vector. Then the code
 * can iterate over the list, removing entries that fail the filter, and
 * serialize the result. We still have to have the entire thing in memory,
 * but only one copy instead of two.
 *
 *
 * @note The ce_eval parameter was being set to false in BESDapResponseBuilder
 * when the code was processing a response from a function. I changed that to
 * 'true' in that code to avoid the special case here. Our tests for the functions
 * are pretty thin at this point, however, so the change should be reviewed
 * when those tests are improved.
 *
 * @param eval
 * @param dds
 * @param m
 * @param ce_eval
 * @return
 */
void D4TabularSequence::serialize(D4StreamMarshaller &m, DMR &dmr, bool /* filter; false */)
{
    D4SeqValues values = value();   // TODO Replace with value_ref(). jhrg 4/20/16

    // write D4TabularSequence::length(); don't include the length in the checksum
    m.put_count(values.size());

    // By this point the d_values object holds all and only the values to be sent;
    // use the serialize methods to send them (but no need to test send_p).
    for (D4SeqValues::iterator i = values.begin(), e = values.end(); i != e; ++i) {
        for (D4SeqRow::iterator j = (*i)->begin(), f = (*i)->end(); j != f; ++j) {
            if ((*j)->send_p())
                (*j)->serialize(m, dmr, false /* filter */);
        }
    }
}

/** @brief dumps information about this object
 * @param strm C++ i/o stream to dump the information to
 * @return void
 */
void
D4TabularSequence::dump(ostream &strm) const
{
    strm << DapIndent::LMarg << "TabularSequence::dump - (" << (void *)this << ")" << endl ;
    DapIndent::Indent() ;
    D4Sequence::dump(strm) ;
    DapIndent::UnIndent() ;
}

} // namespace functions

