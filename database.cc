// database.cc
//
// Copyright (C) 2004, 2005 David J. Goehrig
// All rights reserved
//

#include "database.h"

Result::Result(PGresult* r) 
	: res(r)
{
	int i,j;
	rows =  PQntuples(res);
	columns = PQnfields(res);
	for (i = 0; i < columns; i++) {
		string key(PQfname(res,i));
		for (j = 0; j < rows; j++) {
			string value(PQgetvalue(res,j,i));
			table[key].push_back(value);
		}
	}
}

Result::~Result()
{
	PQclear(res);
}

void Result::dump()
{
	map<string,vector<string> >::iterator i;
	int j,k;
	vector<string> keys;

	if (rows <= 0) return;

	i = table.begin();
	for (j = 0; j < columns; j++) {
		keys.push_back((*i).first);
		cerr << keys[j] << "\t";
		++i;
	}
	cerr << endl;
	for (k = 0; k < rows; k++) {
		for (j = 0; j < columns; j++) {
			cerr << table[keys[j]][k] << "\t";
		}
		cerr << endl;
	}
}

bool Result::success()
{
	int status = PQresultStatus(res);
	return status == PGRES_COMMAND_OK || 
		status == PGRES_EMPTY_QUERY ||
		status == PGRES_TUPLES_OK;

}

const vector<string>& Result::operator[](const string& s)
{
	return table[s];
}

// public interface
//
// The database class exports a pointer to a single instance
// we do not allow others to constrcut database connections
// because we only want 1 per server.  The database is a shared
// resource and as such is protected by mutexes.  Locking on the
// db slows down threads that need the db.  But as there are 
// multiple frontends, the DB server can only handle so much.

Result* Database::query(const string& q)
{
	if (! begun) {
		cerr << "Database::query without begin block [" << q << "]" << endl;
		return NULL;
	}
	PGresult* res;
	res = PQexec(conn,q.data());
	if (res == NULL) {
		// fatal error probably out of memory
		cerr << "Database::query insufficient resources to perform query " << q << endl;
		cerr << PQerrorMessage(conn) << endl;
		return new Result(PQmakeEmptyPGresult(conn,PGRES_EMPTY_QUERY)); 
	}
check_status:
	switch(PQresultStatus(res)) {
	case PGRES_EMPTY_QUERY:
		PQclear(res);
		return new Result(PQmakeEmptyPGresult(conn,PGRES_EMPTY_QUERY)); 
	case PGRES_COMMAND_OK:
		PQclear(res);
		return new Result(PQmakeEmptyPGresult(conn,PGRES_COMMAND_OK)); 
	case PGRES_TUPLES_OK:
		return new Result(res);
	case PGRES_COPY_OUT:
	case PGRES_COPY_IN:
		goto check_status;	
		break;
	case PGRES_BAD_RESPONSE:
	case PGRES_NONFATAL_ERROR:
	case PGRES_FATAL_ERROR:
	default:
		cerr << "Database::query:" << error() << endl;
		break;
	}
	return NULL;
}

bool Database::begin()
{
	begun = true;
	PGresult* res = PQexec(conn,"BEGIN");
	if (res == NULL) {
		cerr << "Database::begin insufficient resources to begin query" << endl;
		return false;
	}
	switch(PQresultStatus(res)) {
	case PGRES_COMMAND_OK:
		PQclear(res);
		return true;
	default:
		cerr << "Database::begin: " << error() << endl;
		PQclear(res);
	}
	return false;
}

bool Database::commit()
{
	if (! begun) {
		cerr << "Database::commit: Commit without begin" << endl;
		return false;
	}
		
	PGresult* res = PQexec(conn,"COMMIT");
	if (res == NULL) {
		cerr << "Database::commit insufficient resources to begin query" << endl;
		return false;
	}
	switch(PQresultStatus(res)) {
	case PGRES_COMMAND_OK:
		PQclear(res);
		begun = false;
		return true;
	default:
		cerr << "Database::commit failed: " << error() << endl;
		PQclear(res);
	}
	return false;
}

bool Database::rollback()
{
	if (! begun) {
		cerr << "Database::rollback: Rollback without begin" << endl;
		return false;
	}
		
	PGresult* res = PQexec(conn,"ROLLBACK");
	if (res == NULL) {
		cerr << "Database::rollback insufficient resources to begin query" << endl;
		return false;
	}
	switch(PQresultStatus(res)) {
	case PGRES_COMMAND_OK:
		PQclear(res);
		begun = false;
		return true;
	default:
		cerr << "Database::rollback failed: " << error() << endl;
		PQclear(res);
	}
	return false;
}

bool Database::connected()
{
	if (PQstatus(conn) == CONNECTION_OK) return true;
	return false;
}

string Database::error()
{
	string r(PQerrorMessage(conn));
	return r;
}

// Constructor

Database::Database(const string& info) :
	conn_string(info)
{
	int retries = 0;
	begun = false;
	cerr << "Database::Database connecting to " << info << endl;
	conn = PQconnectdb(info.c_str());
	if (conn == NULL) {
		cerr << "Database::Database insufficient memory to establish connection" << endl;
		return;
	}

check_status:
	if (PQstatus(conn) == CONNECTION_OK) {
		cerr << "Database::Database connected " << conn_string << endl;
	} else {
		cerr << "Database::Database connection failed "  << conn_string << endl;
		cerr << PQerrorMessage(conn) << endl;
		PQreset(conn);
		if (++retries < MAX_RETRIES) 
			goto check_status;
		cerr << "Database::Database max retries exceeded" << endl;
		PQfinish(conn);
		return;
	}
}

Database::~Database() 
{
	cerr << "Database::~Database disconnecting" << endl;
	PQfinish(conn);
}

// DBPool
//
// This class manages multiple database connections
// It attempts to allocate a collection of Database objects
// and then doles them out locking any thread that attempts
// to grab a handle when they're all allocated.

DBPool* DBPool::dbpool = NULL;

DBPool::DBPool(const string& info, int num)
{
	int i;
	
	for(i=0;i<num;i++) {
		Database* db = new Database(info);
		if (db->connected()) {
			dbs.push_back(db);
			free_dbs.push_back(db);
		} else {
			cerr << "DBPool::DBPool Failed to connect to database (" << i << ") " << info << endl;
		}
	}
}

DBPool::~DBPool()
{
	int i;
	for (i=0; i < dbs.size(); i++) {
		dbs[i]->rollback();
		delete dbs[i];
	}
}

bool DBPool::init(const string& info, int num)
{
	if (dbpool != NULL) {
		cerr << "DBPool::init failed dbpool already initialized" << endl;
		return false;
	}
	dbpool = new DBPool(info, num);
	if (dbpool == NULL) return false;
	return true;
}

bool DBPool::finalize()
{
	if (dbpool == NULL) {
		cerr << "DBPool::finalize failed, already finalized" << endl;
		 return false;
	}
	delete dbpool;
	dbpool = NULL;
	return true;
}

Database* DBPool::grab()
{
	if (dbpool == NULL) {
		cerr << "DBPool::grab failed, DBPool not initialized" << endl;
		return NULL;
	}
	Database* r;
	if (dbpool->free_dbs.size() > 0) {
		r = dbpool->free_dbs.back();
		dbpool->free_dbs.pop_back();
		cerr << "[DBPool::grab] " << dbpool->free_dbs.size() << " handles remaining" << endl;
		if (r != NULL) return r;
	} 
	cerr << "********************************************************************************" << endl;
	cerr << "[DBPool::grab] Out of database handles!!!" << endl;
	cerr << "********************************************************************************" << endl;
	exit(1);
}

bool DBPool::release(Database* db)
{
	if (dbpool == NULL) {
		cerr << "DBPool::grab failed, DBPool not initialized" << endl;
		return false;
	}
	if (db == NULL) return false;
	dbpool->free_dbs.push_back(db);
	cerr << "[DBPool::release] " << dbpool->free_dbs.size() << " handles remaining" << endl;
	if (dbpool->free_dbs.size()) return true;
	return false;
}

