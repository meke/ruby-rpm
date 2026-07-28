/* -*- mode: C; c-basic-offset: 4; tab-width: 4; -*- */
/* Ruby/RPM
 *
 * Copyright (C) 2002 Kenta MURATA <muraken2@nifty.com>.
 */

/* $Id: private.h 17 2004-03-19 05:12:19Z zaki $ */

#include <extconf.h>

#define RPM_VERSION(maj,min,pl) (((maj) << 16) + ((min) << 8) + (pl))

#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
/* Looks missing in rpmds.h */
#include <sys/utsname.h>
#include <unistd.h>

#if RPM_VERSION(5,0,0) <= RPM_VERSION_CODE
#include <rpm4compat.h>
#endif

/* To get structure definitions */ 
#define _RPMDB_INTERNAL 
#define _RPMPS_INTERNAL

#include <rpm/rpmcli.h>
#if HAVE_RPM_RPMLIB_H
#  include <rpm/rpmlib.h>
#endif
#include <rpm/rpmdb.h>
#include <rpm/rpmbuild.h>
#if HAVE_RPM_RPMLOG_H
#  include <rpm/rpmlog.h>
#elif HAVE_RPMMESSAGES_H
#  include <rpm/rpmmessages.h>
#endif
#if HAVE_RPM_RPMTS_H
#  include <rpm/rpmts.h>
#endif
#if HAVE_RPM_RPMPS_H
#  include <rpm/rpmps.h>
#endif
#if HAVE_RPM_RPMDS_H
#  include <rpm/rpmds.h>
#endif
#if HAVE_RPM_RPMSPEC_H
#  include <rpm/rpmspec.h>
#endif

#include "ruby-rpm.h"

#if RPM_VERSION_CODE > RPM_VERSION(4,9,0) || RPM_VERSION_CODE > RPM_VERSION(5,0,0)

/* compat header */

/* from rpmbuild_internal.h */
typedef struct Package_s * Package;

/* from rpmbuild_misc.h */
typedef struct StringBufRec *StringBuf;

/* from rpmbuild_internal.h */

/** \ingroup rpmbuild
 * The structure used to store values parsed from a spec file.
 */
struct rpmSpec_s {
    char * specFile;    /*!< Name of the spec file. */
    char * buildRoot;
    char * buildSubdir;
    const char * rootDir;

    struct OpenFileInfo * fileStack;
    char lbuf[10*BUFSIZ];
    char *lbufPtr;
    char nextpeekc;
    char * nextline;
    char * line;
    int lineNum;

    struct ReadLevelEntry * readStack;

    Header buildRestrictions;
    rpmSpec * BASpecs;
    const char ** BANames;
    int BACount;
    int recursing;              /*!< parse is recursive? */

    rpmSpecFlags flags;

    struct Source * sources;
    int numSources;
    int noSource;

    char * sourceRpmName;
    unsigned char * sourcePkgId;
    Header sourceHeader;
    rpmfi sourceCpioList;

    rpmMacroContext macros;

    StringBuf prep;             /*!< %prep scriptlet. */
    StringBuf build;            /*!< %build scriptlet. */
    StringBuf install;          /*!< %install scriptlet. */
    StringBuf check;            /*!< %check scriptlet. */
    StringBuf clean;            /*!< %clean scriptlet. */

    StringBuf parsed;           /*!< parsed spec contents */

    Package packages;           /*!< Package list. */
};
#endif

#define RPM_DB(v) (((rpm_db_t*)DATA_PTR((v)))->db)
#ifdef PKG_CACHE_TEST
#define RPM_HEADER(v) rpm_package_get_header(v)
#else
#define RPM_HEADER(v) ((Header)DATA_PTR((v)))
#endif
#define RPM_MI(v) (((rpm_mi_t*)DATA_PTR((v)))->mi)
#if RPM_VERSION_CODE < RPM_VERSION(4,1,0)
#define RPM_SPEC(v) ((Spec)DATA_PTR((v)))
#elif RPM_VERSION_CODE < RPM_VERSION(4,9,0) || RPM_VERSION_CODE > RPM_VERSION(5,0,0)
#define RPM_SPEC(v) rpmtsSpec((rpmts)DATA_PTR((v)))
#else
#define RPM_SPEC(v) ((rpmSpec)DATA_PTR((v)))
#endif
#define RPM_TRANSACTION(v) (((rpm_trans_t*)DATA_PTR((v)))->ts)
#define RPM_SCRIPT_FD(v) (((rpm_trans_t*)DATA_PTR((v)))->script_fd)

#if RPM_VERSION_CODE >= RPM_VERSION(4,5,90)
#if RPM_VERSION_CODE < RPM_VERSION(5,0,0)
#define RPMDB_OPAQUE 1
#define RPMPS_OPAQUE 1
#else
#define RPMDB_OPAQUE 1
/* #undef RPMPS_OPAQUE */
#endif
#else
#define RPMTS_AVAILABLE 1
#endif

#if RPM_VERSION_CODE >= RPM_VERSION(4,9,0) && RPM_VERSION_CODE < RPM_VERSION(5,0,0)
/*
 * RPM 4.9 made rpmdb opaque and dropped rpmdbOpen()/rpmdbClose() from the
 * public API, so for this range database access goes through an rpmts
 * that RPM::DB owns for the lifetime of the object.
 */
typedef struct {
	rpmts ts;
	int ref_count;
} rpm_db_t;
#else
typedef struct {
	rpmdb db;
	int ref_count;
} rpm_db_t;
#endif

typedef struct {
#if RPM_VERSION_CODE < RPM_VERSION(4,1,0)
	rpmTransactionSet ts;
#else
	rpmts ts;
#endif
	FD_t script_fd;
	rpm_db_t* db;
} rpm_trans_t;

typedef struct {
	rpmdbMatchIterator mi;
	rpm_db_t* db;
} rpm_mi_t;

/* db.c */
void Init_rpm_DB(void);
void Init_rpm_transaction(void);
void Init_rpm_MatchIterator(void);

/* dependency.c */
void Init_rpm_dependency(void);

/* file.c */
void Init_rpm_file(void);

/* package.c */
#ifdef PKG_CACHE_TEST
Header rpm_package_get_header(VALUE pkg);
#endif
void Init_rpm_package(void);

/* rpm.c */
VALUE ruby_rpm_make_temp_name(void);

/* source.c */
void Init_rpm_source(void);

/* spec.c */
void Init_rpm_spec(void);

/* version.c */
void Init_rpm_version(void);

#if RPM_VERSION_CODE < RPM_VERSION(4,6,0)
inline static void
get_entry(Header hdr, rpmTag tag, rpmTagType* type, void** ptr)
{
	if (!headerGetEntryMinMemory(
			hdr, tag, (hTYP_t)type, (hPTR_t*)ptr, NULL)) {
		*ptr = NULL;
	}
}
#elif RPM_VERSION_CODE < RPM_VERSION(5,0,0)
inline static void
get_entry(Header hdr, rpmTag tag, rpmtd tc)
{
	headerGet(hdr, tag, tc, HEADERGET_MINMEM);
}
#else /* rpm4compat.h */
inline static void
get_entry(Header hdr, rpmTag tag, rpmTagType* type, void** ptr)
{
	if (!headerGetEntry(
			hdr, (int_32)tag, (hTYP_t)type, ptr, NULL)) {
		*ptr = NULL;
	}
}
#endif

inline static void
release_entry(rpmTagType type, void* ptr)
{
	rpmtdFreeData(ptr); //?
}

/* headerAddEntry(), headerAddOrAppendEntry(), headerRemoveEntry() and
 * headerSprintf() were part of the RPM 4.4.x compatibility layer, which
 * only exists for RPM_VERSION_CODE < RPM_VERSION(4,6,0) or the RPM 5.x
 * compat header (rpm4compat.h). For 4.6.0 <= RPM_VERSION_CODE < 5.0.0
 * (which includes every RPM >= 4.14 in use today) that layer is gone, so
 * map onto the modern header{Put,Del,Format}* API instead. */
#if RPM_VERSION_CODE < RPM_VERSION(4,6,0) || RPM_VERSION_CODE >= RPM_VERSION(5,0,0)
#define ruby_rpm_headerFormat(h, fmt, errmsg) \
	headerSprintf((h), (fmt), rpmTagTable, rpmHeaderFormats, (errmsg))
#define ruby_rpm_headerPutString(h, tag, val) \
	headerAddEntry((h), (tag), RPM_STRING_TYPE, (void*)(val), 1)
#define ruby_rpm_headerPutBin(h, tag, val, size) \
	headerAddEntry((h), (tag), RPM_BIN_TYPE, (void*)(val), (size))
#define ruby_rpm_headerPutInt32Array(h, tag, val, size) \
	headerAddOrAppendEntry((h), (tag), RPM_INT32_TYPE, (void*)(val), (size))
#define ruby_rpm_headerPutStringArray(h, tag, val, size) \
	headerAddOrAppendEntry((h), (tag), RPM_STRING_ARRAY_TYPE, (void*)(val), (size))
#define ruby_rpm_headerDel(h, tag) headerRemoveEntry((h), (tag))
#else
#define ruby_rpm_headerFormat(h, fmt, errmsg) \
	headerFormat((h), (fmt), (errmsg))
#define ruby_rpm_headerPutString(h, tag, val) \
	headerPutString((h), (tag), (val))
#define ruby_rpm_headerPutBin(h, tag, val, size) \
	headerPutBin((h), (tag), (const uint8_t*)(val), (size))
#define ruby_rpm_headerPutInt32Array(h, tag, val, size) \
	headerPutUint32((h), (tag), (const uint32_t*)(val), (size))
#define ruby_rpm_headerPutStringArray(h, tag, val, size) \
	headerPutStringArray((h), (tag), (const char**)(val), (size))
#define ruby_rpm_headerDel(h, tag) headerDel((h), (tag))
#endif

/* headerNVR() and headerNEVRA() are also gone from the same 4.4.x
 * compatibility layer; headerGetString() is their modern replacement. */
inline static const char *
ruby_rpm_header_get_name(Header h)
{
#if RPM_VERSION_CODE < RPM_VERSION(4,6,0) || RPM_VERSION_CODE >= RPM_VERSION(5,0,0)
	const char *n = NULL;
	headerNVR(h, &n, NULL, NULL);
	return n;
#else
	return headerGetString(h, RPMTAG_NAME);
#endif
}

inline static void
ruby_rpm_header_get_vr(Header h, const char **v, const char **r)
{
#if RPM_VERSION_CODE < RPM_VERSION(4,6,0) || RPM_VERSION_CODE >= RPM_VERSION(5,0,0)
	headerNVR(h, NULL, v, r);
#else
	*v = headerGetString(h, RPMTAG_VERSION);
	*r = headerGetString(h, RPMTAG_RELEASE);
#endif
}

inline static const char *
ruby_rpm_header_get_arch(Header h)
{
#if RPM_VERSION_CODE < RPM_VERSION(4,6,0) || RPM_VERSION_CODE >= RPM_VERSION(5,0,0)
	const char *a = NULL;
	headerNEVRA(h, NULL, NULL, NULL, NULL, &a);
	return a;
#else
	return headerGetString(h, RPMTAG_ARCH);
#endif
}
