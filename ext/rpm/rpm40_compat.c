/* -*- mode: C; c-basic-offset: 4; tab-width: 4; -*- */
/* Ruby/RPM
 *
 * Copyright (C) 2004 YAMAZAKI Makoto <zaki@zakky.org>
 */

/* $Id: rpm40_compat.c 11 2004-03-13 14:54:21Z zaki $ */

#include "private.h"


#if RPM_VERSION(4,2,0) <= RPM_VERSION_CODE

rpmRC
rpmReadPackageInfo(FD_t fd, Header * sigp, Header * hdrp)
{
    rpmRC rc;

    rpmts ts = rpmtsCreate();
    rc = rpmReadPackageFile(ts, fd, "readRPM", hdrp);
    ts = rpmtsFree(ts);
    
    if (sigp) *sigp = NULL;                 /* XXX HACK */

    return rc;
}

#endif /* RPM_VERSION(4,2,0) <= RPM_VERSION_CODE */

void
ruby_rpm_expand_macros(void *spec, void *mc, char *buf, size_t buflen)
{
#if RPM_VERSION_CODE < RPM_VERSION(4,9,0) || RPM_VERSION_CODE >= RPM_VERSION(5,0,0)
    expandMacros(spec, mc, buf, buflen);
#else
    char *expanded = rpmExpand(buf, NULL);
    if (expanded) {
        strncpy(buf, expanded, buflen - 1);
        buf[buflen - 1] = '\0';
        free(expanded);
    }
#endif
}
