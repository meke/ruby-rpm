#ifndef rpm40_compat_h_Included
#define rpm40_compat_h_Included 1


rpmRC
rpmReadPackageInfo(FD_t fd, Header * sigp, Header * hdrp);

/*
 * expandMacros() was dropped from rpm's public API somewhere around
 * RPM 4.9 in favor of rpmExpand()/rpmExpandMacros(). This wraps
 * whichever API is available so callers don't have to care.
 *
 * Expands the macro expression in `buf` (of size `buflen`) in place.
 * `spec`/`mc` (a Spec/MacroContext pair) are only honored on the
 * pre-4.9 (and RPM 5.x) code path, where they scope the expansion to
 * a spec file's own macro context; elsewhere they're ignored and
 * expansion goes through rpm's global macro context. Pass NULL/NULL
 * for global-context expansion on every RPM version.
 */
void
ruby_rpm_expand_macros(void *spec, void *mc, char *buf, size_t buflen);


#endif
