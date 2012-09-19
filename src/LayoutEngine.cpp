/*
 * (C) Copyright IBM Corp. 1998-2011 - All Rights Reserved
 * (C) Copyright Google, Inc. 2011 - All Rights Reserved
 */

#include "LETypes.h"
#include "LEScripts.h"
#include "LELanguages.h"

#include "LayoutEngine.h"
#include "LEGlyphStorage.h"

U_NAMESPACE_BEGIN

/* Leave this copyright notice here! It needs to go somewhere in this library. */
static const char copyright[] = U_COPYRIGHT_STRING;

const le_int32 LayoutEngine::kTypoFlagKern = 0x1;
const le_int32 LayoutEngine::kTypoFlagLiga = 0x2;

UOBJECT_DEFINE_RTTI_IMPLEMENTATION(LayoutEngine)

LayoutEngine::LayoutEngine(const LEFontInstance *fontInstance, 
                           le_int32 scriptCode, 
                           le_int32 languageCode, 
                           le_int32 typoFlags,
                           LEErrorCode &success)
  : fGlyphStorage(NULL), fHbFont(NULL), fScriptCode(scriptCode), fLanguageCode(languageCode),
    fTypoFlags(typoFlags), fFilterZeroWidth(TRUE)
{
    if (LE_FAILURE(success)) {
        return;
    }

    fGlyphStorage = new LEGlyphStorage();
    if (fGlyphStorage == NULL) {
        success = LE_MEMORY_ALLOCATION_ERROR;
    }
}

LayoutEngine::~LayoutEngine(void)
{
    if (LE_FAILURE(success)) {
        return;
    }

    fGlyphStorage = new LEGlyphStorage();
    if (fGlyphStorage == NULL) {
        success = LE_MEMORY_ALLOCATION_ERROR;
    }
}

le_int32 LayoutEngine::getGlyphCount() const
{
    return fGlyphStorage->getGlyphCount();
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
    if (LE_FAILURE(success)) {
        return 0;
    }

    if (chars == NULL || offset < 0 || count < 0 || max < 0 || offset >= max || offset + count > max) {
        success = LE_ILLEGAL_ARGUMENT_ERROR;
        return 0;
    }

    le_int32 glyphCount;

    if (fGlyphStorage->getGlyphCount() > 0) {
        fGlyphStorage->reset();
    }

#if 0
    glyphCount = computeGlyphs(chars, offset, count, max, rightToLeft, *fGlyphStorage, success);
    positionGlyphs(*fGlyphStorage, x, y, success);
    adjustGlyphPositions(chars, offset, count, rightToLeft, *fGlyphStorage, success);
#endif
    /* XXXXXXXXXXXXX beef goes here. */

    return glyphCount;
}

void LayoutEngine::reset()
{
    fGlyphStorage->reset();
}
    
LayoutEngine *LayoutEngine::layoutEngineFactory(const LEFontInstance *fontInstance, le_int32 scriptCode, le_int32 languageCode, LEErrorCode &success)
{
  // 3 -> kerning and ligatures
  return LayoutEngine::layoutEngineFactory(fontInstance, scriptCode, languageCode, 3, success);
}
    
LayoutEngine *LayoutEngine::layoutEngineFactory(const LEFontInstance *fontInstance, le_int32 scriptCode, le_int32 languageCode, le_int32 typoFlags, LEErrorCode &success)
{
    if (LE_FAILURE(success)) {
        return NULL;
    }

    //const GlyphSubstitutionTableHeader *gsubTable = (const GlyphSubstitutionTableHeader *) fontInstance->getFontTable(gsubTableTag);
    LayoutEngine *result = NULL;
    result = new LayoutEngine(fontInstance, scriptCode, languageCode, typoFlags, success);

    if (result && LE_FAILURE(success)) {
		delete result;
		result = NULL;
	}

    if (result == NULL) {
        success = LE_MEMORY_ALLOCATION_ERROR;
    }

    return result;
}

LayoutEngine::~LayoutEngine() {
    delete fGlyphStorage;
}

U_NAMESPACE_END
