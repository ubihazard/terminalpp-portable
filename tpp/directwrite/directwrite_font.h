#pragma once 
#if (defined ARCH_WINDOWS) 

#include "helpers/string.h"

#include "config.h"

#include "../font.h"
#include "directwrite_application.h"

namespace tpp {

	class DirectWriteFont {
	public:
		Microsoft::WRL::ComPtr<IDWriteFontFace> fontFace;
		float sizeEm;
		DirectWriteFont(Microsoft::WRL::ComPtr<IDWriteFontFace> fontFace, float sizeEm) :
			fontFace{fontFace},
			sizeEm{sizeEm} {
		}
	};

	template<>
	inline Font<DirectWriteFont>* Font<DirectWriteFont>::Create(ui::Font font, unsigned height) {
		// get the system font collection		
		Microsoft::WRL::ComPtr<IDWriteFontCollection> sfc;
		DirectWriteApplication::Instance()->dwFactory_->GetSystemFontCollection(&sfc, false);
		// find the required font family - first get the index then obtain the family by the index
		UINT32 findex;
		BOOL fexists;
		// ok, on windows wchar_t and char16_t are the same (see helpers/char.h)
		helpers::utf16_string fname = helpers::UTF8toUTF16(Config::Instance().fontFamily());
		sfc->FindFamilyName(fname.c_str(), &findex, &fexists);
		Microsoft::WRL::ComPtr<IDWriteFontFamily> ff;
		sfc->GetFontFamily(findex, &ff);
		OSCHECK(ff.Get() != nullptr) << "Unable to find font family " << Config::Instance().fontFamily();
		// now get the nearest font
		Microsoft::WRL::ComPtr<IDWriteFont> drw;
		ff->GetFirstMatchingFont(
			font.bold() ? DWRITE_FONT_WEIGHT_BOLD : DWRITE_FONT_WEIGHT_REGULAR,
			DWRITE_FONT_STRETCH_NORMAL,
			font.italics() ? DWRITE_FONT_STYLE_OBLIQUE : DWRITE_FONT_STYLE_NORMAL,
			&drw);
		// finally get the font face
		Microsoft::WRL::ComPtr<IDWriteFontFace> fface;
		drw->CreateFontFace(&fface);
		// now we need to determine the dimensions of single character, which is relatively involved operation, so first we get the dpi and font metrics
		FLOAT dpiX;
		FLOAT dpiY;
		DirectWriteApplication::Instance()->d2dFactory_->GetDesktopDpi(&dpiX, &dpiY);
		DWRITE_FONT_METRICS metrics;
		fface->GetMetrics(&metrics);
		// the em size is size in pixels divided by (DPI / 96)
		// https://docs.microsoft.com/en-us/windows/desktop/LearnWin32/dpi-and-device-independent-pixels
		float emSize = (height * font.size()) / (dpiY / 96);
		// we have to adjust this number for the actual font metrics
		emSize = emSize * metrics.designUnitsPerEm / (metrics.ascent + metrics.descent + metrics.lineGap);
		// now we have to determine the height of a character, which we can do via glyph metrics
		DWRITE_GLYPH_METRICS glyphMetrics;
		UINT16 glyph;
		UINT32 codepoint = 'M';
		fface->GetGlyphIndices(&codepoint, 1, &glyph);
		fface->GetDesignGlyphMetrics(&glyph, 1, &glyphMetrics);
        Font<DirectWriteFont> * result = new Font<DirectWriteFont>(
			font, /* ui font */
			static_cast<unsigned>(std::round(static_cast<float>(glyphMetrics.advanceWidth) * emSize / metrics.designUnitsPerEm)), /* cell width px */
			height * font.size(), /* cell height px */
			(emSize * metrics.ascent / metrics.designUnitsPerEm), /* ascent */
			DirectWriteFont(
				fface,
				emSize
			)
		);
		result->underlineOffset_ = (emSize * metrics.underlinePosition / metrics.designUnitsPerEm); 
		result->underlineThickness_ = (emSize * metrics.underlineThickness / metrics.designUnitsPerEm);
		result->strikethroughOffset_ = (emSize * metrics.strikethroughPosition / metrics.designUnitsPerEm);
		result->strikethroughThickness_ = (emSize * metrics.strikethroughThickness / metrics.designUnitsPerEm);
		return result;
	}



} // namespace tpp





#endif