#include "data.h"
#include "database.h"
#include "env.h"

#define SAVE(x,y) if ( #x == type ) { \
	y* obj = y::alloc(d); \
	obj->init("type:" + obj->type() + "," + data); \
	cerr << "Saving " << obj->tos() << endl; \
	if (obj->save(d)) goto done; \
	ok = false; \
	cerr << "Failed to save data" << endl; \
	}

#define FIND(x,y) if ( #x == type ) { \
	y* obj = Cache::find<y>(id);	\
	if (obj != NULL) {	\
		cerr << obj->tos() << endl; \
		goto done;\
	} \
	cerr << "Failed to load id " << id << endl; \
	}

int main (int argc, char** argv )
{
	cerr << "argc " << argc << endl;
	if (! (argc == 4 || (argc == 5 && string(argv[2]) == string("add")))) {
		cerr << "Usage: " << argv[0] << " cache [add|id] type [data]" << endl;
		exit(1);
	}

	if (! DBPool::init(Env::env["CSDBCONN"],5)) {
		cerr << "Failed to initialize db connection check CSDBCONN" << endl;
		exit(1);
	}
	
	if (! Cache::create(string(argv[1]))) {
		cerr << "Failed to load cache" << endl;	
		exit(1);
	}

	string id;
	string type;
	string data;
	if (string(argv[2]) == string("add")) {
		type = string(argv[3]);
		data = string(argv[4]);
	} else {
		id = string(argv[2]);
		type = string(argv[3]);
	}

	bool ok = true;;
	Database* d = DBPool::grab();
	d->begin();
	if (argc == 5) {
		cerr << "Adding " << type << endl;
		SAVE(cs,CS);
		SAVE(player,Player)
		SAVE(details,Details)
		SAVE(admins,Administrators)
	} else {
		cerr << "Finding id " << id << endl;
		FIND(cs,CS);
		FIND(player,Player)
		FIND(details,Details)
		FIND(admins,Administrators)
	}
	
done:
	cerr << "Saving cache" << endl;
	if (ok) {
		d->commit();
	} else {
		d->rollback();
	}	
	DBPool::release(d);
	Cache::fini();
	DBPool::finalize();
	return 0;
}

