// database.h
//
// Copyright (C) 2004, 2005 David J. Goehrig
// All rights reserved
//

#ifndef __HAVE_DBCONNECT__
#define __HAVE_DBCONNECT__

#include <stdlib.h>
#include <libpq-fe.h>
#include <iostream>
#include <string>
#include <vector>
#include <map>

using namespace std;

class Result {
	public:
		Result(PGresult* r);
		~Result();
		int columns;
		int rows;
		void dump();
		bool success();
		const vector<string>& operator[](const string& s);
		static const Result Empty;
	private:
		PGresult* res;
		map<string,vector<string> > table;
};

class Database {
	public:
		Result* query(const string& q);
		string error();
		bool connected();
		bool begin();
		bool commit();
		bool rollback();
		Database(const string& info);
		~Database();

	protected:
		Database();
		Database(const Database&);
		Database& operator=(const Database&);

	private:
		static const int MAX_RETRIES = 5; // an arbitrary number
		PGconn* conn;
		string conn_string;
		bool begun;
};

class DBPool {
	public:
		static bool init(const string& info, int num);
		static bool finalize();
		static Database* grab();
		static bool release(Database* db);
	protected:
		DBPool(const string& info, int num);
		~DBPool();
		DBPool();
		DBPool(const DBPool&);
		DBPool& operator=(const DBPool&);
	private:
		static DBPool* dbpool;
		vector<Database*> dbs;
		vector<Database*> free_dbs;
		pthread_mutex_t mut;	// For locking when all the DBs are taken.
		pthread_cond_t cond;	// Wake up those clients looking for a db
};

#endif
