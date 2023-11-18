/***************************************************************************
 * Author: Glenn Upthagrove
 * Date: 11/10/2023
 * Title: garbage.hpp
 * Description: A simple API that allows the user to push all memory
 * management down to this subsystem. To use, replace malloc(int) with
 * gmalloc(int), and call gclear() before any point at which the program
 * can terminate, such as a return in main or an exit(int) call. If the
 * user desires to manually free(void*), they should use gfree(void*).
 * This .hpp version is a C++ compatible version of garbage.h
 * The gnew function calls gmalloc and is equivalent. 
***************************************************************************/
#ifndef GARBAGE
#define GARBAGE

//#define malloc gmalloc
//this is an idea to improve the API at a later date by replaceing all mallocs with gmalloc
//and possible main return with a greturn? that may not be possible but I shall work on this
//at some later date

#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <string>

using std::cin;
using std::cout;
using std::endl;
using std::string;

//cgarbage handler struct. Holds the memory array handle, the size and the capactiy.
struct garbage_handler {
	void** handle;
	int size;
    int cap;
};

//intend to add in static variables 
struct garbage_handler gh;
int inited = 0;
int initial = 8;
bool debug = 0;

//Internal use only: initialize cgrabage subsystem.
void _init_gh() {
	if (debug) {
		cout << "in _init_gh" << endl;
	}
	if (inited != 0) {
		return;
	}
	gh.handle = new void* [initial];
	for (int i = 0; i < initial; i++) {
		gh.handle[i] = NULL;
	}
	gh.size = 0;
	gh.cap = initial;
	inited = 1;
}

//Internal use only: double the size of the cgarbage memory array.
void _grow_gh() {
	void** temp;
	if (debug) {
		cout << "in _grow_gh" << endl;
	}
	temp = new void* [gh.cap * 2];
	for (int i = 0; i < gh.size; i++) {
		temp[i] = gh.handle[i];
	}
	free(gh.handle);
	gh.handle = temp;
	gh.cap = gh.cap * 2;
}

//Internal use only: shrink function for the cgarbage memory array. Makes it half as large.
void _shrink_gh() {
	void** temp;
	if (debug) {
		cout << "in _shrink_gh" << endl;
	}
	if ((gh.cap / 2) < gh.size) {
		cout << "Cannot shrink when current capacity / 2 woudl be smaller than current size" << endl;
		exit(3);
	}
	temp = new void* [gh.cap / 2];
	for (int i = 0; i < gh.size; i++) {
		temp[i] = gh.handle[i];
	}
	free(gh.handle);
	gh.handle = temp;
	gh.cap = (gh.cap / 2);
}

//toggle cgarbage debug flag on or off
void toggle_debug() {
	debug = !debug;
}

//Allows setting of the initial garbage handler array size
void set_initial(int in) {
	if (debug) {
		cout << "in set_initial" << endl;
	}
	if (in < 1) {
		cout << "error: initial must be greater than 0" << endl;
		exit(1);
	}
	if (inited) {
		cout << "error: cannot set initial after initializing system" << endl;
		exit(2);
	}
	initial = in;
}

//C "malloc" equivalent in cgarbage,
void* gmalloc(int in) {
	void* temp;
	if (debug) {
		cout << "in gmalloc" << endl;
	}
	if (inited == 0) {
		_init_gh();
	}
	temp = malloc(in);
	gh.handle[gh.size] = temp;
	gh.size++;
	if (gh.size == gh.cap) {
		_grow_gh();
	}
	return temp;
}

//C++ "new" equvilent in cgarbage, wrapper to gmalloc
void* gnew(int in) {
	return gmalloc(in);
}

//C "free" equivalent in cgarbage.
void gfree(void* in) {
	int i = gh.size;
	if (debug) {
		cout << "in gfree" << endl;
	}
	if (gh.size > 0) {
		for (i; i >= 0; i--) {
			if (gh.handle[i] == in) {
				break;
			}
			continue;
		}
		free(gh.handle[i]);
		if (i < gh.size) {
			gh.handle[i] = gh.handle[gh.size];
		}
		gh.handle[gh.size] = NULL;
		gh.size--;
		if (debug) {
			cout << "size: " << gh.size << " cap: " << gh.cap << endl;
		}
		if ((gh.size == (gh.cap / 4)) && (gh.cap > 8)) {
			if (debug) {
				cout << "calling _shrink_gh()" << endl;
			}
			_shrink_gh();
		}
	}
}

//cgarbage clear the entire memory array.
void gclear() {
	if (debug) {
		cout << "in gclear" << endl;
		cout << "gh: " << &gh << endl;
		cout << "inited: " << inited << endl;
	}
	if (gh.size > 0) {
		while (gh.size > 0) {
			gfree(gh.handle[gh.size - 1]);
		}
		free(gh.handle);
		gh.size = 0;
		gh.cap = 0;
		inited = 0;
	}
	if (debug) {
		cout << "size: " << gh.size << endl;
		cout << "cap: " << gh.cap << endl;
		cout << "inited: " << inited << endl;
	}
}
#endif
