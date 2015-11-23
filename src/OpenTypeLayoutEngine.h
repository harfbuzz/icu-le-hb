/*
 * (C) Copyright IBM Corp. 1998-2014 - All Rights Reserved
 *
 */

#ifndef __OPENTYPELAYOUTENGINE_H
#define __OPENTYPELAYOUTENGINE_H

#include "LETypes.h"

U_NAMESPACE_BEGIN

class  OpenTypeLayoutEngine
{
public:

    /**
     * The array of language tags, indexed by language code.
     *
     * @internal
     */
    static const LETag languageTags[];

    /**
     * The array of script tags, indexed by script code.
     */
    static const LETag scriptTags[];

};

U_NAMESPACE_END
#endif

