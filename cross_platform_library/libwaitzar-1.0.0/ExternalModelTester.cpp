//Don't let Visual Studio warn us to use the _s functions
#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>

#ifdef TEST_FLAG
  #include <fontconv.h>
  #include <WordBuilder.h>
#else
  #ifdef __STDC_ISO_10646__
    //If defined, we are running on Linux
    #include <fontconv.h>
    #include <WordBuilder.h>
  #else
    //On windows:
    #include "../cross_platform_library/waitzar/fontconv.h"
    #include "../cross_platform_library/waitzar/WordBuilder.h"
  #endif
#endif

using namespace waitzar;


//Function prototype:
wchar_t* makeStringFromKeystrokes(std::vector<unsigned short> keystrokes);

//Useful global...
wchar_t returnVal[500];


/**
 * This file demonstrates how to use Wait Zar technology in your own projects. 
 * In particular, we show how the WordBuilder class can be used to disambiguate words.
 * You will need the following to run this sample:
 *   1) Compile or link in fontmap.[cpp|h], fonconv.[cpp|h], lib.[cpp|h], regex.[cpp|h], and WordBuilder.[cpp|h]
 *   2) In WordBuilder.h, uncomment the line "#define __STDC_ISO_10646__  200104L"
 *   3) Have Myanmar.model and mywords.txt on hand (if you plan to use Wait Zar's current model). 
 *   4) Note that the WordBuilder constructor can also be called with detailed information about the internals
 *      of the model. This is a very advanced usage of WordBuilder; you should probably avoid it.
 */
int main(int argc, const char* argv[])
{
	//Switch to wide-character mode
	if (fwide(stdout, 0) == 0) {
		if (fwide(stdout, 1) <= 0) {
			printf("Could not switch to wide char mode!\n");
			return 0;
		} else {
			wprintf(L"Switched to wide mode.\n");
		}
	}
	

	//Set the locale??
	/*char* localeInfo = setlocale(0, "japanese");
    wprintf(L"Locale information set to %s\n", localeInfo);*/

	
	//Create your model as an object.
	//NOTE: You should not mix printf() and wprintf() (it might behave unexpectedly). So, for unicode programs, always use wprintf().
	WordBuilder *model = new WordBuilder("/usr/local/share/waitzar/model2/Myanmar.model", "/usr/local/share/waitzar/model2/mywords.txt");
	wprintf(L"Model loaded correctly.\n\n");


	//////////////////////////////////////////////////////////////////////
	//Use case 1: List all Burmese words for "kote"
	//////////////////////////////////////////////////////////////////////
	//First, we reset the model; this should occur before the first keypress of every NEW word.
	//  Note that "true" will reset the trigrams as well, "false" just resets everything else.
	model->reset(true);
	
	//Now, we must type each letter individually. Since we know that "kote" is in our dictionary, 
	//  we just call typeLetter() four times. If we are not sure if the word exists, we should check
	//  the return value of typeLetter() each time we call it. 
	model->typeLetter('k'); //Transition -->'k'
	model->typeLetter('o'); //Transition (k) --> 'o'
	model->typeLetter('t'); //Transition (k-->o) --> 't'
	model->typeLetter('e'); //Transition (k-->o-->t) --> 'e'
	
	//We have arrived at "kote", so we now just list all possible values.
	std::vector<unsigned short> currWord;
	wchar_t* printWord;
	std::vector<unsigned int> possWords = model->getPossibleWords();
	wprintf(L"\"kote\" can be one of the following %i words \n", possWords.size());
	for (unsigned int i=0; i<possWords.size(); i++) {
		//Note that here, we use getWordKeyStrokes(), NOT getWordString(). 
		//  Calling getWordString() will ALWAYS return the Zawgyi-One encoding.
		//  If you wish to actually print out the Myanmar text, you can use a converter function, like
		//  we do here.
		currWord = model->getWordKeyStrokes(possWords[i]);
		printWord = makeStringFromKeystrokes(currWord);
		
		wprintf(L"  %ls  (", printWord);
		for (unsigned int x=0; x<currWord.size(); x++) {
			wprintf(L" %x", currWord[x]);
		}
		
		
		wprintf(L" )  size:%i\n",currWord.size());
	}
	wprintf(L"\n");
	wprintf(L"NOTE: If you just see \"????\" instead of the Burmese letters, your terminal might not support complex fonts. The hexadecimal values for each character are provided for your reference, check these if all you can see is a series of question marks.\n\n");



	//////////////////////////////////////////////////////////////////////
	//Use case 2: Check custom words & shortcut words. Output in Win Innwa.
	//////////////////////////////////////////////////////////////////////

	//Reset, and change the encoding.
	model->reset(true);
	model->setOutputEncoding(ENCODING_WININNWA);
	
	//Type each letter
	const char* hello = "minggalarpar";
	wprintf(L"Typing \"minggalarpar\" as: ");
	bool flagOn = false;
	for (unsigned int i=0; i<strlen(hello); i++) {
		//Is it in the model?
		if (!model->typeLetter(hello[i])) {
			wprintf(L"\nUh-oh! Couldn't type: %i\n", i);
		}

		//Have we typed enough?
		if (wcslen(model->getParenString())>0 && !flagOn) {
			wprintf(L"%c", hello[i]);
			flagOn = true;
		}
		
		if (flagOn) {
			wprintf(L"-");
		} else {
			wprintf(L"%c", hello[i]);
		}
	}
	wprintf(L"\n");



	//////////////////////////////////////////////////////////////////////
	//Use case 3: Compare different outputs.
	//////////////////////////////////////////////////////////////////////

	//Reset
	model->reset(true);
	
	//Type an interesting word, like one with kinzi in it
	const char* sgp = "singapore";
	for (unsigned int i=0; i<strlen(sgp); i++)
		model->typeLetter(sgp[i]);
	unsigned int wordID = model->getPossibleWords()[0];
	
	//Retrieve all three output encodings
	wchar_t* sgpZawgyi = new wchar_t[100];
	model->setOutputEncoding(ENCODING_ZAWGYI);
	wcscpy(sgpZawgyi, makeStringFromKeystrokes(model->getWordKeyStrokes(wordID)));
	
	//Compare all three output encodings:	
	wprintf(L"\n\"singapore\" in three different output encodings:\n");
	wprintf(L"Zawgyi-One: ");
	for (unsigned int i=0; i<wcslen(sgpZawgyi); i++) {
		wprintf(L" U+%x", sgpZawgyi[i]);
	}
	wprintf(L"\n");

	wchar_t* sgpUnicode = new wchar_t[100];
	model->setOutputEncoding(ENCODING_UNICODE);
	wcscpy(sgpUnicode, makeStringFromKeystrokes(model->getWordKeyStrokes(wordID)));
	
	wchar_t* sgpWinInnwa = new wchar_t[100];
	model->setOutputEncoding(ENCODING_WININNWA);
	wcscpy(sgpWinInnwa, makeStringFromKeystrokes(model->getWordKeyStrokes(wordID)));
	
	wprintf(L"Unicode 5.1:");
	for (unsigned int i=0; i<wcslen(sgpUnicode); i++) {
		wprintf(L" U+%x", sgpUnicode[i]);
	}
	wprintf(L"\n");

	wprintf(L"Win Innwa:  ");
	for (unsigned int i=0; i<wcslen(sgpWinInnwa); i++) {
		wprintf(L"   0x%x", sgpWinInnwa[i]);
	}
	wprintf(L"\n\n");

	

	wprintf(L"\nDone\n" );
	return 0;
}


wchar_t* makeStringFromKeystrokes(std::vector<unsigned short> keystrokes)
{
	for (unsigned int i=0; i<keystrokes.size(); i++) {
		returnVal[i] = keystrokes[i];
	}
	returnVal[keystrokes.size()] = 0x0000; //Note: we need to terminate with a FULL-width zero (not just '\0') to ensure that we actually end the string.
	
	return returnVal;
}
