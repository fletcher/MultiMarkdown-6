/**

	MultiMarkdown 6 -- MultiMarkdown - lightweight markup processor.

	@file i18n.h

	@brief Provide rudimentary ability to provide translation string functionality.
	This file enables calculating hash values of built in strings in order to allow
	swapping out certain strings based on user-specified languages at runtime.

	This does slow down compiling, as multiple hash strings are compiled (seemingly
	quite slowly).  But, to my understanding and testing, it does not affect the
	speed when actually running MMD.  This should allow translating an arbitrary
	number of strings into an arbitrary number of languages without a performance
	penalty.


	@author	Fletcher T. Penney
	@bug

**/

/*

	Copyright © 2016 - 2017 Fletcher T. Penney.

*/


#ifndef I18N_MULTIMARKDOWN_6_H
#define I18N_MULTIMARKDOWN_6_H


#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>


#define kNumberOfLanguages 7
#define kNumberOfStrings 4
#define kLanguage 0

//!< #define this in order to disable translations -- speeds up compiling
// #define I18N_DISABLED


// Hash function from http://lolengine.net/blog/2011/12/20/cpp-constant-string-hash
// via http://stackoverflow.com/questions/2826559/compile-time-preprocessor-hashing-of-string


#ifdef I18N_DISABLED
	#define LC(x) x
	#define LANG_FROM_STR(x) 0
#else
	#define H1(s,i,x)   (x*65599u+(uint8_t)s[(i)<strlen(s)?strlen(s)-1-(i):strlen(s)])
	#define H4(s,i,x)   H1(s,i,H1(s,i+1,H1(s,i+2,H1(s,i+3,x))))
	#define H16(s,i,x)  H4(s,i,H4(s,i+4,H4(s,i+8,H4(s,i+12,x))))
	#define H64(s,i,x)  H16(s,i,H16(s,i+16,H16(s,i+32,H16(s,i+48,x))))
	#define H256(s,i,x) H64(s,i,H64(s,i+64,H64(s,i+128,H64(s,i+192,x))))

	#define HASH(s)    ((uint32_t)(H256(s,0,0)^(H256(s,0,0)>>16)))

	#define LC(x) Translate(HASH(x), scratch->language)

	//#define LC(x) TranslateTest(__COUNTER__, __FILE__, __LINE__, __FUNCTION__ , x)

	#define LANG_FROM_STR(x) i18n_language_from_string(x)


// Create the dictionary array
static const char * lc_lookup[kNumberOfLanguages * kNumberOfStrings] = {
	"return to body",				// English
	"Regresar al texto",			// Español
	"Zum Haupttext",				// Deutsch
	"Retour au texte principal",	// Français
	"return to body",				// Nederlands
	"return to body",				// Svenska
	"חזור/י לגוף הטקסט",				// Hebrew - עברית

	"see footnote",					// English
	"Ver nota a pie de página",		// Español
	"Siehe Fußnote",				// Deutsch
	"Voir note de bas de page",		// Français
	"see footnote",					// Nederlands
	"see footnote",					// Svenska
	"ראה/י הערה",					// Hebrew - עברית

	"see citation",					// English
	"Ver referencia",				// Español
	"Siehe Zitat",					// Deutsch
	"Voir citation",				// Français
	"see citation",					// Nederlands
	"see citation",					// Svenska
	"ראה/י ציטוט",					// Hebrew - עברית

	"see glossary",					// English
	"Ver glosario",					// Español
	"Siehe Glossar",				// Deutsch
	"Voir glossaire",				// Français
	"see glossary",					// Nederlands
	"see glossary",					// Svenska
	"ראה/י מילון מונחים",				// Hebrew - עברית
};


// Used for development when a new string is added to the dictionary and
// we need to know the hash
static inline const char * TranslateTest(int c, char * file, int line, const char func[], char * x) {
	fprintf(stderr, "%s: %d (%s) -> %d\n", file, line, func, c);
	unsigned long h = HASH(x);
	fprintf(stderr, "hash '%s' -> %lu\n", x, h);

	return lc_lookup[(c * kNumberOfLanguages) + kLanguage];
}


// Given a hash and language, return the proper string
static inline const char * Translate(unsigned long x, int l) {
	switch (x) {
		case 3219553713:
			return lc_lookup[0 * kNumberOfLanguages + l];
		case 657226305:
			return lc_lookup[1 * kNumberOfLanguages + l];
		case 2977473004:
			return lc_lookup[2 * kNumberOfLanguages + l];
		case 3851221863:
			return lc_lookup[3 * kNumberOfLanguages + l];
		default:
			return "localization error";
	}
}


#endif


// Based on ISO 639-1 names
// https://en.wikipedia.org/wiki/ISO_639-1
enum lc_languages {
	LC_EN = 0,			//!< English is default
	LC_ES,				//!< Español
	LC_DE,				//!< Deutsch
	LC_FR,				//!< Français
	LC_NL,				//!< Nederlands
	LC_SV,				//!< Svenska
	LC_HE,				//!< Hebrew - עברית
};


// MMD expects a lower case 2 letter code in the metadata or command-line arguments
static inline short i18n_language_from_string(const char * l) {
	if (strcmp(l, "es") == 0) {
		return LC_ES;
	} else if (strcmp(l, "de") == 0) {
		return LC_DE;
	} else if (strcmp(l, "fr") == 0) {
		return LC_FR;
	} else if (strcmp(l, "he") == 0) {
		return LC_HE;
	} else if (strcmp(l, "nl") == 0) {
		return LC_NL;
	} else if (strcmp(l, "sv") == 0) {
		return LC_SV;
	}

	return 0;
}

#endif
