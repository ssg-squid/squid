
/*
 * $Id: Generic.h,v 1.6 2003/09/22 08:50:51 robertc Exp $
 *
 *
 * SQUID Web Proxy Cache          http://www.squid-cache.org/
 * ----------------------------------------------------------
 *
 *  Squid is the result of efforts by numerous individuals from
 *  the Internet community; see the CONTRIBUTORS file for full
 *  details.   Many organizations have provided support for Squid's
 *  development; see the SPONSORS file for full details.  Squid is
 *  Copyrighted (C) 2001 by the Regents of the University of
 *  California; see the COPYRIGHT file for full details.  Squid
 *  incorporates software developed and/or copyrighted by other
 *  sources; see the CREDITS file for full details.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *  
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *  
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111, USA.
 *
 */

#ifndef SQUID_GENERIC_H
#define SQUID_GENERIC_H

template <class _Arg, class _Result>

struct unary_function
{
    typedef _Arg argument_type;
    typedef _Result result_type;
};

template <class L, class T>
T& for_each(L const &head, T& visitor)
{
    for (L const *node = &head; node; node=node->next)
        visitor(*node);

    return visitor;
}

template <class T>
T& for_each(dlink_list const &collection, T& visitor)
{
    for (dlink_node const *node = collection.head; node; node=node->next)
        visitor(*(typename T::argument_type const *)node->data);

    return visitor;
}

template <class S>

class Stack;

template <class E, class T>
T& for_each(Stack<E> const &collection, T& visitor)
{
    for (size_t index = 0; index < collection.count; ++index)
        visitor(*(typename T::argument_type const *)collection.items[index]);

    return visitor;
};

template <class InputIterator , class Visitor>
Visitor& for_each(InputIterator from, InputIterator to, Visitor& visitor)
{
    while (!(from == to)) {
        typename InputIterator::value_type &value = *from;
        ++from;
        visitor(value);
    }

    return visitor;
}

/* generic ostream printer */
template <class Pointer>

struct PointerPrinter
{
    PointerPrinter(std::ostream &astream, std::string aDelimiter) : os(astream), delimiter (aDelimiter) {}

    void operator () (Pointer aNode)
    {
        os << *aNode << delimiter;
    }

    std::ostream &os;
    std::string delimiter;
};

#endif /* SQUID_GENERIC_H */
