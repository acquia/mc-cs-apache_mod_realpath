/* 
 * This module makes the realpath unix function available
 * in ap_expr and mod rewrite.
 *   
 * Apache expresion are used in SetEnvIfExpr,If and other directives
 * you can use the realpath function like this:
 * realpath('/some/path/with/symlinks') =~ /regexptocheckrealpath/
 *
 * Mod rewrite mapping function has to be enabled for use with:
 * RewriteMap realpath int:realpath
 * and then can used like this:
 * RewriteRule "(.*)" "${realpath:$1}" [L]
 *
*/ 

#include "httpd.h"
#include "http_config.h"
#include "http_protocol.h"
#include "ap_config.h"
#include "ap_expr.h"
#include "http_log.h"
#include "mod_rewrite.h"
#include <apr_optional.h>
#include <apr_strings.h>
#include <limits.h> /* PATH_MAX */
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>




static char *realpath_realpath(request_rec *r, const char *key)
{
    char *pathbuf;

    if (key && *key && r) {
        ap_log_rerror(APLOG_MARK, APLOG_DEBUG, 0, r, "Getting the real path of: %s", key);
        pathbuf = apr_palloc(r->pool, PATH_MAX);
        pathbuf = realpath(key, pathbuf);
        if(!pathbuf) {
          ap_log_rerror(APLOG_MARK, APLOG_WARNING, errno, r, "Couldn't determine the real path of: %s", key);
        }
        return pathbuf;
    }

    return NULL;
}
static char *realpath_owneruid(request_rec *r, const char *key)
{
    struct stat sb;

    if (key && *key && r) {
        ap_log_rerror(APLOG_MARK, APLOG_DEBUG, 0, r, "Getting the owner of: %s", key);
        if(stat(key, &sb) == 0) {
          return apr_itoa(r->pool, (long) sb.st_uid);
        }
        ap_log_rerror(APLOG_MARK, APLOG_WARNING, errno, r, "Couldn't determine the owner of: %s", key);
    }

    return NULL;
}

// mod_rewrite entrypoint
static char *rewrite_mapfunc_realpath(request_rec *r, char *key)
{
    return realpath_realpath(r, (const char *) key);
}

// mod_rewrite entrypoint
static char *rewrite_mapfunc_owneruid(request_rec *r, char *key)
{
    return realpath_owneruid(r, (const char *) key);
}

// ap_expr entrypoint - the path to resolve is stored in arg
static const char *realpath_expr_realpath(ap_expr_eval_ctx_t *ctx, const void *data, const char *arg)
{
    return realpath_realpath(ctx->r, arg);
}

// ap_expr entrypoint - the path to resolve is stored in arg
static const char *realpath_expr_owneruid(ap_expr_eval_ctx_t *ctx, const void *data, const char *arg)
{
    return realpath_owneruid(ctx->r, arg);
}

static int realpath_expr_lookup(ap_expr_lookup_parms *parms)
{
    if (parms->type != AP_EXPR_FUNC_STRING) {
        return DECLINED;
    }
    if (strcasecmp(parms->name, "REALPATH") == 0) {
        *parms->func = realpath_expr_realpath;
        *parms->data = parms->arg;
        return OK;
    }
    if (strcasecmp(parms->name, "OWNERUID") == 0) {
        *parms->func = realpath_expr_owneruid;
        *parms->data = parms->arg;
        return OK;
    }
}

static int realpath_pre_config(apr_pool_t *pconf, apr_pool_t *plog, apr_pool_t *ptemp)
{
    APR_OPTIONAL_FN_TYPE(ap_register_rewrite_mapfunc) *map_pfn_register;

    /* register int:realpath function in mod_rewrite */
    map_pfn_register = APR_RETRIEVE_OPTIONAL_FN(ap_register_rewrite_mapfunc);
    if (map_pfn_register) {
        map_pfn_register("realpath", rewrite_mapfunc_realpath);
        map_pfn_register("owneruid", rewrite_mapfunc_realpath);
    }
    
    return OK;
}

static void realpath_register_hooks(apr_pool_t *p)
{
    ap_hook_expr_lookup(realpath_expr_lookup, NULL, NULL, APR_HOOK_MIDDLE);
    ap_hook_pre_config(realpath_pre_config, NULL, NULL, APR_HOOK_MIDDLE);
}

module AP_MODULE_DECLARE_DATA realpath_module = {
    STANDARD20_MODULE_STUFF, 
    NULL,                  /* create per-dir    config structures */
    NULL,                  /* merge  per-dir    config structures */
    NULL,                  /* create per-server config structures */
    NULL,                  /* merge  per-server config structures */
    NULL,                  /* table of config file commands       */
    realpath_register_hooks  /* register hooks                      */
};

