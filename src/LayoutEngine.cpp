/*
 * (C) Copyright IBM Corp. 1998-2011 - All Rights Reserved
 * (C) Copyright Google, Inc. 2012-2013 - All Rights Reserved
 *
 * Google Author(s): Behdad Esfahbod
 */

#include "LETypes.h"
#include "LEScripts.h"
#include "LELanguages.h"
#include "LEFontInstance.h"
#include "LEGlyphStorage.h"
#include "ScriptAndLanguageTags.h"
#include "OpenTypeLayoutEngine.h"

#include "LayoutEngine.h"

#include <hb.h>
#include <hb-ot.h>

#include <math.h>

U_NAMESPACE_BEGIN

/* Leave this copyright notice here! It needs to go somewhere in this library. */
static const char copyright[] = U_COPYRIGHT_STRING;

const le_int32 LayoutEngine::kTypoFlagKern = 0x1;
const le_int32 LayoutEngine::kTypoFlagLiga = 0x2;

UOBJECT_DEFINE_RTTI_IMPLEMENTATION(LayoutEngine)

static inline float
to_float (hb_position_t v)
{
    return scalbnf (v, -12);
}

static inline hb_position_t
from_float (float v)
{
    return scalbnf (v, +12);
}


static hb_blob_t *
icu_le_hb_reference_table (hb_face_t *face, hb_tag_t tag, void *user_data)
{
    const LEFontInstance *fontInstance = (const LEFontInstance *) user_data;

    size_t length = 0;
    const char *data = (const char *) fontInstance->getFontTable (tag, length);

    return hb_blob_create (data, length, HB_MEMORY_MODE_READONLY, NULL, NULL);
}

static hb_script_t
script_to_hb (le_int32 code)
{
    if (code < 0 || code >= scriptCodeCount)
        return HB_SCRIPT_INVALID;
    return hb_ot_tag_to_script (OpenTypeLayoutEngine::scriptTags[code]);
}

static hb_language_t
language_to_hb (le_int32 code)
{
    if (code < 0 || code >= languageCodeCount)
        return HB_LANGUAGE_INVALID;
    return hb_ot_tag_to_language (OpenTypeLayoutEngine::languageTags[code]);
}

LayoutEngine::LayoutEngine(const LEFontInstance *fontInstance,
                           le_int32 scriptCode,
                           le_int32 languageCode,
                           le_int32 typoFlags,
                           LEErrorCode &success)
    : fHbFont(NULL), fHbBuffer(NULL), fGlyphStorage (NULL), fTypoFlags(typoFlags)
{
    if (LE_FAILURE(success))
        return;

    fHbBuffer = hb_buffer_create ();
    if (fHbBuffer == hb_buffer_get_empty ())
    {
        success = LE_MEMORY_ALLOCATION_ERROR;
        return;
    }
    hb_buffer_set_script (fHbBuffer, script_to_hb (scriptCode));
    hb_buffer_set_language (fHbBuffer, language_to_hb (languageCode));

    hb_face_t *face = hb_face_create_for_tables (icu_le_hb_reference_table, (void *) fontInstance, NULL);
    fHbFont = hb_font_create (face);
    hb_face_destroy (face);
    if (fHbFont == hb_font_get_empty ())
    {
        success = LE_MEMORY_ALLOCATION_ERROR;
        return;
    }

    fGlyphStorage = new LEGlyphStorage();
    if (fGlyphStorage == NULL)
    {
        success = LE_MEMORY_ALLOCATION_ERROR;
        return;
    }

#if 0
    float x_scale = fontInstance->getXPixelsPerEm () * fontInstance->getScaleFactorX ();
    float y_scale = fontInstance->getYPixelsPerEm () * fontInstance->getScaleFactorY ();
#else
    /* The previous block is what we actually want.  However,
     * OpenJDK's FontInstanceAdapter::getScaleFactor[XY]() returns
     * totally bogus numbers.  So we use transformFunits() to
     * achieve the same.
     */
    unsigned int upem = fontInstance->getUnitsPerEM ();
    /* Only support scale transforms here... FIXME? HOW? */
    LEPoint p;
    fontInstance->transformFunits (upem, upem, p);
    float x_scale = p.fX;
    float y_scale = p.fY;
#endif

    hb_font_set_scale (fHbFont,
                       +from_float (x_scale),
                       -from_float (y_scale));
    hb_font_set_ppem (fHbFont,
                      fontInstance->getXPixelsPerEm (),
                      fontInstance->getYPixelsPerEm ());
}

LayoutEngine::~LayoutEngine(void)
{
    hb_font_destroy (fHbFont);
    hb_buffer_destroy (fHbBuffer);
    delete fGlyphStorage;
}

le_int32 LayoutEngine::getGlyphCount() const
{
    return hb_buffer_get_length (fHbBuffer);
}

void LayoutEngine::getCharIndices(le_int32 charIndices[], le_int32 indexBase, LEErrorCode &success) const
{
    fGlyphStorage->getCharIndices(charIndices, indexBase, success);
}

void LayoutEngine::getCharIndices(le_int32 charIndices[], LEErrorCode &success) const
{
    fGlyphStorage->getCharIndices(charIndices, success);
}

// Copy the glyphs into caller's (32-bit) glyph array, OR in extraBits
void LayoutEngine::getGlyphs(le_uint32 glyphs[], le_uint32 extraBits, LEErrorCode &success) const
{
    fGlyphStorage->getGlyphs(glyphs, extraBits, success);
}

void LayoutEngine::getGlyphs(LEGlyphID glyphs[], LEErrorCode &success) const
{
    fGlyphStorage->getGlyphs(glyphs, success);
}


void LayoutEngine::getGlyphPositions(float positions[], LEErrorCode &success) const
{
    fGlyphStorage->getGlyphPositions(positions, success);
}

void LayoutEngine::getGlyphPosition(le_int32 glyphIndex, float &x, float &y, LEErrorCode &success) const
{
    fGlyphStorage->getGlyphPosition(glyphIndex, x, y, success);
}

// Input: characters, font?
// Output: glyphs, positions, char indices
// Returns: number of glyphs
le_int32 LayoutEngine::layoutChars(const LEUnicode chars[], le_int32 offset, le_int32 count, le_int32 max, le_bool rightToLeft,
                                   float x, float y, LEErrorCode &success)
{
    if (LE_FAILURE(success))
        return 0;

    if (chars == NULL || offset < 0 || count < 0 || max < 0 || offset >= max || offset + count > max)
    {
        success = LE_ILLEGAL_ARGUMENT_ERROR;
        return 0;
    }

    hb_buffer_set_direction (fHbBuffer, rightToLeft ? HB_DIRECTION_RTL : HB_DIRECTION_LTR);
    hb_buffer_set_length (fHbBuffer, 0);
    hb_buffer_set_flags (fHbBuffer, (hb_buffer_flags_t)
                         ((offset == 0 ? HB_BUFFER_FLAG_BOT : 0) |
                          (offset + count == max ? HB_BUFFER_FLAG_EOT : 0)));
    hb_buffer_set_cluster_level (fHbBuffer, HB_BUFFER_CLUSTER_LEVEL_MONOTONE_CHARACTERS);
    hb_buffer_add_utf16 (fHbBuffer, (const uint16_t*)chars, max, offset, 0);
    hb_buffer_add_utf16 (fHbBuffer, (const uint16_t*)(chars + offset), max - offset, 0, count);

    /* TODO Support features? */
    hb_shape (fHbFont, fHbBuffer, NULL, 0);

    /* ICU LE generates at least one glyph for each and every input 16bit codepoint.
     * Do the same by inserting fillers. */
    int dir   = rightToLeft ? -1 : +1;
    int start = rightToLeft ? count - 1 : 0;
    int end   = rightToLeft ? -1 : count;
    int iter;

    unsigned int hbCount = hb_buffer_get_length (fHbBuffer);
    const hb_glyph_info_t *info = hb_buffer_get_glyph_infos (fHbBuffer, NULL);
    const hb_glyph_position_t *pos = hb_buffer_get_glyph_positions (fHbBuffer, NULL);

    unsigned int outCount = 0;
    iter = start;
    for (unsigned int i = 0; i < hbCount;)
    {
        int cluster = info[i].cluster;
        outCount += dir * (cluster - iter);
        iter = cluster;
        for (; i < hbCount && (int) info[i].cluster == cluster; i++)
            outCount++;
        iter += dir;
    }
    outCount += dir * (end - iter);

    fGlyphStorage->allocateGlyphArray (outCount, rightToLeft, success);
    fGlyphStorage->allocatePositions (success);

    if (LE_FAILURE(success))
        return 0;

    unsigned int j = 0;
    iter = start;
    for (unsigned int i = 0; i < hbCount;)
    {
        int cluster = info[i].cluster;
        while (iter != cluster)
        {
            fGlyphStorage->setGlyphID   (j, 0xFFFF, success);
            fGlyphStorage->setCharIndex (j, iter, success);
            fGlyphStorage->setPosition  (j, x, y, success);
            j++;
            iter += dir;
        }
        for (; i < hbCount && (int) info[i].cluster == cluster; i++)
        {
            fGlyphStorage->setGlyphID   (j, info[i].codepoint, success);
            fGlyphStorage->setCharIndex (j, cluster, success);
            fGlyphStorage->setPosition  (j,
                                         x + to_float (pos[i].x_offset),
                                         y + to_float (pos[i].y_offset),
                                         success);
            j++;
            x += to_float (pos[i].x_advance);
            y += to_float (pos[i].y_advance);
        }
        iter += dir;
    }
    while (iter != end)
    {
        fGlyphStorage->setGlyphID   (j, 0xFFFF, success);
        fGlyphStorage->setCharIndex (j, iter, success);
        fGlyphStorage->setPosition  (j, x, y, success);
        j++;
        iter += dir;
    }
    fGlyphStorage->setPosition  (j, x, y, success);

    hb_buffer_set_length (fHbBuffer, 0);

    return fGlyphStorage->getGlyphCount();
}

void LayoutEngine::reset()
{
    fGlyphStorage->reset ();
}

LayoutEngine *LayoutEngine::layoutEngineFactory(const LEFontInstance *fontInstance, le_int32 scriptCode, le_int32 languageCode, LEErrorCode &success)
{
    // 3 -> kerning and ligatures
    return LayoutEngine::layoutEngineFactory(fontInstance,
                                             scriptCode,
                                             languageCode,
                                             (LayoutEngine::kTypoFlagKern |
                                              LayoutEngine::kTypoFlagLiga),
                                             success);
}

LayoutEngine *LayoutEngine::layoutEngineFactory(const LEFontInstance *fontInstance, le_int32 scriptCode, le_int32 languageCode, le_int32 typoFlags, LEErrorCode &success)
{
    if (LE_FAILURE(success))
        return NULL;

    LayoutEngine *result = NULL;
    result = new LayoutEngine(fontInstance, scriptCode, languageCode, typoFlags, success);

    if (result && LE_FAILURE(success))
    {
        delete result;
        result = NULL;
    }

    if (result == NULL)
        success = LE_MEMORY_ALLOCATION_ERROR;

    return result;
}

U_NAMESPACE_END
