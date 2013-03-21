/*
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
 *
 * Copyright (c) 2003, Robert Collins <robertc@squid-cache.org>
 */

#ifndef SQUID_ACL_H
#define SQUID_ACL_H

#include "Array.h"
#include "cbdata.h"
#include "defines.h"
#include "dlink.h"
#include "MemPool.h"

#if HAVE_OSTREAM
#include <ostream>
#endif
#if HAVE_STRING
#include <string>
#endif

class ConfigParser;
class ACLChecklist;
class ACLList;

typedef char ACLFlag;
// ACLData Flags
#define ACL_F_REGEX_CASE 'i'
#define ACL_F_NO_LOOKUP 'n'
#define ACL_F_END '\0'

/**
 * \ingroup ACLAPI
 * Used to hold a list of one-letter flags which can be passed as parameters
 * to acls  (eg '-i', '-n' etc)
 */
class ACLFlags
{
public:
    explicit ACLFlags(const ACLFlag flags[]) : supported_(flags), flags_(0) {}
    ACLFlags() : flags_(0) {}
    bool supported(const ACLFlag f) const; ///< True if the given flag supported
    void makeSet(const ACLFlag f) { flags_ |= flagToInt(f); } ///< Set the given flag
    /// Return true if the given flag is set
    bool isSet(const ACLFlag f) const { return flags_ & flagToInt(f);}
    /// Parse a flags given in the form -[A..Z|a..z]
    void parseFlags(char * &nextToken);
    const char *flagsStr() const; ///< Convert the flags to a string representation

private:
    /// Convert a flag to a 64bit unsigned integer.
    /// The characters from 'A' to 'z' represented by the values from 65 to 122.
    /// They are 57 different characters which can be fit to the bits of an 64bit
    /// integer.
    uint64_t flagToInt(const ACLFlag f) const {
        assert('A' <= f && f <= 'z');
        return ((uint64_t)1 << (f - 'A'));
    }

    std::string supported_; ///< The supported character flags
    uint64_t flags_; ///< The flags which is set
public:
    static const ACLFlag NoFlags[1]; ///< An empty flags list
};

/// \ingroup ACLAPI
class ACL
{

public:
    void *operator new(size_t);
    void operator delete(void *);

    static ACL *Factory (char const *);
    static void ParseAclLine(ConfigParser &parser, ACL ** head);
    static void Initialize();
    static ACL* FindByName(const char *name);

    ACL();
    explicit ACL(const ACLFlag flgs[]) : cfgline(NULL), next(NULL), flags(flgs) { memset(name, '\0', sizeof(name)); }
    virtual ~ACL();
    virtual ACL *clone()const = 0;
    virtual void parse() = 0;
    virtual char const *typeString() const = 0;
    virtual bool isProxyAuth() const;
    virtual bool requiresRequest() const;
    virtual bool requiresReply() const;
    virtual int match(ACLChecklist * checklist) = 0;
    virtual wordlist *dump() const = 0;
    virtual bool empty () const = 0;
    virtual bool valid () const;
    int checklistMatches(ACLChecklist *);

    int cacheMatchAcl(dlink_list * cache, ACLChecklist *);
    virtual int matchForCache(ACLChecklist *checklist);

    virtual void prepareForUse() {}

    char name[ACL_NAME_SZ];
    char *cfgline;
    ACL *next;
    ACLFlags flags; ///< The list of given ACL flags

public:

    class Prototype
    {

    public:
        Prototype ();
        Prototype (ACL const *, char const *);
        ~Prototype();
        static bool Registered(char const *);
        static ACL *Factory (char const *);

    private:
        ACL const*prototype;
        char const *typeString;

    private:
        static Vector<Prototype const *> * Registry;
        static void *Initialized;
        typedef Vector<Prototype const*>::iterator iterator;
        typedef Vector<Prototype const*>::const_iterator const_iterator;
        void registerMe();
    };
};

/// \ingroup ACLAPI
typedef enum {
    // Authorization ACL result states
    ACCESS_DENIED,
    ACCESS_ALLOWED,
    ACCESS_DUNNO,

    // Authentication ACL result states
    ACCESS_AUTH_REQUIRED,    // Missing Credentials
} aclMatchCode;

/// \ingroup ACLAPI
/// ACL check answer; TODO: Rename to Acl::Answer
class allow_t
{
public:
    // not explicit: allow "aclMatchCode to allow_t" conversions (for now)
    allow_t(const aclMatchCode aCode): code(aCode), kind(0) {}

    allow_t(): code(ACCESS_DUNNO), kind(0) {}

    bool operator ==(const aclMatchCode aCode) const {
        return code == aCode;
    }

    bool operator !=(const aclMatchCode aCode) const {
        return !(*this == aCode);
    }

    operator aclMatchCode() const {
        return code;
    }

    aclMatchCode code; ///< ACCESS_* code
    int kind; ///< which custom access list verb matched
};

inline std::ostream &
operator <<(std::ostream &o, const allow_t a)
{
    switch (a) {
    case ACCESS_DENIED:
        o << "DENIED";
        break;
    case ACCESS_ALLOWED:
        o << "ALLOWED";
        break;
    case ACCESS_DUNNO:
        o << "DUNNO";
        break;
    case ACCESS_AUTH_REQUIRED:
        o << "AUTH_REQUIRED";
        break;
    }
    return o;
}

/// \ingroup ACLAPI
class acl_access
{

public:
    void *operator new(size_t);
    void operator delete(void *);
    allow_t allow;
    ACLList *aclList;
    char *cfgline;
    acl_access *next;

private:
    CBDATA_CLASS(acl_access);
};

/// \ingroup ACLAPI
class ACLList
{

public:
    MEMPROXY_CLASS(ACLList);

    ACLList();
    void negated(bool isNegated);
    bool matches (ACLChecklist *)const;
    int op;
    ACL *_acl;
    ACLList *next;
};

MEMPROXY_CLASS_INLINE(ACLList);

/// \ingroup ACLAPI
class acl_proxy_auth_match_cache
{

public:
    MEMPROXY_CLASS(acl_proxy_auth_match_cache);
    dlink_node link;
    int matchrv;
    void *acl_data;
};

MEMPROXY_CLASS_INLINE(acl_proxy_auth_match_cache);

/// \ingroup ACLAPI
extern const char *AclMatchedName;	/* NULL */

#endif /* SQUID_ACL_H */
