//
// CMS Build Fixes
//

#ifndef CREXX_CMS_H
#define CREXX_CMS_H

/* Shocking hack ... */
#define snprintf(s,sz,...) sprintf(s,__VA_ARGS__)

#endif //CREXX_CMS_H
