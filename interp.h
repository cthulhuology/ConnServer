// interp.h
//
// Copyright (C) 2004, 2005 David J. Goehrig
// All rights reserved
//
// embedding python interpreter into CS
//

#ifndef __HAVE_INTEPRETER__H__
#define __HAVE_INTEPRETER__H__

using namespace std;

class Interpreter {
	public:
		Interpreter(int argc, char** argv);
		~Interpreter();
		static void init(int argc, char** argv);
		static void finalize();
	protected:
		Interpreter();
		Interpreter(const Interpreter&);
		Interpreter& operator=(const Interpreter&);
	private:
		static Interpreter* interp;

};

#endif
