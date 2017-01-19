/**

	MultiMarkdown 6 -- MultiMarkdown - lightweight markup processor.

	@file localization.h

	@brief 


	@author	Fletcher T. Penney
	@bug	

**/

/*

	Copyright © 2016 - 2017 Fletcher T. Penney.


	

*/


#ifndef LOCALIZATION_MULTIMARKDOWN_6_H
#define LOCALIZATION_MULTIMARKDOWN_6_H


#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>



#define kNumberOfLanguages 4
#define kNumberOfStrings 3
#define kLanguage 0

//#define LOCALIZATION_DISABLED


// Hash function from http://lolengine.net/blog/2011/12/20/cpp-constant-string-hash
// via http://stackoverflow.com/questions/2826559/compile-time-preprocessor-hashing-of-string


#ifdef LOCALIZATION_DISABLED
	#define LC(x) x
#else
	#define H1(s,i,x)   (x*65599u+(uint8_t)s[(i)<strlen(s)?strlen(s)-1-(i):strlen(s)])
	#define H4(s,i,x)   H1(s,i,H1(s,i+1,H1(s,i+2,H1(s,i+3,x))))
	#define H16(s,i,x)  H4(s,i,H4(s,i+4,H4(s,i+8,H4(s,i+12,x))))
	#define H64(s,i,x)  H16(s,i,H16(s,i+16,H16(s,i+32,H16(s,i+48,x))))
	#define H256(s,i,x) H64(s,i,H64(s,i+64,H64(s,i+128,H64(s,i+192,x))))

	#define HASH(s)    ((uint32_t)(H256(s,0,0)^(H256(s,0,0)>>16)))

	#define LC(x) Translate(HASH(x), kLanguage)

	//#define LC(x) TranslateTest(__COUNTER__, __FILE__, __LINE__, __FUNCTION__ , x)

static const char * lc_lookup[kNumberOfLanguages * kNumberOfStrings] = {
	"return to body",
	"return to body",
	"return to body",
	"return to le body",

	"see footnote",
	"see footnote",
	"see footnote",
	"Ver nota de pie",

	"see citation",
	"see citation",
	"see citation",
	"ver citation",
};


static inline const char * TranslateTest(int c, char * file, int line, const char func[], char * x) {
	fprintf(stderr, "%s: %d (%s) -> %d\n", file, line, func, c);
	unsigned long h = HASH(x);
	fprintf(stderr, "hash '%s' -> %lu\n", x, h);

	return lc_lookup[(c * kNumberOfLanguages) + kLanguage];
}

static inline const char * Translate(unsigned long x, int l) {
	switch (x) {
		case 3219553713:
			return lc_lookup[0 * kNumberOfLanguages + l];
		case 657226305:
			return lc_lookup[1 * kNumberOfLanguages + l];
		case 2977473004:
			return lc_lookup[2 * kNumberOfLanguages + l];
		default:
			return "localization error";
	}
}


// Based on ISO 639-1 names
// https://en.wikipedia.org/wiki/ISO_639-1
enum lc_languages {
	LC_EN = 0,			//!< English is default
	LC_FR,
	LC_DE,
	LC_ES,
};


#endif

#endif
